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

#include "dcmtk/dcmjpeg/djdecode.h"     /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/djencode.h"     /* for dcmjpeg encoders */
#include "dcmtk/dcmdata/dcrledrg.h"     /* for DcmRLEDecoderRegistration */
#include "dcmtk/dcmdata/dcrleerg.h"     /* for DcmRLEEncoderRegistration */
#include "dcmtk/dcmjpls/djdecode.h"     /* for dcmjpls decoder */
#include "dcmtk/dcmjpls/djencode.h"     /* for dcmjpls encoder */
#include "dcmtk/dcmj2k/djdecode.h"     /* for dcmj2k decoder*/
#include "dcmtk/dcmj2k/djencode.h"     /* for dcmj2k encoder */

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
        int port;
        inline bool valid() {
            return !aet.empty() && !ip.empty() && port > 0;
        } 
    };

    struct sInput {
        sInput() : verbose(false), permissive(false), storeOnly(false), writeFile(true), lossyQuality(80) {}
        sIdent source;
        sIdent target;
        std::string storagePath;
        std::string sourcePath;
        std::string destination;
        std::string netTransferPrefer;
        std::string netTransferPropose;
        std::string writeTransfer;
        std::string charset;
        std::vector<sTag> tags;
        std::vector<sIdent> peers;
        int lossyQuality;
        bool verbose;
        bool permissive;
        bool storeOnly;
        bool writeFile;
        inline bool valid() {
            return source.valid() && target.valid();
        }
    };

    inline std::string toString(const json& in, const std::string& key) {
        try {
            return in.at(key).get<std::string>();
        }
        catch(json::exception&) {
            // no error log on purpose
        }
        return "";
    }

    inline int toInt(const json& in, const std::string& key) {
        try {
            return in.at(key).get<int>();
        }
        catch(json::exception&) {
            // no error log on purpose
        }
        // try again from string
        try {
            return std::stoi(in.at(key).get<std::string>());
        }
        catch(json::exception&) {
            // no error log on purpose
        }
        // fail
        return -1;
    }


    static bool codecsRegistered = false;

    static void registerCodecs() {
        if (!codecsRegistered) {
            DcmRLEDecoderRegistration::registerCodecs();
            DJDecoderRegistration::registerCodecs();
            DJLSDecoderRegistration::registerCodecs();
            FMJPEG2KDecoderRegistration::registerCodecs();
            
            DJEncoderRegistration::registerCodecs();
            DJLSEncoderRegistration::registerCodecs();
            DcmRLEEncoderRegistration::registerCodecs();
            FMJPEG2KEncoderRegistration::registerCodecs();
            codecsRegistered = true;
        }
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
        p.port = toInt(j, "port");
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
        in.sourcePath = toString(j, "sourcePath");
        in.netTransferPrefer = toString(j, "netTransferPrefer");
        in.netTransferPropose = toString(j, "netTransferPropose");
        in.writeTransfer = toString(j, "writeTransfer");
        in.charset = toString(j, "charset");
        try {
            auto tags = j.at("tags");
            for (json::iterator it = tags.begin(); it != tags.end(); ++it) {
                sTag tag = (*it).get<sTag>();
                in.tags.push_back(tag);
            }
        } catch(...) {}
        try {
            auto tags = j.at("peers");
            for (json::iterator it = tags.begin(); it != tags.end(); ++it) {
                sIdent peer = (*it).get<sIdent>();
                in.peers.push_back(peer);
            }
        } catch(...) {}
        try {
            in.permissive = j.at("permissive");
        } catch(...) {}
        try {
            in.verbose = j.at("verbose");
        } catch(...) {}
        try {
            in.storeOnly = j.at("storeOnly");
        } catch (...) {}
        try {
            in.writeFile = j.at("writeFile");
        }
        catch (...) {}
        try {
            in.lossyQuality = toInt(j, "lossyQuality");
        }
        catch (...) {}
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
