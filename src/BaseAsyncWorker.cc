#include "BaseAsyncWorker.h"

#include "Utils.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"


BaseAsyncWorker::BaseAsyncWorker(std::string data, Function &callback) : AsyncProgressWorker<char>(callback),
                                                                           _input(data)
{
        // disable verbose logging
        OFLog::configure(OFLogger::WARN_LOG_LEVEL);
}

void BaseAsyncWorker::OnOK()
{
        HandleScope scope(Env());
        std::string msg = ns::createJsonResponse(ns::SUCCESS, "request succeeded", _jsonOutput);
        if (_error.length() > 0) msg = _error;
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
       _error =  ns::createJsonResponse(ns::FAILURE, message);
       // SetError(sError);
}

void BaseAsyncWorker::SendInfo(const std::string& msg, const ExecutionProgress& progress, ns::eStatus status)
{
    std::string msg2 = ns::createJsonResponse(status, msg);
    progress.Send(msg2.c_str(), msg2.length());
}

void BaseAsyncWorker::EnableVerboseLogging(bool enabled)
{
    if (enabled) {
        OFLog::configure(OFLogger::INFO_LOG_LEVEL);
    } else {
        OFLog::configure(OFLogger::WARN_LOG_LEVEL);
    }
}
