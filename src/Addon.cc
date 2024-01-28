#include <napi.h>

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/spi/logevent.h"
#include "dcmtk/oflog/appender.h"
#include "dcmtk/oflog/fileap.h"

#include "EchoAsyncWorker.h"
#include "FindAsyncWorker.h"
#include "GetAsyncWorker.h"
#include "MoveAsyncWorker.h"
#include "StoreAsyncWorker.h"
#include "ServerAsyncWorker.h"
#include "ParseAsyncWorker.h"
#include "CompressAsyncWorker.h"
#include "ShutdownAsyncWorker.h"

#include <iostream>

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

Value DoStore(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new StoreAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoParse(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new ParseAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoCompress(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new CompressAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value StartScp(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new ServerAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}

Value DoShutdown(const CallbackInfo& info) {
    std::string input = info[0].As<String>().Utf8Value();
    Function cb = info[1].As<Function>();

    auto worker = new ShutdownAsyncWorker(input, cb);
    worker->Queue();
    return info.Env().Undefined();
}


Object Init(Env env, Object exports) {

    using namespace dcmtk::log4cplus;
    Logger rootLogger = Logger::getRoot();
    rootLogger.removeAllAppenders();
    dcmtk::log4cplus::SharedAppenderPtr logfile(new dcmtk::log4cplus::FileAppender("dicom.log"));
    dcmtk::log4cplus::Logger log = dcmtk::log4cplus::Logger::getRoot();
    log.addAppender(logfile);

    exports.Set(String::New(env, "echoScu"),
                Function::New(env, DoEcho));
    exports.Set(String::New(env, "findScu"),
                Function::New(env, DoFind));
    exports.Set(String::New(env, "getScu"),
                Function::New(env, DoGet));
    exports.Set(String::New(env, "moveScu"),
                Function::New(env, DoMove));
    exports.Set(String::New(env, "storeScu"),
                Function::New(env, DoStore));
    exports.Set(String::New(env, "startScp"),
                Function::New(env, StartScp));
    exports.Set(String::New(env, "shutdownScu"),
                Function::New(env, DoShutdown));
    exports.Set(String::New(env, "parseFile"),
                Function::New(env, DoParse));
    exports.Set(String::New(env, "recompress"),
                Function::New(env, DoCompress));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
