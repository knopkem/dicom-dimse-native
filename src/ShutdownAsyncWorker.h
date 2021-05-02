#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class ShutdownAsyncWorker : public BaseAsyncWorker
{
    public:
        ShutdownAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
