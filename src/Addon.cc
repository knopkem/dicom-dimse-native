#include <napi.h>

#include "DataProcessingAsyncWorker.h"
#include "json.h"

using json = nlohmann::json;

using namespace Napi;

namespace ns {
     // a simple struct to model a person
    struct tag {
        std::string key;
        std::string value;
    };

    void to_json(json& j, const tag& p) {
        j = json{{"key", p.key}, {"value", p.value}};
    }

    void from_json(const json& j, tag& p) {
        j.at("key").get_to(p.key);
        j.at("value").get_to(p.value);
    }
} // namespace ns

void ProcessData(const CallbackInfo& info) {
    Buffer<uint8_t> data = info[0].As<Buffer<uint8_t>>();
    Function cb = info[1].As<Function>();
    
    DataProcessingAsyncWorker *worker = new DataProcessingAsyncWorker(data, cb);
    worker->Queue();
}

void DoFind(const CallbackInfo& info) {
    Env env = info.Env();
    String input = info[0].As<String>();
    String output;

    auto j = json::parse(input.Utf8Value());
    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        auto p2 = (*it).get<ns::tag>();
        output = String::New(env, p2.key + " | " + p2.value);
    }

    Function cb = info[1].As<Function>();
    cb.Call(env.Global(), {output});
}

Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "processData"),
                Function::New(env, ProcessData));
    exports.Set(String::New(env, "doFind"),
                Function::New(env, DoFind));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
