#pragma once

#include <napi.h>
#include <iostream>

#include "json.h"
#include "Utils.h"

using namespace Napi;

class BaseAsyncWorker : public AsyncProgressWorker<char>
{
    public:
        BaseAsyncWorker(std::string data, Function &callback);

        virtual void OnOK();
        
        virtual void OnProgress(const char *data, size_t size);

    protected:

        void SetErrorJson(const std::string& message);

        void SendInfo(const std::string& msg, const ExecutionProgress& progress, ns::eStatus status = ns::PENDING);

        std::string _input;
        nlohmann::json _jsonOutput;
};
