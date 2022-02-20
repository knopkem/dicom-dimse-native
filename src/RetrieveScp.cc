#include "RetrieveScp.h"


#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

#ifdef HAVE_WINDOWS_H
#include <direct.h> /* for _mkdir() */
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/ofdatime.h"
#include "dcmtk/dcmnet/dicom.h" /* for DICOM_APPLICATION_ACCEPTOR */
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmnet/dcmtrans.h" /* for dcmSocketSend/ReceiveTimeout */
#include "dcmtk/dcmnet/dcasccfg.h" /* for class DcmAssociationConfiguration */
#include "dcmtk/dcmnet/dcasccff.h" /* for class DcmAssociationConfigurationFile */
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcuid.h" /* for dcmtk version name */
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcostrmz.h" /* for dcmZlibCompressionLevel */

#include "dcmtk/dcmdata/dcostrma.h"
#include "dcmtk/dcmdata/dcostrmb.h"
#include "dcmtk/dcmdata/dcwcache.h"

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

// we assume that the inetd super server is available on all non-Windows systems
#ifndef _WIN32
#define INETD_AVAILABLE
#endif

#include "base64.h"
#include <iostream>

struct StoreCallbackData
{
    char* imageFileName;
    OFString storageDir;
    DcmFileFormat* dcmff;
    T_ASC_Association* assoc;
    Napi::AsyncProgressQueueWorker<char>::ExecutionProgress* progress;
};

// ------------------------------------------------------------------------------------------------------------

void storeSCPCallback(void* callbackData, T_DIMSE_StoreProgress* progress, T_DIMSE_C_StoreRQ* req,
    char* /*imageFileName*/, DcmDataset** imageDataSet, T_DIMSE_C_StoreRSP* rsp, DcmDataset** statusDetail)
{
    DIC_UI sopClass;
    DIC_UI sopInstance;

    // if this is the final call of this function, save the data which was received to a file
    // (note that we could also save the image somewhere else, put it in database, etc.)
    if (progress->state == DIMSE_StoreEnd)
    {
        OFString tmpStr;

        // do not send status detail information
        *statusDetail = NULL;

        // remember callback data
        StoreCallbackData* cbdata = OFstatic_cast(StoreCallbackData*, callbackData);

        if ((imageDataSet != NULL) && (*imageDataSet != NULL))
        {
            OFString studyInstanceUID;
            OFString seriesInstanceUID;
            OFString sopInstanceUID;
            (*imageDataSet)->findAndGetOFString(DCM_StudyInstanceUID, studyInstanceUID);
            (*imageDataSet)->findAndGetOFString(DCM_SeriesInstanceUID, seriesInstanceUID);
            (*imageDataSet)->findAndGetOFString(DCM_SOPInstanceUID, sopInstanceUID);

            // determine the transfer syntax which shall be used to write the information to the file
            E_TransferSyntax xfer = xfer = (*imageDataSet)->getOriginalXfer();

            // if a filename is give we save the image to disk
            if (cbdata->imageFileName) {

                OFString baseStr;
                OFStandard::combineDirAndFilename(baseStr, cbdata->storageDir, studyInstanceUID, OFTrue);
                if (!OFStandard::dirExists(baseStr))
                {
                    if (OFStandard::createDirectory(baseStr, cbdata->storageDir).bad())
                    {
                        std::cerr << "failed to create directory " << baseStr.c_str() << std::endl;
                        return;
                    }
                }

                OFString fileName;
                OFStandard::combineDirAndFilename(fileName, baseStr, cbdata->imageFileName, OFTrue);

                OFCondition cond = cbdata->dcmff->saveFile(fileName.c_str(), xfer, EET_ExplicitLength, EGL_recalcGL, EPD_withoutPadding, 0, 0, EWM_fileformat);

                if (cond.bad())
                {
                    std::cerr << cond.text() << std::endl;
                    std::cerr << "cannot write DICOM file " << fileName.c_str() << std::endl;
                    rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;

                    // delete incomplete file
                    OFStandard::deleteFile(fileName);
                }
                else {
                    json v = json::object();
                    v["StudyInstanceUID"] = studyInstanceUID.c_str();
                    v["SeriesInstanceUID"] = seriesInstanceUID.c_str();
                    v["SOPInstanceUID"] = sopInstanceUID.c_str();
                    v["Filepath"] = fileName.c_str();
                    std::string msg = ns::createJsonResponse(ns::PENDING, "FILE_STORAGE", v);
                    cbdata->progress->Send(msg.c_str(), msg.length());
                }
            }
            // else we store in buffer and send as base64
            else {
                E_EncodingType encodingType = EET_ExplicitLength;

                /* open file for output */
                DcmFileFormat* dcmff = cbdata->dcmff;
                dcmff->validateMetaInfo(xfer, EWM_fileformat);
                dcmff->removeInvalidGroups();
                Uint32 length = dcmff->calcElementLength(xfer, encodingType);
                
                unsigned char* buffer;
                buffer = new unsigned char[length];

                DcmOutputBufferStream buffStream(buffer, length);

                /* check stream status */
                OFCondition cond = buffStream.status();
                if (cond.good())
                {
                    /* write data to buffer*/
                    try
                    {
                      dcmff->transferInit();
                      cond = dcmff->write(buffStream, xfer, encodingType, NULL, EGL_recalcGL, EPD_noChange, 0, 0, EWM_fileformat);
                      dcmff->transferEnd();
                    }
                    catch(const std::exception& e)
                    {
                      std::cerr << "exception: " << e.what()  << std::endl;
                    }

                    if (cond.good()) {
                        std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(buffer), length);
                        json v = json::object();
                        v["StudyInstanceUID"] = studyInstanceUID.c_str();
                        v["SeriesInstanceUID"] = seriesInstanceUID.c_str();
                        v["SOPInstanceUID"] = sopInstanceUID.c_str();
                        v["base64"] = encoded.c_str();
                        std::string msg = ns::createJsonResponse(ns::PENDING, "BUFFER_STORAGE", v);
                        cbdata->progress->Send(msg.c_str(), msg.length());
                    }
                    delete[] buffer;
                } 
                if (cond.bad()) {
                  std::cerr << cond.text() << std::endl;
                }
            }


            // check the image to make sure it is consistent, i.e. that its sopClass and sopInstance correspond
            // to those mentioned in the request. If not, set the status in the response message variable.
            if (rsp->DimseStatus == STATUS_Success)
            {
                // which SOP class and SOP instance ?
                if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sizeof(sopClass), sopInstance, sizeof(sopInstance), OFFalse))
                {
                    rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
                }
                else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
                {
                    rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
                }
                else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
                {
                    rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
                }
            }
        }
        else {
            std::cerr << "dataset is NULL" << std::endl;
        }
    }
}
    
// ------------------------------------------------------------------------------------------------------------

OFCondition RetrieveScp::echoSCP(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID presID)
{
    // assign the actual information of the C-Echo-RQ command to a local variable
    T_DIMSE_C_EchoRQ* req = &msg->msg.CEchoRQ;

    /* the echo succeeded !! */
    OFCondition cond = DIMSE_sendEchoResponse(assoc, presID, req, STATUS_Success, NULL);
    return cond;
}
    
// ------------------------------------------------------------------------------------------------------------

OFCondition RetrieveScp::storeSCP(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID presID, const OFString& outputDirectory, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ* req;
    char imageFileName[2048];

    // assign the actual information of the C-STORE-RQ command to a local variable
    req = &msg->msg.CStoreRQ;

    // format output
    sprintf(imageFileName, "%s.%s", req->AffectedSOPInstanceUID, "dcm");

    // initialize some variables
    StoreCallbackData callbackData;
    callbackData.assoc = assoc;
    callbackData.imageFileName = m_writeFile ? imageFileName : NULL;
    callbackData.storageDir = outputDirectory;
    DcmFileFormat dcmff;
    callbackData.dcmff = &dcmff;
    callbackData.progress = const_cast<Napi::AsyncProgressQueueWorker<char>::ExecutionProgress*>(&progress);

    // define an address where the information which will be received over the network will be stored
    DcmDataset* dset = dcmff.getDataset();

    cond = DIMSE_storeProvider(assoc, presID, req, NULL, OFTrue, &dset, storeSCPCallback, &callbackData, DIMSE_BLOCKING, 0);

    // if some error occurred, dump corresponding information and remove the outfile if necessary
    if (cond.bad())
    {
        std::cerr << "Store SCP failed: " << cond.text() << std::endl;

        // remove file
        if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0)
            OFStandard::deleteFile(imageFileName);
    }
    return cond;
}

// ------------------------------------------------------------------------------------------------------------

OFCondition RetrieveScp::processCommands(T_ASC_Association* assoc, const OFString& outputDirectory, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_Message msg;
    T_ASC_PresentationContextID presID = 0;
    DcmDataset* statusDetail = NULL;

    // start a loop to be able to receive more than one DIMSE command
    while (cond == EC_Normal || cond == DIMSE_NODATAAVAILABLE || cond == DIMSE_OUTOFRESOURCES)
    {
        // receive a DIMSE command over the network
        cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &presID, &msg, &statusDetail);

        // if the command which was received has extra status
        // detail information, dump this information
        if (statusDetail != NULL)
        {
            // OFLOG_DEBUG(storescpLogger, "Status Detail:" << OFendl << DcmObject::PrintHelper(*statusDetail));
            delete statusDetail;
        }

        // check if peer did release or abort, or if we have a valid message
        if (cond == EC_Normal)
        {
            // in case we received a valid message, process this command
            // note that storescp can only process a C-ECHO-RQ and a C-STORE-RQ
            switch (msg.CommandField)
            {
            case DIMSE_C_ECHO_RQ:
                // process C-ECHO-Request
                cond = echoSCP(assoc, &msg, presID);
                break;
            case DIMSE_C_STORE_RQ:
                // process C-STORE-Request
                cond = storeSCP(assoc, &msg, presID, outputDirectory, progress);
                break;
            default:
                OFString tempStr;
                // we cannot handle this kind of message
                cond = DIMSE_BADCOMMANDTYPE;
                std::cerr << "unsupported DIMSE command received" << std::endl;
                break;
            }
        }
    }
    return cond;
}

// ------------------------------------------------------------------------------------------------------------

OFCondition RetrieveScp::acceptAssociation(T_ASC_Network* net, DcmAssociationConfiguration& asccfg, OFBool secureConnection, const OFString& outputDirectory, const OFString& aet, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress)
{
    char buf[BUFSIZ];
    T_ASC_Association* assoc;
    OFCondition cond;
    OFString temp_str;

    const char* knownAbstractSyntaxes[] =
    {
        UID_VerificationSOPClass };

    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 10
                                        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 20
                                        NULL };                                                      // +1
    int numTransferSyntaxes = 0;

    cond = ASC_receiveAssociation(net, &assoc, ASC_DEFAULTMAXPDU, NULL, NULL, secureConnection);

    // if some kind of error occurred, take care of it
    if (cond.bad())
    {
        // check what kind of error occurred. If no association was
        // received, check if certain other conditions are met
        if (cond == DUL_NOASSOCIATIONREQUEST)
        {
        }
        // If something else was wrong we might have to dump an error message.
        else
        {
            std::cerr << "Receiving Assocition failed: " << cond.text() << std::endl;
        }

        // no matter what kind of error occurred, we need to do a cleanup
        goto cleanup;
    }

    // accept all ts
    transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
    transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
    transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
    transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
    transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;
    transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
    transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
    transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
    transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
    transferSyntaxes[9] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
    transferSyntaxes[10] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
    transferSyntaxes[11] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
    transferSyntaxes[12] = UID_MPEG4HighProfileLevel4_2_For2DVideoTransferSyntax;
    transferSyntaxes[13] = UID_MPEG4HighProfileLevel4_2_For3DVideoTransferSyntax;
    transferSyntaxes[14] = UID_MPEG4StereoHighProfileLevel4_2TransferSyntax;
    transferSyntaxes[15] = UID_HEVCMainProfileLevel5_1TransferSyntax;
    transferSyntaxes[16] = UID_HEVCMain10ProfileLevel5_1TransferSyntax;
    transferSyntaxes[17] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;

    if (gLocalByteOrder == EBO_LittleEndian)
    {
        transferSyntaxes[18] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[19] = UID_BigEndianExplicitTransferSyntax;
    }
    else {
        transferSyntaxes[18] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[19] = UID_LittleEndianExplicitTransferSyntax;
    }
    transferSyntaxes[20] = UID_LittleEndianImplicitTransferSyntax;
    numTransferSyntaxes = 21;

    {
        /* accept the Verification SOP Class if presented */
        cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params, knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes), transferSyntaxes, numTransferSyntaxes);
        if (cond.bad())
        {
            goto cleanup;
        }

        /* the array of Storage SOP Class UIDs comes from dcuid.h */
        cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params, dcmAllStorageSOPClassUIDs, numberOfDcmAllStorageSOPClassUIDs, transferSyntaxes, numTransferSyntaxes);
        if (cond.bad())
        {
            goto cleanup;
        }
    }

    /* set our app title */
    ASC_setAPTitles(assoc->params, NULL, NULL, aet.c_str());

    /* acknowledge or reject this association */
    cond = ASC_getApplicationContextName(assoc->params, buf, sizeof(buf));
    if ((cond.bad()) || strcmp(buf, UID_StandardApplicationContext) != 0)
    {
        /* reject: the application context name is not supported */
        T_ASC_RejectParameters rej =
        {
            ASC_RESULT_REJECTEDPERMANENT,
            ASC_SOURCE_SERVICEUSER,
            ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED };

        std::cerr << "Association Rejected: Bad Application Context Name:" << buf << std::endl;

        cond = ASC_rejectAssociation(assoc, &rej);
        if (cond.bad())
        {
            std::cerr << cond.text() << std::endl;
        }
        goto cleanup;
    }
    else
    {
        cond = ASC_acknowledgeAssociation(assoc);
        if (cond.bad())
        {
            std::cerr << cond.text() << std::endl;
            goto cleanup;
        }
    }

    /* now do the real work, i.e. receive DIMSE commands over the network connection */
    /* which was established and handle these commands correspondingly. In case of */
    /* storescp only C-ECHO-RQ and C-STORE-RQ commands can be processed. */
    cond = processCommands(assoc, outputDirectory, progress);

    if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        cond = ASC_acknowledgeRelease(assoc);
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
        std::cerr << "Peer aborted association" << std::endl;
    }
    else
    {
        /* some kind of error so abort the association */
        std::cerr << "DIMSE failure (aborting association): " << cond.text() << std::endl;

        cond = ASC_abortAssociation(assoc);
    }

cleanup:
    cond = ASC_dropSCPAssociation(assoc);
    if (cond.bad())
    {
        std::cerr << cond.text() << std::endl;
        return cond;
    }
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad())
    {
        std::cerr << cond.text() << std::endl;
    }

    return cond;
}


OFCondition RetrieveScp::waitForAssociation(T_ASC_Network* theNet, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress)
{
    return acceptAssociation(theNet, asccfg, false, m_outputDirectory, m_aet, progress);
}