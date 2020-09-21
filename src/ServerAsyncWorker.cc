#include "ServerAsyncWorker.h"

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

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

// we assume that the inetd super server is available on all non-Windows systems
#ifndef _WIN32
#define INETD_AVAILABLE
#endif



namespace
{

struct StoreCallbackData
{
  char *imageFileName;
  OFString storageDir;
  DcmFileFormat *dcmff;
  T_ASC_Association *assoc;
};

// ------------------------------------------------------------------------------------------------------------

OFCondition echoSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID)
{
  // assign the actual information of the C-Echo-RQ command to a local variable
  T_DIMSE_C_EchoRQ *req = &msg->msg.CEchoRQ;

  /* the echo succeeded !! */
  OFCondition cond = DIMSE_sendEchoResponse(assoc, presID, req, STATUS_Success, NULL);
  return cond;
}

// ------------------------------------------------------------------------------------------------------------

void storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *req,
                      char * /*imageFileName*/, DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *rsp, DcmDataset **statusDetail)
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
    StoreCallbackData *cbdata = OFstatic_cast(StoreCallbackData *, callbackData);

    // compose directory and filename structure
    OFString studyInstanceUID;
    (*imageDataSet)->findAndGetOFString(DCM_StudyInstanceUID, studyInstanceUID);

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

    if ((imageDataSet != NULL) && (*imageDataSet != NULL))
    {

      // determine the transfer syntax which shall be used to write the information to the file
      E_TransferSyntax xfer = xfer = (*imageDataSet)->getOriginalXfer();

      OFCondition cond = cbdata->dcmff->saveFile(fileName.c_str(), xfer, EET_ExplicitLength, EGL_recalcGL,
                                                 EPD_withoutPadding, 0, 0, EWM_fileformat);
      if (cond.bad())
      {
        std::cerr << "cannot write DICOM file " << fileName.c_str() << std::endl;
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;

        // delete incomplete file
        OFStandard::deleteFile(fileName);
      }

      // check the image to make sure it is consistent, i.e. that its sopClass and sopInstance correspond
      // to those mentioned in the request. If not, set the status in the response message variable.
      if (rsp->DimseStatus == STATUS_Success)
      {
        // which SOP class and SOP instance ?
        if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sizeof(sopClass), sopInstance, sizeof(sopInstance), OFFalse))
        {
          std::cerr << "bad DICOM file " << fileName.c_str() << std::endl;
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
  }
}

OFCondition storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID, const OFString &outputDirectory)
{
  OFCondition cond = EC_Normal;
  T_DIMSE_C_StoreRQ *req;
  char imageFileName[2048];

  // assign the actual information of the C-STORE-RQ command to a local variable
  req = &msg->msg.CStoreRQ;

  // format output
  sprintf(imageFileName, "%s.%s", req->AffectedSOPInstanceUID, "dcm");

  // initialize some variables
  StoreCallbackData callbackData;
  callbackData.assoc = assoc;
  callbackData.imageFileName = imageFileName;
  callbackData.storageDir = outputDirectory;
  DcmFileFormat dcmff;
  callbackData.dcmff = &dcmff;

  // define an address where the information which will be received over the network will be stored
  DcmDataset *dset = dcmff.getDataset();

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

OFCondition processCommands(T_ASC_Association *assoc, const OFString &outputDirectory)
{
  OFCondition cond = EC_Normal;
  T_DIMSE_Message msg;
  T_ASC_PresentationContextID presID = 0;
  DcmDataset *statusDetail = NULL;

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
        cond = storeSCP(assoc, &msg, presID, outputDirectory);
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

OFCondition acceptAssociation(T_ASC_Network *net, DcmAssociationConfiguration &asccfg, OFBool secureConnection, const OFString &outputDirectory, const OFString &aet, const OFString &ts)
{
  char buf[BUFSIZ];
  T_ASC_Association *assoc;
  OFCondition cond;
  OFString temp_str;

  const char *knownAbstractSyntaxes[] =
      {
          UID_VerificationSOPClass};

  const char *transferSyntaxes[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 10
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 20
                                    NULL};                                                      // +1
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
  } else {
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
            ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED};

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
  cond = processCommands(assoc, outputDirectory);

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

} // end namespace

ServerAsyncWorker::ServerAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
    ns::registerCodecs();
}

void ServerAsyncWorker::Execute(const ExecutionProgress &progress)
{
  ns::sInput in = ns::parseInputJson(_input);

  EnableVerboseLogging(in.verbose);

  if (!in.source.valid())
  {
    SetErrorJson("Source not set");
    return;
  }

  if (in.storagePath.empty())
  {
    in.storagePath = "./data";
    SendInfo("storage path not set, defaulting to " + in.storagePath, progress);
  }

  int opt_port = std::stoi(in.source.port);
  OFString callingAETitle = OFString(in.source.aet.c_str());
  const char *opt_respondingAETitle = in.source.aet.c_str();
  static OFString opt_outputDirectory = OFString(in.storagePath.c_str());

  T_ASC_Network *net;
  DcmAssociationConfiguration asccfg;

  OFStandard::initializeNetwork();

  /* make sure data dictionary is loaded */
  if (!dcmDataDict.isDictionaryLoaded())
  {
    SetErrorJson(std::string("No data dictionary loaded"));
  }

  /* if the output directory does not equal "." (default directory) */
  if (opt_outputDirectory != ".")
  {
    /* if there is a path separator at the end of the path, get rid of it */
    OFStandard::normalizeDirName(opt_outputDirectory, opt_outputDirectory);

    /* check if the specified directory exists and if it is a directory.
     * If the output directory is invalid, dump an error message and terminate execution.
     */
    if (!OFStandard::dirExists(opt_outputDirectory))
    {
      SetErrorJson(std::string("Specified output directory does not exist: ") + std::string(opt_outputDirectory.c_str()));
      return;
    }
  }

  /* check if the output directory is writeable */
  if (!OFStandard::isWriteable(opt_outputDirectory))
  {
    SetErrorJson(std::string("Specified output directory is not writeable: ") + std::string(opt_outputDirectory.c_str()));
    return;
  }

  /* initialize network, i.e. create an instance of T_ASC_Network*. */
  OFCondition cond = ASC_initializeNetwork(NET_ACCEPTOR, opt_port, 30, &net);
  if (cond.bad())
  {
    SetErrorJson(std::string("Cannot create network: ") + std::string(cond.text()));
    return;
  }

  /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
  if (OFStandard::dropPrivileges().bad())
  {
    // OFLOG_FATAL(storescpLogger, "setuid() failed, maximum number of processes/threads for uid already running.");
    SetErrorJson(std::string("setuid() failed, maximum number of threads for uid already running"));
    return;
  }

  while (cond.good())
  {
    /* receive an association and acknowledge or reject it. If the association was */
    /* acknowledged, offer corresponding services and invoke one or more if required. */
    cond = acceptAssociation(net, asccfg, false, opt_outputDirectory, OFString(in.source.aet.c_str()), "");
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
