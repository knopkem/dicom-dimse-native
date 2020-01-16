#pragma once

#include "json.h"
using json = nlohmann::json;

namespace ns {
     // a simple struct to model a person
    struct tag {
        std::string key;
        std::string value;
    };

    inline void to_json(json& j, const tag& p) {
        j = json{{"key", p.key}, {"value", p.value}};
    }

    inline void from_json(const json& j, tag& p) {
        j.at("key").get_to(p.key);
        j.at("value").get_to(p.value);
    }
} // namespace ns
