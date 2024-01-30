#include "BaseAsyncWorker.h"

#include "Utils.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/spi/logevent.h"
#include "dcmtk/oflog/appender.h"

#include <vector>

class BufferAppender : public dcmtk::log4cplus::Appender {
public:
    BufferAppender() {}

    ~BufferAppender() {
          destructorImpl ();
    }

    virtual void close() {}

    std::vector<std::string> StringBuffer;

protected:
    virtual void append(const dcmtk::log4cplus::spi::InternalLoggingEvent& ev) {
        dcmtk::log4cplus::tostringstream oss;
        layout->formatAndAppend(oss, ev);

        StringBuffer.push_back(DCMTK_LOG4CPLUS_TSTRING_TO_STRING(oss.str()));
    }
};


BaseAsyncWorker::BaseAsyncWorker(std::string data, Function &callback) : AsyncProgressQueueWorker<char>(callback),
                                                                           _input(data)
{
    //add the custom appender
    // using namespace dcmtk::log4cplus;
    // Logger rootLogger = Logger::getRoot();
    // this->_appender = new BufferAppender();
    // rootLogger.addAppender(this->_appender);

    // disable verbose logging
    OFLog::configure(OFLogger::WARN_LOG_LEVEL);
}

BaseAsyncWorker::~BaseAsyncWorker() 
{
    // using namespace dcmtk::log4cplus;
    // Logger rootLogger = Logger::getRoot();
    // rootLogger.removeAppender(this->_appender);
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
        OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);
    } else {
        OFLog::configure(OFLogger::WARN_LOG_LEVEL);
    }
}
