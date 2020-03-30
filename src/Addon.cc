#include <napi.h>

#include "FindAsyncWorker.h"
#include "GetAsyncWorker.h"

using namespace Napi;

Value DoFind(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    FindAsyncWorker * worker = new FindAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoGet(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    GetAsyncWorker * worker = new GetAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}


Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "findScu"),
                Function::New(env, DoFind));
    exports.Set(String::New(env, "getScu"),
                Function::New(env, DoGet));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
