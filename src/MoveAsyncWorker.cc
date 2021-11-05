#include "MoveAsyncWorker.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#ifdef HAVE_GUSI_H
#include <GUSI.h>
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmnet/scu.h"
#include "dcmtk/dcmnet/dicom.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcuid.h" /* for dcmtk version name */
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcpath.h"
#include "dcmtk/dcmdata/dcostrmz.h" /* for dcmZlibCompressionLevel */

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

namespace
{

void prepareTS(E_TransferSyntax ts, OFList<OFString> &syntaxes)
{
    /*
  ** We prefer to use Explicitly encoded transfer syntaxes.
  ** If we are running on a Little Endian machine we prefer
  ** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
  ** Some SCP implementations will just select the first transfer
  ** syntax they support (this is not part of the standard) so
  ** organize the proposed transfer syntaxes to take advantage
  ** of such behavior.
  **
  ** The presentation contexts proposed here are only used for
  ** C-FIND and C-MOVE, so there is no need to support compressed
  ** transmission.
  */

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
        /* We prefer explicit transfer syntaxes.
       * If we are running on a Little Endian machine we prefer
       * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
       */
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

void applyOverrideKeys(DcmDataset *dataset, OFList<OFString> overrideKeys)
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
            std::cerr << "Bad override key: " << cond.text() << std::endl;
        }
        path++;
    }
}

} // namespace

MoveAsyncWorker::MoveAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void MoveAsyncWorker::Execute(const ExecutionProgress &progress)
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

    if (in.destination.empty())
    {
        in.destination = in.source.aet;
    }

    // map attributes
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

    // setup SCU
    OFList<OFString> syntaxes;
    prepareTS(EXS_Unknown, syntaxes);
    DcmSCU scu;
    scu.setMaxReceivePDULength(ASC_DEFAULTMAXPDU);
    scu.setACSETimeout(60);
    scu.setDIMSEBlockingMode(DIMSE_BLOCKING);
    scu.setDIMSETimeout(60);
    scu.setAETitle(in.source.aet.c_str());
    scu.setPeerHostName(in.target.ip.c_str());
    scu.setPeerPort(in.target.port);
    scu.setPeerAETitle(in.target.aet.c_str());

    scu.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, syntaxes);

    /* initialize network and negotiate association */
    OFCondition cond = scu.initNetwork();
    if (cond.bad())
    {
        SetErrorJson("network initialization failed");
        return;
    }

    cond = scu.negotiateAssociation();
    if (cond.bad())
    {
        SetErrorJson("association negotiation failed");
        return;
    }

    cond = EC_Normal;
    T_ASC_PresentationContextID pcid = scu.findPresentationContextID(UID_MOVEStudyRootQueryRetrieveInformationModel, "");
    if (pcid == 0)
    {
        SetErrorJson("No adequate Presentation Contexts for sending C-MOVE");
        return;
    }

    // do the real work, i.e. send C-MOVE requests and receive response
    DcmDataset *dset = new DcmDataset;
    applyOverrideKeys(dset, overrideKeys);

    OFList<RetrieveResponse *> responses;
    cond = scu.sendMOVERequest(pcid, in.destination.c_str(), dset, &responses);
    if (cond.bad())
    {
        SetErrorJson("sending move request failed");
        return;
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

    // tear down association
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
            SetErrorJson("MOVE SCU Failed");
            scu.abortAssociation();
            return;
        }
    }
}
