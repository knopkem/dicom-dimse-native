#include "ParseAsyncWorker.h"

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

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

ParseAsyncWorker::ParseAsyncWorker(std::string data, Function &callback)
    : BaseAsyncWorker(data, callback) {
    ns::registerCodecs();
}

void ParseAsyncWorker::Execute(const ExecutionProgress &progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    EnableVerboseLogging(in.verbose);

    OFFilename ifname(in.sourcePath.c_str());
    DcmFileFormat dfile;
    OFCondition status = dfile.loadFile(ifname, EXS_Unknown, EGL_noChange, DCM_MaxReadLength, ERM_autoDetect);
    if (status.bad()) {
        SetErrorJson("Invalid source path set, no DICOM files found");
        return;
    }
    DcmDataset *dset = dfile.getDataset();
    std::ostringstream stream;
    dset->writeJson(stream, DcmJsonFormatPretty());
    _jsonOutput = stream.str();
}
