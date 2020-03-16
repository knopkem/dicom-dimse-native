#include "BaseAsyncWorker.h"

#include "Utils.h"

BaseAsyncWorker::BaseAsyncWorker(std::string data, Function &callback) : AsyncProgressWorker<char>(callback),
                                                                           _input(data)
{
}

void BaseAsyncWorker::OnOK()
{
        HandleScope scope(Env());
        std::string msg = ns::createJsonResponse(ns::SUCCESS, "request succeeded", _jsonOutput);
        String o = String::New(Env(), msg);
        Callback().Call({o});
}

void BaseAsyncWorker::OnProgress(const char *data, size_t size)
{
        HandleScope scope(Env());
        String o = String::New(Env(), data, size);
        Callback().Call({o});
}

void BaseAsyncWorker::SetErrorJson(const std::string& message)
{
       std::string sError =  ns::createJsonResponse(ns::FAILURE, message);
       SetError(sError);
}

void BaseAsyncWorker::SendInfo(const std::string& msg, const ExecutionProgress& progress, ns::eStatus status)
{
    std::string msg2 = ns::createJsonResponse(status, msg);
    progress.Send(msg2.c_str(), msg2.length());
}