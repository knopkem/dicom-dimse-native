#include "FindAsyncWorker.h"

#include "json.h"

using json = nlohmann::json;


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


FindAsyncWorker::FindAsyncWorker(std::string data, Function &callback) : AsyncWorker(callback),
                                                                           _input(data)
{
}

void FindAsyncWorker::Execute()
{
    json js = json::parse(_input);
    auto j = js["tags"];
    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        auto p2 = (*it).get<ns::tag>();
        _output = p2.key + " | " + p2.value;
        // _output = String::New(Env(), p2.key + " | " + p2.value);
        break;
    }
}

void FindAsyncWorker::OnOK()
{
        String output = String::New(Env(), _output);
        Callback().Call({output});
}
