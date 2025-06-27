#include "BaseAsyncWorker.h"

#include "Utils.h"

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/oflog/spi/logevent.h"
#include "dcmtk/oflog/appender.h"

#include <vector>

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

class BufferAppender : public dcmtk::log4cplus::Appender {
public:
    BufferAppender() {}

    ~BufferAppender() {
          destructorImpl ();
    }

    virtual void close() {}

    std::vector<std::string> StringBuffer;

protected:
    virtual void append(const dcmtk::log4cplus::spi::InternalLoggingEvent& ev) {
        dcmtk::log4cplus::tostringstream oss;
        layout->formatAndAppend(oss, ev);

        StringBuffer.push_back(DCMTK_LOG4CPLUS_TSTRING_TO_STRING(oss.str()));
    }
};


BaseAsyncWorker::BaseAsyncWorker(std::string data, Function &callback) : AsyncProgressQueueWorker<char>(callback),
                                                                           _input(data)
{
    //add the custom appender
    // using namespace dcmtk::log4cplus;
    // Logger rootLogger = Logger::getRoot();
    // this->_appender = new BufferAppender();
    // rootLogger.addAppender(this->_appender);

    // disable verbose logging
    OFLog::configure(OFLogger::WARN_LOG_LEVEL);
}

BaseAsyncWorker::~BaseAsyncWorker() 
{
    // using namespace dcmtk::log4cplus;
    // Logger rootLogger = Logger::getRoot();
    // rootLogger.removeAppender(this->_appender);
}

void BaseAsyncWorker::OnOK()
{
      HandleScope scope(Env());
      std::string msg = ns::createJsonResponse(ns::SUCCESS, "request succeeded", _jsonOutput);
      if (_error.length() > 0) msg = _error;
      String o = String::New(Env(), msg);
      Callback().Call({o});
}

void BaseAsyncWorker::OnProgress(const char *data, size_t size)
{
      HandleScope scope(Env());
      String o = String::New(Env(), data, size);
      Callback().Call({o});
}

void BaseAsyncWorker::SetErrorJson(const std::string& message)
{
       _error =  ns::createJsonResponse(ns::FAILURE, message);
       // SetError(sError);
}

void BaseAsyncWorker::SendInfo(const std::string& msg, const ExecutionProgress& progress, ns::eStatus status)
{
      std::string msg2 = ns::createJsonResponse(status, msg);
      progress.Send(msg2.c_str(), msg2.length());
}


void BaseAsyncWorker::EnableVerboseLogging(bool enabled)
{
    if (enabled) {
        OFLog::configure(OFLogger::DEBUG_LOG_LEVEL);
    } else {
        OFLog::configure(OFLogger::WARN_LOG_LEVEL);
    }
}

void BaseAsyncWorker::applyOverrideKeys(DcmDataset *dataset, const OFList<OFString> &overrideKeys)
{
    /* replace specific keys by those in overrideKeys */
    OFListConstIterator(OFString) path = overrideKeys.begin();
    OFListConstIterator(OFString) endOfList = overrideKeys.end();
    DcmPathProcessor proc;
    proc.setItemWildcardSupport(OFFalse);
    proc.checkPrivateReservations(OFFalse);
    OFCondition cond;
    while (path != endOfList)
    {
        cond = proc.applyPathWithValue(dataset, *path);
        if (cond.bad())
        {
            std::cerr << "bad override key provided" << std::endl;
        }
        path++;
    }
}

void BaseAsyncWorker::addDefaultTs(OFList<OFString> &syntaxes) 
{
        syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
        syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
        syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
}

void BaseAsyncWorker::prepareTS(E_TransferSyntax ts,  OFList<OFString> &syntaxes)
{
    switch (ts)
    {
    case EXS_LittleEndianImplicit:
        /* we only support Little Endian Implicit */
        syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
        break;
    case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit */
        syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
        syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
        syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
        break;
    case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit */
        syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
        syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
        syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
        break;
    case EXS_DeflatedLittleEndianExplicit:
        /* we prefer Deflated Little Endian Explicit */
        syntaxes.push_back(UID_DeflatedExplicitVRLittleEndianTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
    case EXS_JPEGProcess14SV1:
        /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
        syntaxes.push_back(UID_JPEGProcess14SV1TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_JPEGProcess1:
        /* we prefer JPEGBaseline (default lossy for 8 bit images) */
        syntaxes.push_back(UID_JPEGProcess1TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_JPEGProcess2_4:
        /* we prefer JPEGExtended (default lossy for 12 bit images) */
        syntaxes.push_back(UID_JPEGProcess2_4TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_JPEG2000LosslessOnly:
        /* we prefer JPEG 2000 lossless */
        syntaxes.push_back(UID_JPEG2000LosslessOnlyTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_JPEG2000:
        /* we prefer JPEG 2000 lossy or lossless */
        syntaxes.push_back(UID_JPEG2000TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_JPEGLSLossless:
        /* we prefer JPEG-LS Lossless */
        syntaxes.push_back(UID_JPEGLSLosslessTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_JPEGLSLossy:
        /* we prefer JPEG-LS Lossy */
        syntaxes.push_back(UID_JPEGLSLossyTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG2MainProfileAtMainLevel:
        /* we prefer MPEG2 MP@ML */
        syntaxes.push_back(UID_MPEG2MainProfileAtMainLevelTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG2MainProfileAtHighLevel:
        /* we prefer MPEG2 MP@HL */
        syntaxes.push_back(UID_MPEG2MainProfileAtHighLevelTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG4HighProfileLevel4_1:
        /* we prefer MPEG4 HP/L4.1 */
        syntaxes.push_back(UID_MPEG4HighProfileLevel4_1TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG4BDcompatibleHighProfileLevel4_1:
        /* we prefer MPEG4 BD HP/L4.1 */
        syntaxes.push_back(UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG4HighProfileLevel4_2_For2DVideo:
        /* we prefer MPEG4 HP/L4.2 for 2D Videos */
        syntaxes.push_back(UID_MPEG4HighProfileLevel4_2_For2DVideoTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG4HighProfileLevel4_2_For3DVideo:
        /* we prefer MPEG4 HP/L4.2 for 3D Videos */
        syntaxes.push_back(UID_MPEG4HighProfileLevel4_2_For3DVideoTransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_MPEG4StereoHighProfileLevel4_2:
        /* we prefer MPEG4 Stereo HP/L4.2 */
        syntaxes.push_back(UID_MPEG4StereoHighProfileLevel4_2TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_HEVCMainProfileLevel5_1:
        /* we prefer HEVC/H.265 Main Profile/L5.1 */
        syntaxes.push_back(UID_HEVCMainProfileLevel5_1TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_HEVCMain10ProfileLevel5_1:
        /* we prefer HEVC/H.265 Main 10 Profile/L5.1 */
        syntaxes.push_back(UID_HEVCMain10ProfileLevel5_1TransferSyntax);
        this.addDefaultTs(syntaxes);
        break;
  case EXS_RLELossless:
        /* we prefer RLE Lossless */
        syntaxes.push_back(UID_RLELosslessTransferSyntax;
        this.addDefaultTs(syntaxes);
        break;
#endif
    default:
        DcmXfer xfer(ts);
        if (xfer.isEncapsulated())
        {
            syntaxes.push_back(xfer.getXferID());
        }
        if (gLocalByteOrder == EBO_LittleEndian) /* defined in dcxfer.h */
        {
            syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
        }
        else
        {
            syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
            syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
        }
        syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
        break;
    }
}