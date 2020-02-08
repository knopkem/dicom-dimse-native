#include <napi.h>
#pragma once

using namespace Napi;

class ServerAsyncWorker : public AsyncProgressWorker<char>
{
    public:
        ServerAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

        void OnOK();

        void OnProgress(const char *data, size_t size);


    private:
    std::string _input;
    std::string _output;
};
