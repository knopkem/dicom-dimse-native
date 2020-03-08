#pragma once 

#include "BaseAsyncWorker.h"

using namespace Napi;

class StoreAsyncWorker : public BaseAsyncWorker
{
    public:
        StoreAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

};
