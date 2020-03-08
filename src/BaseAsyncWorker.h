#pragma once

#include <napi.h>
#include <iostream>

using namespace Napi;

class BaseAsyncWorker : public AsyncProgressWorker<char>
{
    public:
        BaseAsyncWorker(std::string data, Function &callback);

        virtual void OnOK();
        
        virtual void OnProgress(const char *data, size_t size);

    protected:
        std::string _input;
        std::string _output;
};
