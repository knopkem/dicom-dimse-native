#pragma once

#include "BaseAsyncWorker.h"

using namespace Napi;

class FindAsyncWorker : public BaseAsyncWorker
{
    public:
        FindAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);
};
