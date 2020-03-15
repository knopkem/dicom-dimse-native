#pragma once

#include <napi.h>
#include <iostream>

#include "json.h"

using namespace Napi;

class BaseAsyncWorker : public AsyncProgressWorker<char>
{
    public:
        BaseAsyncWorker(std::string data, Function &callback);

        virtual void OnOK();
        
        virtual void OnProgress(const char *data, size_t size);

    protected:

        void SetErrorJson(const std::string& message);

        std::string _input;
        nlohmann::json _jsonOutput;
};
