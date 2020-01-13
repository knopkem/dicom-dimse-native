/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file drawBitmap.cpp
    \brief Implementation of the transform drawBitmap.

*/

#include "drawBitmapImpl.h"
#include "imageImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "transformHighBitImpl.h"
#include "transformsChainImpl.h"

namespace imebra
{

namespace implementation
{


drawBitmap::drawBitmap(std::shared_ptr<transforms::transform> transformsChain):
    m_userTransforms(transformsChain)
{
}


std::shared_ptr<memory> drawBitmap::getBitmap(const std::shared_ptr<const image>& sourceImage, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes)
{
    IMEBRA_FUNCTION_START();

    size_t memorySize(getBitmap(sourceImage, drawBitmapType, rowAlignBytes, 0, 0));

    std::shared_ptr<memory> bitmapMemory = std::make_shared<memory>(memorySize);

    // Retrieve the final bitmap's buffer
    ///////////////////////////////////////////////////////////
    std::uint8_t* pFinalBuffer = (std::uint8_t*)(bitmapMemory->data());

    getBitmap(sourceImage, drawBitmapType, rowAlignBytes, pFinalBuffer, memorySize);

    return bitmapMemory;

    IMEBRA_FUNCTION_END();

}

size_t drawBitmap::getBitmap(const std::shared_ptr<const image>& sourceImage, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes, std::uint8_t* pBuffer, size_t bufferSize)
{
    IMEBRA_FUNCTION_START();

    std::uint32_t width, height;
    sourceImage->getSize(&width, &height);
    std::uint32_t destPixelSize((drawBitmapType == drawBitmapType_t::drawBitmapRGBA || drawBitmapType == drawBitmapType_t::drawBitmapBGRA) ? 4 : 3);

    // Calculate the row' size, in bytes
    ///////////////////////////////////////////////////////////
    std::uint32_t rowSizeBytes = (width * destPixelSize + rowAlignBytes - 1) / rowAlignBytes;
    rowSizeBytes *= rowAlignBytes;

    // Allocate the memory for the final bitmap
    ///////////////////////////////////////////////////////////
    std::uint32_t memorySize(rowSizeBytes * height);
    if(memorySize == 0 || memorySize > bufferSize)
    {
        return memorySize;
    }

    // This chain will contain all the necessary transforms, including color
    //  transforms and high bit shift
    ///////////////////////////////////////////////////////////////////////////////
    transforms::transformsChain chain;
    chain.addTransform(m_userTransforms);

    // Allocate the transforms that obtain an RGB image
    ///////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<const image> chainEndImage(sourceImage);
    if(!chain.isEmpty())
    {
        chainEndImage = chain.allocateOutputImage(sourceImage->getDepth(),
                                                  sourceImage->getColorSpace(),
                                                  sourceImage->getHighBit(),
                                                  sourceImage->getPalette(),
                                                  1, 1);
    }

    std::shared_ptr<transforms::colorTransforms::colorTransformsFactory> pColorTransformsFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
    std::shared_ptr<transforms::transform> rgbColorTransform(pColorTransformsFactory->getTransform(chainEndImage->getColorSpace(), "RGB"));

    if(rgbColorTransform != 0 && !rgbColorTransform->isEmpty())
    {
        chain.addTransform(rgbColorTransform);
        chainEndImage = chain.allocateOutputImage(sourceImage->getDepth(),
                                                  sourceImage->getColorSpace(),
                                                  sourceImage->getHighBit(),
                                                  sourceImage->getPalette(),
                                                  1, 1);
    }

    if(chainEndImage->getHighBit() != 7 || chainEndImage->getDepth() != bitDepth_t::depthU8)
    {
        std::shared_ptr<transforms::transformHighBit> highBitTransform(std::make_shared<transforms::transformHighBit>());
        chain.addTransform(highBitTransform);
    }

    // If a transform chain is active then allocate a temporary
    //  output image
    ///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandlerNumericBase> imageHandler;

    if(chain.isEmpty())
    {
        imageHandler = sourceImage->getReadingDataHandler();
    }
    else
    {
        std::shared_ptr<image> outputImage = std::make_shared<image>(width, height, bitDepth_t::depthU8, "RGB",7);
        chain.runTransform(sourceImage, 0, 0, width, height, outputImage, 0, 0);
        imageHandler = outputImage->getReadingDataHandler();
    }

    const std::uint8_t* pImagePointer = imageHandler->getMemoryBuffer();

    std::uint32_t nextRowGap = rowSizeBytes - (width * destPixelSize);

    // Scan all the final bitmap's rows
    ///////////////////////////////////////////////////////////
    switch(drawBitmapType)
    {
    case drawBitmapType_t::drawBitmapRGB:
    {
        for(std::uint32_t scanY(height); scanY != 0; --scanY)
        {
            for(std::uint32_t scanX(width); scanX != 0; --scanX)
            {
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = *pImagePointer++;
            }
            pBuffer += nextRowGap;
        }
        break;
    }
    case drawBitmapType_t::drawBitmapBGR:
    {
        std::uint8_t r, g;
        for(std::uint32_t scanY(height); scanY != 0; --scanY)
        {
            for(std::uint32_t scanX(width); scanX != 0; --scanX)
            {
                r = *pImagePointer++;
                g = *pImagePointer++;
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = g;
                *pBuffer++ = r;
            }
            pBuffer += nextRowGap;
        }
        break;
    }
    case drawBitmapType_t::drawBitmapRGBA:
    {
        for(std::uint32_t scanY(height); scanY != 0; --scanY)
        {
            for(std::uint32_t scanX(width); scanX != 0; --scanX)
            {
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = 0xff;
            }
            pBuffer += nextRowGap;
        }
        break;
    }
    default:
    {
        std::uint8_t r, g;
        for(std::uint32_t scanY(height); scanY != 0; --scanY)
        {
            for(std::uint32_t scanX(width); scanX != 0; --scanX)
            {
                r = *pImagePointer++;
                g = *pImagePointer++;
                *pBuffer++ = *pImagePointer++;
                *pBuffer++ = g;
                *pBuffer++ = r;
                *pBuffer++ = 0xff;
            }
            pBuffer += nextRowGap;
        }
        break;
    }
    }

    return memorySize;

    IMEBRA_FUNCTION_END();
}



} // namespace implementation

} // namespace imebra
