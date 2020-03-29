#include "FindAsyncWorker.h"

#include "dcmtk/config/osconfig.h"      /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcostrmz.h"     /* for dcmZlibCompressionLevel */

#ifdef WITH_ZLIB
#include <zlib.h>                       /* for zlibVersion() */
#endif
#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
#include "dcmtk/ofstd/ofchrenc.h"       /* for OFCharacterEncoding */
#endif

#define APPLICATIONTITLE        "FINDSCU"
#define PEERAPPLICATIONTITLE    "ANY-SCP"

#define SHORTCOL 4
#define LONGCOL 20

#include "json.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <iomanip>

using json = nlohmann::json;

namespace {

    std::string str_toupper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), 
                    [](unsigned char c){ return std::toupper(c); }
                    );
        return s;
    }

    std::string int_to_hex( uint16_t i )
    {
    std::stringstream stream;
    stream  << std::setfill ('0') << std::setw(sizeof(uint16_t)*2) 
            << std::hex << i;
    return str_toupper(stream.str());
    }
}

FindAsyncWorker::FindAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void FindAsyncWorker::Execute(const ExecutionProgress& progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    if (in.tags.empty()) {
        SetErrorJson("Tags not set");
        return;
    }

    if (!in.source.valid()) {
        SetErrorJson("Source not set");
        return;
    }

    if (!in.target.valid()) {
        SetErrorJson("Target not set");
        return;
    }
    
    OFList<OFString>      fileNameList;
    OFBool                opt_abortAssociation = OFFalse;
    const char *          opt_abstractSyntax = UID_FINDModalityWorklistInformationModel;
    int                   opt_acse_timeout = 30;
    T_DIMSE_BlockingMode  opt_blockMode = DIMSE_BLOCKING;
    OFCmdSignedInt        opt_cancelAfterNResponses = -1;
    int                   opt_dimse_timeout = 0;
    int                   opt_outputResponsesToLogger = 0;
    DcmFindSCUExtractMode opt_extractResponses = FEM_none;
    OFString              opt_extractXMLFilename;
    OFString              opt_outputDirectory = ".";
    OFCmdUnsignedInt      opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
    E_TransferSyntax      opt_networkTransferSyntax = EXS_Unknown;
    const char *          opt_ourTitle = APPLICATIONTITLE;
    const char *          opt_peer;
    const char *          opt_peerTitle = PEERAPPLICATIONTITLE;
    OFCmdUnsignedInt      opt_port = 104;
    OFCmdUnsignedInt      opt_repeatCount = 1;
    OFList<OFString>      overrideKeys;

#ifdef WITH_ZLIB
    OFCmdUnsignedInt      opt_compressionLevel = 0;
#endif

    /*
    ** By default. don't let "dcmdata" remove trailing padding or
    ** perform other manipulations. We want to see the real data.
    */
    OFBool                opt_automaticDataCorrection = OFFalse;

    OFStandard::initializeNetwork();

   // enabled or disable removal of trailing padding
    dcmEnableAutomaticInputDataCorrection.set(opt_automaticDataCorrection);

    // declare findSCU handler and initialize network
    DcmFindSCU findscu;
    OFCondition cond = findscu.initializeNetwork(opt_acse_timeout);
    if (cond.bad()) {
        
    }


    // do the main work: negotiate network association, perform C-FIND transaction,
    // process results, and finally tear down the association.
    cond = findscu.performQuery(
      opt_peer,
      opt_port,
      opt_ourTitle,
      opt_peerTitle,
      opt_abstractSyntax,
      opt_networkTransferSyntax,
      opt_blockMode,
      opt_dimse_timeout,
      opt_maxReceivePDULength,
      false,
      opt_abortAssociation,
      opt_repeatCount,
      opt_extractResponses,
      opt_cancelAfterNResponses,
      &overrideKeys,
      NULL, /* we want to use the default callback */
      &fileNameList,
      opt_outputDirectory.c_str(),
      opt_extractXMLFilename.c_str());

    // make sure that an appropriate exit code is returned
    int exitCode = cond.good() ? 0 : 2;

    // destroy network structure
    cond = findscu.dropNetwork();
    if (cond.bad()) {
        
    }

    OFStandard::shutdownNetwork();



}
