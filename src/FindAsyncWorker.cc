#include "FindAsyncWorker.h"

#include "json.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <iomanip>
#include <vector>

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcostrmz.h"
#include "dcmtk/dcmdata/dcdeftag.h"

#ifdef WITH_ZLIB
#include <zlib.h>
#endif
#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
#include "dcmtk/ofstd/ofchrenc.h"
#endif

namespace
{
std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::string to_utf8(std::string str)
{
    std::string strOut;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    {
        uint8_t ch = *it;
        if (ch < 0x80)
        {
            strOut.push_back(ch);
        }
        else
        {
            strOut.push_back(0xc0 | ch >> 6);
            strOut.push_back(0x80 | (ch & 0x3f));
        }
    }
    return strOut;
}

std::string str_toupper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}

std::string int_to_hex(uint16_t i)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(uint16_t) * 2)
           << std::hex << i;
    return str_toupper(stream.str());
}


class FindScuCallback : public DcmFindSCUCallback
{
public:
    FindScuCallback(const ns::DicomObject &rqContainer, std::list<ns::DicomObject> *rspContainer);

    ~FindScuCallback() {}

    void callback(
        T_DIMSE_C_FindRQ *request,
        int &responseCount,
        T_DIMSE_C_FindRSP *rsp,
        DcmDataset *responseIdentifiers);

    std::string charset;

private:
    ns::DicomObject m_requestContainer;
    std::list<ns::DicomObject> *m_responseContainer;
};

FindScuCallback::FindScuCallback(const ns::DicomObject &rqContainer, std::list<ns::DicomObject> *rspContainer)
    : m_requestContainer(rqContainer), m_responseContainer(rspContainer)
{
}

void FindScuCallback::callback(T_DIMSE_C_FindRQ *request, int &responseCount, T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers)
{
    OFLogger rspLogger = OFLog::getLogger(DCMNET_LOGGER_NAME ".responses");

    if (DCM_dcmnetLogger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
    {
        OFString temp_str;
        DCMNET_INFO("Received Find Response " << responseCount);
        DCMNET_DEBUG(DIMSE_dumpMessage(temp_str, *rsp, DIMSE_INCOMING));
        if (rspLogger.isEnabledFor(OFLogger::INFO_LOG_LEVEL)) {
            DCMNET_DEBUG("Response Identifiers:" << OFendl << DcmObject::PrintHelper(*responseIdentifiers));
        }
    }
    /* otherwise check whether special response logger is enabled */
    else if (rspLogger.isEnabledFor(OFLogger::INFO_LOG_LEVEL))
    {
        OFLOG_INFO(rspLogger, "---------------------------");
        OFLOG_INFO(rspLogger, "Find Response: " << responseCount << " (" << DU_cfindStatusString(rsp->DimseStatus) << ")");
        OFLOG_INFO(rspLogger, DcmObject::PrintHelper(*responseIdentifiers));
    }

    // convert characterset if requested
    bool useSimpleUtf8Convert = true;
#if DCMTK_ENABLE_CHARSET_CONVERSION == DCMTK_CHARSET_CONVERSION_ICONV
      OFString sourceCharset;
      responseIdentifiers->findAndGetOFStringArray(DCM_SpecificCharacterSet, sourceCharset, OFFalse);
      if (sourceCharset.empty() && !charset.empty()) {
        sourceCharset = OFString(charset.c_str());
      }
      if (!sourceCharset.empty()) {
          if (responseIdentifiers->convertCharacterSet(sourceCharset, OFString("ISO_IR 192")).bad()) {
              OFLOG_WARN(rspLogger, "failed to convert " << sourceCharset << " to ISO_IR 192");
          } else {
            useSimpleUtf8Convert = false;
          }
      }
#endif

    ns::DicomObject responseItem;
    for (const ns::DicomElement &element : m_requestContainer)
    {
        OFString value;
        // TODO: use correct method depending on VR type 
        OFCondition status = responseIdentifiers->findAndGetOFStringArray(element.xtag, value);
        if (status.good()) {
            ns::DicomElement cp;
            cp.xtag = element.xtag;
            cp.value = useSimpleUtf8Convert ? to_utf8(value.c_str()) : std::string(value.c_str());
            responseItem.push_back(cp);
        }
    }
    m_responseContainer->push_back(responseItem);
}

} // namespace

FindAsyncWorker::FindAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void FindAsyncWorker::Execute(const ExecutionProgress &progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    EnableVerboseLogging(in.verbose);

    if (in.tags.empty())
    {
        SetErrorJson("Tags not set");
        return;
    }

    if (!in.source.valid())
    {
        SetErrorJson("Source not set");
        return;
    }

    if (!in.target.valid())
    {
        SetErrorJson("Target not set");
        return;
    }

    ns::DicomObject queryAttributes;
    OFList<OFString> overrideKeys;

    for (std::vector<ns::sTag>::iterator it = in.tags.begin(); it != in.tags.end(); ++it)
    {
        auto tag = (*it);
        queryAttributes.push_back(ns::toElement(tag.key, tag.value));
    }

    for (const ns::DicomElement &element : queryAttributes)
    {
        OFString key = ns::convertElement(element);
        overrideKeys.push_back(key);
    }

    OFStandard::initializeNetwork();

    // enabled or disable removal of trailing padding
    dcmEnableAutomaticInputDataCorrection.set(OFTrue);

    // declare findSCU handler and initialize network
    DcmFindSCU findscu;
    OFCondition cond = findscu.initializeNetwork(30);
    if (cond.bad())
    {
        SetErrorJson(cond.text());
        return;
    }

    std::list<ns::DicomObject> result;
    FindScuCallback callback(queryAttributes, &result);
    callback.charset = in.charset;

    // do the main work: negotiate network association, perform C-FIND transaction,
    // process results, and finally tear down the association.
    cond = findscu.performQuery(
        in.target.ip.c_str(),
        in.target.port,
        in.source.aet.c_str(),
        in.target.aet.c_str(),
        UID_FINDStudyRootQueryRetrieveInformationModel,
        EXS_Unknown,
        DIMSE_BLOCKING,
        30,
        ASC_DEFAULTMAXPDU,
        false,
        false,
        1,
        FEM_none,
        0,
        &overrideKeys,
        &callback,
        NULL,
        NULL,
        NULL);

    // destroy network structure
    cond = findscu.dropNetwork();
    if (cond.bad())
    {
        SetErrorJson(cond.text());
    }

    OFStandard::shutdownNetwork();

    // convert result
    json outJson = json::array();
    for (const ns::DicomObject &obj : result)
    {
        json v = json::object();
        for (const ns::DicomElement &elm : obj)
        {
            std::string value = elm.value;
            std::string keyName = int_to_hex(elm.xtag.getGroup()) + int_to_hex(elm.xtag.getElement());
            DcmTag t(elm.xtag);
            std::string vr = std::string(t.getVR().getVRName());
            json jsonValue = json::array();
            if (vr == "PN") {
                jsonValue.push_back({json{{"Alphabetic", value}}});
            }
            else if (vr == "IS" || vr == "SL" || vr == "SS" || vr == "UL" || vr == "US") {
                if (value.length() == 0) {
                    jsonValue.push_back(nullptr);
                }
                else {
                    std::vector<std::string> splitValue = split(value, '\\');
                    for (auto i : splitValue) {
                        try {
                            jsonValue.push_back(std::stoi(i));
                        }
                        catch (...) {
                            jsonValue.push_back(nullptr);
                        }
                    }
                }
            }
            else if (vr == "DS" || vr == "FL" || vr == "FD") {
                if (value.length() == 0) {
                    jsonValue.push_back(nullptr);
                }
                else {
                    std::vector<std::string> splitValue = split(value, '\\');
                    for (auto i : splitValue) {
                        try {
                            jsonValue.push_back(std::stof(i));
                        }
                        catch (...) {
                            jsonValue.push_back(nullptr);
                        }
                    }
                }
            }
            else {
                jsonValue = split(value, '\\');
            }

            v[keyName]["vr"] = vr;
            if (!(jsonValue.size() == 1 &&  jsonValue[0] == nullptr)) {
                v[keyName]["Value"] = jsonValue;
            }
        }
        outJson.push_back(v);
        _jsonOutput = outJson.dump();
    }
}
