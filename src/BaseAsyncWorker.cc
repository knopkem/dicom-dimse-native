#include "BaseAsyncWorker.h"

#include "Utils.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"

class BufferAppender : public dcmtk::log4cplus::Appender {
public:
    BufferAppender() {}

    ~QDebugAppender() {
          destructorImpl ();
    }

    virtual void close() {}

    void setExecutionProgress(const ExecutionProgress &progress) {
        this->_progress = &progress;
    }

protected:
    virtual void append(const dcmtk::log4cplus::spi::InternalLoggingEvent& ev) {
        dcmtk::log4cplus::tostringstream oss;
        layout->formatAndAppend(oss, ev);

        if (this->_progress) {
          std::string msg2 = ns::createJsonResponse(ns::PENDING, DCMTK_LOG4CPLUS_TSTRING_TO_STRING(oss.str()).c_str());
          this->_progress->Send(msg2.c_str(), msg2.length());
        }
    }

private:
      ExecutionProgress* _progress;
};


BaseAsyncWorker::BaseAsyncWorker(std::string data, Function &callback) : AsyncProgressQueueWorker<char>(callback),
                                                                           _input(data)
{
        // remove default appender and add the custom appender
        using namespace dcmtk::log4cplus;
        Logger rootLogger = Logger::getRoot();
        rootLogger.removeAllAppenders();
        dcmtk::log4cplus::SharedAppenderPtr bufferAppender(new dcmtk::log4cplus::FileAppender(logfilename));
        rootLogger.addAppender( bufferAppender );

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


void BaseAsyncWorker::EnableVerboseLogging(bool enabled, const ExecutionProgress &progress)
{
    if (enabled) {
        OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);
    } else {
        OFLog::configure(OFLogger::WARN_LOG_LEVEL);
    }
    this->_bufferAppender.setExecutionProgress(progress);
}
