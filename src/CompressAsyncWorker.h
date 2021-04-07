#pragma once

#include "BaseAsyncWorker.h"

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/ofstd/offile.h"
#include "dcmtk/dcmdata/dcxfer.h"
#include "dcmtk/dcmdata/dcfilefo.h"

using namespace Napi;

class CompressAsyncWorker : public BaseAsyncWorker
{
    public:
        CompressAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

    protected:
        OFBool isDicomFile( const OFFilename &fname );
        OFBool CompressAsyncWorker::recompress(const OFFilename& infile, const OFString& storePath, E_TransferSyntax prefXfer);
};
