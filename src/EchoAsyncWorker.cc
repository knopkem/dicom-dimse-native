#include "EchoAsyncWorker.h"

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

EchoAsyncWorker::EchoAsyncWorker(std::string data, Function &callback) : AsyncWorker(callback),
                                                                           _input(data)
{
}

void EchoAsyncWorker::Execute()
{
   ns::sInput in = ns::parseInputJson(_input);

    if (!in.source.valid()) {
        SetError("Source not set");
        return;
    }

    if (!in.target.valid()) {
        SetError("Target not set");
        return;
    }

    const std::string abstractSyntax = imebra::uidVerificationSOPClass_1_2_840_10008_1_1;

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

    imebra::CEchoCommand command( abstractSyntax,
        dimse.getNextCommandID(),
        dimseCommandPriority_t::medium,
        abstractSyntax
        );
    dimse.sendCommandOrResponse(command);

    try
    {
        imebra::DimseResponse response(dimse.getCEchoResponse(command));

        if (response.getStatus() == imebra::dimseStatus_t::success)
        {
                _output = "Echo-scu succeeded";
        }
        else
        {
                SetError("Echo-scu request failed: " + std::to_string(response.getStatusCode()));
        }
    }
    catch (std::exception &error)
    {
        SetError("stream error: " + std::string(error.what()));
    }

}

void EchoAsyncWorker::OnOK()
{
        String output = String::New(Env(), _output);
        Callback().Call({output});
}
