#pragma once

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/ofstd/ofcond.h"    /* for class OFCondition */
#include "dcmtk/dcmdata/dcxfer.h"  /* for E_TransferSyntax */
#include "dcmtk/dcmnet/dimse.h"    /* for T_DIMSE_BlockingMode */


#include "json.h"
using json = nlohmann::json;

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

    inline void applyTags( const sInput& in, DicomObject* queryAttributes, OFList<OFString>* overrideKeys) {
        for (std::vector<ns::sTag>::iterator it = in.tags.begin(); it != in.tags.end(); ++it) {
            auto tag = (*it);
            ns::DicomElement el;
            unsigned int grp = 0xffff;
            unsigned int elm = 0xffff;

            sscanf(tag.key.substr(0, 4).c_str(), "%x", &grp);
            sscanf(tag.key.substr(4, 4).c_str(), "%x", &elm);
            el.xtag = DcmTagKey(grp, elm);
            el.value = tag.value;
            queryAttributes->push_back(el);
        }

        for (const ns::DicomElement &element : queryAttributes)
        {
            OFString key = convertElement(element);
            overrideKeys->push_back(key);
        }
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
 
    std::string createJsonResponse(eStatus status, const std::string& message, const json& j = {});

} // namespace ns
