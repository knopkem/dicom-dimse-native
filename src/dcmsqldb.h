#ifndef DCMSQLDB_H
#define DCMSQLDB_H

#include "dcmsqldef.h"

#include "dcmtk/config/osconfig.h"     /* make sure OS specific configuration is included first */
#include "dcmtk/dcmqrdb/dcmqrcnf.h"

#include <vector>
#include <list>
#include <map>

class DcmTagKey;
class DcmSQLiteDatabasePrivate;


// private field to be used to store filename of imported files
#define DCM_PrivateFileName                            DcmTagKey(0x0011, 0x0011)

class Db_Id
{
public:
    Db_Id() : level(PATIENT_LEVEL), primaryKey(0), isNew(true) {}

    DB_LEVEL level;
    OFlonglong primaryKey;
    bool isNew;
};

struct sQRes {
    OFlonglong id;
    std::list< DcmSmallDcmElm > resultList;
};


class DcmSQLiteDatabase
{
public:
    DcmSQLiteDatabase(const OFFilename& path);
    ~DcmSQLiteDatabase();

    std::list< std::list<DcmSmallDcmElm> > find(std::list<DcmSmallDcmElm> findRequestList,
        DB_LEVEL queryLevel, DB_LEVEL qLevel, DB_LEVEL lLevel) const;

    OFCondition insertMetaData(DcmDataset* dataset, const OFString& filename);

    std::vector<DB_FindAttrExt> definedAttributes() const;


protected:

    std::vector<std::string> modalitiesInStudy(long long studyReferenceId, int& seriesSearched) const;

    int numSeriesInstances(long long seriesReferenceId) const;

    bool insertDb(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList);

    Db_Id insertpat(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList);

    Db_Id insertstd(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList, Db_Id patientIdent);

    Db_Id insertser(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList, Db_Id studyIdent);

    Db_Id insertimg(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList, Db_Id seriesIdent);

    Db_Id insert(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList, Db_Id parentIdent, const DcmTagKey& primary);

    OFlonglong insertatt(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare>& keyValueList, OFlonglong id, DB_LEVEL level);

    std::string hashv(const std::map< DB_FindAttrExt, std::string, DB_FindAttrExtCompare >& keyValueList, DcmTagKey key);

    std::list< sQRes > query(const std::list<DcmSmallDcmElm>& findRequestList, DB_LEVEL queryLevel, DB_LEVEL currentLevel,
        const sQRes& preResults, std::list< std::list<DcmSmallDcmElm> >& finalResuls) const;

    bool isDateField(const DcmTagKey& key) const;
    bool isTimeField(const DcmTagKey& key) const;
    bool isDateOrTimeField(const DcmTagKey& key) const;

    std::string firstp(const std::string& value) const;
    std::string secondp(const std::string& value) const;

    DB_LEVEL tagLevel( DcmTagKey key ) const;

    bool createTables();
    bool createTable(DB_LEVEL level);
    bool createIndex(DB_LEVEL level);

private:
    DcmSQLiteDatabasePrivate* d;

};
#endif
