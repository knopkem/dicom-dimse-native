#include "StoreAsyncWorker.h"

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



StoreAsyncWorker::StoreAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void StoreAsyncWorker::Execute(const ExecutionProgress& progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    if (!in.source.valid()) {
        SetErrorJson("Source not set");
        return;
    }

    if (!in.target.valid()) {
        SetErrorJson("Target not set");
        return;
    }
    // TODO: read sop class from file(s)
    const std::string abstractSyntax = "1.2.840.10008.5.1.4.1.1.4";

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
    imebra::PresentationContext context(abstractSyntax); // MR Image Storage
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
    // TODO: load file(s)
    DataSet payload = CodecFactory::load("myfilename");


    imebra::CStoreCommand command(
                abstractSyntax,
                dimse.getNextCommandID(),
                dimseCommandPriority_t::medium,
                payload.getString(TagId(tagId_t::SOPClassUID_0008_0016), 0),
                payload.getString(TagId(tagId_t::SOPInstanceUID_0008_0018), 0),
                "",
                0,
                payload);
    dimse.sendCommandOrResponse(command);
    try
    {
        for (;;)
        {
            imebra::DimseResponse response(dimse.getCStoreResponse(command));
            if (response.getStatus() == imebra::dimseStatus_t::success)
            {
                _jsonOutput = {};
                break;
            }
            else if (response.getStatus() == imebra::dimseStatus_t::pending)
            {
                // just continue for now
            }
            else {
                SetErrorJson("Store-scu request failed: " + std::to_string(response.getStatusCode()));
                break;
            }
        }
    }
    catch (const StreamEOFError & error)
    {
        // The association has been closed
        SetErrorJson("stream error: " + std::string(error.what()));
    }

}
