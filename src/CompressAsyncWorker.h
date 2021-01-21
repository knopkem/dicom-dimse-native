#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class CompressAsyncWorker : public BaseAsyncWorker
{
    public:
        CompressAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
