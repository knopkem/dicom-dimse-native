#include "GetAsyncWorker.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/oflist.h"
#include "dcmtk/dcmnet/scu.h"
#include "dcmtk/dcmdata/dcuid.h"    /* for dcmtk version name */
#include "dcmtk/dcmdata/dcpath.h"   /* for DcmPathProcessor */
// #include "dcmtk/dcmdata/dcostrmz.h" /* for dcmZlibCompressionLevel */

/*
#ifdef WITH_ZLIB
#include <zlib.h>
#endif
*/

namespace
{
    typedef enum
    {
        QMPatientRoot = 0,
        QMStudyRoot = 1,
        QMPatientStudyOnly = 2
    } QueryModel;

    const char *querySyntax[3] = {
        UID_GETPatientRootQueryRetrieveInformationModel,
        UID_GETStudyRootQueryRetrieveInformationModel,
        UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel};

    void applyOverrideKeys(DcmDataset *dataset, const OFList<OFString> &overrideKeys)
    {
        /* replace specific keys by those in overrideKeys */
        OFListConstIterator(OFString) path = overrideKeys.begin();
        OFListConstIterator(OFString) endOfList = overrideKeys.end();
        DcmPathProcessor proc;
        proc.setItemWildcardSupport(OFFalse);
        proc.checkPrivateReservations(OFFalse);
        OFCondition cond;
        while (path != endOfList)
        {
            cond = proc.applyPathWithValue(dataset, *path);
            if (cond.bad())
            {
                std::cerr << "bad override key provided" << std::endl;
            }
            path++;
        }
    }

    void prepareTS(E_TransferSyntax ts,  OFList<OFString> &syntaxes)
    {
        switch (ts)
        {
        case EXS_LittleEndianImplicit:
            /* we only support Little Endian Implicit */
            syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
            break;
        case EXS_LittleEndianExplicit:
            /* we prefer Little Endian Explicit */
            syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
            break;
        case EXS_BigEndianExplicit:
            /* we prefer Big Endian Explicit */
            syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
            break;
    #ifdef WITH_ZLIB
        case EXS_DeflatedLittleEndianExplicit:
            /* we prefer Deflated Little Endian Explicit */
            syntaxes.push_back(UID_DeflatedExplicitVRLittleEndianTransferSyntax);
            syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
            break;
    #endif
        default:
            DcmXfer xfer(ts);
            if (xfer.isEncapsulated())
            {
                syntaxes.push_back(xfer.getXferID());
            }
            if (gLocalByteOrder == EBO_LittleEndian) /* defined in dcxfer.h */
            {
                syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
                syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
            }
            else
            {
                syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
                syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
            }
            syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
            break;
        }
    }

    class NanNotifier : public DcmNotifier 
    {
        public:

            NanNotifier(const AsyncProgressWorker<char>::ExecutionProgress& progress): _progress(progress) {

            }
            inline void sendMessage(const OFString& msg, const OFString& container) {
                std::string msg2 = ns::createJsonResponse(ns::PENDING, msg.c_str());
                _progress.Send(msg2.c_str(), msg2.length());
            }
        private:
        AsyncProgressWorker<char>::ExecutionProgress _progress;

    };
} // namespace

GetAsyncWorker::GetAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void GetAsyncWorker::Execute(const ExecutionProgress &progress)
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

    if (in.destination.empty())
    {
        in.destination = in.source.aet;
    }

    if (in.storagePath.empty())
    {
        in.storagePath = "./data";
        SendInfo("storage path not set, defaulting to " + in.storagePath, progress);
    }

    OFCmdUnsignedInt opt_maxPDU = ASC_DEFAULTMAXPDU;
    E_TransferSyntax opt_store_networkTransferSyntax = EXS_JPEGLSLossless;
    E_TransferSyntax opt_get_networkTransferSyntax = EXS_Unknown;
    DcmStorageMode opt_storageMode = DCMSCU_STORAGE_DISK;
    OFBool opt_showPresentationContexts = OFFalse;
    OFBool opt_abortAssociation = OFFalse;
    OFCmdUnsignedInt opt_repeatCount = 1;
    QueryModel opt_queryModel = QMPatientRoot;
    T_DIMSE_BlockingMode opt_blockMode = DIMSE_BLOCKING;
    int opt_dimse_timeout = 0;
    int opt_acse_timeout = 30;
    OFString opt_outputDirectory = in.storagePath.c_str();

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


    /* setup SCU */
    OFList<OFString> syntaxes;
    prepareTS(opt_get_networkTransferSyntax, syntaxes);
    NanNotifier notifier(progress);
    DcmSCU scu;
    scu.setNotifier(&notifier);
    scu.setMaxReceivePDULength(opt_maxPDU);
    scu.setACSETimeout(opt_acse_timeout);
    scu.setDIMSEBlockingMode(opt_blockMode);
    scu.setDIMSETimeout(opt_dimse_timeout);
    scu.setAETitle(in.source.aet.c_str());
    scu.setPeerHostName(in.target.ip.c_str());
    scu.setPeerPort(OFstatic_cast(Uint16, std::stoi(in.target.port)));
    scu.setPeerAETitle(in.source.aet.c_str());
    scu.setVerbosePCMode(opt_showPresentationContexts);

    /* add presentation contexts for get and find (we do not actually need find...)
   * (only uncompressed)
   */
    scu.addPresentationContext(querySyntax[opt_queryModel], syntaxes);

    /* add storage presentation contexts (long list of storage SOP classes, uncompressed) */
    syntaxes.clear();
    prepareTS(opt_store_networkTransferSyntax, syntaxes);
    for (Uint16 j = 0; j < numberOfDcmLongSCUStorageSOPClassUIDs; j++)
    {
        scu.addPresentationContext(dcmLongSCUStorageSOPClassUIDs[j], syntaxes, ASC_SC_ROLE_SCP);
    }

    /* set the storage mode */
    scu.setStorageMode(opt_storageMode);
    if (opt_storageMode != DCMSCU_STORAGE_IGNORE)
    {
        scu.setStorageDir(opt_outputDirectory);
    }

    /* initialize network and negotiate association */
    OFCondition cond = scu.initNetwork();
    if (cond.bad())
    {
        SetErrorJson(cond.text());
        return;
    }
    cond = scu.negotiateAssociation();
    if (cond.bad())
    {
        SetErrorJson(std::string("Could not negotiate association: ") + std::string(cond.text()));
        return;
    }
    cond = EC_Normal;
    T_ASC_PresentationContextID pcid = scu.findPresentationContextID(querySyntax[opt_queryModel], "");
    if (pcid == 0)
    {
        SetErrorJson("No adequate Presentation Contexts for sending C-GET");
        return;
    }

    /* do the real work, i.e. send C-GET requests and receive objects */
    DcmFileFormat dcmff;
    DcmDataset *dset = dcmff.getDataset();

    OFList<RetrieveResponse *> responses;
    /* for all files (or at least one run from override keys) */
    applyOverrideKeys(dset, overrideKeys);
    cond = scu.sendCGETRequest(pcid, dset, &responses);
    if (cond.bad())
    {
        SetErrorJson(std::string("C-Get request failed") + std::string(cond.text()));
    }

    if (!responses.empty())
    {
        /* delete responses */
        OFListIterator(RetrieveResponse *) iter = responses.begin();
        OFListConstIterator(RetrieveResponse *) last = responses.end();
        while (iter != last)
        {
            delete (*iter);
            iter = responses.erase(iter);
        }
    }

    /* tear down association */
    if (cond == EC_Normal)
    {
        scu.releaseAssociation();
    }
    else
    {
        if (cond == DUL_PEERREQUESTEDRELEASE)
        {
            scu.closeAssociation(DCMSCU_PEER_REQUESTED_RELEASE);
        }
        else if (cond == DUL_PEERABORTEDASSOCIATION)
        {
            scu.closeAssociation(DCMSCU_PEER_ABORTED_ASSOCIATION);
        }
        else
        {
            SetErrorJson(std::string("Get SCU Failed: ") + std::string(cond.text()));
            scu.abortAssociation();
        }
    }
}
