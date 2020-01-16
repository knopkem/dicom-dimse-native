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
	// Allocate a TCP stream that connects to the DICOM SCP
	imebra::TCPStream tcpStream(TCPActiveAddress("localhost", "5678"));

	// Allocate a stream reader and a writer that use the TCP stream.
	// If you need a more complex stream (e.g. a stream that uses your
	// own services to send and receive data) then use a Pipe
	imebra::StreamReader readSCU(tcpStream.getStreamInput());
	imebra::StreamWriter writeSCU(tcpStream.getStreamOutput());

	// Add all the abstract syntaxes and the supported transfer
	// syntaxes for each abstract syntax (the pair abstract/transfer syntax is
	// called "presentation context")
	imebra::PresentationContext context(uidStudyRootQueryRetrieveInformationModelFIND_1_2_840_10008_5_1_4_1_2_2_1); // move
	context.addTransferSyntax("1.2.840.10008.1.2"); // Implicit VR little endian
	context.addTransferSyntax("1.2.840.10008.1.2.1"); // Explicit VR little endian
	imebra::PresentationContexts presentationContexts;
	presentationContexts.addPresentationContext(context);

	// The AssociationSCU constructor will negotiate a connection through
	// the readSCU and writeSCU stream reader and writer
	imebra::AssociationSCU scu("IMEBRA", "CONQUESTSRV1", 1, 1, presentationContexts, readSCU, writeSCU, 10);

	// The DIMSE service will use the negotiated association to send and receive
	// DICOM commands
	imebra::DimseService dimse(scu);

	// Let's prepare a dataset to store on the SCP
	imebra::MutableDataSet payload; // We will use the negotiated transfer syntax
    /*
	payload.setString(TagId(imebra::tagId_t::QueryRetrieveLevel_0008_0052), "STUDY", imebra::tagVR_t::CS);
	payload.setString(TagId(tagId_t::StudyInstanceUID_0020_000D), "1.3.46.670589.11.0.1.1996082307380006", imebra::tagVR_t::UI);
	payload.setString(TagId(tagId_t::PatientName_0010_0010), "", imebra::tagVR_t::PN);
    */
    json js = json::parse(_input);
    auto j = js["tags"];
    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        auto tag = (*it).get<ns::tag>();
    	payload.setString(TagId((tagId_t)std::stoi(tag.key, 0, 16)), tag.value);
    }

	const std::string abstractSyntax = uidStudyRootQueryRetrieveInformationModelFIND_1_2_840_10008_5_1_4_1_2_2_1;

	imebra::CFindCommand command( abstractSyntax,
		dimse.getNextCommandID(),
		dimseCommandPriority_t::medium,
		uidStudyRootQueryRetrieveInformationModelFIND_1_2_840_10008_5_1_4_1_2_2_1,
		payload);
	dimse.sendCommandOrResponse(command);

	try
	{
		for (;;)
		{

		imebra::DimseResponse rsp(dimse.getCFindResponse(command));

		if (rsp.getStatus() == imebra::dimseStatus_t::success)
		{
			break;
		}
		else if (imebra::dimseStatus_t::pending == rsp.getStatus())
		{
			imebra::DataSet data = rsp.getPayloadDataSet();
			try
			{
                imebra::tagsIds_t allTags = data.getTags();
                for (imebra::tagsIds_t::iterator it = allTags.begin() ; it != allTags.end(); ++it) {
                    imebra::TagId tag(*it);
    				std::string value = data.getString(tag, 0);
                    _output += value;
                }

			}
			catch (std::exception & e)
			{
			}
		}
		else
		{
			break;
		}
		}

	}
	catch (const StreamEOFError & error)
	{
		// The association has been closed
	}
}

void MoveAsyncWorker::OnOK()
{
        String output = String::New(Env(), _output);
        Callback().Call({output});
}
