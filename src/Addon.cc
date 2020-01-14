#include <napi.h>

#include "DataProcessingAsyncWorker.h"
#include "FindAsyncWorker.h"

using namespace Napi;

void ProcessData(const CallbackInfo& info) {
    Buffer<uint8_t> data = info[0].As<Buffer<uint8_t>>();
    Function cb = info[1].As<Function>();
    
    DataProcessingAsyncWorker *worker = new DataProcessingAsyncWorker(data, cb);
    worker->Queue();
}

void DoFind(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    FindAsyncWorker * worker = new FindAsyncWorker(input, cb);
    worker->Queue();
}

Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "processData"),
                Function::New(env, ProcessData));
    exports.Set(String::New(env, "doFind"),
                Function::New(env, DoFind));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
