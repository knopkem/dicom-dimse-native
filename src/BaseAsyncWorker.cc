#include "BaseAsyncWorker.h"

BaseAsyncWorker::BaseAsyncWorker(std::string data, Function &callback) : AsyncProgressWorker<char>(callback),
                                                                           _input(data)
{
}

void BaseAsyncWorker::OnOK()
{
        HandleScope scope(Env());
        String output = String::New(Env(), _output);
        Callback().Call({output});
}

void BaseAsyncWorker::OnProgress(const char *data, size_t size)
{
        HandleScope scope(Env());
        String output = String::New(Env(), data, size);
        Callback().Call({output});
}
