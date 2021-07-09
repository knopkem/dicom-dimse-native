#pragma once

#include "BaseAsyncWorker.h"


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/ofstd/offile.h"


using namespace Napi;

class DcmDataset;
class DcmItem;


class StoreAsyncWorker : public BaseAsyncWorker
{
    public:
        StoreAsyncWorker(std::string data, Function &callback);

        void Execute(const ExecutionProgress& progress);

    protected:
        bool setScanDirectory(const OFFilename &dir);

        bool sendStoreRequest(const OFString& peerTitle, const OFString& peerIP, Uint16 peerPort,  const OFString& ourTitle);

private:

        OFFilename            m_sourceDirectory;
        unsigned long         m_acse_timeout;
        unsigned long         m_dimse_timeout;
};
