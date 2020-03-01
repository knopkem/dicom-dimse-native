#include "MoveAsyncWorker.h"

#include "../library/include/imebra/imebra.h"
#include "../library/include/imebra/tag.h"
#include "../library/include/imebra/exceptions.h"
#include "../library/include/imebra/dimse.h"
#include "../library/include/imebra/acse.h"
#include "../library/include/imebra/dataSet.h"
#include "../library/include/imebra/tcpAddress.h"
#include "../library/include/imebra/tcpStream.h"
#include "../library/include/imebra/streamReader.h"
#include "../library/include/imebra/streamWriter.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>

using namespace imebra;


#include "json.h"
#include "Utils.h"

using json = nlohmann::json;


MoveAsyncWorker::MoveAsyncWorker(std::string data, Function &callback) : AsyncWorker(callback),
                                                                           _input(data)
{
}

void MoveAsyncWorker::Execute()
{
    ns::sInput in = ns::parseInputJson(_input);

    if (in.tags.empty()) {
        SetError("Tags not set");
        return;
    }

    if (!in.source.valid()) {
        SetError("Source not set");
        return;
    }

    if (!in.target.valid()) {
        SetError("Target not set");
        return;
    }

    if (in.destination.empty()) {
        in.destination = in.source.aet;
    }

    const std::string abstractSyntax = uidStudyRootQueryRetrieveInformationModelMOVE_1_2_840_10008_5_1_4_1_2_2_2;

    // Allocate a TCP stream that connects to the DICOM SCP
    imebra::TCPStream tcpStream(TCPActiveAddress(in.target.ip, in.target.port));

    // Allocate a stream reader and a writer that use the TCP stream.
    // If you need a more complex stream (e.g. a stream that uses your
    // own services to send and receive data) then use a Pipe
    imebra::StreamReader readSCU(tcpStream.getStreamInput());
    imebra::StreamWriter writeSCU(tcpStream.getStreamOutput());

    // Add all the abstract syntaxes and the supported transfer
    // syntaxes for each abstract syntax (the pair abstract/transfer syntax is
    // called "presentation context")
    imebra::PresentationContext context(abstractSyntax);
    context.addTransferSyntax(imebra::uidImplicitVRLittleEndian_1_2_840_10008_1_2);
    context.addTransferSyntax(imebra::uidExplicitVRLittleEndian_1_2_840_10008_1_2_1);
    imebra::PresentationContexts presentationContexts;
    presentationContexts.addPresentationContext(context);

    // The AssociationSCU constructor will negotiate a connection through
    // the readSCU and writeSCU stream reader and writer
    imebra::AssociationSCU scu(in.source.aet, in.target.aet, 1, 1, presentationContexts, readSCU, writeSCU, 10);

    // The DIMSE service will use the negotiated association to send and receive
    // DICOM commands
    imebra::DimseService dimse(scu);

        // Let's prepare a dataset to store on the SCP
    imebra::MutableDataSet payload; // We will use the negotiated transfer syntax
        for (std::vector<ns::sTag>::iterator it = in.tags.begin(); it != in.tags.end(); ++it) {
                auto tag = (*it);
                payload.setString(TagId((tagId_t)std::stoi(tag.key, 0, 16)), tag.value);
        }


    imebra::CMoveCommand command( abstractSyntax,
        dimse.getNextCommandID(),
        dimseCommandPriority_t::medium,
        abstractSyntax,
        in.destination,
        payload);
    dimse.sendCommandOrResponse(command);

    try
    {
        for (;;)
        {
            imebra::DimseResponse response(dimse.getCMoveResponse(command));
            if (response.getStatus() == imebra::dimseStatus_t::success)
            {
                _output = "Move-scu request succeeded";
                break;
            }
            else if (response.getStatus() == imebra::dimseStatus_t::pending)
            {
                imebra::DataSet data  = response.getPayloadDataSet();
                try
                {
                    int remainNum = data.getSignedLong(imebra::TagId(imebra::tagId_t::NumberOfRemainingSuboperations_0000_1020), 0);
                    int completeNum = data.getSignedLong(imebra::TagId(imebra::tagId_t::NumberOfCompletedSuboperations_0000_1021), 0);
                    int faileNum = data.getSignedLong(imebra::TagId(imebra::tagId_t::NumberOfFailedSuboperations_0000_1022), 0);
                    // int warnNum = data.getSignedLong(imebra::TagId(imebra::tagId_t::NumberOfWarningSuboperations_0000_1023), 0);
                    _output = "remaining: " + std::to_string(remainNum) + " completed: " + std::to_string(completeNum);
                    if (faileNum > 0 ) {
                        _output += " failed: " + std::to_string(remainNum);
                    }
                }
                catch (std::exception &error)
                {
                    SetError("parse error: " + std::string(error.what()));
                }

            }
            else {
                SetError("Move-scu request failed: " + std::to_string(response.getStatusCode()));
                break;
            }
        }
    }
    catch (const StreamEOFError & error)
    {
        // The association has been closed
        SetError("stream error: " + std::string(error.what()));
    }

}

void MoveAsyncWorker::OnOK()
{
        HandleScope scope(Env());
        String output = String::New(Env(), _output);
        Callback().Call({output});
}
