#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class GetAsyncWorker : public BaseAsyncWorker
{
    public:
        GetAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
