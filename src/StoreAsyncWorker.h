#pragma once

#include "BaseAsyncWorker.h"


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/ofstd/offile.h"


using namespace Napi;

class DcmDataset;
class DcmItem;


class StoreAsyncWorker : public BaseAsyncWorker
{
    public:
        StoreAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

    protected:
        bool setScanDirectory(const OFFilename &dir);

        bool sendStoreRequest(const OFString& peerTitle, const OFString& peerIP, Uint16 peerPort, 
            const OFString& ourTitle, const OFString& ourIP, Uint16 ourPort);

        OFCondition storeSCU(T_ASC_Association *assoc, const OFFilename &fname);

        OFBool findSOPClassAndInstanceInFile(const OFFilename &fname, char *sopClass, char *sopInstance);

        OFBool isaListMember(OFList<OFString> &lst, OFString &s);

        OFCondition addPresentationContext(T_ASC_Parameters *params, int presentationContextId,
                                                    const OFString &abstractSyntax, const OFString &transferSyntax,
                                                    T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT);

        OFCondition addPresentationContext(T_ASC_Parameters *params, int presentationContextId,
                                                    const OFString &abstractSyntax, const OFList<OFString> &transferSyntaxList,
                                                    T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT);

        OFCondition addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString> &sopClasses);


        static void progressCallback(void * /*callbackData*/, T_DIMSE_StoreProgress *progress,
                            T_DIMSE_C_StoreRQ * req);


        OFBool isDicomFile(const OFFilename &fname);

private:

        OFFilename            m_sourceDirectory;
        E_FileReadMode        m_readMode;
        OFBool                m_proposeOnlyRequiredPresentationContexts;
        OFCondition           m_lastCondition;
        E_TransferSyntax      m_networkTransferSyntax;
        unsigned long         m_acse_timeout;
        unsigned long         m_dimse_timeout;

};
