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
#include <thread>
#include <future>
#include <chrono>

using namespace imebra;

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

namespace {

        void StoreProc(imebra::DimseService* dimse, const AsyncProgressWorker<char>::ExecutionProgress& progress, std::future<void> futureObj)
        {
        while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
        {
                try
                {
                    // receive a C-Store
                    imebra::CStoreCommand command(dimse->getCommand().getAsCStoreCommand());

                    // The store command has a payload. We can do something with it, or we can
                    // use the methods in CStoreCommand to get other data sent by the peer
                    imebra::DataSet payload = command.getPayloadDataSet();

                    // Do something with the payload
                    std::string sop = payload.getString(TagId(tagId_t::SOPInstanceUID_0008_0018), 0);
                    imebra::CodecFactory::save(payload, sop + std::string(".dcm"), imebra::codecType_t::dicom);
                   
                    std::string msg = ns::createJsonResponse(ns::PENDING, "storing: " + sop);
                    progress.Send(msg.c_str(), msg.length());

                    // Send a response
                    dimse->sendCommandOrResponse(CStoreResponse(command, dimseStatusCode_t::success));
                }
                catch (std::exception e)
                {
                    std::cerr << "exception: " << e.what() << std::endl;
                    break;
                }
        }    
        }
}

GetAsyncWorker::GetAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}


void GetAsyncWorker::Execute(const ExecutionProgress& progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    if (in.tags.empty()) {
        SetErrorJson("Tags not set");
        return;
    }

    if (!in.source.valid()) {
        SetErrorJson("Source not set");
        return;
    }

    if (!in.target.valid()) {
        SetErrorJson("Target not set");
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

    std::vector<std::string> dcmLongSCUStorageSOPClassUIDs = ns::dcmLongSCUStorageSOPClassUIDs();
    for (int j = 0; j < dcmLongSCUStorageSOPClassUIDs.size(); j++)
    {
        imebra::PresentationContext context(dcmLongSCUStorageSOPClassUIDs[j], true, true);
        context.addTransferSyntax(imebra::uidImplicitVRLittleEndian_1_2_840_10008_1_2);
        context.addTransferSyntax(imebra::uidExplicitVRLittleEndian_1_2_840_10008_1_2_1);
        presentationContexts.addPresentationContext(context);
    }

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

    // Create a std::promise object
	std::promise<void> exitSignal;
 
	//Fetch std::future object associated with promise
	std::future<void> futureObj = exitSignal.get_future();

    dimse.sendCommandOrResponse(command);
    std::thread storeProc(StoreProc, &dimse, progress, std::move(futureObj));

    try
    {

        for (;;)
        {
            imebra::DimseResponse response(dimse.getCGetResponse(command));
            if (response.getStatus() == imebra::dimseStatus_t::success)
            {
                _jsonOutput = {};
            	exitSignal.set_value();
                break;
            }
            else if (response.getStatus() == imebra::dimseStatus_t::pending)
            {
                /*
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
                    SetErrorJson("parse error: " + std::string(error.what()));
                }
                */
            }
            else {
                SetErrorJson("Get-scu request failed: " + response.getStatusCode());
	            exitSignal.set_value();
                break;
            }
        }
        storeProc.join();
    }
    catch (const StreamEOFError & error)
    {
        // The association has been closed
        SetErrorJson("stream error: " + std::string(error.what()));
    }
}