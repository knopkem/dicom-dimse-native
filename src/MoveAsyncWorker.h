#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class MoveAsyncWorker : public BaseAsyncWorker
{
    public:
        MoveAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
