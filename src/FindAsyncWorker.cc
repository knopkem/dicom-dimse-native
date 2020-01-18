#include "FindAsyncWorker.h"

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

#include "json.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <iomanip>

using namespace imebra;
using json = nlohmann::json;

namespace {
	std::string int_to_hex( uint16_t i )
	{
	std::stringstream stream;
	stream  << std::setfill ('0') << std::setw(sizeof(uint16_t)*2) 
			<< std::hex << i;
	return stream.str();
	}
}

FindAsyncWorker::FindAsyncWorker(std::string data, Function &callback) : AsyncWorker(callback),
                                                                           _input(data)
{
}

void FindAsyncWorker::Execute()
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

	const std::string abstractSyntax = uidStudyRootQueryRetrieveInformationModelFIND_1_2_840_10008_5_1_4_1_2_2_1;

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
	context.addTransferSyntax("1.2.840.10008.1.2"); // Implicit VR little endian
	context.addTransferSyntax("1.2.840.10008.1.2.1"); // Explicit VR little endian
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


	imebra::CFindCommand command( abstractSyntax,
		dimse.getNextCommandID(),
		dimseCommandPriority_t::medium,
		abstractSyntax,
		payload);
	dimse.sendCommandOrResponse(command);

	try
	{
		json outJson = json::array();
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
					json v = json::object();
					std::string keyName =  int_to_hex(tag.getGroupId()) + int_to_hex(tag.getTagId());
					v[keyName] = { 
						{"Value", {value}},
						{"vr", "UN"}
					};
					outJson.push_back(v);
                }

			}
			catch (std::exception & e)
			{
				SetError("Exception: " + std::string(e.what()));
			}
		}
		else
		{
			SetError("Find-scu request failed");
			break;
		}
		}
		_output = outJson.dump();
	}
	catch (const StreamEOFError & error)
	{
		// The association has been closed
		SetError("Association was closed: " + std::string(error.what()));
	}
}

void FindAsyncWorker::OnOK()
{
	HandleScope scope(Env());
	String output = String::New(Env(), _output);
	Callback().Call({output});
}
