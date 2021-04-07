#include "StoreAsyncWorker.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmnet/dicom.h"      /* for DICOM_APPLICATION_REQUESTOR */
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmnet/dcasccfg.h"   /* for class DcmAssociationConfiguration */
#include "dcmtk/dcmnet/dcasccff.h"   /* for class DcmAssociationConfigurationFile */
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/dcostrmz.h"  /* for dcmZlibCompressionLevel */


#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#define INCLUDE_CERRNO
#define INCLUDE_CCTYPE
#define INCLUDE_CSIGNAL

BEGIN_EXTERN_C
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>       /* needed on Solaris for O_RDONLY */
#endif
#ifdef HAVE_SIGNAL_H
// On Solaris with Sun Workshop 11, <signal.h> declares signal() but <csignal> does not
#include <signal.h>
#endif
END_EXTERN_C

#ifdef HAVE_WINDOWS_H
#include <direct.h>      /* for _mkdir() */
#endif

#if defined(HAVE_MKTEMP) && !defined(HAVE_PROTOTYPE_MKTEMP)
extern "C" {
//char * mktemp(char *);
}
#endif

BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
END_EXTERN_C


#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif

StoreAsyncWorker::StoreAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
    ns::registerCodecs();

    m_networkTransferSyntax = EXS_Unknown;
    m_readMode = ERM_autoDetect;
    m_acse_timeout = 60;
    m_dimse_timeout = 60;
    m_proposeOnlyRequiredPresentationContexts = OFFalse;
    m_lastCondition = EC_Normal;
}

void StoreAsyncWorker::Execute(const ExecutionProgress &progress)
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

    if (!setScanDirectory(in.sourcePath.c_str())) {
        SetErrorJson("Invalid source path set, no DICOM files found");
        return;
    }

    DcmXfer netTransPropose = in.netTransferPropose.empty() ? DcmXfer(EXS_Unknown) : DcmXfer(in.netTransferPropose.c_str());
    DCMNET_INFO("proposed network transfer syntax for outgoing associations: " << netTransPropose.getXferName());
    m_networkTransferSyntax = netTransPropose.getXfer();

    bool success = sendStoreRequest(in.target.aet.c_str(), in.target.ip.c_str(), OFstatic_cast(Uint16, std::stoi(in.target.port)),
        in.source.aet.c_str(), in.source.ip.c_str(), OFstatic_cast(Uint16, std::stoi(in.source.port)) );

    if (!success) {
        SetErrorJson("Failed to send DICOM files to target");
    }

}

bool StoreAsyncWorker::setScanDirectory(const OFFilename &dir)
{
    std::string fullPath(dir.getCharPointer());
    // unfortunately we need to convert slashes to backslashes on windows and vice versa on linux
#ifdef HAVE_WINDOWS_H
    std::replace(fullPath.begin(),fullPath.end(),'/','\\');
#else
    std::replace(fullPath.begin(),fullPath.end(),'\\','/');
#endif
   
    const OFFilename directory(fullPath.c_str(),true);
    
    // test if the directory exists:
    if (!OFStandard::dirExists(directory)) {
        DCMNET_ERROR("Directory does not exist" << directory);
        return false;
    }

    m_sourceDirectory = directory;
    return true;

}

bool StoreAsyncWorker::sendStoreRequest(const OFString& peerTitle, const OFString& peerIP, Uint16 peerPort, 
    const OFString& ourTitle, const OFString& ourIP, Uint16 ourPort)
{
    bool result = true;
    OFList<OFFilename> inputFiles;

    OFList<OFFilename> fileNameList;     // list of files to transfer to SCP
    OFList<OFString> sopClassUIDList;    // the list of SOP classes
    OFList<OFString> sopInstanceUIDList; // the list of SOP instances

    DIC_NODENAME peerHost;
    DcmAssociationConfiguration asccfg;  // handler for association configuration profiles

    OFString temp_str;

    T_ASC_Association* assoc = NULL;
    T_ASC_Network* net = NULL;
    T_ASC_Parameters* params = NULL;

    /* create list of input files */
    DCMNET_INFO("determining input files ...");

    OFStandard::searchDirectoryRecursively(m_sourceDirectory, inputFiles, 
        OFFilename() /*Pattern */, OFFilename() /*dirPrefix*/, OFTrue);

 
    /* check whether there are any input files at all */
    if (inputFiles.empty())
    {
      DCMNET_ERROR("no input files to be sent");
      return false;
    }

    /* check input files */
    OFString errormsg;
    DcmFileFormat dfile;
    char sopClassUID[128];
    char sopInstanceUID[128];
    OFBool ignoreName;
    OFListIterator(OFFilename) if_iter = inputFiles.begin();
    OFListIterator(OFFilename) if_last = inputFiles.end();
    
    DCMNET_INFO("checking input files ...");
    /* iterate over all input filenames */
    while (if_iter != if_last)
    {
      ignoreName = OFFalse;
      const OFFilename & currentFilename = (*if_iter);
      if (OFStandard::fileExists(currentFilename))
      {
        if (!isDicomFile(currentFilename)) {
            ignoreName = OFTrue;
        }

        if (m_proposeOnlyRequiredPresentationContexts)
        {
          if (!findSOPClassAndInstanceInFile(currentFilename, sopClassUID, sopInstanceUID))
          {
            ignoreName = OFTrue;
            errormsg = "missing SOP class (or instance) in file: ";
            errormsg += OFString(currentFilename.getCharPointer());
            DCMNET_WARN(errormsg << ", ignoring file");
          }
          else if (!dcmIsaStorageSOPClassUID(sopClassUID))
          {
            ignoreName = OFTrue;
            errormsg = "unknown storage SOP class in file: ";
            errormsg += OFString( currentFilename.getCharPointer() );
            errormsg += ": ";
            errormsg += sopClassUID;
            DCMNET_WARN(errormsg << ", ignoring file");
          }
          else
          {
            sopClassUIDList.push_back(sopClassUID);
            sopInstanceUIDList.push_back(sopInstanceUID);
          }
        }
        if (!ignoreName) 
        {
            fileNameList.push_back(currentFilename);
        }
      }
      else
      {
        errormsg = "cannot access file: ";
        errormsg += OFString(currentFilename.getCharPointer());
        DCMNET_WARN(errormsg << ", ignoring file");
      }
      ++if_iter;
    }

    /* do the real work, i.e. for all files which were specified in the */
    /* command line, transmit the encapsulated DICOM objects to the SCP. */
    OFListIterator(OFFilename) iter = fileNameList.begin();
    OFListIterator(OFFilename) enditer = fileNameList.end();


    /* initialize network, i.e. create an instance of T_ASC_Network*. */
    m_lastCondition = ASC_initializeNetwork(NET_REQUESTOR, 0, m_acse_timeout, &net);
    if (m_lastCondition.bad()) {
      DCMNET_ERROR(DimseCondition::dump(temp_str, m_lastCondition));
      return false;
    }


    /* initialize association parameters, i.e. create an instance of T_ASC_Parameters*. */
    m_lastCondition = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    if (m_lastCondition.bad()) {
      DCMNET_ERROR(DimseCondition::dump(temp_str, m_lastCondition));
      return false;
    }
    /* sets this application's title and the called application's title in the params */
    /* structure. The default values to be set here are "STORESCU" and "ANY-SCP". */
    ASC_setAPTitles(params, ourTitle.c_str(), peerTitle.c_str(), ourTitle.c_str());

    /* Set the transport layer type (type of network connection) in the params */
    /* structure. The default is an insecure connection; where OpenSSL is  */
    /* available the user is able to request an encrypted,secure connection. */
    m_lastCondition = ASC_setTransportLayerType(params, OFFalse);
    if (m_lastCondition.bad()) {
      DCMNET_ERROR(DimseCondition::dump(temp_str, m_lastCondition));
      return false;
    }

    /* Figure out the presentation addresses and copy the */
    /* corresponding values into the association parameters.*/
    sprintf(peerHost, "%s:%d", peerIP.c_str(), (int)peerPort);
    ASC_setPresentationAddresses(params, OFStandard::getHostName().c_str(), peerHost);


    /* Set the presentation contexts which will be negotiated */
    /* when the network connection will be established */
    m_lastCondition = addStoragePresentationContexts(params, sopClassUIDList);

    if (m_lastCondition.bad()) {
      DCMNET_ERROR(DimseCondition::dump(temp_str, m_lastCondition));
      return false;
    }

    DCMNET_INFO("Request Parameters:");
    DCMNET_INFO(ASC_dumpParameters(temp_str, params, ASC_ASSOC_RQ));

    /* create association, i.e. try to establish a network connection to another */
    /* DICOM application. This call creates an instance of T_ASC_Association*. */
    DCMNET_INFO("Requesting Association");
    m_lastCondition = ASC_requestAssociation(net, params, &assoc, NULL, 0, DUL_BLOCK, m_dimse_timeout );
    if (m_lastCondition.bad()) {
      if (m_lastCondition == DUL_ASSOCIATIONREJECTED) {
        T_ASC_RejectParameters rej;

        ASC_getRejectParameters(params, &rej);
        DCMNET_ERROR("Association Rejected:");
        DCMNET_ERROR(ASC_printRejectParameters(temp_str, &rej));
      } else {
        DCMNET_ERROR("StoreScu - Association Request Failed: " << DimseCondition::dump(temp_str, m_lastCondition));
      }
      return false;
    }

    /* dump the connection parameters if in debug mode*/
    DCMNET_INFO(ASC_dumpConnectionParameters(temp_str, assoc));


    DCMNET_INFO("Association Parameters Negotiated:");
    DCMNET_INFO(ASC_dumpParameters(temp_str, params, ASC_ASSOC_AC));

    /* count the presentation contexts which have been accepted by the SCP */
    /* If there are none, finish the execution */
    if (ASC_countAcceptedPresentationContexts(params) == 0) {
      DCMNET_ERROR("No Acceptable Presentation Contexts");
      return false;
    }

    /* dump general information concerning the establishment of the network connection if required */
    DCMNET_INFO("Association Accepted (Max Send PDV: " << assoc->sendPDVLength << ")");

    float fileCount = static_cast<float>(fileNameList.size());
    float count = 0;
    while ((iter != enditer) && m_lastCondition.good())
    {
      m_lastCondition = storeSCU(assoc, *iter);
      ++iter;
      ++count;
      int progress = (int)(count / fileCount * 100.0f);
      DCMNET_INFO("C-Store progress: " << progress);
    }

    /* tear down association, i.e. terminate network connection to SCP */
    if (m_lastCondition == EC_Normal)
    {
        /* release association */
            DCMNET_INFO("Releasing Association");
        m_lastCondition = ASC_releaseAssociation(assoc);
        if (m_lastCondition.bad())
        {
            DCMNET_ERROR("Association Release Failed: " << DimseCondition::dump(temp_str, m_lastCondition));
            return false;
        }
    }
    else if (m_lastCondition == DUL_PEERREQUESTEDRELEASE)
    {
      DCMNET_ERROR("Protocol Error: Peer requested release (Aborting)");
      DCMNET_INFO("Aborting Association");
      m_lastCondition = ASC_abortAssociation(assoc);
      if (m_lastCondition.bad()) {
        DCMNET_ERROR("Association Abort Failed: " << DimseCondition::dump(temp_str, m_lastCondition));
        return false;
      }
    }
    else if (m_lastCondition == DUL_PEERABORTEDASSOCIATION)
    {
      DCMNET_INFO("Peer Aborted Association");
    }
    else
    {
      DCMNET_ERROR(DimseCondition::dump(temp_str, m_lastCondition));
      DCMNET_INFO("Aborting Association");
      m_lastCondition = ASC_abortAssociation(assoc);
      if (m_lastCondition.bad()) {
        DCMNET_ERROR("Association Abort Failed: " << DimseCondition::dump(temp_str, m_lastCondition));
        return false;
      }
    }

    return result;
}

OFBool StoreAsyncWorker::isaListMember(OFList<OFString> &lst, OFString &s)
{
  OFListIterator(OFString) cur = lst.begin();
  OFListIterator(OFString) end = lst.end();

  OFBool found = OFFalse;
  while (cur != end && !found) {
    found = (s == *cur);
    ++cur;
  }

  return found;
}

OFCondition StoreAsyncWorker::addPresentationContext(T_ASC_Parameters *params, int presentationContextId,
                                                  const OFString &abstractSyntax, const OFString &transferSyntax,
                                                  T_ASC_SC_ROLE proposedRole)
{
  const char *c_p = transferSyntax.c_str();
  OFCondition cond = ASC_addPresentationContext(params, presentationContextId,
    abstractSyntax.c_str(), &c_p, 1, proposedRole);
  return cond;
}

OFCondition StoreAsyncWorker::addPresentationContext(T_ASC_Parameters *params, int presentationContextId,
                                                  const OFString &abstractSyntax, const OFList<OFString> &transferSyntaxList,
                                                  T_ASC_SC_ROLE proposedRole)
{
  // create an array of supported/possible transfer syntaxes
  const char **transferSyntaxes = new const char*[transferSyntaxList.size()];
  int transferSyntaxCount = 0;
  OFListConstIterator(OFString) s_cur = transferSyntaxList.begin();
  OFListConstIterator(OFString) s_end = transferSyntaxList.end();
  while (s_cur != s_end) {
    transferSyntaxes[transferSyntaxCount++] = (*s_cur).c_str();
    ++s_cur;
  }

  OFCondition cond = ASC_addPresentationContext(params, presentationContextId,
    abstractSyntax.c_str(), transferSyntaxes, transferSyntaxCount, proposedRole);

  delete[] transferSyntaxes;
  return cond;
}

OFCondition StoreAsyncWorker::addStoragePresentationContexts(T_ASC_Parameters *params,
                                                          OFList<OFString> &sopClasses)
{

  // Which transfer syntax was preferred on the command line
  OFString preferredTransferSyntax;
  if (m_networkTransferSyntax == EXS_Unknown) {
    /* gLocalByteOrder is defined in dcxfer.h */
    if (gLocalByteOrder == EBO_LittleEndian) {
      /* we are on a little endian machine */
      preferredTransferSyntax = UID_LittleEndianExplicitTransferSyntax;
    } else {
      /* we are on a big endian machine */
      preferredTransferSyntax = UID_BigEndianExplicitTransferSyntax;
    }
  } else {
    DcmXfer xfer(m_networkTransferSyntax);
    preferredTransferSyntax = xfer.getXferID();
  }

  OFListIterator(OFString) s_cur;
  OFListIterator(OFString) s_end;

  OFList<OFString> fallbackSyntaxes;
  // - If little endian implicit is preferred, we don't need any fallback syntaxes
  //   because it is the default transfer syntax and all applications must support it.
  // - If MPEG2 MP@ML is preferred, we don't want to propose any fallback solution
  //   because this is not required and we cannot decompress the movie anyway.
  if ((m_networkTransferSyntax != EXS_LittleEndianImplicit) &&
      (m_networkTransferSyntax != EXS_MPEG2MainProfileAtMainLevel))
  {
    fallbackSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
    fallbackSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
    fallbackSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
    // Remove the preferred syntax from the fallback list
    fallbackSyntaxes.remove(preferredTransferSyntax);
  }

  // create a list of transfer syntaxes combined from the preferred and fallback syntaxes
  OFList<OFString> combinedSyntaxes;
  s_cur = fallbackSyntaxes.begin();
  s_end = fallbackSyntaxes.end();
  combinedSyntaxes.push_back(preferredTransferSyntax);
  while (s_cur != s_end)
  {
    if (!isaListMember(combinedSyntaxes, *s_cur)) combinedSyntaxes.push_back(*s_cur);
    ++s_cur;
  }

  if (!m_proposeOnlyRequiredPresentationContexts) {
    // add the (short list of) known storage SOP classes to the list
    // the array of Storage SOP Class UIDs comes from dcuid.h
    for (int i = 0; i < numberOfDcmShortSCUStorageSOPClassUIDs; i++)
      sopClasses.push_back(dcmShortSCUStorageSOPClassUIDs[i]);
  }

  // thin out the SOP classes to remove any duplicates
  OFList<OFString> sops;
  s_cur = sopClasses.begin();
  s_end = sopClasses.end();
  while (s_cur != s_end) {
    if (!isaListMember(sops, *s_cur)) {
      sops.push_back(*s_cur);
    }
    ++s_cur;
  }

  // add a presentations context for each SOP class / transfer syntax pair
  OFCondition cond = EC_Normal;
  int pid = 1; // presentation context id
  s_cur = sops.begin();
  s_end = sops.end();
  while (s_cur != s_end && cond.good()) {

    if (pid > 255) {
      DCMNET_ERROR("Too many presentation contexts");
      return ASC_BADPRESENTATIONCONTEXTID;
    }

    // SOP class with preferred transfer syntax
    cond = addPresentationContext(params, pid, *s_cur, preferredTransferSyntax);
    pid += 2;   /* only odd presentation context id's */

    if (fallbackSyntaxes.size() > 0) {
    if (pid > 255) {
        DCMNET_ERROR("Too many presentation contexts");
        return ASC_BADPRESENTATIONCONTEXTID;
    }

    // SOP class with fallback transfer syntax
    cond = addPresentationContext(params, pid, *s_cur, fallbackSyntaxes);
    pid += 2; /* only odd presentation context id's */
    }
    ++s_cur;
  }

  return cond;
}

void StoreAsyncWorker::progressCallback(void * /*callbackData*/, T_DIMSE_StoreProgress *progress,
                             T_DIMSE_C_StoreRQ * req)
{
  if (progress->state == DIMSE_StoreBegin)
  {
    OFString str;
    DCMNET_INFO(DIMSE_dumpMessage(str, *req, DIMSE_OUTGOING));
  }

}

OFCondition StoreAsyncWorker::storeSCU(T_ASC_Association *assoc, const OFFilename &fname)
{
  DIC_US msgId = assoc->nextMsgID++;
  T_ASC_PresentationContextID presID;
  T_DIMSE_C_StoreRQ req;
  T_DIMSE_C_StoreRSP rsp;
  DIC_UI sopClass;
  DIC_UI sopInstance;
  DcmDataset *statusDetail = NULL;

  DCMNET_INFO("Sending file: " << fname);

  /* read information from file. After the call to DcmFileFormat::loadFile(...) the information */
  /* which is encapsulated in the file will be available through the DcmFileFormat object. */
  /* In detail, it will be available through calls to DcmFileFormat::getMetaInfo() (for */
  /* meta header information) and DcmFileFormat::getDataset() (for data set information). */
  DcmFileFormat dcmff;
  OFCondition cond = dcmff.loadFile(fname, EXS_Unknown, EGL_noChange, DCM_MaxReadLength, m_readMode);

  /* figure out if an error occurred while the file was read*/
  if (cond.bad()) {
    DCMNET_ERROR("Bad DICOM file: " << fname << ": " << cond.text());
    return cond;
  }

  /* figure out which SOP class and SOP instance is encapsulated in the file */
  if (!DU_findSOPClassAndInstanceInDataSet(dcmff.getDataset(), sopClass, sizeof(sopClass), sopInstance, sizeof(sopInstance), OFFalse)) {
      DCMNET_ERROR("No SOP Class or Instance UID in file: " << fname);
      return DIMSE_BADDATA;
  }

  /* figure out which of the accepted presentation contexts should be used */
  DcmXfer filexfer(dcmff.getDataset()->getOriginalXfer());

  /* special case: if the file uses an unencapsulated transfer syntax (uncompressed
   * or deflated explicit VR) and we prefer deflated explicit VR, then try
   * to find a presentation context for deflated explicit VR first.
   */
  if (filexfer.isNotEncapsulated() &&
    m_networkTransferSyntax == EXS_DeflatedLittleEndianExplicit)
  {
    filexfer = EXS_DeflatedLittleEndianExplicit;
  }

  if (filexfer.getXfer() != EXS_Unknown) {
      presID = ASC_findAcceptedPresentationContextID(assoc, sopClass, filexfer.getXferID());
  } else {
      presID = ASC_findAcceptedPresentationContextID(assoc, sopClass);
  }

  if (presID == 0) {
    const char *modalityName = dcmSOPClassUIDToModality(sopClass);
    if (!modalityName) modalityName = dcmFindNameOfUID(sopClass);
    if (!modalityName) modalityName = "unknown SOP class";
    DCMNET_ERROR("No presentation context for: (" << modalityName << ") " << sopClass);
    return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
  }

  T_ASC_PresentationContext pc;
  ASC_findAcceptedPresentationContext(assoc->params, presID, &pc);
  DcmXfer netTransfer(pc.acceptedTransferSyntax);

  /* if required, dump general information concerning transfer syntaxes */
  DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
  DCMNET_INFO("Transfer Syntax: " << dcmFindNameOfUID(fileTransfer.getXferID())
        << " -> " << dcmFindNameOfUID(netTransfer.getXferID()));

  /* prepare the transmission of data */
  bzero((char *)&req, sizeof(req));
  req.MessageID = msgId;
  strcpy(req.AffectedSOPClassUID, sopClass);
  strcpy(req.AffectedSOPInstanceUID, sopInstance);
  req.DataSetType = DIMSE_DATASET_PRESENT;
  req.Priority = DIMSE_PRIORITY_LOW;

  /* if required, dump some more general information */
  DCMNET_INFO("Sending Store Request: MsgID " << msgId << ", ("
    << dcmSOPClassUIDToModality(sopClass, "OT") << ")");

  // need to recode depending on accepted xfer
  DcmDataset* dataset = dcmff.getDataset();
  dataset->chooseRepresentation(netTransfer.getXfer(), NULL);

  // test
  if (!dataset->canWriteXfer(netTransfer.getXfer())) {
      DCMNET_ERROR("unable to write Xfer: " << netTransfer.getXferName());
  }

  /* finally conduct transmission of data */
  cond = DIMSE_storeUser(assoc, presID, &req, NULL, dataset, progressCallback, NULL,
    DIMSE_BLOCKING, m_dimse_timeout, &rsp, &statusDetail, NULL, (long)OFStandard::getFileSize(fname));

  /* dump some more general information */
  if (cond == EC_Normal)
  {
    OFString str;
    DCMNET_INFO("Received Store Response");
    DCMNET_INFO(DIMSE_dumpMessage(str, rsp, DIMSE_INCOMING, NULL, presID));
  }
  else
  {
    OFString temp_str;
    DCMNET_ERROR("Store Failed, file: " << fname << ":");
    DCMNET_ERROR( DimseCondition::dump(temp_str, cond));
  }

  /* dump status detail information if there is some */
  if (statusDetail != NULL) {
    DCMNET_INFO("Status Detail:");
    DCMNET_INFO(DcmObject::PrintHelper(*statusDetail));
    delete statusDetail;
  }
  /* return */
  return cond;
}

OFBool StoreAsyncWorker::findSOPClassAndInstanceInFile(const OFFilename &fname, char *sopClass, 
                                                           char *sopInstance)
{
    DcmFileFormat ff;
    if (!ff.loadFile(fname, EXS_Unknown, EGL_noChange, DCM_MaxReadLength, m_readMode).good())
        return OFFalse;

    /* look in the meta-header first */
    OFBool found = DU_findSOPClassAndInstanceInDataSet(ff.getMetaInfo(), sopClass, sizeof(sopClass), sopInstance, sizeof(sopInstance),  OFFalse);

    if (!found)
      found = DU_findSOPClassAndInstanceInDataSet(ff.getDataset(), sopClass, sizeof(sopClass), sopInstance, sizeof(sopInstance),  OFFalse);

    return found;
}

OFBool StoreAsyncWorker::isDicomFile( const OFFilename &fname )
{
    return OFTrue;
}


