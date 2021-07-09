#include "StoreAsyncWorker.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/ofstd/ofstd.h"       /* for OFStandard functions */
#include "dcmtk/ofstd/ofconapp.h"    /* for OFConsoleApplication */
#include "dcmtk/ofstd/ofstream.h"    /* for OFStringStream et al. */
#include "dcmtk/dcmdata/dcdict.h"    /* for global data dictionary */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/cmdlnarg.h"  /* for prepareCmdLineArgs */
#include "dcmtk/dcmdata/dcostrmz.h"  /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmnet/dstorscu.h"   /* for DcmStorageSCU */

#include "dcmtk/dcmjpeg/djdecode.h"  /* for JPEG decoders */
#include "dcmtk/dcmjpls/djdecode.h"  /* for JPEG-LS decoders */
#include "dcmtk/dcmdata/dcrledrg.h"  /* for RLE decoder */
#include "dcmtk/dcmjpeg/dipijpeg.h"  /* for dcmimage JPEG plugin */



#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif

StoreAsyncWorker::StoreAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
    ns::registerCodecs();

    m_acse_timeout = 60;
    m_dimse_timeout = 60;
    m_sourceDirectory = "";
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

    // DcmXfer netTransPropose = in.netTransferPropose.empty() ? DcmXfer(EXS_Unknown) : DcmXfer(in.netTransferPropose.c_str());
    // DCMNET_INFO("proposed network transfer syntax for outgoing associations: " << netTransPropose.getXferName());
    // m_networkTransferSyntax = netTransPropose.getXfer();

    bool success = sendStoreRequest(in.target.aet.c_str(), in.target.ip.c_str(), OFstatic_cast(Uint16, std::stoi(in.target.port)), in.source.aet.c_str() );

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

bool StoreAsyncWorker::sendStoreRequest(const OFString& peerTitle, const OFString& peerIP, Uint16 peerPort, const OFString& ourTitle)
{
    bool m_checkUIDValues = false;

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        DCMNET_WARN("no data dictionary loaded, check environment variable: " << DCM_DICT_ENVIRONMENT_VARIABLE);
    }

    OFList<OFFilename> inputFiles;

    OFList<OFFilename> fileNameList;     // list of files to transfer to SCP
    OFList<OFString> sopClassUIDList;    // the list of SOP classes
    OFList<OFString> sopInstanceUIDList; // the list of SOP instances

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



    DcmStorageSCU storageSCU;
    OFCondition status;
    unsigned long numInvalidFiles = 0;

    /* set parameters used for processing the input files */
    storageSCU.setReadFromDICOMDIRMode(OFFalse);
    storageSCU.setHaltOnInvalidFileMode(OFFalse);

    DCMNET_INFO("checking input files ...");
    /* iterate over all input filenames */
    const char* currentFilename = NULL;
    OFListIterator(OFFilename) if_iter = inputFiles.begin();
    OFListIterator(OFFilename) if_last = inputFiles.end();
    while (if_iter != if_last)
    {
        const OFFilename& currentFilename = (*if_iter);
        const char* filename = currentFilename.getCharPointer();
        /* and add them to the list of instances to be transmitted */
        status = storageSCU.addDicomFile(currentFilename, ERM_fileOnly, m_checkUIDValues);
        if (status.bad())
        {
            /* check for empty filename */
            if (strlen(filename) == 0)
                filename = "<empty string>";
            DCMNET_ERROR("bad DICOM file: " << filename << ": " << status.text() << ", ignoring file");
            ++numInvalidFiles;
        }
        ++if_iter;
    }

    /* check whether there are any valid input files */
    if (storageSCU.getNumberOfSOPInstances() == 0)
    {
        DCMNET_FATAL("no valid input files to be processed");
        return false;
    }


    DCMNET_INFO("in total, there are " << storageSCU.getNumberOfSOPInstances()
        << " SOP instances to be sent, " << numInvalidFiles << " invalid files are ignored");

    /* set network parameters */
    storageSCU.setPeerHostName(peerIP);
    storageSCU.setPeerPort(OFstatic_cast(Uint16, peerPort));
    storageSCU.setPeerAETitle(peerTitle);
    storageSCU.setAETitle(ourTitle);
    storageSCU.setMaxReceivePDULength(OFstatic_cast(Uint32, ASC_DEFAULTMAXPDU));
    storageSCU.setACSETimeout(OFstatic_cast(Uint32, m_acse_timeout));
    storageSCU.setDIMSETimeout(OFstatic_cast(Uint32, m_dimse_timeout));
    storageSCU.setDIMSEBlockingMode(DIMSE_BLOCKING);
    storageSCU.setVerbosePCMode(OFTrue);
    storageSCU.setDatasetConversionMode(OFTrue);
    storageSCU.setDecompressionMode(DcmStorageSCU::DM_losslessOnly);
    storageSCU.setHaltOnUnsuccessfulStoreMode(OFFalse);
    storageSCU.setAllowIllegalProposalMode(OFTrue);


    /* add presentation contexts to be negotiated (if there are still any) */
    while ((status = storageSCU.addPresentationContexts()).good())
    {
        DCMNET_INFO("initializing network ...");
        /* initialize network */
        status = storageSCU.initNetwork();
        if (status.bad())
        {
            DCMNET_ERROR("cannot initialize network: " << status.text());
            return false;
        }
        DCMNET_INFO("negotiating network association ...");
        /* negotiate network association with peer */
        status = storageSCU.negotiateAssociation();
        if (status.bad())
        {
            // check whether we can continue with a new association
            if (status == NET_EC_NoAcceptablePresentationContexts)
            {
                DCMNET_ERROR("cannot negotiate network association: " << status.text());
                // check whether there are any SOP instances to be sent
                const size_t numToBeSent = storageSCU.getNumberOfSOPInstancesToBeSent();
                if (numToBeSent > 0)
                {
                    DCMNET_WARN("trying to continue with a new association "
                        << "in order to send the remaining " << numToBeSent << " SOP instances");
                }
            }
            else {
                DCMNET_ERROR("cannot negotiate network association: " << status.text());
                return false;
            }
        }
        if (status.good())
        {
            DCMNET_INFO("sending SOP instances ...");
            /* send SOP instances to be transferred */
            status = storageSCU.sendSOPInstances();
            if (status.bad())
            {
                DCMNET_ERROR("cannot send SOP instance: " << status.text());
                // handle certain error conditions (initiated by the communication peer)
                if (status == DUL_PEERREQUESTEDRELEASE)
                {
                    // peer requested release (aborting)
                    storageSCU.closeAssociation(DCMSCU_PEER_REQUESTED_RELEASE);
                }
                else if (status == DUL_PEERABORTEDASSOCIATION)
                {
                    // peer aborted the association
                    storageSCU.closeAssociation(DCMSCU_PEER_ABORTED_ASSOCIATION);
                }
                return false;
            }
        }
        /* close current network association */
        storageSCU.releaseAssociation();
    }

    /* if anything went wrong, report it to the logger */
    if (status.bad() && (status != NET_EC_NoPresentationContextsDefined))
    {
        DCMNET_ERROR("cannot add presentation contexts: " << status.text());
        return false;
    }

    /* make sure that everything is cleaned up properly */
    return true;
}
