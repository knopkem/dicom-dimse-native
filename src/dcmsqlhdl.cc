#include "dcmsqlhdl.h"

#include "dcmsqldb.h"

#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/dcmqrdb/dcmqrdbs.h"
#include "dcmtk/dcmqrdb/dcmqrcnf.h"
#include "dcmtk/dcmqrdb/dcmqropt.h"
#include "dcmtk/dcmqrdb/dcmqridx.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/dcmdata/dcdeftag.h"

#include <queue>


//------------------------------------------------------------------------------------------------------

void DcmQueryRetriveConfigExt::addPeer(const char* AETitle, const char* HostName, int PortNumber)
{
    sPeer p;
    p.aet = std::string(AETitle);
    p.hostname = std::string(HostName);
    p.port = PortNumber;
    _peers.push_back(p);
}

//------------------------------------------------------------------------------------------------------

int DcmQueryRetriveConfigExt::peerForAETitle(const char* AETitle, const char** HostName, int* PortNumber) const {

    std::list<sPeer>::const_iterator it;
    for (it = _peers.begin(); it != _peers.end(); it++)
    {
        // Access the object through iterator
        if (strcmp(it->aet.c_str(), AETitle) == 0) {
            size_t len = strlen(it->hostname.c_str());
            char* name = new char[len + 1];
            strcpy(name, it->hostname.c_str());
            *HostName = name;
            *PortNumber = it->port;
            return 1;
        }
    }
    DCMNET_WARN("No matching AET found for AET:" << AETitle);
    return 0;
}

//------------------------------------------------------------------------------------------------------

int DcmQueryRetriveConfigExt::peerInAETitle(const char* calledAETitle, const char* callingAETitle, const char* HostName) const
{
    if (_permissive) return 1;

    std::list<sPeer>::const_iterator it;
    for (it = _peers.begin(); it != _peers.end(); it++)
    {
        // Access the object through iterator
        if (strcmp(it->aet.c_str(), callingAETitle) == 0 /* &&
            strcmp(it->hostname.c_str(), HostName) == 0 */) {
            return 1;
        }
    }
    DCMNET_WARN("No matching AET found for AET:" << calledAETitle);
    return 0;
}

//------------------------------------------------------------------------------------------------------

int DcmQueryRetriveConfigExt::checkForSameVendor(const char* AETitle1, const char* AETitle2) const {
    return 1;
}

//------------------------------------------------------------------------------------------------------

DcmQueryRetrieveSQLiteDatabaseHandleFactory::DcmQueryRetrieveSQLiteDatabaseHandleFactory(const DcmQueryRetrieveConfig* config)
    : DcmQueryRetrieveDatabaseHandleFactory()
    , config_(config)
{

}

//------------------------------------------------------------------------------------------------------

DcmQueryRetrieveDatabaseHandle* DcmQueryRetrieveSQLiteDatabaseHandleFactory::createDBHandle(
    const char* /* callingAETitle */,
    const char* calledAETitle,
    OFCondition& result) const
{
    return new DcmQueryRetrieveSQLiteDatabaseHandle(config_->getStorageArea(calledAETitle));
}

//------------------------------------------------------------------------------------------------------

class DcmQueryRetrieveSQLiteDatabaseHandlePrivate
{
public:

    enum ctype {
        CMOVE,
        CSTORE,
        CFIND,
        CECHO
    };

    DcmQueryRetrieveSQLiteDatabaseHandlePrivate(const OFFilename& path);
    ~DcmQueryRetrieveSQLiteDatabaseHandlePrivate();

    bool isSOPClassSupported(const std::string& SOPClassUID, ctype type);

    int parseRequestList(DcmDataset* requestIdentifiers, DB_ElementList* plist);

    bool DB_TagSupported(const DcmTagKey& key);

    void DB_DuplicateElement(DB_SmallDcmElmt* src, DB_SmallDcmElmt* dst);

    void DB_MakeResponseList(std::list<DcmSmallDcmElm> lst);

    OFCondition DB_FreeElementList(DB_ElementList* lst);

    OFCondition DB_GetTagLevel(DcmTagKey tag, DB_LEVEL* level);

    OFCondition DB_GetTagKeyAttr(DcmTagKey tag, DB_KEY_TYPE* keyAttr);

    std::list<DcmSmallDcmElm> convertList(DB_ElementList* plist);

    void determineLevelBoundaries(const DB_QUERY_CLASS& root, DB_LEVEL& qLevel, DB_LEVEL& lLevel);

    OFCondition testRequestList(DB_ElementList* requestList, DB_LEVEL queryLevel, DB_LEVEL infLevel, DB_LEVEL lowestLevel);

    std::string levelToString(DB_LEVEL level);

    bool containsAttribute(const std::list<DcmSmallDcmElm>& list, DcmTagKey key);

    DcmSQLiteDatabase* db;
    DB_Private_Handle* handle;
    OFFilename storagePath;
    std::queue< std::list< DcmSmallDcmElm > > findResult;
    OFFilename rootPath;
};



DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DcmQueryRetrieveSQLiteDatabaseHandlePrivate(const OFFilename& path)
{
    db = new DcmSQLiteDatabase(path);
    handle = new DB_Private_Handle;
    storagePath = path;
}

//------------------------------------------------------------------------------------------------------

DcmQueryRetrieveSQLiteDatabaseHandlePrivate::~DcmQueryRetrieveSQLiteDatabaseHandlePrivate()
{
    DB_FreeElementList(handle->findRequestList);
    DB_FreeElementList(handle->findResponseList);
    delete handle;
    handle = NULL;
}

//------------------------------------------------------------------------------------------------------

bool DcmQueryRetrieveSQLiteDatabaseHandlePrivate::isSOPClassSupported(const std::string& SOPClassUID, ctype type)
{
    bool result = true;

    switch (type) {
    case CFIND:
        if (SOPClassUID == UID_FINDPatientRootQueryRetrieveInformationModel)
            handle->rootLevel = PATIENT_ROOT;
        else if (SOPClassUID == UID_FINDStudyRootQueryRetrieveInformationModel)
            handle->rootLevel = STUDY_ROOT;
        else if (SOPClassUID == UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel)
            handle->rootLevel = PATIENT_STUDY;
        else
            result = false;
            break;

    case CMOVE:
        if (SOPClassUID == UID_MOVEPatientRootQueryRetrieveInformationModel)
            handle->rootLevel = PATIENT_ROOT;
        else if (SOPClassUID == UID_MOVEStudyRootQueryRetrieveInformationModel)
            handle->rootLevel = STUDY_ROOT;
        else if (SOPClassUID == UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel)
            handle->rootLevel = PATIENT_STUDY;
        else if (SOPClassUID == UID_GETPatientRootQueryRetrieveInformationModel)
            handle->rootLevel = PATIENT_ROOT;
        else if (SOPClassUID == UID_GETStudyRootQueryRetrieveInformationModel)
            handle->rootLevel = STUDY_ROOT;
        else if (SOPClassUID == UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel)
            handle->rootLevel = PATIENT_STUDY;
        else
            result = false;
            break;
    default:
        result = false;
        break;
    }

    return result;
}

//------------------------------------------------------------------------------------------------------

int DcmQueryRetrieveSQLiteDatabaseHandlePrivate::parseRequestList(DcmDataset* requestIdentifiers, DB_ElementList* plist)
{
    DB_SmallDcmElmt     elem;
    DB_ElementList* last = NULL;
    OFBool              qrLevelFound = OFFalse;
    handle->findRequestList = NULL;

    int elemCount = (int)(requestIdentifiers->card());
    for (int elemIndex = 0; elemIndex < elemCount; elemIndex++) {

        DcmElement* dcelem = requestIdentifiers->getElement(elemIndex);

        elem.XTag = dcelem->getTag().getXTag();
        if (elem.XTag == DCM_QueryRetrieveLevel || DB_TagSupported(elem.XTag)) {
            elem.ValueLength = dcelem->getLength();
            if (elem.ValueLength == 0) {
                elem.PValueField = NULL;
            }
            else if ((elem.PValueField = (char*)malloc((size_t)(elem.ValueLength + 1))) == NULL) {
                return 1;
            }
            else {
                char* s = NULL;
                dcelem->getString(s);
                strcpy(elem.PValueField, s);
            }

            if (elem.XTag == DCM_QueryRetrieveLevel) {
                char* pc;
                char level[50];

                strncpy(level, (char*)elem.PValueField,
                    (elem.ValueLength < 50) ? (size_t)(elem.ValueLength) : 49);

                for (pc = level; *pc; pc++)
                    *pc = ((*pc >= 'a') && (*pc <= 'z')) ? 'A' - 'a' + *pc : *pc;

                if (strncmp(level, PATIENT_LEVEL_STRING,
                    strlen(PATIENT_LEVEL_STRING)) == 0)
                    handle->queryLevel = PATIENT_LEVEL;
                else if (strncmp(level, STUDY_LEVEL_STRING,
                    strlen(STUDY_LEVEL_STRING)) == 0)
                    handle->queryLevel = STUDY_LEVEL;
                else if (strncmp(level, SERIE_LEVEL_STRING,
                    strlen(SERIE_LEVEL_STRING)) == 0)
                    handle->queryLevel = SERIE_LEVEL;
                else if (strncmp(level, IMAGE_LEVEL_STRING,
                    strlen(IMAGE_LEVEL_STRING)) == 0)
                    handle->queryLevel = IMAGE_LEVEL;
                else {
                    if (elem.PValueField)
                        free(elem.PValueField);
                    DCMNET_WARN("Illegal query level (" << level << ")");
                    return 2;
                }
                qrLevelFound = OFTrue;
            }
            else {
                if (DB_TagSupported(elem.XTag)) {

                    plist = (DB_ElementList*)malloc(sizeof(DB_ElementList));
                    if (plist == NULL) {
                        return 1;
                    }
                    plist->next = NULL;
                    DB_DuplicateElement(&elem, &(plist->elem));
                    if (handle->findRequestList == NULL) {
                        handle->findRequestList = last = plist;
                    }
                    else {
                        last->next = plist;
                        last = plist;
                    }
                }
            }

            if (elem.PValueField) {
                free(elem.PValueField);
            }
        }
    }

    if (!qrLevelFound) {
        DCMNET_WARN("missing Query/Retrieve Level!");
        DB_FreeElementList(handle->findRequestList);
        handle->findRequestList = NULL;
        return 3;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------

bool DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DB_TagSupported(const DcmTagKey& key)
{
    for (auto item : db->definedAttributes()) {
        if (item.tag == key)
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------

void DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DB_DuplicateElement(DB_SmallDcmElmt* src, DB_SmallDcmElmt* dst)
{
    bzero((char*)dst, sizeof(DB_SmallDcmElmt));
    dst->XTag = src->XTag;
    dst->ValueLength = src->ValueLength;

    if (src->ValueLength == 0)
        dst->PValueField = NULL;
    else {
        dst->PValueField = (char*)malloc((int)src->ValueLength + 1);
        bzero(dst->PValueField, (size_t)(src->ValueLength + 1));
        if (dst->PValueField != NULL) {
            memcpy(dst->PValueField, src->PValueField,
                (size_t)src->ValueLength);
        }
        else {
            DCMNET_WARN("DB_DuplicateElement: out of memory");
        }
    }
}

//------------------------------------------------------------------------------------------------------

void DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DB_MakeResponseList(std::list<DcmSmallDcmElm> lst)
{
    DB_ElementList* pRequestList = NULL;
    DB_ElementList* plist = NULL;
    DB_ElementList* last = NULL;

    this->handle->findResponseList = NULL;

    for (pRequestList = this->handle->findRequestList; pRequestList; pRequestList = pRequestList->next) {

        DB_SmallDcmElmt dcmElm;
        bool elemFound = false;
        for (auto qElm : lst) {
            if (qElm.XTag() == pRequestList->elem.XTag) {
                elemFound = true;
                dcmElm.XTag = qElm.XTag();
                char* cstr = new char[qElm.valueField().length() + 1];
                strcpy(cstr, qElm.valueField().c_str());
                dcmElm.PValueField = cstr;
                dcmElm.ValueLength = (Uint32)qElm.valueField().length() + 1;
                break;
            }

        }

        if (!elemFound)
            continue;

        plist = (DB_ElementList*)malloc(sizeof(DB_ElementList));
        if (plist == NULL) {
            DCMNET_ERROR("DB_MakeResponseList: out of memory");
            return;
        }
        plist->next = NULL;
        DB_DuplicateElement(&dcmElm, &plist->elem);

        if (this->handle->findResponseList == NULL) {
            this->handle->findResponseList = last = plist;
        }
        else {
            last->next = plist;
            last = plist;
        }

    }
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DB_FreeElementList(DB_ElementList* lst)
{
    if (lst == NULL) return EC_Normal;

    OFCondition cond = DB_FreeElementList(lst->next);
    if (lst->elem.PValueField != NULL) {
        free((char*)lst->elem.PValueField);
    }
    free(lst);
    return (cond);
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DB_GetTagLevel(DcmTagKey tag, DB_LEVEL* level)
{
    for (auto item : db->definedAttributes()) {
        if (item.tag == tag) {
            *level = item.level;
            return (EC_Normal);
        }
    }
    DCMNET_WARN("getTagLevel: tag not registered" << tag.toString().c_str());
    return EC_IllegalParameter;
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandlePrivate::DB_GetTagKeyAttr(DcmTagKey tag, DB_KEY_TYPE* keyAttr)
{
    for (auto item : db->definedAttributes()) {
        if (item.tag == tag) {
            *keyAttr = item.keyAttr;
            return (EC_Normal);
        }
    }
    DCMNET_WARN("getTagLevel: tag not registered" << tag.toString().c_str());
    return (EC_IllegalParameter);
}

//------------------------------------------------------------------------------------------------------

std::list<DcmSmallDcmElm> DcmQueryRetrieveSQLiteDatabaseHandlePrivate::convertList(DB_ElementList* plist)
{
    std::list<DcmSmallDcmElm> qElmlist;
    for (; plist; plist = plist->next) {
        if (plist->elem.PValueField) {
            DcmSmallDcmElm elem(plist->elem.XTag, std::string(plist->elem.PValueField));
            qElmlist.push_back(elem);
        }
        else {
            DcmSmallDcmElm elem(plist->elem.XTag, std::string(""));
            qElmlist.push_back(elem);
        }
    }
    return qElmlist;
}

//------------------------------------------------------------------------------------------------------

void DcmQueryRetrieveSQLiteDatabaseHandlePrivate::determineLevelBoundaries(const DB_QUERY_CLASS& root, DB_LEVEL& qLevel, DB_LEVEL& lLevel)
{
    switch (root) {
    case PATIENT_ROOT:
        qLevel = PATIENT_LEVEL;
        lLevel = IMAGE_LEVEL;
        break;
    case STUDY_ROOT:
        qLevel = STUDY_LEVEL;
        lLevel = IMAGE_LEVEL;
        break;
    case PATIENT_STUDY:
        qLevel = PATIENT_LEVEL;
        lLevel = STUDY_LEVEL;
        break;
    }
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandlePrivate::testRequestList(DB_ElementList* requestList, DB_LEVEL queryLevel, DB_LEVEL infLevel, DB_LEVEL lowestLevel)
{
    DB_ElementList* plist;
    DB_LEVEL    XTagLevel = PATIENT_LEVEL;
    DB_KEY_TYPE XTagType = OPTIONAL_KEY;
    int level;

    if (queryLevel < infLevel) {
        DCMNET_WARN("Level incompatible with Information Model (" << levelToString(queryLevel) << ")");
        return EC_IllegalParameter;
    }

    if (queryLevel > lowestLevel) {
        DCMNET_WARN("Level incompatible with Information Model (" << levelToString(queryLevel) << ")");
        return EC_IllegalParameter;
    }

    for (level = PATIENT_LEVEL; level <= IMAGE_LEVEL; level++) {

        if ((level == PATIENT_LEVEL) && (infLevel == STUDY_LEVEL)) {

            int atLeastOneKeyFound = OFFalse;
            for (plist = requestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != static_cast<DB_LEVEL>(level))
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (atLeastOneKeyFound && (queryLevel != STUDY_LEVEL)) {
                DCMNET_WARN("Key found in Study Root Information Model (" << levelToString(static_cast<DB_LEVEL>(level)) << ")");
                return EC_IllegalParameter;
            }
        }
        else if (static_cast<DB_LEVEL>(level) < queryLevel) {

            int uniqueKeyFound = OFFalse;
            for (plist = requestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != static_cast<DB_LEVEL>(level))
                    continue;
                DB_GetTagKeyAttr(plist->elem.XTag, &XTagType);
                if (XTagType != UNIQUE_KEY) {
                    DCMNET_WARN("Non Unique Key found (" << levelToString(static_cast<DB_LEVEL>(level)) << ")");
                    return EC_IllegalParameter;
                }
                else if (uniqueKeyFound) {
                    DCMNET_WARN("More than one Unique Key found (" << levelToString(static_cast<DB_LEVEL>(level)) << ")");
                    return EC_IllegalParameter;
                }
                else
                    uniqueKeyFound = OFTrue;
            }
        }
        else if (static_cast<DB_LEVEL>(level) == queryLevel) {

            int atLeastOneKeyFound = OFFalse;
            for (plist = requestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != static_cast<DB_LEVEL>(level))
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (!atLeastOneKeyFound) {
                DCMNET_WARN("No Key found at query level (" << levelToString(static_cast<DB_LEVEL>(level)) << ")");
                return EC_IllegalParameter;
            }
        }
        else if (static_cast<DB_LEVEL>(level) > queryLevel) {

            int atLeastOneKeyFound = OFFalse;
            for (plist = requestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != static_cast<DB_LEVEL>(level))
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (atLeastOneKeyFound) {
                DCMNET_WARN("Key found beyond query level (" << levelToString(static_cast<DB_LEVEL>(level)) << ")");
                return EC_IllegalParameter;
            }
        }

    }
    return EC_Normal;
}

//------------------------------------------------------------------------------------------------------

std::string DcmQueryRetrieveSQLiteDatabaseHandlePrivate::levelToString(DB_LEVEL level)
{
    std::string lvl("LEVEL_ERROR");
    switch (level)
    {
    case PATIENT_LEVEL:
        lvl = "PATIENT";
        break;
    case STUDY_LEVEL:
        lvl = "STUDY";
        break;
    case SERIE_LEVEL:
        lvl = "SERIES";
        break;
    case IMAGE_LEVEL:
        lvl = "IMAGE";
        break;
    default:
        DCMNET_ERROR("Level not defined");
        break;
    }
    return lvl;
}

//------------------------------------------------------------------------------------------------------

bool DcmQueryRetrieveSQLiteDatabaseHandlePrivate::containsAttribute(const std::list<DcmSmallDcmElm>& list, DcmTagKey key)
{
    for (auto item : list) {
        if (item.XTag() == key) return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------

DcmQueryRetrieveSQLiteDatabaseHandle::DcmQueryRetrieveSQLiteDatabaseHandle(const OFFilename& path) 
    : d(new DcmQueryRetrieveSQLiteDatabaseHandlePrivate(path))
{
}

//------------------------------------------------------------------------------------------------------

DcmQueryRetrieveSQLiteDatabaseHandle::~DcmQueryRetrieveSQLiteDatabaseHandle()
{
    delete d;
    d = NULL;
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::startFindRequest( const char *SOPClassUID, DcmDataset *findRequestIdentifiers, 
    DcmQueryRetrieveDatabaseStatus *status )
{
    OFCondition         cond = EC_Normal;
    DB_ElementList*     plist = NULL;
    DB_LEVEL            qLevel = PATIENT_LEVEL;
    DB_LEVEL            lLevel = IMAGE_LEVEL;

    if (!d->isSOPClassSupported(SOPClassUID, DcmQueryRetrieveSQLiteDatabaseHandlePrivate::CFIND)) {
        status->setStatus(STATUS_FIND_Refused_SOPClassNotSupported);
        return (EC_IllegalParameter);
    }

    int err = d->parseRequestList(findRequestIdentifiers, plist);
    if (err > 0) {
        if (err == 1)
            status->setStatus(STATUS_FIND_Refused_OutOfResources);
        else if (err == 2)
            status->setStatus(STATUS_FIND_Failed_UnableToProcess);
        else if (err == 3)
            status->setStatus(STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass);
        return (EC_IllegalParameter);
    }

    d->determineLevelBoundaries(d->handle->rootLevel, qLevel, lLevel);

    if (d->testRequestList(d->handle->findRequestList, d->handle->queryLevel, qLevel, lLevel).bad()) {
        DCMNET_ERROR("DB_startFindRequest () : STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass - Invalid RequestList");
    }

    std::list<DcmSmallDcmElm> findRequestList = d->convertList(d->handle->findRequestList);

    std::list< std::list<DcmSmallDcmElm> > resultList = d->db->find(findRequestList, d->handle->queryLevel, qLevel, lLevel);

    for( auto lst: resultList) {
        d->findResult.push(lst);
    }

    if (!d->findResult.empty()) {
        auto item = d->findResult.front();
        d->findResult.pop();
        d->DB_MakeResponseList(item);
    }

    return cond;
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::nextFindResponse(DcmDataset **findResponseIdentifiers,
    DcmQueryRetrieveDatabaseStatus *status, const DcmQueryRetrieveCharacterSetOptions& characterSetOptions)
{
    DB_ElementList*     plist = NULL;
    const char*         queryLevelString = NULL;
    OFCondition         cond = EC_Normal;

    if (d->handle->findResponseList == NULL) {
       DCMNET_INFO("nextFindResponse() : STATUS_Success");
        *findResponseIdentifiers = NULL;
        status->setStatus(STATUS_Success);
        return (EC_Normal);
    }

    *findResponseIdentifiers = new DcmDataset;
    if (*findResponseIdentifiers != NULL) {

        // Put responses
        for (plist = d->handle->findResponseList; plist != NULL; plist = plist->next) {
            DcmTag t(plist->elem.XTag);
            DcmElement* dce = DcmItem::newDicomElement(t);
            if (dce == NULL) {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return QR_EC_IndexDatabaseError;
            }
            if (plist->elem.PValueField != NULL &&
                strlen(plist->elem.PValueField) > 0) {
                OFCondition ec = dce->putString(plist->elem.PValueField);
                if (ec != EC_Normal) {
                    DCMNET_WARN("nextFindResponse(): cannot put");
                    status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                    return QR_EC_IndexDatabaseError;
                }
            }
            OFCondition ec = (*findResponseIdentifiers)->insert(dce, OFTrue /*replaceOld*/);
            if (ec != EC_Normal) {
                DCMNET_WARN("nextFindResponse(): cannot insert");
                status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                return QR_EC_IndexDatabaseError;
            }
        }

        switch (d->handle->queryLevel) {
        case PATIENT_LEVEL:
            queryLevelString = PATIENT_LEVEL_STRING;
            break;
        case STUDY_LEVEL:
            queryLevelString = STUDY_LEVEL_STRING;
            break;
        case SERIE_LEVEL:
            queryLevelString = SERIE_LEVEL_STRING;
            break;
        case IMAGE_LEVEL:
            queryLevelString = IMAGE_LEVEL_STRING;
            break;
        }
        DU_putStringDOElement(*findResponseIdentifiers, DCM_QueryRetrieveLevel, queryLevelString);
    }
    else {
        return (QR_EC_IndexDatabaseError);
    }

    d->DB_FreeElementList(d->handle->findResponseList);
    d->handle->findResponseList = NULL;

    if (!d->findResult.empty()) {
        auto item = d->findResult.front();
        d->findResult.pop();
        d->DB_MakeResponseList(item);
    }

    return cond;

}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::cancelFindRequest( DcmQueryRetrieveDatabaseStatus *status )
{
    d->DB_FreeElementList(d->handle->findRequestList);
    d->handle->findRequestList = NULL;
    d->DB_FreeElementList(d->handle->findResponseList);
    d->handle->findResponseList = NULL;
    while (!d->findResult.empty()) {
        d->findResult.pop();
    }

    status->setStatus(STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest);
    return (EC_Normal);
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::startMoveRequest( const char *SOPClassUID, 
    DcmDataset *moveRequestIdentifiers,  DcmQueryRetrieveDatabaseStatus *status )
{
    OFCondition         cond = EC_Normal;
    DB_ElementList*     plist = NULL;
    DB_LEVEL            qLevel = PATIENT_LEVEL;
    DB_LEVEL            lLevel = IMAGE_LEVEL;

    if (!d->isSOPClassSupported(SOPClassUID, DcmQueryRetrieveSQLiteDatabaseHandlePrivate::CMOVE)) {
        status->setStatus(STATUS_MOVE_Failed_SOPClassNotSupported);
        return (EC_IllegalParameter);
    }

    int err = d->parseRequestList(moveRequestIdentifiers, plist);
    if (err > 0) {
        if (err == 1)
            status->setStatus(STATUS_MOVE_Refused_OutOfResourcesSubOperations);
        else if (err == 2)
            status->setStatus(STATUS_MOVE_Failed_UnableToProcess);
        else if (err == 3)
            status->setStatus(STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass);
        return (EC_IllegalParameter);
    }

    d->determineLevelBoundaries(d->handle->rootLevel, qLevel, lLevel);

    std::list<DcmSmallDcmElm> findRequestList = d->convertList(d->handle->findRequestList);

    if (!d->containsAttribute(findRequestList, DCM_SeriesInstanceUID)) {
        findRequestList.push_back(DcmSmallDcmElm(DCM_SeriesInstanceUID, ""));
    }

    std::list< std::list<DcmSmallDcmElm> > seriesResult = d->db->find(findRequestList, SERIE_LEVEL, qLevel, lLevel);
    for (auto seriesList: seriesResult) {
        std::list<DcmSmallDcmElm> imgRequestList(seriesList);

        if (!d->containsAttribute(findRequestList, DCM_SOPInstanceUID)) {
            imgRequestList.push_back(DcmSmallDcmElm(DCM_SOPInstanceUID, ""));
        }

        imgRequestList.push_back(DcmSmallDcmElm(DCM_PrivateFileName, ""));
        imgRequestList.push_back(DcmSmallDcmElm(DCM_SOPClassUID, ""));

        std::list< std::list<DcmSmallDcmElm> > imgResult = d->db->find(imgRequestList, IMAGE_LEVEL, qLevel, lLevel);
        status->setStatus(STATUS_Pending);

        for(auto imgList: imgResult) {
            d->findResult.push(imgList);
        }
    }

    if (d->findResult.empty()) {
        DCMNET_ERROR("MoveRequest: no image found.");
        status->setStatus(STATUS_Success);
    }

    return cond;
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::nextMoveResponse(char* SOPClassUID, size_t SOPClassUIDSize, 
    char* SOPInstanceUID, size_t SOPInstanceUIDSize, char* imageFileName, size_t imageFileNameSize, 
    unsigned short* numberOfRemainingSubOperations, DcmQueryRetrieveDatabaseStatus* status)
{
    if (d->findResult.empty()) {
       DCMNET_INFO("no more images to send, finishing");
        *numberOfRemainingSubOperations = 0;
        status->setStatus(STATUS_Success);
        return (EC_Normal);
    }

    std::list<DcmSmallDcmElm> lst = d->findResult.front();
    d->findResult.pop();
    *numberOfRemainingSubOperations = (int)d->findResult.size();

    for(auto el: lst) {
        if (el.XTag() == DCM_PrivateFileName) {
            // std::string databaseRootPath = std::string(d->rootPath.getCharPointer());
            std::string filename = /*databaseRootPath + "/" +*/ el.valueField();
            strcpy(imageFileName, (char*)filename.c_str());
        }
        else if (el.XTag() == DCM_SOPClassUID) {
            strcpy(SOPClassUID, (char*)el.valueField().c_str());
        }
        else if (el.XTag() == DCM_SOPInstanceUID) {
            strcpy(SOPInstanceUID, (char*)el.valueField().c_str());
        }
    }

    if (strlen(SOPInstanceUID) == 0 || strlen(SOPClassUID) == 0 || strlen(imageFileName) == 0) {
        DCMNET_ERROR("failed to extract meta");
        return OFCondition(EC_IllegalParameter);
    }

    status->setStatus(STATUS_Pending);
    return (EC_Normal);
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::cancelMoveRequest( DcmQueryRetrieveDatabaseStatus *status )
{
    // not implemented on purpose
    return OFCondition(EC_IllegalParameter);
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::makeNewStoreFileName(const char* SOPClassUID, const char* SOPInstanceUID,
    char* newImageFileName, size_t newImageFileNameLen)
{
    /*
    OFString filename;
    char prefix[12];

    const char* m = dcmSOPClassUIDToModality(SOPClassUID);
    if (m == NULL) m = "XX";
    sprintf(prefix, "%s_", m);
    // unsigned int seed = fnamecreator.hashString(SOPInstanceUID);
    unsigned int seed = (unsigned int)time(NULL);
    newImageFileName[0] = 0; // return empty string in case of error
    OFFilenameCreator fnamecreator;
    if (!fnamecreator.makeFilename(seed, d->storagePath.getCharPointer(), prefix, ".dcm", filename))
        return QR_EC_IndexDatabaseError;

    OFStandard::strlcpy(newImageFileName, filename.c_str(), newImageFileNameLen);
    
    */
    // Note: filename ist overridden in storeCallback
    return OFCondition(EC_Normal);
}

//------------------------------------------------------------------------------------------------------

OFCondition DcmQueryRetrieveSQLiteDatabaseHandle::storeRequest(const char* SOPClassUID, 
    const char* SOPInstanceUID, const char* imageFileName, DcmQueryRetrieveDatabaseStatus* status, OFBool isNew)
{
    DcmFileFormat dcmff;
    OFFilename file(imageFileName);
    if (dcmff.loadFile(imageFileName).bad())
    {
        DCMNET_ERROR("DB: Cannot open file: " << imageFileName << ": " << OFStandard::getLastSystemErrorCode().message());
        status->setStatus(STATUS_STORE_Error_CannotUnderstand);
        return (QR_EC_IndexDatabaseError);
    }

    return d->db->insertMetaData(dcmff.getDataset(), imageFileName);
}

//------------------------------------------------------------------------------------------------------

