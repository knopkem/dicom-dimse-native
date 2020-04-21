#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <iomanip>

#include "json.h"
using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/ofstd/ofcond.h"    /* for class OFCondition */
#include "dcmtk/dcmdata/dcxfer.h"  /* for E_TransferSyntax */
#include "dcmtk/dcmnet/dimse.h"    /* for T_DIMSE_BlockingMode */

namespace ns {

    
    struct DicomElement
    {
        DcmTagKey xtag;
        std::string value;
    };

    typedef std::list<DicomElement> DicomObject;

     // a simple struct to model a person
    struct sTag {
        std::string key;
        std::string value;
        inline bool valid() {
            return !key.empty();
        }
    };

    inline OFString convertElement(const ns::DicomElement &element) {
        return element.xtag.toString().substr(1, 9) + OFString("=") + OFString(element.value.c_str());
    }

    inline DicomElement toElement(const std::string& key, const std::string& value) {
            DicomElement el;
            unsigned int grp = 0xffff;
            unsigned int elm = 0xffff;

            sscanf(key.substr(0, 4).c_str(), "%x", &grp);
            sscanf(key.substr(4, 4).c_str(), "%x", &elm);
            el.xtag = DcmTagKey(OFstatic_cast(Uint16,grp),OFstatic_cast(Uint16, elm));
            el.value = value;
            return el;
    }

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
        std::string storagePath;
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
        in.storagePath = toString(j, "storagePath");
        try {
            auto tags = j.at("tags");
            for (json::iterator it = tags.begin(); it != tags.end(); ++it) {
                sTag tag = (*it).get<sTag>();
                in.tags.push_back(tag);
            }
        } catch(...) {}
        return in;
    }


    enum eStatus {
        SUCCESS = 0,
        PENDING = 1,
        FAILURE = 2
    };
 
    inline std::string createJsonResponse(eStatus status, const std::string& message, const json& j = {}) {
  	std::string meaning = "success";
        if (status == PENDING) {
            meaning = "pending";
        }
        if (status == FAILURE) {
            meaning = "failure";
        }
        json v = json::object();
        v["container"] = j;
        v["message"] = message;
        v["code"] = (int)status;
        v["status"] = meaning;
        return v.dump(-1, ' ', true, nlohmann::detail::error_handler_t::replace);
    }

} // namespace ns
