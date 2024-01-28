#pragma once

#include <napi.h>
#include <iostream>

#include "json.h"
#include "Utils.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/spi/logevent.h"
#include "dcmtk/oflog/appender.h"

using namespace Napi;


class BaseAsyncWorker : public AsyncProgressQueueWorker<char>
{
    public:
        BaseAsyncWorker(std::string data, Function &callback);

        ~BaseAsyncWorker();

        virtual void OnOK();
        
        virtual void OnProgress(const char *data, size_t size);

    protected:

        void SetErrorJson(const std::string& message);

        void SendInfo(const std::string& msg, const ExecutionProgress& progress, ns::eStatus status = ns::PENDING);

        void EnableVerboseLogging(bool enabled);


        std::string _input;
        nlohmann::json _jsonOutput;
        std::string _error;
        dcmtk::log4cplus::SharedAppenderPtr _appender;
};
