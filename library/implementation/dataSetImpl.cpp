/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataSet.cpp
    \brief Implementation of the class dataSet.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryStreamImpl.h"
#include "dataSetImpl.h"
#include "dataHandlerNumericImpl.h"
#include "dicomDictImpl.h"
#include "codecFactoryImpl.h"
#include "streamCodecImpl.h"
#include "imageCodecImpl.h"
#include "imageImpl.h"
#include "LUTImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "transformsChainImpl.h"
#include "transformHighBitImpl.h"
#include "modalityVOILUTImpl.h"
#include "bufferImpl.h"
#include "dateImpl.h"
#include "ageImpl.h"
#include "VOIDescriptionImpl.h"
#include "overlayImpl.h"
#include "dicomNativeImageCodecImpl.h"
#include "codecFactoryImpl.h"
#include <iostream>
#include <string.h>
#include <limits>


namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataSet
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

dataSet::dataSet(const std::shared_ptr<charsetsList_t>& pCharsetsList): m_itemOffset(0), m_pCharsetsList(pCharsetsList)
{
}

dataSet::dataSet(const std::string& transferSyntax, const std::shared_ptr<charsetsList_t>& pCharsetsList):
    m_itemOffset(0), m_pCharsetsList(pCharsetsList)
{
    setString(0x0002, 0x0, 0x0010, 0, transferSyntax);
}

dataSet::dataSet(const std::string& transferSyntax, const charsetsList_t& charsetsList):
    m_itemOffset(0), m_pCharsetsList(std::make_shared<charsetsList_t>(charsetsList))
{
    setString(0x0002, 0x0, 0x0010, 0, transferSyntax);

    // Set the charsets list
    if(!charsetsList.empty())
    {
        std::shared_ptr<handlers::writingDataHandler> charsetsWriter(getWritingDataHandler(0x0008, 0, 0x0005, 0));
        size_t index(0);
        for(const std::string& charset: charsetsList)
        {
            charsetsWriter->setString(index++, charset);
        }
    }
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<data> dataSet::getTag(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    tGroups::const_iterator findGroup(m_groups.find(groupId));
    if(findGroup == m_groups.end())
    {
        IMEBRA_THROW(MissingGroupError, "The requested group is missing");
    }

    if(findGroup->second.size() <= order)
    {
        IMEBRA_THROW(MissingGroupError, "The requested group is missing");
    }

    const tTags& tagsMap = findGroup->second.at(order);
    tTags::const_iterator findTag(tagsMap.find(tagId));
    if(findTag == tagsMap.end())
    {
        IMEBRA_THROW(MissingTagError, "The requested tag is missing");
    }
    return findTag->second;

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<data> dataSet::getTagCreate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if(m_groups[groupId].size() <= order)
    {
        m_groups[groupId].resize(order + 1);
    }

    if(m_groups[groupId][order][tagId] == nullptr)
    {
        m_groups[groupId][order][tagId] = std::make_shared<data>(tagVR, m_pCharsetsList);
    }

    return m_groups[groupId][order][tagId];

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<data> dataSet::getTagCreate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId)
{
    IMEBRA_FUNCTION_START();

    return getTagCreate(groupId, order, tagId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

bool dataSet::bufferExists(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getTag(groupId, order, tagId)->bufferExists(bufferId);
    }
    catch(const MissingDataElementError&)
    {
        return false;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the image from the structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<image> dataSet::getImage(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    // Retrieve the transfer syntax
    ///////////////////////////////////////////////////////////
    std::string transferSyntax = getString(0x0002, 0x0, 0x0010, 0, 0, "1.2.840.10008.1.2");

    // Get the right codec
    ///////////////////////////////////////////////////////////
    std::shared_ptr<const codecs::imageCodec> pCodec(codecs::codecFactory::getCodecFactory()->getImageCodec(transferSyntax));

    try
    {
        std::string colorSpace = getString(0x0028, 0x0, 0x0004, 0, 0);
        std::uint32_t channelsNumber = getUnsignedLong(0x0028, 0x0, 0x0002, 0, 0, 1);
        if(colorSpace.empty() && (channelsNumber == 0 || channelsNumber == 1))
        {
            colorSpace = "MONOCHROME2";
            channelsNumber = 1;
        }
        else if(colorSpace.empty() && channelsNumber == 3)
        {
            colorSpace = "RGB";
        }
        std::uint32_t requiredChannels = transforms::colorTransforms::colorTransformsFactory::getNumberOfChannels(colorSpace);
        if(requiredChannels == 0)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Unrecognized color space " << colorSpace);
        }
        if(requiredChannels != channelsNumber)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "The color space " << colorSpace << " requires " << requiredChannels << " but the dataset declares " << channelsNumber << " channels");
        }
        std::uint32_t imageWidth = getUnsignedLong(0x0028, 0x0, 0x0011, 0, 0);
        std::uint32_t imageHeight = getUnsignedLong(0x0028, 0x0, 0x0010, 0, 0);
        if(
                imageWidth > codecs::codecFactory::getCodecFactory()->getMaximumImageWidth() ||
                imageHeight > codecs::codecFactory::getCodecFactory()->getMaximumImageHeight())
        {
            IMEBRA_THROW(CodecImageTooBigError, "The factory settings prevented the loading of this image. Consider using codecFactory::setMaximumImageSize() to modify the settings");
        }
        if((imageWidth == 0) || (imageHeight == 0))
        {
            IMEBRA_THROW(CodecCorruptedFileError, "The size tags are not available");
        }
        bool bInterleaved(getUnsignedLong(0x0028, 0x0, 0x0006, 0, 0, pCodec->defaultInterleaved() ? 0 : 1u) == 0);
        bool b2Complement(getUnsignedLong(0x0028, 0x0, 0x0103, 0, 0, 0) != 0);
        std::uint8_t allocatedBits = static_cast<std::uint8_t>(getUnsignedLong(0x0028, 0x0, 0x0100, 0, 0));
        std::uint8_t storedBits = static_cast<std::uint8_t>(getUnsignedLong(0x0028, 0x0, 0x0101, 0, 0));
        std::uint8_t highBit = static_cast<std::uint8_t>(getUnsignedLong(0x0028, 0x0, 0x0102, 0, 0));
        if(highBit < storedBits - 1)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "The tag 0028,0102 (high bit) cannot be less than (tag 0028,0101 (stored bit) - 1)");
        }
        bool bSubSampledY = transforms::colorTransforms::colorTransformsFactory::isSubsampledY(colorSpace);
        bool bSubSampledX = transforms::colorTransforms::colorTransformsFactory::isSubsampledX(colorSpace);

        std::shared_ptr<implementation::data> imageTag = getTag(0x7fe0, 0x0, 0x0010);

        // Get the number of frames
        ///////////////////////////////////////////////////////////
        std::uint32_t numberOfFrames = getUnsignedLong(0x0028, 0, 0x0008, 0, 0, 1);

        if(frameNumber >= numberOfFrames)
        {
            IMEBRA_THROW(DataSetImageDoesntExistError, "The requested image doesn't exist");
        }

        // Placeholder for the stream containing the image
        ///////////////////////////////////////////////////////////
        std::shared_ptr<streamReader> imageStream;

        if(pCodec->encapsulated(transferSyntax))
        {
            if(imageTag->bufferExists(1))
            {
                std::uint32_t firstBufferId(0), endBufferId(0);
                size_t totalLength(0);
                if(imageTag->getBufferSize(0) == 0 && numberOfFrames + 1 == imageTag->getBuffersCount())
                {
                    firstBufferId = frameNumber + 1;
                    endBufferId = firstBufferId + 1;
                    totalLength = imageTag->getBufferSize(firstBufferId);
                }
                else
                {
                    totalLength = getFrameBufferIds(frameNumber, &firstBufferId, &endBufferId);
                }
                if(firstBufferId == endBufferId - 1)
                {
                    imageStream = imageTag->getStreamReader(firstBufferId);
                }
                else
                {
                    std::shared_ptr<memory> temporaryMemory(std::make_shared<memory>(totalLength));
                    std::uint8_t* pDest = temporaryMemory->data();
                    for(std::uint32_t scanBuffers = firstBufferId; scanBuffers != endBufferId; ++scanBuffers)
                    {
                        std::shared_ptr<handlers::readingDataHandlerRaw> bufferHandler = imageTag->getReadingDataHandlerRaw(scanBuffers);
                        const std::uint8_t* pSource = bufferHandler->getMemoryBuffer();
                        ::memcpy(pDest, pSource, bufferHandler->getSize());
                        pDest += bufferHandler->getSize();
                    }
                    std::shared_ptr<baseStreamInput> compositeStream(std::make_shared<memoryStreamInput>(temporaryMemory));
                    imageStream = std::make_shared<streamReader>(compositeStream);
                }
            }
        }
        else
        {

            size_t imageSizeBits = codecs::dicomNativeImageCodec::getNativeImageSizeBits(allocatedBits,
                                                                                          imageWidth,
                                                                                          imageHeight,
                                                                                          channelsNumber,
                                                                                          bSubSampledX,
                                                                                          bSubSampledY);

            if(((imageSizeBits * frameNumber) & 7) != 0)
            {
                size_t startPosition = (imageSizeBits * frameNumber) / 8;
                size_t endPosition = ((imageSizeBits * (frameNumber + 1)) + 7) / 8;

                std::shared_ptr<streamReader> pTagStream = imageTag->getStreamReader(0);
                pTagStream->seek(startPosition);

                memory originalData(endPosition - startPosition);
                pTagStream->read(originalData.data(), originalData.size());

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
                imageStream = std::make_shared<streamReader>(pTemporaryMemoryStream);
            }
            else
            {
                std::shared_ptr<streamReader> pTagStream = imageTag->getStreamReader(0);
                pTagStream->seek(imageSizeBits * frameNumber / 8);
                imageStream = pTagStream->getReader((imageSizeBits + 7) / 8);
            }
        }

        // If the image cannot be found, then probably we are
        //  handling an old dicom format.
        // Then try to read the image from the next group with
        //  id=0x7fe
        ///////////////////////////////////////////////////////////
        if(imageStream == nullptr)
        {
            imageStream = getStreamReader(0x7fe0, static_cast<std::uint16_t>(frameNumber), 0x0010, 0x0);
        }

        std::shared_ptr<image> pImage;
        pImage = pCodec->getImage(transferSyntax,
                                  colorSpace,
                                  channelsNumber,
                                  imageWidth,
                                  imageHeight,
                                  bSubSampledX,
                                  bSubSampledY,
                                  bInterleaved,
                                  b2Complement,
                                  allocatedBits,
                                  storedBits,
                                  highBit,
                                  imageStream);

        if(pImage->getColorSpace() == "PALETTE COLOR")
        {
            std::shared_ptr<lut> red(std::make_shared<lut>(getReadingDataHandlerNumeric(0x0028, 0x0, 0x1101, 0), getReadingDataHandlerNumeric(0x0028, 0x0, 0x1201, 0), L"", pImage->isSigned()));
            std::shared_ptr<lut> green(std::make_shared<lut>(getReadingDataHandlerNumeric(0x0028, 0x0, 0x1102, 0), getReadingDataHandlerNumeric(0x0028, 0x0, 0x1202, 0), L"", pImage->isSigned()));
            std::shared_ptr<lut> blue(std::make_shared<lut>(getReadingDataHandlerNumeric(0x0028, 0x0, 0x1103, 0), getReadingDataHandlerNumeric(0x0028, 0x0, 0x1203, 0), L"", pImage->isSigned()));
            std::shared_ptr<palette> imagePalette(std::make_shared<palette>(red, green, blue));
            pImage->setPalette(imagePalette);
        }

        return pImage;
    }
    catch(const MissingDataElementError&)
    {
        IMEBRA_THROW(DataSetImageDoesntExistError, "The requested image doesn't exist");
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get an image from the dataset and apply the modality
//  transform.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<image> dataSet::getModalityImage(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<image> originalImage = getImage(frameNumber);

    std::shared_ptr<transforms::colorTransforms::colorTransformsFactory> colorFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
    if(!colorFactory->isMonochrome(originalImage->getColorSpace()))
    {
        return originalImage;
    }

    // Get the functional group dataset if available
    std::shared_ptr<const dataSet> functionalGroupDataSet;
    try
    {
        functionalGroupDataSet = getFunctionalGroupDataSet(frameNumber);
        try
        {
            functionalGroupDataSet->getDouble(0x0028, 0, 0x1052, 0, 0); // Get intercept
            functionalGroupDataSet->getDouble(0x0028, 0, 0x1053, 0, 0); // Get slope
        }
        catch(const MissingDataElementError&)
        {
            functionalGroupDataSet->getLut(0x0028, 0x3000, 0); // Try to get the modality lut
        }
    }
    catch(const MissingDataElementError&)
    {
        functionalGroupDataSet = shared_from_this();
    }
    std::shared_ptr<transforms::modalityVOILUT> modalityVOILUT(std::make_shared<transforms::modalityVOILUT>(functionalGroupDataSet));

    // Convert to MONOCHROME2 if a modality transform is not present
    ////////////////////////////////////////////////////////////////
    if(modalityVOILUT->isEmpty())
    {
        std::shared_ptr<transforms::transform> monochromeColorTransform(colorFactory->getTransform(originalImage->getColorSpace(), "MONOCHROME2"));
        if(monochromeColorTransform != nullptr)
        {
            std::uint32_t width, height;
            originalImage->getSize(&width, &height);
            std::shared_ptr<image> outputImage = monochromeColorTransform->allocateOutputImage(originalImage->getDepth(),
                                                                                               originalImage->getColorSpace(),
                                                                                               originalImage->getHighBit(),
                                                                                               originalImage->getPalette(),
                                                                                               width, height);
            monochromeColorTransform->runTransform(originalImage, 0, 0, width, height, outputImage, 0, 0);
            return outputImage;
        }

        return originalImage;
    }

    // Apply the modality VOI/LUT transform
    ///////////////////////////////////////
    std::uint32_t width, height;
    originalImage->getSize(&width, &height);
    std::shared_ptr<image> outputImage = modalityVOILUT->allocateOutputImage(originalImage->getDepth(),
                                                                             originalImage->getColorSpace(),
                                                                             originalImage->getHighBit(),
                                                                             originalImage->getPalette(),
                                                                             width, height);
    modalityVOILUT->runTransform(originalImage, 0, 0, width, height, outputImage, 0, 0);
    return outputImage;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert an image into the dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setImage(std::uint32_t frameNumber, std::shared_ptr<image> pImage, imageQuality_t quality)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    // bDontChangeAttributes is true if some images already
    //  exist in the dataset and we must save the new image
    //  using the attributes already stored
    ///////////////////////////////////////////////////////////
    std::uint32_t numberOfFrames = getUnsignedLong(0x0028, 0, 0x0008, 0, 0, 0);
    if(frameNumber != numberOfFrames)
    {
        IMEBRA_THROW(DataSetWrongFrameError, "The frames must be inserted in sequence");
    }
    const bool bDontChangeAttributes = (numberOfFrames != 0);
    const std::string transferSyntax = getString(0x0002, 0x0, 0x0010, 0, 0, "1.2.840.10008.1.2");

    // Select the right codec
    ///////////////////////////////////////////////////////////
    std::shared_ptr<const codecs::imageCodec> saveCodec(codecs::codecFactory::getCodecFactory()->getImageCodec(transferSyntax));

    // Do we have to save the basic offset table?
    ///////////////////////////////////////////////////////////
    const std::uint16_t groupId(0x7fe0), tagId(0x0010); // The tag where the image must be stored
    const bool bEncapsulated = saveCodec->encapsulated(transferSyntax) || bufferExists(groupId, 0x0, tagId, 0x1);

    // Set the subsampling flags
    ///////////////////////////////////////////////////////////
    bool bSubSampledX = static_cast<std::uint32_t>(quality) > static_cast<std::uint32_t>(imageQuality_t::high);
    bool bSubSampledY = static_cast<std::uint32_t>(quality) > static_cast<std::uint32_t>(imageQuality_t::medium);
    if( !transforms::colorTransforms::colorTransformsFactory::canSubsample(pImage->getColorSpace()) )
    {
        bSubSampledX = bSubSampledY = false;
    }

    std::uint32_t width, height;
    pImage->getSize(&width, &height);
    const bool b2complement = pImage->isSigned();
    const std::uint32_t channelsNumber = pImage->getChannelsNumber();
    const std::uint32_t highBit = pImage->getHighBit();
    const bool bInterleaved = (getUnsignedLong(0x0028, 0x0, 0x0006, 0, 0, channelsNumber > 1 ? 0 : 1) == 0x0);

    // If the attributes cannot be changed, then check the
    //  attributes already stored in the dataset
    ///////////////////////////////////////////////////////////
    if(bDontChangeAttributes)
    {
        const std::string currentColorSpace = getString(0x0028, 0x0, 0x0004, 0, 0);
        if(
                transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(pImage->getColorSpace()) !=
                    transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(currentColorSpace) ||
                bSubSampledX != transforms::colorTransforms::colorTransformsFactory::isSubsampledX(currentColorSpace) ||
                bSubSampledY != transforms::colorTransforms::colorTransformsFactory::isSubsampledY(currentColorSpace) ||
                b2complement != (getUnsignedLong(0x0028, 0, 0x0103, 0, 0) != 0) ||
                highBit != getUnsignedLong(0x0028, 0x0, 0x0102, 0, 0) ||
                channelsNumber != getUnsignedLong(0x0028, 0x0, 0x0002, 0, 0) ||
                width != getUnsignedLong(0x0028, 0, 0x0011, 0, 0) ||
                height != getUnsignedLong(0x0028, 0, 0x0010, 0, 0))
        {
            IMEBRA_THROW(DataSetDifferentFormatError, "An image already exists in the dataset and has different attributes");
        }
    }

    // Select the data type OB if not already set in the
    //  dataset
    ///////////////////////////////////////////////////////////
    tagVR_t dataHandlerType;
    if(transferSyntax == "1.2.840.10008.1.2")
    {
        dataHandlerType = dicomDictionary::getDicomDictionary()->getTagType(0x7FE0, 0x0010);
    }
    else
    {
        dataHandlerType = (bEncapsulated || highBit <= 7) ? tagVR_t::OB : tagVR_t::OW;
    }

    const std::uint32_t allocatedBits = saveCodec->suggestAllocatedBits(transferSyntax, pImage->getHighBit());

    // Encapsulated mode. Check if we have the offsets table
    ///////////////////////////////////////////////////////////
    std::uint32_t firstBufferId(0);
    if(bEncapsulated)
    {
        try
        {
            if(bufferExists(groupId, 0, tagId, 0))
            {
                dataHandlerType = getDataType(groupId, 0x0, tagId);
            }
            else
            {
                std::shared_ptr<handlers::readingDataHandlerRaw> imageHandler = getReadingDataHandlerRaw(groupId, 0, tagId, 0);
                size_t bufferSize(imageHandler->getSize());

                if(bufferSize != 0 && !bufferExists(groupId, 0x0, tagId, 0x1))
                {
                    // The first image must be moved forward, in order to
                    //  make some room for the offset table
                    ///////////////////////////////////////////////////////////
                    dataHandlerType = imageHandler->getDataType();
                    std::shared_ptr<handlers::writingDataHandlerRaw> moveFirstImage = getWritingDataHandlerRaw(groupId, 0x0, tagId, 0x1, dataHandlerType);
                    moveFirstImage->setSize(bufferSize);
                    ::memcpy(moveFirstImage->getMemoryBuffer(), imageHandler->getMemoryBuffer(), bufferSize);
                }
            }
        }
        catch(const MissingDataElementError&)
        {
            // Nothing to do. No image has been stored yet
        }

        firstBufferId = getFirstAvailFrameBufferId();
    }

    // Get a stream to save the image
    ///////////////////////////////////////////////////////////
    {
        std::shared_ptr<memory> uncompressedImage(std::make_shared<memory>());
        std::shared_ptr<streamWriter> outputStream;
        if(bEncapsulated)
        {
            outputStream = getStreamWriter(groupId, 0, tagId, firstBufferId, dataHandlerType);
        }
        else
        {
            std::shared_ptr<memoryStreamOutput> memStream(std::make_shared<memoryStreamOutput>(uncompressedImage));
            outputStream = std::make_shared<streamWriter>(memStream);
        }

        // Save the image in the stream
        ///////////////////////////////////////////////////////////
        saveCodec->setImage(
            outputStream,
            pImage,
            transferSyntax,
            quality,
            allocatedBits,
            bSubSampledX, bSubSampledY,
            bInterleaved,
            b2complement);

        outputStream->flushDataBuffer();

        if(!bEncapsulated)
        {
            size_t imageSizeBits = codecs::dicomNativeImageCodec::getNativeImageSizeBits(allocatedBits,
                                                                                          width,
                                                                                          height,
                                                                                          channelsNumber,
                                                                                          bSubSampledX,
                                                                                          bSubSampledY);

            // Images with allocatedBits == 1 need special treatment (if not byte aligned)
            if(allocatedBits == 1 && ((frameNumber * imageSizeBits) & 0x7) != 0)
            {
                std::shared_ptr<data> imageTag = getTag(groupId, 0, tagId);
                std::shared_ptr<handlers::readingDataHandlerRaw> dataHandler = imageTag->getBuffer(firstBufferId)->getReadingDataHandlerRaw(dataHandlerType);
                size_t newSize = ((frameNumber + 1) * imageSizeBits + 7) / 8;
                std::shared_ptr<handlers::writingDataHandlerRaw> writeDataHandler = imageTag->getBuffer(firstBufferId)->getWritingDataHandlerRaw(dataHandlerType, static_cast<std::uint32_t>(newSize));
                memcpy(writeDataHandler->getMemoryBuffer(), dataHandler->getMemoryBuffer(), (frameNumber * imageSizeBits + 7) / 8);
                std::uint8_t* pDestination = writeDataHandler->getMemoryBuffer() + (frameNumber * imageSizeBits) / 8;
                const std::uint8_t* pSource = uncompressedImage->data();
                const size_t offsetBit = (imageSizeBits * frameNumber) & 7;
                for(size_t copyBits(0); copyBits < imageSizeBits; copyBits += 8)
                {
                    *pDestination = *pDestination | static_cast<std::uint8_t>(*pSource << offsetBit);
                    ++pDestination;
                    *pDestination = static_cast<std::uint8_t>(*pSource++ >> (8 - offsetBit));
                }
            }
            else
            {
                getTagCreate(groupId, 0, tagId, dataHandlerType)->getBufferCreate(firstBufferId, streamController::tByteOrdering::lowByteEndian)->appendMemory(uncompressedImage);
            }
        }
    }

    // Write the attributes in the dataset
    ///////////////////////////////////////////////////////////
    if(!bDontChangeAttributes)
    {
        std::shared_ptr<handlers::writingDataHandler> dataHandlerTransferSyntax = getWritingDataHandler(0x0002, 0x0, 0x0010, 0x0);
        dataHandlerTransferSyntax->setString(0, transferSyntax);

        std::string colorSpace = pImage->getColorSpace();
        setString(0x0028, 0x0, 0x0004, 0, transforms::colorTransforms::colorTransformsFactory::makeSubsampled(colorSpace, bSubSampledX, bSubSampledY));
        if(channelsNumber > 1)
        {
            setUnsignedLong(0x0028, 0x0, 0x0006, 0, bInterleaved ? 0 : 1);
        }
        setUnsignedLong(0x0028, 0x0, 0x0100, 0, allocatedBits);            // allocated bits
        setUnsignedLong(0x0028, 0x0, 0x0101, 0, pImage->getHighBit() + 1); // stored bits
        setUnsignedLong(0x0028, 0x0, 0x0102, 0, pImage->getHighBit());     // high bit
        setUnsignedLong(0x0028, 0x0, 0x0103, 0, b2complement ? 1 : 0);
        setUnsignedLong(0x0028, 0x0, 0x0002, 0, channelsNumber);
        pImage->getSize(&width, &height);
        setUnsignedLong(0x0028, 0x0, 0x0011, 0, width);
        setUnsignedLong(0x0028, 0x0, 0x0010, 0, height);

        if(colorSpace == "PALETTE COLOR")
        {
            IMEBRA_THROW(DataSetImagePaletteColorIsReadOnly, "Cannot set images with color space PALETTE COLOR");
        }
    }

    // Update the number of frames
    ///////////////////////////////////////////////////////////
    numberOfFrames = frameNumber + 1;
    setUnsignedLong(0x0028, 0, 0x0008, 0, numberOfFrames );

    // Update the offsets tag with the image's offsets
    ///////////////////////////////////////////////////////////
    if(bEncapsulated)
    {
        std::uint32_t calculatePosition(0);
        std::shared_ptr<const data> tag(getTag(groupId, 0, tagId));
        for(std::uint32_t scanBuffers = 1; scanBuffers < firstBufferId; ++scanBuffers)
        {
            size_t bufferSize = tag->getBufferSize(scanBuffers);
            if((bufferSize & 1u) == 1u)
            {
                ++bufferSize;
            }
            calculatePosition += static_cast<std::uint32_t>(bufferSize);
            calculatePosition += 8;
        }
        std::shared_ptr<handlers::writingDataHandlerRaw> offsetHandler(getWritingDataHandlerRaw(groupId, 0, tagId, 0, dataHandlerType));
        offsetHandler->setSize(4 * (frameNumber + 1));
        std::shared_ptr<handlers::readingDataHandlerRaw> originalOffsetHandler(getReadingDataHandlerRaw(groupId, 0, tagId, 0));
        originalOffsetHandler->copyTo(offsetHandler->getMemoryBuffer(), offsetHandler->getSize());
        std::uint8_t* pOffsetFrame(offsetHandler->getMemoryBuffer() + (frameNumber * 4));
        *( reinterpret_cast<std::uint32_t*>(pOffsetFrame) ) = calculatePosition;
        streamController::adjustEndian(pOffsetFrame, 4, streamController::lowByteEndian, 1);
    }
    IMEBRA_FUNCTION_END();
}


std::shared_ptr<overlay> dataSet::getOverlay(std::uint32_t overlayNumber) const
{
    IMEBRA_FUNCTION_START();

    std::uint16_t groupId(static_cast<std::uint16_t>(0x6000 + overlayNumber * 2));
    if(groupId >= 0x6100u)
    {
        IMEBRA_THROW(MissingGroupError, "Cannot store more than 127 overlays");
    }
    try
    {
        std::uint32_t height = getUnsignedLong(groupId, 0, 0x0010, 0, 0);
        std::uint32_t width = getUnsignedLong(groupId, 0, 0x0011, 0, 0);
        std::uint32_t firstFrame = getUnsignedLong(groupId, 0, 0x0051, 0, 0, 1) - 1;
        std::uint32_t framesCount = getUnsignedLong(groupId, 0, 0x0015, 0, 0, 1);

        // Construct the overlay and return it
        overlayType_t overlayType = getString(groupId, 0, 0x0040, 0, 0) == "R" ? overlayType_t::ROI : overlayType_t::graphic;
        std::int32_t originY = getSignedLong(groupId, 0, 0x0050, 0, 0);
        std::int32_t originX = getSignedLong(groupId, 0, 0x0050, 0, 1);
        std::uint32_t allocatedBits = getUnsignedLong(groupId, 0, 0x0100, 0, 0); // Fail if different than 1
        if(allocatedBits != 1)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Cannot handle overlays with allocatedBits greater than one");
        }
        std::uint32_t bitPosition = getUnsignedLong(groupId, 0, 0x0102, 0, 0); // Fail if different than 0
        if(bitPosition != 0)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Cannot handle overlays with bitPosition different than one");
        }
        std::shared_ptr<const memory> pData = getReadingDataHandlerNumeric(groupId, 0, 0x3000, 0)->getMemory();
        std::wstring description = getUnicodeString(groupId, 0, 0x0022, 0, 0, L"");
        std::string subType = getString(groupId, 0, 0x0045, 0, 0, "");
        std::wstring label = getUnicodeString(groupId, 0, 0x1500, 0, 0, L"");

        bool roiAreaPixelsPresent = true;
        std::uint32_t roiAreaPixels = 0;
        try
        {
            roiAreaPixels = getUnsignedLong(groupId, 0, 0x1301, 0, 0);
        }
        catch(const MissingDataElementError&)
        {
            roiAreaPixelsPresent = false;
        }

        bool roiMeanPresent = true;
        double roiMean = 0.0;
        try
        {
            roiMean = getDouble(groupId, 0, 0x1302, 0, 0);
        }
        catch(const MissingDataElementError&)
        {
            roiMeanPresent = false;
        }

        bool roiStandardDeviationPresent = true;
        double roiStandardDeviation = 0.0;
        try
        {
            roiStandardDeviation = getDouble(groupId, 0, 0x1303, 0, 0);
        }
        catch(const MissingDataElementError&)
        {
            roiStandardDeviationPresent = false;
        }

        return std::make_shared<overlay>(
                    width,
                    height,
                    firstFrame,
                    framesCount,
                    originX - 1,
                    originY - 1,
                    overlayType,
                    subType,
                    label,
                    description,
                    roiAreaPixels,
                    roiAreaPixelsPresent,
                    roiMean,
                    roiMeanPresent,
                    roiStandardDeviation,
                    roiStandardDeviationPresent,
                    getTag(groupId, 0, 0x3000)->getBuffer(0));
    }
    catch(MissingDataElementError)
    {
            IMEBRA_THROW(MissingGroupError, "None of the 60XX groups contain the overlay data");
    }

    IMEBRA_FUNCTION_END();
}


void dataSet::setOverlay(std::uint32_t overlayNumber, std::shared_ptr<overlay> pOverlay)
{
    IMEBRA_FUNCTION_START();

    std::uint16_t groupId(static_cast<std::uint16_t>(0x6000 + overlayNumber * 2));
    if(groupId >= 0x6100u)
    {
        IMEBRA_THROW(std::logic_error, "Cannot store more than 127 overlays");
    }

    std::uint32_t height, width;
    pOverlay->getImage(0)->getSize(&width, &height);
    setUnsignedLong(groupId, 0, 0x0010, 0, height);
    setUnsignedLong(groupId, 0, 0x0011, 0, width);
    setUnsignedLong(groupId, 0, 0x0051, 0, pOverlay->getFirstFrame() + 1);
    setUnsignedLong(groupId, 0, 0x0015, 0, pOverlay->getFramesCount());
    setString(groupId, 0, 0x0040, 0, pOverlay->getType() == overlayType_t::ROI ? "R" : "G");
    setString(groupId, 0, 0x0045, 0, pOverlay->getSubType());
    std::shared_ptr<handlers::writingDataHandler> originHandler(getWritingDataHandler(groupId, 0, 0x0050, 0));
    originHandler->setSize(2);
    originHandler->setSignedLong(0, pOverlay->getOneBasedOriginY());
    originHandler->setSignedLong(1, pOverlay->getOneBasedOriginX());
    setUnsignedLong(groupId, 0, 0x0100, 0, 1); // bits allocated
    setUnsignedLong(groupId, 0, 0x0102, 0, 0); // bit position
    setUnicodeString(groupId, 0, 0x1500, 0, pOverlay->getUnicodeLabel());
    setUnicodeString(groupId, 0, 0x0022, 0, pOverlay->getUnicodeDescription());

    if(pOverlay->getROIAreaPresent())
    {
        setUnsignedLong(groupId, 0, 0x1301, 0, pOverlay->getROIArea());
    }

    if(pOverlay->getROIMeanPresent())
    {
        setDouble(groupId, 0, 0x1302, 0, pOverlay->getROIMean());
    }

    if(pOverlay->getROIStandardDeviationPresent())
    {
        setDouble(groupId, 0, 0x1303, 0, pOverlay->getROIStandardDeviation());
    }

    std::shared_ptr<data> pTag = getTagCreate(groupId, 0, 0x3000);
    pTag->setBuffer(0, pOverlay->getBuffer());

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Get the offset, in bytes, of the specified frame
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameOffset(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        // Retrieve the buffer containing the offsets
        ///////////////////////////////////////////////////////////
        std::shared_ptr<handlers::readingDataHandlerRaw> framesPointer = getReadingDataHandlerRaw(0x7fe0, 0x0, 0x0010, 0);

        // Get the offset table's size, in number of offsets
        ///////////////////////////////////////////////////////////
        std::uint32_t offsetsCount = static_cast<std::uint32_t>(framesPointer->getSize() / sizeof(std::uint32_t));

        // If the requested frame doesn't exist then return
        //  0xffffffff (the maximum value)
        ///////////////////////////////////////////////////////////
        if(frameNumber >= offsetsCount && frameNumber != 0)
        {
            return std::numeric_limits<std::uint32_t>::max();
        }

        // Return the requested offset. If the requested frame is
        //  the first and is offset is not specified, then return
        //  0 (the first position)
        ///////////////////////////////////////////////////////////
        if(frameNumber < offsetsCount)
        {
            const std::uint32_t* pOffsets = reinterpret_cast<const std::uint32_t*>(framesPointer->getMemoryBuffer());
            std::uint32_t returnOffset(pOffsets[frameNumber]);
            streamController::adjustEndian((std::uint8_t*)&returnOffset, 4, streamController::lowByteEndian);
            return returnOffset;
        }
        return 0;
    }
    catch(const MissingDataElementError&)
    {
        return std::numeric_limits<std::uint32_t>::max();
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Return the buffer that starts at the specified offset
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameBufferId(std::uint32_t offset) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> imageTag = getTag(0x7fe0, 0, 0x0010);

    // Start from the buffer n. 1 (the buffer 0 contains
    //  the offset table
    ///////////////////////////////////////////////////////////
    std::uint32_t scanBuffers(1);

    if(offset == std::numeric_limits<std::uint32_t>::max())
    {
        while(imageTag->bufferExists(scanBuffers))
        {
            ++scanBuffers;
        }
        return scanBuffers;
    }

    while(offset != 0)
    {
        // Calculate the total size of the buffer, including
        //  its descriptor (tag group and id and length)
        ///////////////////////////////////////////////////////////
        std::uint32_t bufferSize = static_cast<std::uint32_t>(imageTag->getBufferSize(scanBuffers));
        bufferSize += 4; // one WORD for the group id, one WORD for the tag id
        bufferSize += 4; // one DWORD for the tag length
        if(bufferSize > offset)
        {
            IMEBRA_THROW(DataSetImageDoesntExistError, "Image not in the offset table");
        }
        offset -= bufferSize;
        ++scanBuffers;
    }

    return scanBuffers;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the first buffer and the end buffer occupied by an
//  image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t dataSet::getFrameBufferIds(std::uint32_t frameNumber, std::uint32_t* pFirstBuffer, std::uint32_t* pEndBuffer) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    try
    {
        std::uint32_t startOffset = getFrameOffset(frameNumber);
        std::uint32_t endOffset = getFrameOffset(frameNumber + 1);

        if(startOffset == std::numeric_limits<std::uint32_t>::max())
        {
            IMEBRA_THROW(DataSetImageDoesntExistError, "Image not in the table offset");
        }

        *pFirstBuffer = getFrameBufferId(startOffset);
        *pEndBuffer = getFrameBufferId(endOffset);

        std::shared_ptr<data> imageTag;
        try
        {
            imageTag = getTag(0x7fe0, 0, 0x0010);
        }
        catch(MissingDataElementError&)
        {
            return 0;
        }

        size_t totalSize(0);
        for(std::uint32_t scanBuffers(*pFirstBuffer); scanBuffers != *pEndBuffer; ++scanBuffers)
        {
            totalSize += imageTag->getBufferSize(scanBuffers);
        }
        return totalSize;
    }
    catch(const MissingDataElementError&)
    {
        IMEBRA_THROW(DataSetCorruptedOffsetTableError, "The basic offset table is corrupted");
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the id of the first available buffer that can
//  be used to store a new frame
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFirstAvailFrameBufferId() const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t availableId(1);
    while(bufferExists(0x7fe0, 0, 0x0010, availableId))
    {
        ++availableId;
    }

    return availableId;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a sequence item as a dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> dataSet::getSequenceItem(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t itemId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getSequenceItem(itemId);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a sequence item
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> dataSet::appendSequenceItem(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId)
{
    IMEBRA_FUNCTION_START();

    return getTagCreate(groupId, order, tagId, tagVR_t::SQ)->appendSequenceItem();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a LUT from the data set
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<lut> dataSet::getLut(std::uint16_t groupId, std::uint16_t tagId, size_t lutId) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<dataSet> embeddedLUT = getSequenceItem(groupId, 0, tagId, lutId);
    std::shared_ptr<handlers::readingDataHandlerNumericBase> descriptorHandle = embeddedLUT->getReadingDataHandlerNumeric(0x0028, 0x0, 0x3002, 0x0);
    std::shared_ptr<handlers::readingDataHandlerNumericBase> dataHandle = embeddedLUT->getReadingDataHandlerNumeric(0x0028, 0x0, 0x3006, 0x0);

    std::shared_ptr<lut> pLUT = std::make_shared<lut>(
        descriptorHandle,
        dataHandle,
        embeddedLUT->getUnicodeString(0x0028, 0x0, 0x3003, 0, 0, L""),
        getUnsignedLong(0x0028, 0, 0x0103, 0, 0, 0) != 0);
    return pLUT;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the settings for the values of interest
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::list<std::shared_ptr<const VOIDescription>> dataSet::getVOIs() const
{
    IMEBRA_FUNCTION_START();

    std::list<std::shared_ptr<const VOIDescription>> vois;

    try
    {
        for(size_t voiIndex(0); ; ++voiIndex)
        {
            std::string functionName = getString(0x0028, 0, 0x1056, 0, 0, "LINEAR");
            dicomVOIFunction_t function = dicomVOIFunction_t::linear;
            if(functionName == "LINEAR_EXACT")
            {
                function = dicomVOIFunction_t::linearExact;
            }
            else if(functionName == "SIGMOID")
            {
                function = dicomVOIFunction_t::sigmoid;
            }
            std::shared_ptr<const VOIDescription> voi = std::make_shared<VOIDescription>(
                getDouble(0x0028, 0, 0x1050, 0, voiIndex),
                getDouble(0x0028, 0, 0x1051, 0, voiIndex),
                function,
                getUnicodeString(0x0028, 0, 0x1055, 0, voiIndex, L""));
            vois.push_back(voi);
        }
    }
    catch(const MissingDataElementError&)
    {
        // VOI not present
    }

    return vois;

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<dataSet> dataSet::getFunctionalGroupDataSet(size_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getSequenceItem(0x5200, 0, 0x9230, frameNumber);
    }
    catch(const MissingDataElementError&)
    {
        // Nothing to do, try the common sequence item below
    }

    return getSequenceItem(0x5200, 0, 0x9229, 0);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a tag as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t dataSet::getSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getSignedLong(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::int32_t dataSet::getSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::int32_t defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getSignedLong(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a tag as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::int32_t newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setSignedLong(0, newValue);

    IMEBRA_FUNCTION_END();
}


void dataSet::setSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::int32_t newValue)
{
    IMEBRA_FUNCTION_START();

    setSignedLong(groupId, order, tagId, bufferId, newValue, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getUnsignedLong(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::uint32_t dataSet::getUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::uint32_t defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getUnsignedLong(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setUnsignedLong(0, newValue);

    IMEBRA_FUNCTION_END();
}


void dataSet::setUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t newValue)
{
    IMEBRA_FUNCTION_START();

    setUnsignedLong(groupId, order, tagId, bufferId, newValue, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double dataSet::getDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getDouble(elementNumber);

    IMEBRA_FUNCTION_END();
}

double dataSet::getDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, double defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getDouble(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, double newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setDouble(0, newValue);

    IMEBRA_FUNCTION_END();
}


void dataSet::setDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, double newValue)
{
    IMEBRA_FUNCTION_START();

    setDouble(groupId, order, tagId, bufferId, newValue, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataSet::getString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getString(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::string dataSet::getString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::string& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getString(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as an unicode string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataSet::getUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getUnicodeString(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::wstring dataSet::getUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::wstring& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getUnicodeString(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<patientName> dataSet::getPatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getPatientName(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<patientName> dataSet::getPatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::shared_ptr<patientName>& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getPatientName(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<unicodePatientName> dataSet::getUnicodePatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getUnicodePatientName(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<unicodePatientName> dataSet::getUnicodePatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::shared_ptr<unicodePatientName>& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getUnicodePatientName(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}

void dataSet::setPatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const patientName>& pPatientName)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR_t::PN);
    dataHandler->setSize(1);
    dataHandler->setPatientName(0, pPatientName);

    IMEBRA_FUNCTION_END();
}

void dataSet::setUnicodePatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const unicodePatientName>& pPatientName)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR_t::PN);
    dataHandler->setSize(1);
    dataHandler->setUnicodePatientName(0, pPatientName);

    IMEBRA_FUNCTION_END();
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::string& newString, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setString(0, newString);

    IMEBRA_FUNCTION_END();
}


void dataSet::setString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::string& newString)
{
    IMEBRA_FUNCTION_START();

    setString(groupId, order, tagId, bufferId, newString, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::wstring& newString, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setUnicodeString(0, newString);

    IMEBRA_FUNCTION_END();
}


void dataSet::setUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::wstring& newString)
{
    IMEBRA_FUNCTION_START();

    setUnicodeString(groupId, order, tagId, bufferId, newString, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


void dataSet::setAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const age>& pAge)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR_t::AS);
    dataHandler->setSize(1);
    dataHandler->setAge(0, pAge);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<age> dataSet::getAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getAge(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<age> dataSet::getAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId,
                              size_t elementNumber, const std::shared_ptr<age>& pDefaultAge) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getAge(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return pDefaultAge;
    }

    IMEBRA_FUNCTION_END();

}


void dataSet::setDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const date>& pDate, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setDate(0, pDate);

    IMEBRA_FUNCTION_END();
}

void dataSet::setDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const date>& pDate)
{
    IMEBRA_FUNCTION_START();

    setDate(groupId, order, tagId, bufferId, pDate, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<date> dataSet::getDate(uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getDate(elementNumber);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<date> dataSet::getDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::shared_ptr<date> defautlDate) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getDate(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defautlDate;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a data handler for the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandler> dataSet::getReadingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getReadingDataHandler(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandler> dataSet::getWritingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> tag(getTagCreate(groupId, order, tagId, tagVR));

    return tag->getWritingDataHandler(bufferId);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<handlers::writingDataHandler> dataSet::getWritingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getWritingDataHandler(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a stream reader that works on the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<streamReader> dataSet::getStreamReader(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getStreamReader(bufferId);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a stream writer for the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<streamWriter> dataSet::getStreamWriter(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t dataType)
{
    IMEBRA_FUNCTION_START();

    return getTagCreate(groupId, order, tagId, dataType)->getStreamWriter(bufferId);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<streamWriter> dataSet::getStreamWriter(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getStreamWriter(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a raw data handler for the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandlerRaw> dataSet::getReadingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getReadingDataHandlerRaw(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerRaw> dataSet::getWritingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> tag = getTagCreate(groupId, order, tagId, tagVR);

    return tag->getWritingDataHandlerRaw(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerRaw> dataSet::getWritingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getWritingDataHandlerRaw(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::readingDataHandlerNumericBase> dataSet::getReadingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getReadingDataHandlerNumeric(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerNumericBase> dataSet::getWritingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> tag = getTagCreate(groupId, order, tagId, tagVR);

    return tag->getWritingDataHandlerNumeric(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerNumericBase> dataSet::getWritingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getWritingDataHandlerNumeric(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the data type of a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tagVR_t dataSet::getDataType(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getDataType();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the item's position in the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setItemOffset(std::uint32_t offset)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_itemOffset = offset;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the item's position in the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getItemOffset() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    return m_itemOffset;
}

dataSet::tGroupsIds dataSet::getGroups() const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    dataSet::tGroupsIds groups;

    for(tGroups::const_iterator scanGroups(m_groups.begin()), endGroups(m_groups.end()); scanGroups != endGroups; ++scanGroups)
    {
        groups.insert(scanGroups->first);
    }

    return groups;

    IMEBRA_FUNCTION_END();
}

std::uint32_t dataSet::getGroupsNumber(uint16_t groupId) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    dataSet::tGroups::const_iterator findGroup(m_groups.find(groupId));

    if(findGroup == m_groups.end())
    {
        return 0;
    }

    return static_cast<std::uint32_t>(findGroup->second.size());

    IMEBRA_FUNCTION_END();
}

const dataSet::tTags& dataSet::getGroupTags(std::uint16_t groupId, size_t groupOrder) const
{
    IMEBRA_FUNCTION_START();

    static const dataSet::tTags emptyTags;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    tGroups::const_iterator findGroup(m_groups.find(groupId));
    if(findGroup == m_groups.end() || findGroup->second.size() <= groupOrder)
    {
        return emptyTags;
    }

    return findGroup->second.at(groupOrder);

    IMEBRA_FUNCTION_END();
}

void dataSet::setCharsetsList(const charsetsList_t& charsets)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    *m_pCharsetsList = charsets;

    IMEBRA_FUNCTION_END();
}

} // namespace implementation

} // namespace imebra
