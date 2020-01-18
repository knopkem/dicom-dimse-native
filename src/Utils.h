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

    inline std::string toString(const json& in, const std::string& key) {
        try {
            return in.at(key).get<std::string>();
        }
        catch(json::exception&) {

        }
        return "";
    }

    inline void to_json(json& j, const sTag& p) {
        j = json{{"key", p.key}, {"value", p.value}};
    }

    inline void from_json(const json& j, sTag& p) {
        p.key = toString(j, "key");
        p.value = toString(j, "value");
    }

    inline void to_json(json& j, const sIdent& p) {
        j = json{{"aet", p.aet}, {"ip", p.ip}, {"port", p.port}};
    }

    inline void from_json(const json& j, sIdent& p) {
        p.aet = toString(j, "aet");
        p.ip = toString(j, "ip");
        p.port = toString(j, "port");
    }


    inline sInput parseInputJson(const std::string& _input) {
       	json j = json::parse(_input);
        sInput in;
        try {
            in.source = j.at("source").get<sIdent>();
        } catch(...) {}
        try {
            in.target = j.at("target").get<sIdent>();
        } catch(...) {}
        in.destination = toString(j, "destination");
        try {
            auto tags = j.at("tags");
            for (json::iterator it = tags.begin(); it != tags.end(); ++it) {
                sTag tag = (*it).get<sTag>();
                in.tags.push_back(tag);
            }
        } catch(...) {}
        return in;
    }


} // namespace ns
