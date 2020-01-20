#include "GetAsyncWorker.h"

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

GetAsyncWorker::GetAsyncWorker(std::string data, Function &callback) : AsyncWorker(callback),
                                                                           _input(data)
{
}

void GetAsyncWorker::Execute()
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

    const std::string abstractSyntax = uidStudyRootQueryRetrieveInformationModelGET_1_2_840_10008_5_1_4_1_2_2_3;

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


    imebra::CGetCommand command( abstractSyntax,
        dimse.getNextCommandID(),
        dimseCommandPriority_t::medium,
        abstractSyntax,
        payload);
    dimse.sendCommandOrResponse(command);

    try
    {
        for (;;)
        {
            imebra::DimseResponse response(dimse.getCGetResponse(command));
            if (response.getStatus() == imebra::dimseStatus_t::success)
            {
                _output = "Get-scu request succeeded";
                break;
            }
            else if (response.getStatus() == imebra::dimseStatus_t::pending)
            {
                // just continue for now
            }
            else {
                SetError("Get-scu request failed: " + response.getStatusCode());
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

void GetAsyncWorker::OnOK()
{
        String output = String::New(Env(), _output);
        Callback().Call({output});
}
