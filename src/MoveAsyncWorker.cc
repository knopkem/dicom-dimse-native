#include "MoveAsyncWorker.h"


#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;


MoveAsyncWorker::MoveAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void MoveAsyncWorker::Execute(const ExecutionProgress& progress)
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


}
