#include <napi.h>

#include "EchoAsyncWorker.h"
#include "FindAsyncWorker.h"
#include "GetAsyncWorker.h"
#include "MoveAsyncWorker.h"

using namespace Napi;

Value DoEcho(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new EchoAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoFind(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new FindAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoGet(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new GetAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoMove(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new MoveAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}


Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "echoScu"),
                Function::New(env, DoEcho));
    exports.Set(String::New(env, "findScu"),
                Function::New(env, DoFind));
    exports.Set(String::New(env, "getScu"),
                Function::New(env, DoGet));
    exports.Set(String::New(env, "moveScu"),
                Function::New(env, DoMove));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
