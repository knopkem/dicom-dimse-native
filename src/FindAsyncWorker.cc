#include "FindAsyncWorker.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcostrmz.h" /* for dcmZlibCompressionLevel */

#ifdef WITH_ZLIB
#include <zlib.h> /* for zlibVersion() */
#endif
#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
#include "dcmtk/ofstd/ofchrenc.h" /* for OFCharacterEncoding */
#endif

#define APPLICATIONTITLE "IMEBRA"
#define PEERAPPLICATIONTITLE "CONQUESTSRV1"

#define SHORTCOL 4
#define LONGCOL 20

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

struct DicomElement
{
    DcmTagKey xtag;
    std::string value;
};

typedef std::list<DicomElement> DicomObject;

class FindScuCallback : public DcmFindSCUCallback
{
public:
    FindScuCallback(const DicomObject &rqContainer, std::list<DicomObject> *rspContainer);

    ~FindScuCallback() {}

    void callback(
        T_DIMSE_C_FindRQ *request,
        int &responseCount,
        T_DIMSE_C_FindRSP *rsp,
        DcmDataset *responseIdentifiers);

private:
    DicomObject m_requestContainer;
    std::list<DicomObject> *m_responseContainer;
};

FindScuCallback::FindScuCallback(const DicomObject &rqContainer, std::list<DicomObject> *rspContainer)
    : m_requestContainer(rqContainer), m_responseContainer(rspContainer)
{
}

void FindScuCallback::callback(T_DIMSE_C_FindRQ *request, int &responseCount, T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers)
{
    DicomObject responseItem;
    for (const DicomElement &dicomElement : m_requestContainer)
    {
        OFString value;
        responseIdentifiers->findAndGetOFStringArray(dicomElement.xtag, value);
        DicomElement cp;
        cp.xtag = dicomElement.xtag;
        cp.value = value.c_str();
        responseItem.push_back(dicomElement);
    }
    m_responseContainer->push_back(responseItem);
}

OFString convertElement(const DicomElement &element)
{
    return element.xtag.toString().substr(1, 9) + OFString("=") + OFString(element.value.c_str());
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

    DicomObject queryAttributes;

    for (std::vector<ns::sTag>::iterator it = in.tags.begin(); it != in.tags.end(); ++it) {
        auto tag = (*it);
        DicomElement el;
        unsigned int grp = 0xffff;
        unsigned int elm = 0xffff;

        sscanf(tag.key.substr(0, 4).c_str(), "%x", &grp);
        sscanf(tag.key.substr(4, 4).c_str(), "%x", &elm);
        el.xtag = DcmTagKey(grp, elm);
        el.value = tag.value;
        queryAttributes.push_back(el);
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

    std::list<DicomObject> result;


    OFList<OFString> overrideKeys;
    for (const DicomElement &element : queryAttributes)
    {
        OFString key = convertElement(element);
        overrideKeys.push_back(key);
    }

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
    for (const DicomObject& obj: result) {
        json v = json::object();
        for (const DicomElement& elm: obj) {

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
