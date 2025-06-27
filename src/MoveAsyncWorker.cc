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

    DcmXfer netTransPrefer = in.netTransferPrefer.empty() ? DcmXfer(EXS_Unknown) : DcmXfer(in.netTransferPrefer.c_str());

    // setup SCU
    OFList<OFString> syntaxes;
    this->prepareTS(netTransPrefer.getXferID(), syntaxes);
    DcmSCU scu;
    scu.setMaxReceivePDULength(ASC_DEFAULTMAXPDU);
    scu.setACSETimeout(60);
    scu.setDIMSEBlockingMode(DIMSE_BLOCKING);
    scu.setDIMSETimeout(60);
    scu.setAETitle(in.source.aet.c_str());
    scu.setPeerHostName(in.target.ip.c_str());
    scu.setPeerPort(in.target.port);
    scu.setPeerAETitle(in.target.aet.c_str());

    scu.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, syntaxes); // FIXME: this should depend on query level selected

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
    this->applyOverrideKeys(dset, overrideKeys);

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
