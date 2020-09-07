#include "EchoAsyncWorker.h"

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
    UID_JPEGProcess1TransferSyntax,
    UID_JPEGProcess2_4TransferSyntax,
    UID_JPEGProcess3_5TransferSyntax,
    UID_JPEGProcess6_8TransferSyntax,
    UID_JPEGProcess7_9TransferSyntax,
    UID_JPEGProcess10_12TransferSyntax,
    UID_JPEGProcess11_13TransferSyntax,
    UID_JPEGProcess14TransferSyntax,
    UID_JPEGProcess15TransferSyntax,
    UID_JPEGProcess16_18TransferSyntax,
    UID_JPEGProcess17_19TransferSyntax,
    UID_JPEGProcess20_22TransferSyntax,
    UID_JPEGProcess21_23TransferSyntax,
    UID_JPEGProcess24_26TransferSyntax,
    UID_JPEGProcess25_27TransferSyntax,
    UID_JPEGProcess28TransferSyntax,
    UID_JPEGProcess29TransferSyntax,
    UID_JPEGProcess14SV1TransferSyntax,
    UID_RLELosslessTransferSyntax,
    UID_DeflatedExplicitVRLittleEndianTransferSyntax,
    UID_JPEGLSLosslessTransferSyntax,
    UID_JPEGLSLossyTransferSyntax,
    UID_JPEG2000LosslessOnlyTransferSyntax,
    UID_JPEG2000TransferSyntax,
    UID_JPEG2000Part2MulticomponentImageCompressionLosslessOnlyTransferSyntax,
    UID_JPEG2000Part2MulticomponentImageCompressionTransferSyntax,
    UID_MPEG2MainProfileAtMainLevelTransferSyntax,
    UID_MPEG2MainProfileAtHighLevelTransferSyntax,
    UID_MPEG4HighProfileLevel4_1TransferSyntax,
    UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax,
    UID_MPEG4HighProfileLevel4_2_For2DVideoTransferSyntax,
    UID_MPEG4HighProfileLevel4_2_For3DVideoTransferSyntax,
    UID_MPEG4StereoHighProfileLevel4_2TransferSyntax,
    UID_HEVCMainProfileLevel5_1TransferSyntax,
    UID_HEVCMain10ProfileLevel5_1TransferSyntax};

OFCondition echoSCU(T_ASC_Association *assoc)
/*
     * This function will send a C-ECHO-RQ over the network to another DICOM application
     * and handle the response.
     *
     * Parameters:
     *   assoc - [in] The association (network connection to another DICOM application).
     */
{
    DIC_US msgId = assoc->nextMsgID++;
    DIC_US status;
    DcmDataset *statusDetail = NULL;

    /* dump information if required */
    // OFLOG_INFO(echoscuLogger, "Sending Echo Request (MsgID " << msgId << ")");

    /* send C-ECHO-RQ and handle response */
    OFCondition cond = DIMSE_echoUser(assoc, msgId, DIMSE_BLOCKING, 0, &status, &statusDetail);

    /* depending on if a response was received, dump some information */
    if (cond.good())
    {
        // OFLOG_INFO(echoscuLogger, "Received Echo Response (" << DU_cechoStatusString(status) << ")");
    }
    else
    {
        // OFString temp_str;
        // OFLOG_ERROR(echoscuLogger, "Echo Failed: " << DimseCondition::dump(temp_str, cond));
    }

    /* return result value */
    return cond;
}

OFCondition cecho(T_ASC_Association *assoc, unsigned long num_repeat)
/*
     * This function will send num_repeat C-ECHO-RQ messages to the DICOM application
     * this application is connected with and handle corresponding C-ECHO-RSP messages.
     *
     * Parameters:
     *   assoc      - [in] The association (network connection to another DICOM application).
     *   num_repeat - [in] The amount of C-ECHO-RQ messages which shall be sent.
     */
{
    OFCondition cond = EC_Normal;
    unsigned long n = num_repeat;

    /* as long as no error occurred and the counter does not equal 0 */
    /* send an C-ECHO-RQ and handle the response */
    while (cond.good() && n--)
        cond = echoSCU(assoc);

    return cond;
}

} // namespace

EchoAsyncWorker::EchoAsyncWorker(std::string data, Function &callback)
    : BaseAsyncWorker(data, callback) {}

void EchoAsyncWorker::Execute(const ExecutionProgress &progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    EnableVerboseLogging(in.verbose);

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
    OFCmdUnsignedInt opt_port = std::stoi(in.target.port);
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
        cond = ASC_addPresentationContext(params, presentationContextID, UID_VerificationSOPClass,
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

    /* count the presentation contexts which have been accepted by the SCP */
    /* If there are none, finish the execution */
    if (ASC_countAcceptedPresentationContexts(params) == 0)
    {
        SetErrorJson(std::string("No Acceptable Presentation Contexts"));
        return;
    }

    /* do the real work, i.e. send a number of C-ECHO-RQ messages to the DICOM application */
    /* this application is connected with and handle corresponding C-ECHO-RSP messages. */
    cond = cecho(assoc, opt_repeatCount);

    /* tear down association, i.e. terminate network connection to SCP */
    if (cond == EC_Normal)
    {
        if (opt_abortAssociation)
        {
            cond = ASC_abortAssociation(assoc);
            if (cond.bad())
            {
                SetErrorJson(std::string("Association Abort Failed: ") + std::string(cond.text()));
                return;
            }
        }
        else
        {
            /* release association */
            cond = ASC_releaseAssociation(assoc);
            if (cond.bad())
            {
                SetErrorJson(std::string("Association Release Failed: ") + std::string(cond.text()));
                return;
            }
        }
    }
    else if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        // OFLOG_FATAL(echoscuLogger, "Protocol Error: Peer requested release (Aborting)");
        // OFLOG_INFO(echoscuLogger, "Aborting Association");
        cond = ASC_abortAssociation(assoc);
        if (cond.bad())
        {
            SetErrorJson(std::string("Association Abort Failed: ") + std::string(cond.text()));
            return;
        }
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
        // OFLOG_INFO(echoscuLogger, "Peer Aborted Association");
    }
    else
    {
        // OFLOG_ERROR(echoscuLogger, "Echo SCU Failed: " << DimseCondition::dump(temp_str, cond));
        // OFLOG_INFO(echoscuLogger, "Aborting Association");
        cond = ASC_abortAssociation(assoc);
        if (cond.bad())
        {
            SetErrorJson(std::string("Association Abort Failed: ") + std::string(cond.text()));
            return;
        }
    }

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
