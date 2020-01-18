#pragma once

#include "json.h"
using json = nlohmann::json;

namespace ns {
     // a simple struct to model a person
    struct sTag {
        std::string key;
        std::string value;
        inline bool valid() {
            return !key.empty();
        }
    };

    struct sIdent {
        std::string aet;
        std::string ip;
        std::string port;
        inline bool valid() {
            return !aet.empty() && !ip.empty() && !port.empty();
        } 
    };

    struct sInput {
        sIdent source;
        sIdent target;
        std::string destination;
        std::vector<sTag> tags;
        inline bool valid() {
            return source.valid() && target.valid();
        }
    };

    inline void to_json(json& j, const sTag& p) {
        j = json{{"key", p.key}, {"value", p.value}};
    }

    inline void from_json(const json& j, sTag& p) {
        j.at("key").get_to(p.key);
        j.at("value").get_to(p.value);
    }

    inline void to_json(json& j, const sIdent& p) {
        j = json{{"aet", p.aet}, {"ip", p.ip}, {"port", p.port}};
    }

    inline void from_json(const json& j, sIdent& p) {
        j.at("aet").get_to(p.aet);
        j.at("ip").get_to(p.ip);
        j.at("port").get_to(p.port);
    }


    inline sInput parseInputJson(const std::string& _input) {
       	json jsonInput = json::parse(_input);
        sInput in;
        in.source = jsonInput["source"].get<sIdent>();
        in.target = jsonInput["target"].get<sIdent>();
        in.destination = jsonInput["destination"].get<std::string>();
        auto tags = jsonInput["tags"];
        for (json::iterator it = tags.begin(); it != tags.end(); ++it) {
            sTag tag = (*it).get<sTag>();
            in.tags.push_back(tag);
        }
        return in;
    }


} // namespace ns
