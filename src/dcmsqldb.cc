#include "dcmsqldb.h"

#include "dcmtk/config/osconfig.h"  /* make sure OS specific configuration is included first */
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmnet/diutil.h"

#include "sqlite3pp.h"

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <sstream>

namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}

namespace {

    static GlobalDcmDataDictionary* DICT = new GlobalDcmDataDictionary();

    std::string getTagName(const DcmTagKey& tag) {
        return DICT->rdlock().findEntry(tag, NULL)->getTagName();
    }


    bool contains(const std::string& text, const std::string& value) {
        return (text.find(value) != std::string::npos);
    }

    bool replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    template <typename Range, typename Value = typename Range::value_type>
    std::string join(Range const& elements, const char* const delimiter) {
        std::ostringstream os;
        auto b = begin(elements), e = end(elements);

        if (b != e) {
            std::copy(b, prev(e), std::ostream_iterator<Value>(os, delimiter));
            b = prev(e);
        }
        if (b != e) {
            os << *b;
        }

        return os.str();
    }

    template <typename Input, typename Output, typename Value = typename Output::value_type>
    void split(char delimiter, Output& output, Input const& input) {
        using namespace std;
        for (auto cur = begin(input), beg = cur; ; ++cur) {
            if (cur == end(input) || *cur == delimiter || !*cur) {
                output.insert(output.end(), Value(beg, cur));
                if (cur == end(input) || !*cur)
                    break;
                else
                    beg = next(cur);
            }
        }
    }
    std::string levelName(DB_LEVEL level) {
        std::string result;
        switch (level) {
        case PATIENT_LEVEL:
            result = "patient";
            break;
        case STUDY_LEVEL:
            result = "study";
            break;
        case SERIE_LEVEL:
            result = "series";
            break;
        case IMAGE_LEVEL:
            result = "image";
            break;
        default:
            DCMNET_ERROR("tableName not defined");
            break;
        }

        return result;
    }


    std::vector<DB_FindAttrExt> definedAttribs() {
        std::vector<DB_FindAttrExt> result;

        // PATIENT
        result.push_back(DB_FindAttrExt(DCM_PatientBirthDate, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientSex, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientName, PATIENT_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientID, PATIENT_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientBirthTime, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_RETIRED_OtherPatientIDs, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_OtherPatientNames, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_EthnicGroup, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientComments, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfPatientRelatedStudies, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfPatientRelatedSeries, PATIENT_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfPatientRelatedInstances, PATIENT_LEVEL, OPTIONAL_KEY));

        // STUDY
        result.push_back(DB_FindAttrExt(DCM_StudyDate, STUDY_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_StudyTime, STUDY_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_StudyID, STUDY_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_AccessionNumber, STUDY_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_ReferringPhysicianName, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_StudyDescription, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NameOfPhysiciansReadingStudy, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_StudyInstanceUID, STUDY_LEVEL, UNIQUE_KEY));
        result.push_back(DB_FindAttrExt(DCM_RETIRED_OtherStudyNumbers, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_AdmittingDiagnosesDescription, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientAge, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientSize, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientWeight, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_Occupation, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_AdditionalPatientHistory, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfStudyRelatedSeries, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfStudyRelatedInstances, STUDY_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_ModalitiesInStudy, STUDY_LEVEL, OPTIONAL_KEY));

        // SERIES
        result.push_back(DB_FindAttrExt(DCM_SeriesNumber, SERIE_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_SeriesInstanceUID, SERIE_LEVEL, UNIQUE_KEY));
        result.push_back(DB_FindAttrExt(DCM_Modality, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_SeriesDescription, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_SeriesDate, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_SeriesTime, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_BodyPartExamined, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PatientPosition, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_ProtocolName, SERIE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfSeriesRelatedInstances, SERIE_LEVEL, OPTIONAL_KEY));


        // IMAGE
        result.push_back(DB_FindAttrExt(DCM_InstanceNumber, IMAGE_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_SOPInstanceUID, IMAGE_LEVEL, UNIQUE_KEY));
        result.push_back(DB_FindAttrExt(DCM_SliceLocation, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_ImageType, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_NumberOfFrames, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_Rows, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_Columns, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_WindowWidth, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_WindowCenter, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PhotometricInterpretation, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_RescaleSlope, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_RescaleIntercept, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_PixelSpacing, IMAGE_LEVEL, OPTIONAL_KEY));
        result.push_back(DB_FindAttrExt(DCM_SOPClassUID, IMAGE_LEVEL, REQUIRED_KEY));
        result.push_back(DB_FindAttrExt(DCM_PrivateFileName, IMAGE_LEVEL, OPTIONAL_KEY));
        return result;
    }

}


class DcmSQLiteDatabasePrivate {
public:
    std::vector<DB_FindAttrExt> definedTags;
    sqlite3pp::database* db;
    bool initialized;
};

//--------------------------------------------------------------------------------------------

DcmSQLiteDatabase::DcmSQLiteDatabase(const OFFilename& path) : d(new DcmSQLiteDatabasePrivate)
{
     std::string storage(path.getCharPointer());
     storage.append("/image.db");
     d->db = new sqlite3pp::database(storage.c_str());
     d->definedTags = definedAttribs();
     d->initialized = createTables();
}

//--------------------------------------------------------------------------------------------

DcmSQLiteDatabase::~DcmSQLiteDatabase()
{
    delete d;
    d = NULL;
}

//--------------------------------------------------------------------------------------------

std::list< std::list<DcmSmallDcmElm> > DcmSQLiteDatabase::find(std::list<DcmSmallDcmElm> findRequestList,
    DB_LEVEL queryLevel, DB_LEVEL qLevel, DB_LEVEL lLevel) const
{

    std::list< std::list<DcmSmallDcmElm> > resultContainer;

    if (!d->initialized) {
        DCMNET_WARN("database not initialized");
        return resultContainer;
    }

    if (findRequestList.empty()) {
        DCMNET_WARN("request is empty!");
        return resultContainer;
    }

    sQRes queryRoot;
    queryRoot.id = 0;

    std::list< sQRes > patRes = query(findRequestList, queryLevel, PATIENT_LEVEL, queryRoot, resultContainer);
    for(auto queryPat: patRes) {
        std::list< sQRes > stdRes = query(findRequestList, queryLevel, STUDY_LEVEL, queryPat, resultContainer);
        for(auto queryStd: stdRes) {
            std::list< sQRes > serRes = query(findRequestList, queryLevel, SERIE_LEVEL, queryStd, resultContainer);
            for(auto querySer: serRes) {
                std::list< sQRes > imgRes = query(findRequestList, queryLevel, IMAGE_LEVEL, querySer, resultContainer);
                for(auto queryImg: imgRes) {
                    query(findRequestList, PATIENT_LEVEL, IMAGE_LEVEL, queryImg, resultContainer);
                }
            }
        }
    }
    return resultContainer;
}

//--------------------------------------------------------------------------------------------

std::list< sQRes > DcmSQLiteDatabase::query(const std::list<DcmSmallDcmElm> & findRequestList, 
    DB_LEVEL queryLevel, DB_LEVEL currentLevel, const sQRes & preResults, std::list< std::list<DcmSmallDcmElm> > & finalResuls) const
{
    std::list< sQRes > responseContainer;

    if (queryLevel < currentLevel) {
        finalResuls.push_back(preResults.resultList);
        return responseContainer;
    }

    std::vector <DcmTagKey> trackList;
    std::vector < std::string > selectColumns;
    std::vector < std::string > whereColumns;
    std::vector < std::string > whereBindings;
    std::vector <DcmTagKey> whereTag;


    for(auto qElm: findRequestList) {

        if (tagLevel(qElm.XTag()) != currentLevel) {
            continue;
        }

        if ( !qElm.valueField().empty() ) {

            std::string whereStr = getTagName( qElm.XTag() );
            std::string valueStr = qElm.valueField();

            if (contains(valueStr, "*") || contains(valueStr, "?") || contains(valueStr, "^") || contains(valueStr, " ")) {
                std::string whereColumnStr = whereStr + " LIKE UPPER( :" +  whereStr + " )";
                std::string testStr = valueStr;
                whereColumns.push_back(whereColumnStr);
                replace(valueStr, std::string("*"), std::string("%"));
                replace(valueStr, std::string("?"), std::string("_"));
                whereBindings.push_back(valueStr);
            }
            else if ( isDateOrTimeField(qElm.XTag()) && contains(valueStr, "-")) {
                whereColumns.push_back(whereStr + std::string(" BETWEEN :" ) + whereStr);
                whereBindings.push_back(firstp(valueStr) + " AND " + secondp(valueStr));
            }
            else {
                whereColumns.push_back(whereStr + std::string(" = :") + whereStr);
                whereBindings.push_back(valueStr);
            }
            whereTag.push_back(qElm.XTag());
        }

        selectColumns.push_back(getTagName( qElm.XTag() ));
        trackList.push_back( qElm.XTag() );
    }

    selectColumns.push_back("id");
    whereColumns.push_back("referenceId = :referenceId");
    whereBindings.push_back(std::to_string(preResults.id));
    trackList.push_back(DcmTagKey());
 
    std::string prepare = std::string("SELECT ") + join(selectColumns, " , ") + std::string(" FROM ") + levelName(currentLevel) 
        + std::string(" WHERE ") + join(whereColumns, " AND ");

    sqlite3pp::query query(*d->db, prepare.c_str());
    
    for (int i = 0; i < whereBindings.size(); ++i) {
        const std::string bindValue = whereBindings.at(i);
        if (i == whereBindings.size() -1) {
            query.bind(":referenceId", std::stoi(bindValue));
        }
        else {
            const std::string bindName = ":" + getTagName(whereTag.at(i));
            query.bind(bindName.c_str(), bindValue, sqlite3pp::copy);
            DCMNET_WARN(bindName + bindValue);
        }
    }
    DCMNET_WARN(prepare);
    for (sqlite3pp::query::iterator i = query.begin(); i != query.end(); ++i) {

        std::list<DcmSmallDcmElm> responseList;
        if (trackList.size() == selectColumns.size() ) {
            for (int j = 0; j < selectColumns.size(); j++) {
                if (j == selectColumns.size() - 1) {
                    OFlonglong value = (*i).get<OFlonglong>(j);
                    DcmSmallDcmElm newElem(trackList[j], std::to_string(value));
                    responseList.push_back(newElem);
                }
                else {
                    const char* value = (*i).get<char const*>(j);
                    DcmSmallDcmElm newElem(trackList[j], value ? value : "");
                    responseList.push_back(newElem);
                }
            }
        }
        else {
            DCMNET_ERROR("track list doesn't match selection");
        }

        sQRes res;
        res.id = std::stoi(responseList.back().valueField());
        responseList.pop_back();

        for (auto elm : preResults.resultList) {
            responseList.push_back(elm);
        }
        res.resultList = responseList;
        responseContainer.push_back(res);

    }
    return responseContainer;
}

//--------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::isDateField(const DcmTagKey& key) const
{
    DcmTag t(key);
    return t.getVR().getEVR() == EVR_DA;
}

//------------------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::isTimeField(const DcmTagKey& key) const
{
    DcmTag t(key);
    return t.getVR().getEVR() == EVR_TM;
}

//------------------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::isDateOrTimeField(const DcmTagKey& key) const
{
    DcmTag t(key);
    DcmEVR evr = t.getVR().getEVR();
    return  (evr == EVR_DA || evr == EVR_DT || evr == EVR_TM);
}

//--------------------------------------------------------------------------------------------

std::string DcmSQLiteDatabase::firstp( const std::string& value ) const
{
    std::size_t pos = value.find("-");
    if (pos == std::string::npos) {
        DCMNET_WARN("no range sign found");
        return value;
    }
    return value.substr(0, pos-1);
}

//--------------------------------------------------------------------------------------------

std::string DcmSQLiteDatabase::secondp( const std::string& value ) const
{
    std::size_t pos = value.find("-");
    if (pos == std::string::npos) {
        DCMNET_WARN("no range sign found");
        return value;
    }
    return value.substr(pos, value.length()-1);
}

//--------------------------------------------------------------------------------------------

DB_LEVEL DcmSQLiteDatabase::tagLevel( DcmTagKey key ) const
{
    for(auto item: d->definedTags) {
        if (item.tag == key)
            return item.level;
    }
    DCMNET_WARN("tag not registered");
    return PATIENT_LEVEL;
}

//--------------------------------------------------------------------------------------------

std::vector<DB_FindAttrExt> DcmSQLiteDatabase::definedAttributes() const
{
    return d->definedTags;
}

//--------------------------------------------------------------------------------------------

OFCondition DcmSQLiteDatabase::insertMetaData(DcmDataset* dataset, const OFString& filename)
{
    if (!d->initialized) {
        DCMNET_WARN("database not initialized");
        return EC_IllegalParameter;
    }

    OFCondition status = EC_Normal;

    dataset->convertToUTF8();
    DcmXfer original_xfer(dataset->getOriginalXfer());

    std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare > insertMap;

    for (int i = 0; i < d->definedTags.size(); ++i) {

        OFCondition ec = EC_Normal;
        const char* strPtr = NULL;
        Uint16 intPtr = 0;
        ec = dataset->findAndGetString(d->definedTags[i].tag, strPtr);
        if ((ec == EC_Normal) && (strPtr != NULL)) {
            DB_FindAttrExt tag = d->definedTags[i];
            insertMap[tag] = std::string(strPtr);
        }
        else {
            ec = dataset->findAndGetUint16(d->definedTags[i].tag, intPtr);
            if ((ec == EC_Normal) && (intPtr != 0)) {
                DB_FindAttrExt tag = d->definedTags[i];
                insertMap[tag] = std::to_string(intPtr);
            }
        }
    }

    // add filename to private field
    insertMap[DB_FindAttrExt(DCM_PrivateFileName, IMAGE_LEVEL, OPTIONAL_KEY)] = filename.c_str();

    if (!insertDb(insertMap)) {
        DCMNET_ERROR("Failed inserting metadata into db");
        status = EC_IllegalParameter;
    }
    return status;
}

//--------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::insertDb(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList)
{
    Db_Id patIdent = insertpat(keyValueList);
    Db_Id stdIdent = insertstd(keyValueList, patIdent);
    Db_Id serIdent = insertser(keyValueList, stdIdent);
    Db_Id imgIdent = insertimg(keyValueList, serIdent);

    if (!imgIdent.isNew) {
        DCMNET_WARN("instance already registered, ignoring");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------

Db_Id DcmSQLiteDatabase::insertpat(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList)
{
    Db_Id ident;
    ident.level = PATIENT_LEVEL;
    ident.isNew = true;

    // try to find the patient with Name + IPP
    std::string patientId = hashv(keyValueList, DCM_PatientID);
    std::string patientName = hashv(keyValueList, DCM_PatientName);

    // if patient id is empty, try to find the patient first
    if (patientId.empty()) {
        std::string prepare("SELECT " + getTagName(DCM_PatientID) + " FROM patient WHERE " + getTagName(DCM_PatientName) + "= :patName");
        sqlite3pp::query query(*d->db, prepare.c_str());
        query.bind(":patName", patientName.c_str(), sqlite3pp::nocopy);
        
        for (sqlite3pp::query::iterator i = query.begin(); i != query.end(); ++i) {
            for (int j = 0; j < query.column_count(); ++j) {
                patientId = (*i).get<const char*>(j);
                break;
            }
        }
    }

    // if still empty, create patient id
    if (patientId.empty()) {
        patientId = uuid::generate_uuid_v4();
    }

    std::string prepare("SELECT id FROM patient WHERE " + getTagName(DCM_PatientID) + "= :patId AND " + getTagName(DCM_PatientName) + "= :patName");
    sqlite3pp::query query(*d->db, prepare.c_str());

    query.bind(":patId", patientId.c_str(), sqlite3pp::nocopy);
    query.bind(":patName", patientName.c_str(), sqlite3pp::nocopy);

    for (sqlite3pp::query::iterator i = query.begin(); i != query.end(); ++i) {
        for (int j = 0; j < query.column_count(); ++j) {
            ident.primaryKey = (*i).get<OFlonglong>(j);
            ident.isNew = false;
            break;
        }
    }

    // new patient
    if (ident.isNew) {
        std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare> modifiedKeyValueList(keyValueList);
        modifiedKeyValueList[DB_FindAttrExt(DCM_PatientID, PATIENT_LEVEL, REQUIRED_KEY)] = patientId;
        ident.primaryKey = insertatt(modifiedKeyValueList, 0, PATIENT_LEVEL);
    }
    
    return ident;
}

//--------------------------------------------------------------------------------------------

Db_Id DcmSQLiteDatabase::insertstd(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList, Db_Id patientIdent)
{
    return insert(keyValueList, patientIdent, DCM_StudyInstanceUID);
}

Db_Id DcmSQLiteDatabase::insertser(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList, Db_Id studyIdent)
{
    return insert(keyValueList, studyIdent, DCM_SeriesInstanceUID);
}

//--------------------------------------------------------------------------------------------

Db_Id DcmSQLiteDatabase::insertimg(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList, Db_Id seriesIdent)
{
    return insert(keyValueList, seriesIdent, DCM_SOPInstanceUID);
}

//--------------------------------------------------------------------------------------------

Db_Id DcmSQLiteDatabase::insert(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList, Db_Id parentIdent, const DcmTagKey& primary)
{
    Db_Id ident;
    ident.level = static_cast<DB_LEVEL>(static_cast<int>(parentIdent.level) + 1);
    ident.isNew = true;

    std::string table = levelName(ident.level);

    std::string uid = hashv(keyValueList, primary);

    std::string prepare("SELECT id FROM " + table + " WHERE " + getTagName(primary) + "= :uid");

    sqlite3pp::query query(*d->db, prepare.c_str());
    query.bind(":uid", uid, sqlite3pp::nocopy);

    for (sqlite3pp::query::iterator i = query.begin(); i != query.end(); ++i) {
        for (int j = 0; j < query.column_count(); ++j) {
            ident.primaryKey = (*i).get<OFlonglong>(j);
            ident.isNew = false;
            break;
        }
    }
    if (ident.isNew) {
        ident.primaryKey = insertatt(keyValueList, parentIdent.primaryKey, ident.level);
    }
    return ident;
}

//--------------------------------------------------------------------------------------------

OFlonglong DcmSQLiteDatabase::insertatt(const std::map< DB_FindAttrExt, 
    std::string, DB_FindAttrExtCompare >& keyValueList, OFlonglong id, DB_LEVEL level)
{
    std::vector<std::string> attributs;
    std::vector<std::string> values;
    std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare>::const_iterator iter;
    for (iter = keyValueList.begin(); iter != keyValueList.end(); iter++) {
        if (iter->first.level == level) {
            std::string attr = getTagName(iter->first.tag);
            if (!attr.empty()) {
                attributs.push_back(attr);
                values.push_back(iter->second);
            }
            else {
                DCMNET_WARN("Tag not found:" << iter->first.tag.toString());
            }
        }

        if ((level == IMAGE_LEVEL) && (iter->first.tag == DCM_PrivateCreator)) {
            DCMNET_ERROR("Missing filename" << iter->second);
        }
    }

    std::string prepare = "INSERT INTO " + levelName(level) + " ( " + join(attributs, " , ")
        + ", referenceId ) VALUES ( :" + join(attributs, ", :") + ", :referenceId ) ";

    sqlite3pp::command cmd(*d->db, prepare.c_str());

    for (int u = 0; u < attributs.size(); u++) {
        cmd.bind(std::string(std::string(":") + attributs.at(u)).c_str(), values.at(u), sqlite3pp::nocopy);
    }
    cmd.bind(":referenceId", id);

    cmd.execute();
    return d->db->last_insert_rowid();
}

//--------------------------------------------------------------------------------------------


std::string DcmSQLiteDatabase::hashv(const std::map< DB_FindAttrExt, std::string, 
    DB_FindAttrExtCompare >& keyValueList, DcmTagKey key)
{
    std::string result;
    std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare>::const_iterator iter;
    iter = keyValueList.find(DB_FindAttrExt(key, PATIENT_LEVEL, UNIQUE_KEY));
    if (iter != keyValueList.end()) {
        result = iter->second;
    }
    return result;
}

//--------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::createTables()
{
    return createTable(PATIENT_LEVEL) && 
           createTable(STUDY_LEVEL) && 
           createTable(SERIE_LEVEL) && 
           createTable(IMAGE_LEVEL);
}

//--------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::createTable(DB_LEVEL level)
{
    std::string table = levelName(level);
    std::vector<std::string> list;
    list.push_back("CREATE TABLE IF NOT EXISTS " + table + "(");
    list.push_back("id INTEGER PRIMARY KEY AUTOINCREMENT,");
    list.push_back("referenceId INTEGER,");

    for (auto attr : d->definedTags) {
        if (attr.level == level) {
            std::string terminator = ",";
            if (attr.keyAttr == UNIQUE_KEY)
                terminator = "UNIQUE,";
            list.push_back(getTagName(attr.tag) + " TEXT " + terminator);
        }
    }
    list.push_back(" PlaceHoler TEXT");
    list.push_back(");");

    std::string prepare = join(list, " ");

    if (d->db->execute(prepare.c_str()) != 0) {
        DCMNET_ERROR("Failed to create table: " + table);
        return false;
    }
    return createIndex(level);
}

//--------------------------------------------------------------------------------------------

bool DcmSQLiteDatabase::createIndex(DB_LEVEL level)
{
    std::string table = levelName(level);
    std::string prepare = "CREATE INDEX IF NOT EXISTS "+ table +"Index ON " + table + "(referenceId);";
 
    if (d->db->execute(prepare.c_str()) != 0) {
        DCMNET_ERROR("Failed to create index on table: " + table);
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------


