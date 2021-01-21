#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class ParseAsyncWorker : public BaseAsyncWorker
{
    public:
        ParseAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
