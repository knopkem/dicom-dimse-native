#include <napi.h>

#include "EchoAsyncWorker.h"
#include "FindAsyncWorker.h"
#include "MoveAsyncWorker.h"
#include "GetAsyncWorker.h"
#include "StoreAsyncWorker.h"
#include "ServerAsyncWorker.h"

using namespace Napi;

void DoEcho(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    EchoAsyncWorker * worker = new EchoAsyncWorker(input, cb);
    worker->Queue();
}

void DoFind(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    FindAsyncWorker * worker = new FindAsyncWorker(input, cb);
    worker->Queue();
}

void DoMove(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    MoveAsyncWorker * worker = new MoveAsyncWorker(input, cb);
    worker->Queue();
}

void DoGet(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    GetAsyncWorker * worker = new GetAsyncWorker(input, cb);
    worker->Queue();
}

void DoStore(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    StoreAsyncWorker * worker = new StoreAsyncWorker(input, cb);
    worker->Queue();
}

void StartScp(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    ServerAsyncWorker * worker = new ServerAsyncWorker(input, cb);
    worker->Queue();
}

Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "echoScu"),
                Function::New(env, DoFind));
    exports.Set(String::New(env, "findScu"),
                Function::New(env, DoFind));
    exports.Set(String::New(env, "moveScu"),
                Function::New(env, DoMove));
    exports.Set(String::New(env, "getScu"),
                Function::New(env, DoGet));
    exports.Set(String::New(env, "storeScu"),
                Function::New(env, DoStore));
    exports.Set(String::New(env, "startScp"),
                Function::New(env, StartScp));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
