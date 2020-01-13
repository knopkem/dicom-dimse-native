/*
Copyright 2005 - 2019 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file overlayImpl.cpp
    \brief Implementation of the class overlay.

*/

#include "overlayImpl.h"
#include "exceptionImpl.h"
#include "imageImpl.h"
#include "memoryImpl.h"
#include "bufferImpl.h"
#include "dicomNativeImageCodecImpl.h"
#include "memoryStreamImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include <locale>
#include <codecvt>
#include <cstring>

namespace imebra
{

namespace implementation
{

overlay::overlay(
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t firstFrame,
        std::uint32_t framesCount,
        std::int32_t zeroBasedOriginX,
        std::int32_t zeroBasedOriginY,
        overlayType_t type,
        const std::string& subType,
        const std::wstring& label,
        const std::wstring& description,
        std::uint32_t roiArea,
        bool roiAreaPresent,
        double roiMean,
        bool roiMeanPresent,
        double roiStandardDeviation,
        bool roiStandardDeviationPresent,
        const std::shared_ptr<buffer>& pBuffer):
    m_width(width), m_height(height),
    m_firstFrame(firstFrame), m_framesCount(framesCount),
    m_originX(zeroBasedOriginX), m_originY(zeroBasedOriginY),
    m_type(type), m_subType(subType),
    m_label(label), m_description(description),
    m_roiArea(roiArea), m_roiAreaPresent(roiAreaPresent),
    m_roiMean(roiMean), m_roiMeanPresent(roiMeanPresent),
    m_roiStandardDeviation(roiStandardDeviation), m_roiStandardDeviationPresent(roiStandardDeviationPresent),
    m_pBuffer(pBuffer)
{
}


overlay::overlay(
        overlayType_t type,
        const std::string& subType,
        std::uint32_t firstFrame,
        std::int32_t zeroBasedOriginX, std::int32_t zeroBasedOriginY,
        const std::wstring& label,
        const std::wstring& description):
    m_width(0), m_height(0),
    m_firstFrame(firstFrame), m_framesCount(0),
    m_originX(zeroBasedOriginX), m_originY(zeroBasedOriginY),
    m_type(type), m_subType(subType),
    m_label(label), m_description(description),
    m_roiArea(0), m_roiAreaPresent(false),
    m_roiMean(0.0), m_roiMeanPresent(false),
    m_roiStandardDeviation(0.0), m_roiStandardDeviationPresent(false),
    m_pBuffer(std::make_shared<buffer>(std::shared_ptr<charsetsList_t>(), streamController::lowByteEndian))
{
}


overlay::overlay(
        overlayType_t type,
        const std::string& subType,
        std::uint32_t firstFrame,
        std::int32_t zeroBasedOriginX, std::int32_t zeroBasedOriginY,
        const std::string& label,
        const std::string& description):
    m_width(0), m_height(0),
    m_firstFrame(firstFrame), m_framesCount(0),
    m_originX(zeroBasedOriginX), m_originY(zeroBasedOriginY),
    m_type(type), m_subType(subType),
    m_label(std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(label)),
    m_description(std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(description)),
    m_roiArea(0), m_roiAreaPresent(false),
    m_roiMean(0.0), m_roiMeanPresent(false),
    m_roiStandardDeviation(0.0), m_roiStandardDeviationPresent(false),
    m_pBuffer(std::make_shared<buffer>(std::shared_ptr<charsetsList_t>(), streamController::lowByteEndian))
{
}


void overlay::setFirstFrame(std::uint32_t firstFrame)
{
    m_firstFrame = firstFrame;
}

void overlay::setZeroBasedOrigin(std::int32_t x, std::int32_t y)
{
    m_originX = x;
    m_originY = y;
}

void overlay::setOneBasedOrigin(std::int32_t x, std::int32_t y)
{
    m_originX = x - 1;
    m_originY = y - 1;
}

void overlay::setUnicodeLabel(const std::wstring& label)
{
    m_label = label;
}

void overlay::setLabel(const std::string& label)
{
    m_label = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(label);
}

void overlay::setUnicodeDescription(const std::wstring& description)
{
    m_description = description;
}

void overlay::setDescription(const std::string& description)
{
    m_description = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(description);
}

void overlay::setROIArea(std::uint32_t roiArea)
{
    m_roiArea = roiArea;
    m_roiAreaPresent = true;
}

void overlay::setROIMean(double roiMean)
{
    m_roiMean = roiMean;
    m_roiMeanPresent = true;
}

void overlay::setROIStandardDeviation(double roiStandardDeviation)
{
    m_roiStandardDeviation = roiStandardDeviation;
    m_roiStandardDeviationPresent = true;
}

void overlay::setImage(std::uint32_t frame, const std::shared_ptr<const image>& pImage)
{
    IMEBRA_FUNCTION_START();

    if(frame != m_framesCount)
    {
        IMEBRA_THROW(std::logic_error, "Overlays must be stored in order (first frame 0, then frame 1, ...)");
    }

    std::shared_ptr<memory> uncompressedImage(std::make_shared<memory>());
    std::shared_ptr<streamWriter> outputStream;
    std::shared_ptr<memoryStreamOutput> memStream(std::make_shared<memoryStreamOutput>(uncompressedImage));
    outputStream = std::make_shared<streamWriter>(memStream);

    if(frame == 0)
    {
        pImage->getSize(&m_width, &m_height);
    }
    else
    {
        std::uint32_t width, height;
        pImage->getSize(&width, &height);
        if(m_width != width || m_height != height)
        {
            IMEBRA_THROW(DataSetDifferentFormatError, "An image already exists in the overlay and has different attributes");
        }
    }

    // Save the image in the stream
    ///////////////////////////////////////////////////////////
    codecs::dicomNativeImageCodec imageCodec;
    imageCodec.setImage(
        outputStream,
        pImage,
        uidImplicitVRLittleEndian_1_2_840_10008_1_2,
        imageQuality_t::veryHigh,
        1,
        false, false,
        true,
        false);

    outputStream->flushDataBuffer();

    size_t imageSizeBits = codecs::dicomNativeImageCodec::getNativeImageSizeBits(1,
                                                                                  m_width,
                                                                                  m_height,
                                                                                  1,
                                                                                  false,
                                                                                  false);

    if(((frame * imageSizeBits) & 0x7) != 0)
    {
        std::shared_ptr<handlers::readingDataHandlerRaw> dataHandler = m_pBuffer->getReadingDataHandlerRaw(m_dataType);
        size_t newSize = ((frame + 1) * imageSizeBits + 7) / 8;
        std::shared_ptr<handlers::writingDataHandlerRaw> writeDataHandler = m_pBuffer->getWritingDataHandlerRaw(m_dataType, static_cast<std::uint32_t>(newSize));
        memcpy(writeDataHandler->getMemoryBuffer(), dataHandler->getMemoryBuffer(), (frame * imageSizeBits + 7) / 8);
        std::uint8_t* pDestination = writeDataHandler->getMemoryBuffer() + (frame * imageSizeBits) / 8;
        const std::uint8_t* pSource = uncompressedImage->data();
        const size_t offsetBit = (imageSizeBits * frame) & 7;
        for(size_t copyBits(0); copyBits < imageSizeBits; copyBits += 8)
        {
            *pDestination = *pDestination | static_cast<std::uint8_t>(*pSource << offsetBit);
            ++pDestination;
            *pDestination = static_cast<std::uint8_t>(*pSource++ >> (8 - offsetBit));
        }
    }
    else
    {
        m_pBuffer->appendMemory(uncompressedImage);
    }

    m_framesCount = frame + 1;

    IMEBRA_FUNCTION_END();
}

std::uint32_t overlay::getFirstFrame() const
{
    return m_firstFrame;
}

std::uint32_t overlay::getFramesCount() const
{
    return m_framesCount;
}

std::int32_t overlay::getZeroBasedOriginX() const
{
    return m_originX;
}

std::int32_t overlay::getZeroBasedOriginY() const
{
    return m_originY;
}

std::int32_t overlay::getOneBasedOriginX() const
{
    return m_originX + 1;
}

std::int32_t overlay::getOneBasedOriginY() const
{
    return m_originY + 1;
}

overlayType_t overlay::getType() const
{
    return m_type;
}

std::string overlay::getSubType() const
{
    return m_subType;
}

std::wstring overlay::getUnicodeLabel() const
{
    return m_label;
}

std::string overlay::getLabel() const
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(m_label);
}

std::wstring overlay::getUnicodeDescription() const
{
    return m_description;
}

std::string overlay::getDescription() const
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(m_description);
}

std::uint32_t overlay::getROIArea() const
{
    return m_roiArea;
}

bool overlay::getROIAreaPresent() const
{
    return m_roiAreaPresent;
}

double overlay::getROIMean() const
{
    return m_roiMean;
}

bool overlay::getROIMeanPresent() const
{
    return m_roiMeanPresent;
}

double overlay::getROIStandardDeviation() const
{
    return m_roiStandardDeviation;
}

bool overlay::getROIStandardDeviationPresent() const
{
    return m_roiStandardDeviationPresent;
}

std::shared_ptr<image> overlay::getImage(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    if(frameNumber >= m_framesCount)
    {
        IMEBRA_THROW(DataSetImageDoesntExistError, "The requested overlay bitmap does not exist");
    }

    size_t imageSizeBits = codecs::dicomNativeImageCodec::getNativeImageSizeBits(1,
                                                                                  m_width,
                                                                                  m_height,
                                                                                  1,
                                                                                  false,
                                                                                  false);
    size_t startPosition = (imageSizeBits * frameNumber) / 8;
    size_t endPosition = ((imageSizeBits * (frameNumber + 1)) + 7) / 8;


    std::shared_ptr<streamReader> pStream = m_pBuffer->getStreamReader();
    pStream->seek(startPosition);

    std::shared_ptr<streamReader> pImageStream;

    if(((imageSizeBits * frameNumber) & 7) != 0)
    {
        memory originalData(endPosition - startPosition);
        pStream->read(originalData.data(), originalData.size());

        std::shared_ptr<memory> pTemporaryMemory(std::make_shared<memory>((imageSizeBits + 7) / 8));

        const std::uint8_t* scanData = originalData.data();
        std::uint8_t* destination = pTemporaryMemory->data();
        const size_t offsetBit = (imageSizeBits * frameNumber) & 7;
        for(size_t copyBits(0); copyBits < imageSizeBits; copyBits += 8)
        {
            std::uint8_t value = static_cast<std::uint8_t>(*scanData++ >> offsetBit);
            value = value | static_cast<std::uint8_t>(*scanData << (8 - offsetBit));
            *destination++ = value;
        }
        std::shared_ptr<memoryStreamInput> pTemporaryMemoryStream = std::make_shared<memoryStreamInput>(pTemporaryMemory);
        pImageStream = std::make_shared<streamReader>(pTemporaryMemoryStream);
    }
    else
    {
        pImageStream = pStream->getReader((imageSizeBits + 7) / 8);
    }

    codecs::dicomNativeImageCodec imageCodec;
    return imageCodec.getImage(uidImplicitVRLittleEndian_1_2_840_10008_1_2,
                        "MONOCHROME2",
                        1,
                        m_width,
                        m_height,
                        false,
                        false,
                        true,
                        false,
                        1,
                        1,
                        0,
                        pImageStream);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<buffer> overlay::getBuffer() const
{
    return m_pBuffer;
}


} // namespace implementation

} // namespace imebra
