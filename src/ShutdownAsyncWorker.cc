#include "ShutdownAsyncWorker.h"

#include <iostream>
#include <list>
#include <memory>
#include <sstream>

#include "Utils.h"
#include "json.h"

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcuid.h"   /* for dcmtk version name */
#include "dcmtk/dcmnet/dcmtrans.h" /* for dcmSocketSend/ReceiveTimeout */
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/ofstd/ofconapp.h"

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

namespace
{

/* DICOM standard transfer syntaxes */
const char *transferSyntaxes[] = {
    UID_LittleEndianImplicitTransferSyntax, /* default xfer syntax first */
    UID_LittleEndianExplicitTransferSyntax,
    UID_BigEndianExplicitTransferSyntax,
 };


} // namespace

ShutdownAsyncWorker::ShutdownAsyncWorker(std::string data, Function &callback)
    : BaseAsyncWorker(data, callback) {}

void ShutdownAsyncWorker::Execute(const ExecutionProgress &progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    EnableVerboseLogging(in.verbose, progress);

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

    OFOStringStream optStream;
    int result = EXITCODE_NO_ERROR;

    const char *opt_peer = in.target.ip.c_str();
    OFCmdUnsignedInt opt_port = in.target.port;
    const char *opt_peerTitle = in.target.aet.c_str();
    const char *opt_ourTitle = in.source.aet.c_str();

    OFCmdUnsignedInt opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
    OFCmdUnsignedInt opt_repeatCount = 1;
    OFBool opt_abortAssociation = OFFalse;
    OFCmdUnsignedInt opt_numXferSyntaxes = 1;
    OFCmdUnsignedInt opt_numPresentationCtx = 1;
    OFCmdUnsignedInt maxXferSyntaxes =
        OFstatic_cast(OFCmdUnsignedInt, (DIM_OF(transferSyntaxes)));
    int opt_acse_timeout = 30;

    T_ASC_Network *net;
    T_ASC_Parameters *params;
    DIC_NODENAME peerHost;
    T_ASC_Association *assoc;
    OFString temp_str;

    OFStandard::initializeNetwork();

    /* initialize network, i.e. create an instance of T_ASC_Network*. */
    OFCondition cond = ASC_initializeNetwork(NET_REQUESTOR, 0, opt_acse_timeout, &net);
    if (cond.bad())
    {
        SetErrorJson(cond.text());
        return;
    }

    /* initialize association parameters, i.e. create an instance of T_ASC_Parameters*. */
    cond = ASC_createAssociationParameters(&params, opt_maxReceivePDULength);
    if (cond.bad())
    {
        SetErrorJson(cond.text());
        return;
    }

    /* sets this application's title and the called application's title in the params */
    /* structure. The default values to be set here are "STORESCU" and "ANY-SCP". */
    ASC_setAPTitles(params, opt_ourTitle, opt_peerTitle, NULL);

    /* Figure out the presentation addresses and copy the */
    /* corresponding values into the association parameters.*/
    sprintf(peerHost, "%s:%d", opt_peer, OFstatic_cast(int, opt_port));
    ASC_setPresentationAddresses(params, OFStandard::getHostName().c_str(), peerHost);

    /* Set the presentation contexts which will be negotiated */
    /* when the network connection will be established */
    int presentationContextID = 1; /* odd byte value 1, 3, 5, .. 255 */
    for (unsigned long ii = 0; ii < opt_numPresentationCtx; ii++)
    {
        cond = ASC_addPresentationContext(params, presentationContextID, UID_PrivateShutdownSOPClass,
                                          transferSyntaxes, OFstatic_cast(int, opt_numXferSyntaxes));
        presentationContextID += 2;
        if (cond.bad())
        {
            SetErrorJson(cond.text());
            return;
        }
    }

    /* dump presentation contexts if required */
    SendInfo(std::string("Request Parameters: ") + std::string(ASC_dumpParameters(temp_str, params, ASC_ASSOC_RQ).c_str()), progress, ns::PENDING);

    /* create association, i.e. try to establish a network connection to another */
    /* DICOM application. This call creates an instance of T_ASC_Association*. */
    SendInfo(std::string("Requesting Association"), progress, ns::PENDING);
    cond = ASC_requestAssociation(net, params, &assoc);
    if (cond.bad())
    {
        if (cond == DUL_ASSOCIATIONREJECTED)
        {
            T_ASC_RejectParameters rej;
            ASC_getRejectParameters(params, &rej);
            SetErrorJson(std::string("Association Rejected: ") + std::string(ASC_printRejectParameters(temp_str, &rej).c_str()));
            return;
        }
        else
        {
            SetErrorJson(std::string("Association Request failed: ") + std::string(cond.text()));
            return;
        }
    }

    /* dump the presentation contexts which have been accepted/refused */
    SendInfo(std::string("Association Parameters Negotiated: ") + std::string(ASC_dumpParameters(temp_str, params, ASC_ASSOC_RQ).c_str()), progress, ns::PENDING);
    
    /* destroy the association, i.e. free memory of T_ASC_Association* structure. This */
    /* call is the counterpart of ASC_requestAssociation(...) which was called above. */
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad())
    {
        SetErrorJson(std::string(cond.text()));
        return;
    }

    /* drop the network, i.e. free memory of T_ASC_Network* structure. This call */
    /* is the counterpart of ASC_initializeNetwork(...) which was called above. */
    cond = ASC_dropNetwork(&net);
    if (cond.bad())
    {
        SetErrorJson(std::string(cond.text()));
        return;
    }

    OFStandard::shutdownNetwork();
}
