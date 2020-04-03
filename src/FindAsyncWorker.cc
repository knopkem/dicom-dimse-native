#include "FindAsyncWorker.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcostrmz.h" /* for dcmZlibCompressionLevel */

#ifdef WITH_ZLIB
#include <zlib.h> /* for zlibVersion() */
#endif
#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
#include "dcmtk/ofstd/ofchrenc.h" /* for OFCharacterEncoding */
#endif


#include "json.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <iomanip>

using json = nlohmann::json;

namespace
{

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

unsigned int hex_to_int(std::string hex) {
    return std::strtoul(hex.c_str(), 0, 16);
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
    ns::DicomObject responseItem;
    for (const ns::DicomElement &element : m_requestContainer)
    {
        OFString value;
        responseIdentifiers->findAndGetOFStringArray(element.xtag, value);
        ns::DicomElement cp;
        cp.xtag = element.xtag;
        cp.value = value.c_str();
        responseItem.push_back(element);
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

    for (std::vector<ns::sTag>::iterator it = in.tags.begin(); it != in.tags.end(); ++it) {
        auto tag = (*it);
        queryAttributes.push_back(ns::toElement(tag.key, tag.value));
    }

    for (const ns::DicomElement &element : queryAttributes) {
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

    // do the main work: negotiate network association, perform C-FIND transaction,
    // process results, and finally tear down the association.
    cond = findscu.performQuery(
        in.target.ip.c_str(),
        std::stoi(in.target.port),
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
    for (const ns::DicomObject& obj: result) {
        json v = json::object();
        for (const ns::DicomElement& elm: obj) {

            std::string value = elm.value;
            std::string keyName =  int_to_hex(elm.xtag.getGroup()) + int_to_hex(elm.xtag.getElement());
            std::string vr = "PN";
            v[keyName] = { 
                {"Value", json::array({value})},
                {"vr", vr}
            };
        }
        outJson.push_back(v);
        _jsonOutput = outJson.dump();
    }

}
