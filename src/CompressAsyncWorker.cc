#include "CompressAsyncWorker.h"

#include <iostream>
#include <list>
#include <memory>
#include <sstream>

#include "Utils.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/ofstd/offilsys.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmnet/dcmtrans.h" /* for dcmSocketSend/ReceiveTimeout */
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcjson.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dcvrui.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcdeftag.h"

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

namespace {
  E_TransferSyntax lookForXfer(DcmMetaInfo *metainfo)
  {
      E_TransferSyntax newxfer = EXS_Unknown;
      DcmStack stack;
      /* check whether meta header is present (and non-empty, i.e. contains elements) */
      if (metainfo && !metainfo->isEmpty())
      {
          if (metainfo->search(DCM_TransferSyntaxUID, stack).good())
          {
              DcmUniqueIdentifier *xferUI = OFstatic_cast(DcmUniqueIdentifier *, stack.top());
              if (xferUI->getTag().getXTag() == DCM_TransferSyntaxUID)
              {
                  char *xferid = NULL;
                  xferUI->getString(xferid);
                  DcmXfer localXfer(xferid);      // decode to E_TransferSyntax
                  newxfer = localXfer.getXfer();
              }
          } else {
              /* there is no transfer syntax UID element in the meta header */
              DCMNET_DEBUG("DcmFileFormat::lookForXfer() no TransferSyntax in MetaInfo");
          }
      } else {
          /* no meta header present at all (or it is empty, i.e. contains no elements) */
          DCMNET_DEBUG("DcmFileFormat::lookForXfer() no MetaInfo found");
      }
      return newxfer;
  }
}

CompressAsyncWorker::CompressAsyncWorker(std::string data, Function &callback)
    : BaseAsyncWorker(data, callback) {
    ns::registerCodecs();
}

void CompressAsyncWorker::Execute(const ExecutionProgress &progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    EnableVerboseLogging(in.verbose);

    if (in.sourcePath.empty()) {
        SetErrorJson("No source path set");
        return;
    }

    if (in.storagePath.empty()) {
        SetErrorJson("No target path set");
        return;
    }

    DcmXfer writeTrans = in.writeTransfer.empty() ? DcmXfer(EXS_LittleEndianImplicit) : DcmXfer(in.writeTransfer.c_str());
    DCMNET_INFO("write transfer syntax: " << writeTrans.getXferName());

    OFList<OFFilename> inputFiles;
    OFList<OFFilename> fileNameList;

    /* create list of input files */
    DCMNET_INFO("determining input files ...");

    OFFilename sourcePath(in.sourcePath.c_str());

    if (OFStandard::dirExists(sourcePath)) {
      OFStandard::searchDirectoryRecursively(sourcePath, inputFiles, 
        OFFilename() /*Pattern */, OFFilename() /*dirPrefix*/, OFTrue);
    } 
    else if (OFStandard::fileExists(sourcePath)) {
      inputFiles.push_back(sourcePath);
    }
 
    /* check whether there are any input files at all */
    if (inputFiles.empty())
    {
      SetErrorJson("Invalid source path set, no DICOM files found");
      return;
    }

    /* check input files */
    OFString errormsg;
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

    OFListIterator(OFFilename) iter = fileNameList.begin();
    OFListIterator(OFFilename) enditer = fileNameList.end();

    float fileCount = static_cast<float>(fileNameList.size());
    float count = 0;
    bool validFileFound = false;
    while ((iter != enditer))
    {
      if (recompress(*iter, OFString(in.storagePath.c_str()), writeTrans.getXfer()) ) {
        validFileFound = true;
      }
      ++iter;
      ++count;
      int progress = (int)(count / fileCount * 100.0f);
      DCMNET_INFO("Compress progress: " << progress);
    }

    if (!validFileFound) {
      SetErrorJson("Invalid source path set, no DICOM files found");
    }
}

OFBool CompressAsyncWorker::isDicomFile( const OFFilename &fname )
{
    return OFTrue;
}

OFBool CompressAsyncWorker::recompress(const OFFilename& infile, const OFString& storePath, E_TransferSyntax prefXfer) {
    DcmFileFormat dfile;
    OFCondition status = dfile.loadFile(infile, EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect);
    if (status.bad()) {
        DCMNET_WARN("Failed loading file: " << infile.getCharPointer());
        return OFFalse;
    }

    char sopClassUID[128];
    char sopInstanceUID[128];
    OFBool found = DU_findSOPClassAndInstanceInDataSet(dfile.getDataset(), sopClassUID, sizeof(sopClassUID), sopInstanceUID, sizeof(sopInstanceUID),  OFFalse);
    if (!found) {
       DCMNET_WARN("Failed reading SOPInstanceUid in file: " << infile.getCharPointer());
       return OFFalse;
    }
    OFString outfile = storePath + OFString("/") + OFString(sopInstanceUID);
    DCMNET_INFO(outfile);

    if (OFpath(infile.getCharPointer()) == OFpath(outfile) && lookForXfer(dfile.getMetaInfo()) == prefXfer) {
      DCMNET_INFO("file has correct ts already skipping...");
      return OFTrue;
    }

    OFCondition cond = dfile.chooseRepresentation(prefXfer, NULL);
    if (cond.bad() || !dfile.canWriteXfer(prefXfer)) {
      DCMNET_WARN("Failed compressing file: " << infile.getCharPointer());
      return OFFalse;
    }
    return dfile.saveFile(outfile, prefXfer).good();
}
