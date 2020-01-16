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



StoreAsyncWorker::StoreAsyncWorker(std::string data, Function &callback) : AsyncWorker(callback),
                                                                           _input(data)
{
}

void StoreAsyncWorker::Execute()
{

}

void StoreAsyncWorker::OnOK()
{
        String output = String::New(Env(), _output);
        Callback().Call({output});
}
