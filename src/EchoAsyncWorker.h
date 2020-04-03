#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class EchoAsyncWorker : public BaseAsyncWorker
{
    public:
        EchoAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
