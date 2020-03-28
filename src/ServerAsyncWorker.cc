#include "ServerAsyncWorker.h"

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

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <experimental/filesystem>

using namespace imebra;


#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

ServerAsyncWorker::ServerAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void ServerAsyncWorker::Execute(const ExecutionProgress& progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    if (!in.source.valid()) {
        SetErrorJson("Source not set");
        return;
    }

    if (in.storagePath.empty()) {
        in.storagePath = "./data";
        SendInfo("storage path not set, defaulting to " + in.storagePath, progress);
    }

    std::string msg(std::string("starting c-store scp: ") + in.source.ip + " : " + in.source.port);
    SendInfo(msg, progress);

    imebra::TCPListener tcpListener(TCPPassiveAddress(in.source.ip, in.source.port));

    // Wait until a connection arrives or terminate() is called on the tcpListener
    imebra::TCPStream tcpStream(tcpListener.waitForConnection());

    // Allocate a stream reader and a writer that use the TCP stream.
    // If you need a more complex stream (e.g. a stream that uses your
    // own services to send and receive data) then use a Pipe
    imebra::StreamReader readSCU(tcpStream.getStreamInput());
    imebra::StreamWriter writeSCU(tcpStream.getStreamOutput());

    // Add all the abstract syntaxes and the supported transfer
    // syntaxes for each abstract syntax (the pair abstract/transfer syntax is
    // called "presentation context")
    imebra::PresentationContexts presentationContexts;
    std::vector<std::string> dcmLongSCUStorageSOPClassUIDs = ns::dcmLongSCUStorageSOPClassUIDs();
    for (int j = 0; j < dcmLongSCUStorageSOPClassUIDs.size(); j++)
    {
        imebra::PresentationContext context(dcmLongSCUStorageSOPClassUIDs[j], true, true);
        context.addTransferSyntax(imebra::uidImplicitVRLittleEndian_1_2_840_10008_1_2);
        context.addTransferSyntax(imebra::uidExplicitVRLittleEndian_1_2_840_10008_1_2_1);
        presentationContexts.addPresentationContext(context);
    }

    // The AssociationSCP constructor will negotiate the assocation
    imebra::AssociationSCP scp(in.source.aet, 1, 1, presentationContexts, readSCU, writeSCU, 0, 10);

    // The DIMSE service will use the negotiated association to send and receive
    // DICOM commands
    imebra::DimseService dimse(scp);

    try
    {
        // Receive commands until the association is closed
        for(;;)
        {
            // receive a C-Store
            imebra::CStoreCommand command(dimse.getCommand().getAsCStoreCommand());

            // The store command has a payload. We can do something with it, or we can
            // use the methods in CStoreCommand to get other data sent by the peer
            imebra::DataSet payload = command.getPayloadDataSet();

            // Do something with the payload
            std::string sop = payload.getString(TagId(tagId_t::SOPInstanceUID_0008_0018), 0);
            std::string study = payload.getString(TagId(tagId_t::StudyInstanceUID_0020_000D), 0);
            std::experimental::filesystem::path p(in.storagePath + std::string("/") + study);
            std::experimental::filesystem::create_directories(p);
            imebra::CodecFactory::save(payload, p.string() + std::string("/") + sop + std::string(".dcm"), imebra::codecType_t::dicom);

            std::string msg = ns::createJsonResponse(ns::PENDING, "storing: " + sop);
            SendInfo(msg, progress);

            // Send a response
            dimse.sendCommandOrResponse(CStoreResponse(command, dimseStatusCode_t::success));
        }
    }
    catch(const StreamEOFError& e)
    {
        // The association has been closed, do not abort
        SendInfo("assoc closed, reason: " + std::string(e.what()), progress, ns::FAILURE);
    }

    SendInfo("shutting down scp...", progress);
}
