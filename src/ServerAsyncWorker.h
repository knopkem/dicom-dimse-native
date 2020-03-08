#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class ServerAsyncWorker : public BaseAsyncWorker
{
    public:
        ServerAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
