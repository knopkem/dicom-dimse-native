/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file codecFactory.cpp
    \brief Implementation of the codecFactory class.

*/

#include "codecFactoryImpl.h"
#include "configurationImpl.h"
#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamCodecImpl.h"
#include "imageCodecImpl.h"
#include "jpegStreamCodecImpl.h"
#include "dicomStreamCodecImpl.h"

#include "jpegImageCodecImpl.h"
#include "dicomNativeImageCodecImpl.h"
#include "dicomRLEImageCodecImpl.h"

#ifdef JPEG2000
#include "jpeg2000ImageCodecImpl.h"
#endif

#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Force the creation of the codec factory before main()
//  starts.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static codecFactory::forceCodecFactoryCreation forceCreation;


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
codecFactory::codecFactory(): m_maximumImageWidth(MAXIMUM_IMAGE_WIDTH), m_maximumImageHeight(MAXIMUM_IMAGE_HEIGHT)
{
    IMEBRA_FUNCTION_START();

    registerStreamCodec(codecType_t::dicom, std::make_shared<dicomStreamCodec>());
    registerStreamCodec(codecType_t::jpeg, std::make_shared<jpegStreamCodec>());

    registerImageCodec(std::make_shared<jpegImageCodec>());
    registerImageCodec(std::make_shared<dicomNativeImageCodec>());
    registerImageCodec(std::make_shared<dicomRLEImageCodec>());

#ifdef JPEG2000
    registerImageCodec(std::make_shared<jpeg2000ImageCodec>());
#endif

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a codec
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void codecFactory::registerStreamCodec(codecType_t codecType, std::shared_ptr<streamCodec> pCodec)
{
    IMEBRA_FUNCTION_START();

    m_streamCodecs[codecType] = pCodec;

    IMEBRA_FUNCTION_END();
}

void codecFactory::registerImageCodec(std::shared_ptr<imageCodec> pCodec)
{
    IMEBRA_FUNCTION_START();

    m_imageCodecs.push_back(pCodec);

    IMEBRA_FUNCTION_END();

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a codec that can handle the specified
//  transfer syntax
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<const imageCodec> codecFactory::getImageCodec(const std::string& transferSyntax)
{
    IMEBRA_FUNCTION_START();

    for(std::list<std::shared_ptr<const imageCodec> >::iterator scanCodecs(m_imageCodecs.begin()); scanCodecs != m_imageCodecs.end(); ++scanCodecs)
    {
        if((*scanCodecs)->canHandleTransferSyntax(transferSyntax))
        {
            return *scanCodecs;
        }
    }

    IMEBRA_THROW(DataSetUnknownTransferSyntaxError, "None of the codecs support the specified transfer syntax");

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a codec that can handle the specified
//  transfer syntax
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<const streamCodec> codecFactory::getStreamCodec(codecType_t codecType)
{
    IMEBRA_FUNCTION_START();

    return m_streamCodecs[codecType];

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the only instance of the codec factory
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<codecFactory> codecFactory::getCodecFactory()
{
    IMEBRA_FUNCTION_START();

    // Violation to requirement REQ_MAKE_SHARED due to protected constructor
    static std::shared_ptr<codecFactory> m_codecFactory(new codecFactory());

    return m_codecFactory;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Load the data from the specified stream and build a
//  dicomSet structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> codecFactory::load(std::shared_ptr<streamReader> pStream, std::uint32_t maxSizeBufferLoad /* = 0xffffffff */)
{
    IMEBRA_FUNCTION_START();

    // This hack is necessary to keep compatibility across
    // the imebra 4.X series.
    // Imebra 5.X should have a base non-seekable object with
    // a derived seekable one.
    ///////////////////////////////////////////////////////////
    if(maxSizeBufferLoad != 0xffffffff && !pStream->seekable())
    {
        IMEBRA_THROW(std::logic_error, "The codec factory supports only file and memory streams")
    }

    std::uint8_t buffer[IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE];

    size_t startPosition = pStream->position();
    size_t bufferSize(pStream->readSome(buffer, sizeof(buffer)));

    // Copy the list of codecs in a local list so we don't have
    //  to lock the object for a long time
    ///////////////////////////////////////////////////////////
    for(std::map<codecType_t, std::shared_ptr<const streamCodec> >::const_iterator scanCodecs(m_streamCodecs.begin()); scanCodecs != m_streamCodecs.end(); ++scanCodecs)
    {
        std::shared_ptr<streamReader> pTempReader;
        if(pStream->getVirtualLength() != 0)
        {
            pTempReader = std::make_shared<streamReader>(pStream->getControlledStream(),
                                                         startPosition,
                                                         pStream->getVirtualLength(),
                                                         buffer,
                                                         bufferSize);
        }
        else
        {
            pTempReader = std::make_shared<streamReader>(pStream->getControlledStream(),
                                                         startPosition,
                                                         buffer,
                                                         bufferSize);
        }

        try
        {
            std::shared_ptr<dataSet> pDataSet(scanCodecs->second->read(pTempReader, maxSizeBufferLoad));
            return pDataSet;
        }
        catch(CodecWrongFormatError&)
        {
            exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().getMessage(); // Reset the messages stack
            continue;
        }
    }

    IMEBRA_THROW(CodecWrongFormatError, "none of the codecs recognized the file format");

    IMEBRA_FUNCTION_END();
}


void codecFactory::setMaximumImageSize(const uint32_t maximumWidth, const uint32_t maximumHeight)
{
    m_maximumImageWidth = maximumWidth;
    m_maximumImageHeight = maximumHeight;
}


std::uint32_t codecFactory::getMaximumImageWidth()
{
    return m_maximumImageWidth;
}

std::uint32_t codecFactory::getMaximumImageHeight()
{
    return m_maximumImageHeight;
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

