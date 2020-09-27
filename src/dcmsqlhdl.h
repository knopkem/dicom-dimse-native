#ifndef dcmtkDatabaseHandle_h__
#define dcmtkDatabaseHandle_h__

#include "dcmtk/config/osconfig.h"     /* make sure OS specific configuration is included first */
#include "dcmtk/dcmqrdb/dcmqrdba.h"    /* for class DcmQueryRetrieveDatabaseHandle */
#include "dcmtk/ofstd/offile.h"
#include "dcmtk/dcmqrdb/dcmqrcnf.h"

#include <list>

class DcmQueryRetrieveSQLiteDatabaseHandlePrivate;
class DcmQueryRetrieveConfig;


class DcmQueryRetriveConfigExt : public DcmQueryRetrieveConfig
{

public:
    DcmQueryRetriveConfigExt() : _permissive(false) {}
    void addPeer(const char* AETitle, const char* HostName, int PortNumber);
    void setStorageArea(const OFFilename& filename) { _storageArea = filename; }
    void setPermissiveMode(bool enabled) { _permissive = enabled;  }

    // override
    int peerForAETitle(const char* AETitle, const char** HostName, int* PortNumber) const;
    int peerInAETitle(const char* calledAETitle, const char* callingAETitle, const char* HostName) const;
    int checkForSameVendor(const char* AETitle1, const char* AETitle2) const;

    OFBool writableStorageArea(const char* aeTitle) const { return OFTrue; }
    const char* getStorageArea(const char* aeTitle) const { return _storageArea.getCharPointer(); }
private:

    struct sPeer {
        std::string aet;
        std::string hostname;
        int port;
    };
    std::list<sPeer> _peers;
    OFFilename _storageArea;
    bool _permissive;
};

class DcmQueryRetrieveSQLiteDatabaseHandleFactory : public DcmQueryRetrieveDatabaseHandleFactory
{
public:

     DcmQueryRetrieveSQLiteDatabaseHandleFactory(const DcmQueryRetrieveConfig* config);
     ~DcmQueryRetrieveSQLiteDatabaseHandleFactory() {}

     DcmQueryRetrieveDatabaseHandle* createDBHandle(
        const char* callingAETitle,
        const char* calledAETitle,
        OFCondition& result) const;

private:
    DcmQueryRetrieveSQLiteDatabaseHandleFactory(const DcmQueryRetrieveSQLiteDatabaseHandleFactory& other);
    DcmQueryRetrieveSQLiteDatabaseHandleFactory& operator=(const DcmQueryRetrieveSQLiteDatabaseHandleFactory& other);
    const DcmQueryRetrieveConfig* config_;
};


class DcmQueryRetrieveSQLiteDatabaseHandle : public DcmQueryRetrieveDatabaseHandle
{
public:

    DcmQueryRetrieveSQLiteDatabaseHandle(const OFFilename &path);

    ~DcmQueryRetrieveSQLiteDatabaseHandle();

    OFCondition makeNewStoreFileName(
        const char* SOPClassUID,
        const char* SOPInstanceUID,
        char* newImageFileName,
        size_t      newImageFileNameLen);

     OFCondition startFindRequest(
        const char *SOPClassUID,
        DcmDataset *findRequestIdentifiers,
        DcmQueryRetrieveDatabaseStatus *status);     
                
     OFCondition nextFindResponse(
        DcmDataset **findResponseIdentifiers,
        DcmQueryRetrieveDatabaseStatus *status,
        const DcmQueryRetrieveCharacterSetOptions& characterSetOptions);
   
     OFCondition cancelFindRequest(DcmQueryRetrieveDatabaseStatus *status);

     OFCondition startMoveRequest( const char *SOPClassUID, DcmDataset *moveRequestIdentifiers, 
        DcmQueryRetrieveDatabaseStatus *status);

     OFCondition nextMoveResponse(char* SOPClassUID,
        size_t SOPClassUIDSize,
        char* SOPInstanceUID,
        size_t SOPInstanceUIDSize,
        char* imageFileName,
        size_t imageFileNameSize,
        unsigned short* numberOfRemainingSubOperations,
        DcmQueryRetrieveDatabaseStatus* status);

     OFCondition cancelMoveRequest(DcmQueryRetrieveDatabaseStatus *status);

     OFCondition storeRequest( const char *SOPClassUID, const char *SOPInstanceUID, const char *imageFileName, 
        DcmQueryRetrieveDatabaseStatus  *status, OFBool isNew = OFTrue );

     OFCondition pruneInvalidRecords() { return OFCondition(EC_IllegalParameter); }

     void setIdentifierChecking(OFBool checkFind, OFBool checkMove) { }

private:

    /* not defined */ DcmQueryRetrieveSQLiteDatabaseHandle();
    /* not defined */ DcmQueryRetrieveSQLiteDatabaseHandle(const DcmQueryRetrieveSQLiteDatabaseHandle& clone);

    DcmQueryRetrieveSQLiteDatabaseHandlePrivate* d;
};

#endif
