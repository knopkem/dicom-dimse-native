#include "ServerAsyncWorker.h"

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
#include <zlib.h> /* for zlibVersion() */
#endif

// we assume that the inetd super server is available on all non-Windows systems
#ifndef _WIN32
#define INETD_AVAILABLE
#endif

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <experimental/filesystem>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

namespace
{
/* sort study mode */
enum E_SortStudyMode
{
  ESM_None,
  ESM_Timestamp,
  ESM_StudyInstanceUID,
  ESM_PatientName
};

struct StoreCallbackData
{
  char *imageFileName;
  DcmFileFormat *dcmff;
  T_ASC_Association *assoc;
};

// ------------------------------------------------------------------------------------------------------------

OFCondition echoSCP( T_ASC_Association * assoc, T_DIMSE_Message * msg, T_ASC_PresentationContextID presID)
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
  /*
  DIC_UI sopClass;
  DIC_UI sopInstance;


  // dump some information if required (depending on the progress state)
  // We can't use oflog for the PDU output, but we use a special logger for
  // generating this output. If it is set to level "INFO" we generate the
  // output, if it's set to "DEBUG" then we'll assume that there is debug output
  // generated for each PDU elsewhere.
  OFLogger progressLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION ".progress");
  if (progressLogger.getChainedLogLevel() == OFLogger::INFO_LOG_LEVEL)
  {
    switch (progress->state)
    {
    case DIMSE_StoreBegin:
      COUT << "RECV: ";
      break;
    case DIMSE_StoreEnd:
      COUT << OFendl;
      break;
    default:
      COUT << '.';
      break;
    }
    COUT.flush();
  }

  // if this is the final call of this function, save the data which was received to a file
  // (note that we could also save the image somewhere else, put it in database, etc.)
  if (progress->state == DIMSE_StoreEnd)
  {
    OFString tmpStr;

    // do not send status detail information
    *statusDetail = NULL;

    // remember callback data
    StoreCallbackData *cbdata = OFstatic_cast(StoreCallbackData *, callbackData);

    // Concerning the following line: an appropriate status code is already set in the resp structure,
    // it need not be success. For example, if the caller has already detected an out of resources problem
    // then the status will reflect this.  The callback function is still called to allow cleanup.
    //rsp->DimseStatus = STATUS_Success;

    // we want to write the received information to a file only if this information
    // is present and the options opt_bitPreserving and opt_ignore are not set.
    if ((imageDataSet != NULL) && (*imageDataSet != NULL))
    {
      OFString fileName;

      // in case one of the --sort-xxx options is set, we need to perform some particular steps to
      // determine the actual name of the output file
      if (opt_sortStudyMode != ESM_None)
      {
        // determine the study instance UID in the (current) DICOM object that has just been received
        OFString currentStudyInstanceUID;
        if ((*imageDataSet)->findAndGetOFString(DCM_StudyInstanceUID, currentStudyInstanceUID).bad() || currentStudyInstanceUID.empty())
        {
          OFLOG_ERROR(storescpLogger, "element StudyInstanceUID " << DCM_StudyInstanceUID << " absent or empty in data set");
          rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
          return;
        }

        // if --sort-on-patientname is active, we need to extract the
        // patient's name (format: last_name^first_name)
        OFString currentPatientName;
        if (opt_sortStudyMode == ESM_PatientName)
        {
          OFString tmpName;
          if ((*imageDataSet)->findAndGetOFString(DCM_PatientName, tmpName).bad() || tmpName.empty())
          {
            // default if patient name is missing or empty
            tmpName = "ANONYMOUS";
            OFLOG_WARN(storescpLogger, "element PatientName " << DCM_PatientName << " absent or empty in data set, using '"
                                                              << tmpName << "' instead");
          }
          else
          {
            DcmElement *patElem = NULL;
            OFString charset;
            (*imageDataSet)->findAndGetElement(DCM_PatientName, patElem); // patElem cannot be NULL, see above
            (*imageDataSet)->findAndGetOFStringArray(DCM_SpecificCharacterSet, charset);
            if (!charset.empty() && !(charset == "ISO_IR 100") && (patElem->containsExtendedCharacters()))
            {
              OFLOG_WARN(storescpLogger, "Patient name not in Latin-1 (charset: " << charset << "), ASCII dir name may be broken");
            }
          }

          // substitute non-ASCII characters in patient name to ASCII "equivalent"
          const size_t length = tmpName.length();
          for (size_t i = 0; i < length; i++)
            mapCharacterAndAppendToString(tmpName[i], currentPatientName);
        }

        // if this is the first DICOM object that was received or if the study instance UID in the
        // current DICOM object does not equal the last object's study instance UID we need to create
        // a new subdirectory in which the current DICOM object will be stored
        if (lastStudyInstanceUID.empty() || (lastStudyInstanceUID != currentStudyInstanceUID))
        {
          // if lastStudyInstanceUID is non-empty, we have just completed receiving all objects for one
          // study. In such a case, we need to set a certain indicator variable (lastStudySubdirectoryPathAndName),
          // so that we know that executeOnEndOfStudy() might have to be executed later. In detail, this indicator
          // variable will contain the path and name of the last study's subdirectory, so that we can still remember
          // this directory, when we execute executeOnEndOfStudy(). The memory that is allocated for this variable
          // here will be freed after the execution of executeOnEndOfStudy().
          if (!lastStudyInstanceUID.empty())
            lastStudySubdirectoryPathAndName = subdirectoryPathAndName;

          // create the new lastStudyInstanceUID value according to the value in the current DICOM object
          lastStudyInstanceUID = currentStudyInstanceUID;

          // get the current time (needed for subdirectory name)
          OFDateTime dateTime;
          dateTime.setCurrentDateTime();

          // create a name for the new subdirectory.
          char timestamp[32];
          sprintf(timestamp, "%04u%02u%02u_%02u%02u%02u%03u",
                  dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
                  dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond());

          OFString subdirectoryName;
          switch (opt_sortStudyMode)
          {
          case ESM_Timestamp:
            // pattern: "[prefix]_[YYYYMMDD]_[HHMMSSMMM]"
            subdirectoryName = opt_sortStudyDirPrefix;
            if (!subdirectoryName.empty())
              subdirectoryName += '_';
            subdirectoryName += timestamp;
            break;
          case ESM_StudyInstanceUID:
            // pattern: "[prefix]_[Study Instance UID]"
            subdirectoryName = opt_sortStudyDirPrefix;
            if (!subdirectoryName.empty())
              subdirectoryName += '_';
            subdirectoryName += currentStudyInstanceUID;
            break;
          case ESM_PatientName:
            // pattern: "[Patient's Name]_[YYYYMMDD]_[HHMMSSMMM]"
            subdirectoryName = currentPatientName;
            subdirectoryName += '_';
            subdirectoryName += timestamp;
            break;
          case ESM_None:
            break;
          }

          // create subdirectoryPathAndName (string with full path to new subdirectory)
          OFStandard::combineDirAndFilename(subdirectoryPathAndName, OFStandard::getDirNameFromPath(tmpStr, cbdata->imageFileName), subdirectoryName);

          // check if the subdirectory already exists
          // if it already exists dump a warning
          if (OFStandard::dirExists(subdirectoryPathAndName))
            OFLOG_WARN(storescpLogger, "subdirectory for study already exists: " << subdirectoryPathAndName);
          else
          {
            // if it does not exist create it
            OFLOG_INFO(storescpLogger, "creating new subdirectory for study: " << subdirectoryPathAndName);
#ifdef HAVE_WINDOWS_H
            if (_mkdir(subdirectoryPathAndName.c_str()) == -1)
#else
            if (mkdir(subdirectoryPathAndName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
#endif
            {
              OFLOG_ERROR(storescpLogger, "could not create subdirectory for study: " << subdirectoryPathAndName);
              rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
              return;
            }
            // all objects of a study have been received, so a new subdirectory is started.
            // ->timename counter can be reset, because the next filename can't cause a duplicate.
            // if no reset would be done, files of a new study (->new directory) would start with a counter in filename
            if (opt_timeNames)
              timeNameCounter = -1;
          }
        }

        // integrate subdirectory name into file name (note that cbdata->imageFileName currently contains both
        // path and file name; however, the path refers to the output directory captured in opt_outputDirectory)
        OFStandard::combineDirAndFilename(fileName, subdirectoryPathAndName, OFStandard::getFilenameFromPath(tmpStr, cbdata->imageFileName));
      }
      // if no --sort-xxx option is set, the determination of the output file name is simple
      else
      {
        fileName = cbdata->imageFileName;
      }

      // determine the transfer syntax which shall be used to write the information to the file
      E_TransferSyntax xfer = xfer = (*imageDataSet)->getOriginalXfer();

      // store file either with meta header or as pure dataset
      // OFLOG_INFO(storescpLogger, "storing DICOM file: " << fileName);
      if (OFStandard::fileExists(fileName))
      {
        // OFLOG_WARN(storescpLogger, "DICOM file already exists, overwriting: " << fileName);
      }
      OFCondition cond = cbdata->dcmff->saveFile(fileName.c_str(), xfer, EET_ExplicitLength, EGL_recalcGL,
                                                 EPD_withoutPadding, 0, 0, EWM_fileformat);
      if (cond.bad())
      {
        // OFLOG_ERROR(storescpLogger, "cannot write DICOM file: " << fileName << ": " << cond.text());
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
          // OFLOG_ERROR(storescpLogger, "bad DICOM file: " << fileName);
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
  */
}

OFCondition storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID)
{
  OFCondition cond = EC_Normal;
  T_DIMSE_C_StoreRQ *req;
  char imageFileName[2048];

  // assign the actual information of the C-STORE-RQ command to a local variable
  req = &msg->msg.CStoreRQ;

  /*
  {
    // 3 possibilities: create unique filenames (fn), create timestamp fn, create fn from SOP Instance UIDs
    if (opt_uniqueFilenames)
    {
      // create unique filename by generating a temporary UID and using ".X." as an infix
      char buf[70];
      dcmGenerateUniqueIdentifier(buf);
      sprintf(imageFileName, "%s%c%s.X.%s%s", opt_outputDirectory.c_str(), PATH_SEPARATOR, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"),
              buf, opt_fileNameExtension.c_str());
    }
    else if (opt_timeNames)
    {
      // create a name for the new file. pattern: "[YYYYMMDDHHMMSSMMM]_[NUMBER].MODALITY[EXTENSION]" (use current datetime)
      // get the current time (needed for file name)
      OFDateTime dateTime;
      dateTime.setCurrentDateTime();
      // used to hold prospective filename
      char cmpFileName[2048];
      // next if/else block generates prospective filename, that is compared to last written filename
      if (timeNameCounter == -1)
      {
        // timeNameCounter not set -> last written filename has to be without "serial number"
        sprintf(cmpFileName, "%04u%02u%02u%02u%02u%02u%03u.%s%s",
                dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
                dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond(),
                dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"), opt_fileNameExtension.c_str());
      }
      else
      {
        // counter was active before, so generate filename with "serial number" for comparison
        sprintf(cmpFileName, "%04u%02u%02u%02u%02u%02u%03u_%04u.%s%s", // millisecond version
                dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
                dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond(),
                timeNameCounter, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"), opt_fileNameExtension.c_str());
      }
      if ((outputFileNameArray.size() != 0) && (outputFileNameArray.back() == cmpFileName))
      {
        // if this is not the first run and the prospective filename is equal to the last written filename
        // generate one with a serial number (incremented by 1)
        timeNameCounter++;
        sprintf(imageFileName, "%s%c%04u%02u%02u%02u%02u%02u%03u_%04u.%s%s", opt_outputDirectory.c_str(), PATH_SEPARATOR, // millisecond version
                dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
                dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond(),
                timeNameCounter, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"), opt_fileNameExtension.c_str());
      }
      else
      {
        // first run or filenames are different: create filename without serial number
        sprintf(imageFileName, "%s%c%04u%02u%02u%02u%02u%02u%03u.%s%s", opt_outputDirectory.c_str(), PATH_SEPARATOR, // millisecond version
                dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
                dateTime.getTime().getHour(), dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond(),
                dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"), opt_fileNameExtension.c_str());
        // reset counter, because timestamp and therefore filename has changed
        timeNameCounter = -1;
      }
    }
    else
    {
      // don't create new UID, use the study instance UID as found in object
      sprintf(imageFileName, "%s%c%s.%s%s", opt_outputDirectory.c_str(), PATH_SEPARATOR, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"),
              req->AffectedSOPInstanceUID, opt_fileNameExtension.c_str());
    }
  }
*/
  // initialize some variables
  StoreCallbackData callbackData;
  callbackData.assoc = assoc;
  callbackData.imageFileName = imageFileName;
  DcmFileFormat dcmff;
  callbackData.dcmff = &dcmff;

  // store SourceApplicationEntityTitle in metaheader
  if (assoc && assoc->params)
  {
    const char *aet = assoc->params->DULparams.callingAPTitle;
    if (aet) {
      // dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
    }
  }

  // define an address where the information which will be received over the network will be stored
  DcmDataset *dset = dcmff.getDataset();

  cond = DIMSE_storeProvider(assoc, presID, req, NULL, OFTrue, &dset, storeSCPCallback, &callbackData, DIMSE_BLOCKING, 0);

  // if some error occurred, dump corresponding information and remove the outfile if necessary
  if (cond.bad())
  {
    OFString temp_str;
    // OFLOG_ERROR(storescpLogger, "Store SCP Failed: " << DimseCondition::dump(temp_str, cond));
    // remove file
    if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0)
      OFStandard::deleteFile(imageFileName);
  }

  // return return value
  return cond;
}

// ------------------------------------------------------------------------------------------------------------

OFCondition processCommands(T_ASC_Association *assoc)
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
        cond = storeSCP(assoc, &msg, presID);
        break;
      default:
        OFString tempStr;
        // we cannot handle this kind of message
        cond = DIMSE_BADCOMMANDTYPE;
        /*
        OFLOG_ERROR(storescpLogger, "Expected C-ECHO or C-STORE request but received DIMSE command 0x"
                                        << STD_NAMESPACE hex << STD_NAMESPACE setfill('0') << STD_NAMESPACE setw(4)
                                        << OFstatic_cast(unsigned, msg.CommandField));
        OFLOG_DEBUG(storescpLogger, DIMSE_dumpMessage(tempStr, msg, DIMSE_INCOMING, NULL, presID));
        */
        break;
      }
    }
  }
  return cond;
}

// ------------------------------------------------------------------------------------------------------------

OFCondition acceptAssociation(T_ASC_Network *net, DcmAssociationConfiguration &asccfg, OFBool secureConnection)
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
      // OFLOG_ERROR(storescpLogger, "Receiving Association failed: " << DimseCondition::dump(temp_str, cond));
    }

    // no matter what kind of error occurred, we need to do a cleanup
    goto cleanup;
  }

  // OFLOG_INFO(storescpLogger, "Association Received");

  if (gLocalByteOrder == EBO_LittleEndian) /* defined in dcxfer.h */
  {
    transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
    transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
  }
  else
  {
    transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
    transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
  }
  transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
  numTransferSyntaxes = 3;

  {
    /* accept the Verification SOP Class if presented */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params, knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes), transferSyntaxes, numTransferSyntaxes);
    if (cond.bad())
    {
      // OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
      goto cleanup;
    }

    /* the array of Storage SOP Class UIDs comes from dcuid.h */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params, dcmAllStorageSOPClassUIDs, numberOfDcmAllStorageSOPClassUIDs, transferSyntaxes, numTransferSyntaxes);
    if (cond.bad())
    {
      // OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
      goto cleanup;
    }
  }

  /* set our app title */
  ASC_setAPTitles(assoc->params, NULL, NULL, "IMEBRA"/*in.source.aet.c_str()*/);

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

    // OFLOG_INFO(storescpLogger, "Association Rejected: Bad Application Context Name: " << buf);
    cond = ASC_rejectAssociation(assoc, &rej);
    if (cond.bad())
    {
      // OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
    }
    goto cleanup;
  }
  else
  {
    cond = ASC_acknowledgeAssociation(assoc);
    if (cond.bad())
    {
      // OFLOG_ERROR(storescpLogger, DimseCondition::dump(temp_str, cond));
      goto cleanup;
    }
  }

  /* now do the real work, i.e. receive DIMSE commands over the network connection */
  /* which was established and handle these commands correspondingly. In case of */
  /* storescp only C-ECHO-RQ and C-STORE-RQ commands can be processed. */
  cond = processCommands(assoc);

  if (cond == DUL_PEERREQUESTEDRELEASE)
  {
    // OFLOG_INFO(storescpLogger, "Association Release");
    cond = ASC_acknowledgeRelease(assoc);
  }
  else if (cond == DUL_PEERABORTEDASSOCIATION)
  {
    // OFLOG_INFO(storescpLogger, "Association Aborted");
  }
  else
  {
    // OFLOG_ERROR(storescpLogger, "DIMSE failure (aborting association): " << DimseCondition::dump(temp_str, cond));
    /* some kind of error so abort the association */
    cond = ASC_abortAssociation(assoc);
  }

cleanup:
  cond = ASC_dropSCPAssociation(assoc);
  if (cond.bad())
  {
    // OFLOG_FATAL(storescpLogger, DimseCondition::dump(temp_str, cond));
    return cond;
  }
  cond = ASC_destroyAssociation(&assoc);
  if (cond.bad())
  {
    // OFLOG_FATAL(storescpLogger, DimseCondition::dump(temp_str, cond));
  }

  return cond;
}



} // end namespace

ServerAsyncWorker::ServerAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void ServerAsyncWorker::Execute(const ExecutionProgress &progress)
{
  ns::sInput in = ns::parseInputJson(_input);

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

  // OFBool opt_uniqueFilenames = OFFalse;
  // OFString opt_fileNameExtension;
  // OFBool opt_timeNames = OFFalse;
  // int timeNameCounter = -1; // "serial number" to differentiate between files with same timestamp

  int opt_port = std::stoi(in.target.port);
  OFString callingAETitle = OFString(in.source.aet.c_str());
  OFString calledAETitle = OFString(in.target.aet.c_str());
  OFString callingPresentationAddress = OFString(in.target.ip.c_str());
  const char *opt_respondingAETitle = in.source.aet.c_str();
  static OFString opt_outputDirectory = ".";        // default: output directory equals "."
  // E_SortStudyMode opt_sortStudyMode = ESM_None;     // default: no sorting
  // static const char *opt_sortStudyDirPrefix = NULL; // default: no directory prefix
  // OFString lastStudyInstanceUID;
  // OFString subdirectoryPathAndName;
  // OFString lastStudySubdirectoryPathAndName;

  T_ASC_Network *net;
  DcmAssociationConfiguration asccfg;

  OFStandard::initializeNetwork();

  /* make sure data dictionary is loaded */
  if (!dcmDataDict.isDictionaryLoaded())
  {
    // OFLOG_WARN(storescpLogger, "no data dictionary loaded, check environment variable: " << DCM_DICT_ENVIRONMENT_VARIABLE);
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
      // OFLOG_FATAL(storescpLogger, "specified output directory does not exist");
      return;
    }
  }

  /* check if the output directory is writeable */
  if (!OFStandard::isWriteable(opt_outputDirectory))
  {
    // OFLOG_FATAL(storescpLogger, "specified output directory is not writeable");
    return;
  }

  /* initialize network, i.e. create an instance of T_ASC_Network*. */
  OFCondition cond = ASC_initializeNetwork(NET_ACCEPTOR, opt_port, 30, &net);
  if (cond.bad())
  {
    // OFLOG_ERROR(storescpLogger, "cannot create network: " << DimseCondition::dump(temp_str, cond));
    return;
  }

  /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
  if (OFStandard::dropPrivileges().bad())
  {
    // OFLOG_FATAL(storescpLogger, "setuid() failed, maximum number of processes/threads for uid already running.");
    return;
  }

  while (cond.good())
  {
    /* receive an association and acknowledge or reject it. If the association was */
    /* acknowledged, offer corresponding services and invoke one or more if required. */
    cond = acceptAssociation(net, asccfg, false);
  }

  /* drop the network, i.e. free memory of T_ASC_Network* structure. This call */
  /* is the counterpart of ASC_initializeNetwork(...) which was called above. */
  cond = ASC_dropNetwork(&net);
  if (cond.bad())
  {
    // OFLOG_ERROR(storescpLogger, DimseCondition::dump(temp_str, cond));
    return;
  }

  OFStandard::shutdownNetwork();
}
