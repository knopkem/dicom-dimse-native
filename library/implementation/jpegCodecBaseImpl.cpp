/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegCodecBaseImpl.cpp
    \brief Implementation of the class jpegCodecBaseImpl.

*/

#include "exceptionImpl.h"
#include "huffmanTableImpl.h"
#include "jpegCodecBaseImpl.h"
#include "imageCodecImpl.h"
#include "streamWriterImpl.h"
#include "streamReaderImpl.h"
#include "codecFactoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <vector>
#include <stdlib.h>
#include <string.h>

namespace imebra
{

namespace implementation
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default luminance or RGB quantization table
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegStdLuminanceQuantTbl[] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default chrominance quantization table
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegStdChrominanceQuantTbl[] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default scale factors for FDCT/IDCT calculation
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const float JpegDctScaleFactor[]=
{
    (float)1.0,
    (float)1.387039845,
    (float)1.306562965,
    (float)1.175875602,
    (float)1.0,
    (float)0.785694958,
    (float)0.541196100,
    (float)0.275899379
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodec
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

namespace jpeg
{
jpegChannel::jpegChannel():
        m_quantTable(0),
        m_blockMcuX(0),
        m_blockMcuY(0),
        m_blockMcuXY(0),
        m_lastDCValue(0),
        m_defaultDCValue(0),
        m_losslessPositionX(0),
        m_losslessPositionY(0),
        m_unprocessedAmplitudesCount(0),
        m_unprocessedAmplitudesPredictor(0),
        m_huffmanTableDC(0),
        m_huffmanTableAC(0),
        m_pActiveHuffmanTableDC(0),
        m_pActiveHuffmanTableAC(0),
        m_valuesMask(0)
{
}

jpegChannel::~jpegChannel()
{
}

}
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
jpegCodecBase::jpegCodecBase()
{
    IMEBRA_FUNCTION_START();

    // Register all the tag classes
    ///////////////////////////////////////////////////////////

    // Unknown tag must be registered
    ///////////////////////////////////////////////////////////
    registerTag(unknown, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagUnknown>()));

    // Register SOF
    ///////////////////////////////////////////////////////////
    registerTag(sof0, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof1, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof2, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof3, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof5, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof6, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof7, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof9, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofA, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofB, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofD, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofE, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofF, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));

    // Register DHT
    ///////////////////////////////////////////////////////////
    registerTag(dht, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagDHT>()));

    // Register DQT
    ///////////////////////////////////////////////////////////
    registerTag(dqt, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagDQT>()));

    // Register SOS
    ///////////////////////////////////////////////////////////
    registerTag(sos, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOS>()));

    // Register EOI
    ///////////////////////////////////////////////////////////
    registerTag(eoi, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagEOI>()));

    // Register RST
    ///////////////////////////////////////////////////////////
    registerTag(rst0, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst1, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst2, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst3, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst4, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst5, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst6, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst7, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));

    // Register DRI
    ///////////////////////////////////////////////////////////
    registerTag(dri, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagDRI>()));

    IMEBRA_FUNCTION_END();
}


jpegCodecBase::~jpegCodecBase()
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodecBase::registerTag(tTagId tagId, std::shared_ptr<jpeg::tag> pTag)
{
    IMEBRA_FUNCTION_START();

    m_tagsMap[(std::uint8_t)tagId]=pTag;

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write a single jpeg tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodecBase::writeTag(streamWriter* pDestinationStream, tTagId tagId, jpeg::jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<jpeg::tag> pTag;
    tTagsMap::const_iterator findTag = m_tagsMap.find((std::uint8_t)tagId);
    if(findTag == m_tagsMap.end())
    {
        return;
    }
    const std::uint8_t ff(0xff);
    std::uint8_t byteTagId(tagId);
    pDestinationStream->write(&ff, 1);
    pDestinationStream->write(&byteTagId, 1);
    findTag->second->writeTag(pDestinationStream, information);

    IMEBRA_FUNCTION_END();
}


namespace jpeg
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegInformation
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

jpegInformation::jpegInformation()
{
    reset(imageQuality_t::veryHigh);
}


void jpegInformation::reset(imageQuality_t compQuality)
{
    IMEBRA_FUNCTION_START();

    // Factor used to calculate the quantization tables used
    //  for the compression
    ///////////////////////////////////////////////////////////
    float compQuantization = (float)compQuality / (float)imageQuality_t::medium;

    // Resets the channels list
    ///////////////////////////////////////////////////////////
    m_channelsList.clear();

    // Allocate the huffman tables
    ///////////////////////////////////////////////////////////
    for(int resetHuffmanTables = 0; resetHuffmanTables<16; ++resetHuffmanTables)
    {
        std::shared_ptr<huffmanTable> huffmanDC(std::make_shared<huffmanTable>(9));
        m_pHuffmanTableDC[resetHuffmanTables] = huffmanDC;

        std::shared_ptr<huffmanTable> huffmanAC(std::make_shared<huffmanTable>(9));
        m_pHuffmanTableAC[resetHuffmanTables] = huffmanAC;
    }

    eraseChannels();

    m_bEndOfImage = false;

    m_imageWidth = m_imageHeight = 0;

    m_precision = 8;
    m_valuesMask = ((std::int32_t)1 << m_precision)-1;

    m_process = 0;

    m_mcuPerRestartInterval = 0;

    m_mcuLastRestart = 0;

    m_spectralIndexStart = 0;
    m_spectralIndexEnd = 63;

    m_bLossless = false;

    // The number of MCUs (horizontal, vertical, total)
    ///////////////////////////////////////////////////////////
    m_mcuNumberX = 0;
    m_mcuNumberY = 0;
    m_mcuNumberTotal = 0;

    m_maxSamplingFactorX = 0;
    m_maxSamplingFactorY = 0;

    m_mcuProcessed = 0;
    m_mcuProcessedX = 0;
    m_mcuProcessedY = 0;
    m_eobRun = 0;

    m_jpegImageWidth = 0;
    m_jpegImageHeight = 0;

    // Reset the QT tables
    ///////////////////////////////////////////////////////////
    for(int resetQT = 0; resetQT<16; ++resetQT)
    {
        const std::uint32_t* pSourceTable = (resetQT == 0) ? JpegStdLuminanceQuantTbl : JpegStdChrominanceQuantTbl;

        std::uint8_t tableIndex = 0;
        for(std::uint8_t row = 0; row < 8; ++row)
        {
            for(std::uint8_t col = 0; col < 8; ++col)
            {
                std::uint32_t quant = (std::uint32_t) ((float)(pSourceTable[tableIndex]) * compQuantization);
                if(quant < 1)
                {
                    quant = 1;
                }
                if(quant > 255)
                {
                    quant = 255;
                }
                m_quantizationTable[resetQT][tableIndex++] = quant;
            }
        }
        recalculateQuantizationTables(resetQT);
    }

    // Reset the huffman tables
    ///////////////////////////////////////////////////////////
    for(int resetHT=0; resetHT < 16; ++resetHT)
    {
        m_pHuffmanTableDC[resetHT]->reset();
        m_pHuffmanTableAC[resetHT]->reset();
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Erase all the allocated channels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegInformation::eraseChannels()
{
    m_channelsMap.clear();
    m_channelsList.clear();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate the channels. This function is called when a
//  SOF tag is found
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegInformation::allocChannels()
{
    IMEBRA_FUNCTION_START();

    m_maxSamplingFactorX=1L;
    m_maxSamplingFactorY=1L;

    m_valuesMask = ((std::int32_t)1 << m_precision)-1;

    // Find the maximum sampling factor
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator channelsIterator0=m_channelsMap.begin(); channelsIterator0!=m_channelsMap.end(); ++channelsIterator0)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel=channelsIterator0->second;

        if(pChannel->m_samplingFactorX>m_maxSamplingFactorX)
            m_maxSamplingFactorX=pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY>m_maxSamplingFactorY)
            m_maxSamplingFactorY=pChannel->m_samplingFactorY;
    }

    if(m_bLossless)
    {
        m_jpegImageWidth=(m_imageWidth+(m_maxSamplingFactorX-1))/m_maxSamplingFactorX;
        m_jpegImageWidth*=m_maxSamplingFactorX;
        m_jpegImageHeight=(m_imageHeight+(m_maxSamplingFactorY-1))/m_maxSamplingFactorY;
        m_jpegImageHeight*=m_maxSamplingFactorY;
    }
    else
    {
        m_jpegImageWidth=(m_imageWidth+((m_maxSamplingFactorX<<3)-1))/(m_maxSamplingFactorX<<3);
        m_jpegImageWidth*=(m_maxSamplingFactorX<<3);
        m_jpegImageHeight=(m_imageHeight+((m_maxSamplingFactorY<<3)-1))/(m_maxSamplingFactorY<<3);
        m_jpegImageHeight*=(m_maxSamplingFactorY<<3);
    }

    // Allocate the channels' buffers
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator channelsIterator1=m_channelsMap.begin(); channelsIterator1 != m_channelsMap.end(); ++channelsIterator1)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel=channelsIterator1->second;
        pChannel->m_defaultDCValue = m_bLossless ? ((std::int32_t)1<<(m_precision - 1)) : 0;
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        pChannel->allocate(
                    m_jpegImageWidth*(std::uint32_t)pChannel->m_samplingFactorX/m_maxSamplingFactorX,
                    m_jpegImageHeight*(std::uint32_t)pChannel->m_samplingFactorY/m_maxSamplingFactorY);
        pChannel->m_valuesMask = m_valuesMask;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Find the MCU's size
// This function is called when a SOS tag is found
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegInformation::findMcuSize()
{
    // Find the maximum sampling factor for all the channels
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorChannelsX=1;
    std::uint32_t maxSamplingFactorChannelsY=1;
    for(tChannelsMap::iterator allChannelsIterator=m_channelsMap.begin(); allChannelsIterator!=m_channelsMap.end(); ++allChannelsIterator)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel = allChannelsIterator->second;

        if(pChannel->m_samplingFactorX > maxSamplingFactorChannelsX)
            maxSamplingFactorChannelsX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY > maxSamplingFactorChannelsY)
            maxSamplingFactorChannelsY = pChannel->m_samplingFactorY;
    }


    // Find the minimum and maximum sampling factor in the scan
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorX=1;
    std::uint32_t maxSamplingFactorY=1;
    std::uint32_t minSamplingFactorX=256;
    std::uint32_t minSamplingFactorY=256;

    for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: m_channelsList)
    {
        if(pChannel->m_samplingFactorX > maxSamplingFactorX)
            maxSamplingFactorX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY > maxSamplingFactorY)
            maxSamplingFactorY = pChannel->m_samplingFactorY;
        if(pChannel->m_samplingFactorX < minSamplingFactorX)
            minSamplingFactorX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY < minSamplingFactorY)
            minSamplingFactorY = pChannel->m_samplingFactorY;
    }

    // Find the number of blocks per MCU per channel
    ///////////////////////////////////////////////////////////
    for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: m_channelsList)
    {
        pChannel->m_blockMcuX = pChannel->m_samplingFactorX / minSamplingFactorX;
        pChannel->m_blockMcuY = pChannel->m_samplingFactorY / minSamplingFactorY;
        pChannel->m_blockMcuXY = pChannel->m_blockMcuX * pChannel->m_blockMcuY;
        pChannel->m_losslessPositionX = 0;
        pChannel->m_losslessPositionY = 0;
        pChannel->m_unprocessedAmplitudesCount = 0;
        pChannel->m_unprocessedAmplitudesPredictor = 0;
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;
    }

    // Find the MCU size, in image's pixels
    ///////////////////////////////////////////////////////////
    if(m_bLossless)
    {
        m_mcuNumberX = m_jpegImageWidth * minSamplingFactorX / maxSamplingFactorChannelsX;
        m_mcuNumberY = m_jpegImageHeight * minSamplingFactorY / maxSamplingFactorChannelsY;
    }
    else
    {
        std::uint32_t xBoundary = 8 * maxSamplingFactorChannelsX / minSamplingFactorX;
        std::uint32_t yBoundary = 8 * maxSamplingFactorChannelsY / minSamplingFactorY;

        m_mcuNumberX = (m_imageWidth + xBoundary - 1) / xBoundary;
        m_mcuNumberY = (m_imageHeight + yBoundary - 1) / yBoundary;
    }
    m_mcuNumberTotal = m_mcuNumberX*m_mcuNumberY;
    m_mcuProcessed = 0;
    m_mcuProcessedX = 0;
    m_mcuProcessedY = 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the quantization tables with the correction
//  factor needed by the IDCT/FDCT
//
//
///////////////////////////////////////////////////////////;
///////////////////////////////////////////////////////////
void jpegInformation::recalculateQuantizationTables(int table)
{
    // Adjust the tables for compression/decompression
    ///////////////////////////////////////////////////////////
    std::uint8_t tableIndex = 0;
    for(std::uint8_t row = 0; row<8; ++row)
    {
        for(std::uint8_t col = 0; col<8; ++col)
        {
            m_decompressionQuantizationTable[table][tableIndex]=(long long)((float)((m_quantizationTable[table][tableIndex])<<JPEG_DECOMPRESSION_BITS_PRECISION)*JpegDctScaleFactor[col]*JpegDctScaleFactor[row]);
            m_compressionQuantizationTable[table][tableIndex]=1.0f/((float)((m_quantizationTable[table][tableIndex])<<3)*JpegDctScaleFactor[col]*JpegDctScaleFactor[row]);
            ++tableIndex;
        }
    }
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegChannel
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void jpegChannel::processUnprocessedAmplitudes()
{
    IMEBRA_FUNCTION_START();

    if(m_unprocessedAmplitudesCount == 0)
    {
        return;
    }

    std::int32_t* pDest = m_pBuffer + (m_losslessPositionY * m_width + m_losslessPositionX);
    std::int32_t* pSource = m_unprocessedAmplitudesBuffer;

    // Find missing pixels
    std::int32_t missingPixels = (std::int32_t)m_width - (std::int32_t)m_losslessPositionX + (std::int32_t)m_width * ((std::int32_t)m_height - (std::int32_t)m_losslessPositionY - 1);
    if(missingPixels < (std::int32_t)m_unprocessedAmplitudesCount)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Excess data in the lossless jpeg stream");
    }

    if(m_unprocessedAmplitudesPredictor == 0)
    {
        while(m_unprocessedAmplitudesCount != 0)
        {
            --m_unprocessedAmplitudesCount;
            *(pDest++) = *(pSource++) & m_valuesMask;
            if(++m_losslessPositionX == m_width)
            {
                m_losslessPositionX = 0;
                ++m_losslessPositionY;
            }
        }
        m_lastDCValue = *(pDest - 1);
        return;
    }

    int applyPrediction;
    std::int32_t* pPreviousLine = pDest - m_width;
    std::int32_t* pPreviousLineColumn = pDest - m_width - 1;
    while(m_unprocessedAmplitudesCount != 0)
    {
        --m_unprocessedAmplitudesCount;
        applyPrediction = (int)m_unprocessedAmplitudesPredictor;
        if(m_losslessPositionY == 0)
        {
            applyPrediction = 1;
        }
        else if(m_losslessPositionX == 0)
        {
            applyPrediction = 2;
        }
        switch(applyPrediction)
        {
        case 1:
            m_lastDCValue += *(pSource++);
            break;
        case 2:
            m_lastDCValue = *(pSource++) + *pPreviousLine;
            break;
        case 3:
            m_lastDCValue = *(pSource++) + *pPreviousLineColumn;
            break;
        case 4:
            m_lastDCValue += *(pSource++) + *pPreviousLine - *pPreviousLineColumn;
            break;
        case 5:
            m_lastDCValue += *(pSource++) + ((*pPreviousLine - *pPreviousLineColumn)>>1);
            break;
        case 6:
            m_lastDCValue -= *pPreviousLineColumn;
            m_lastDCValue >>= 1;
            m_lastDCValue += *(pSource++) + *pPreviousLine;
            break;
        case 7:
            m_lastDCValue += *pPreviousLine;
            m_lastDCValue >>= 1;
            m_lastDCValue += *(pSource++);
            break;
        default:
            IMEBRA_THROW(CodecCorruptedFileError, "Wrong predictor index in lossless jpeg stream");
        }

        m_lastDCValue &= m_valuesMask;
        *pDest++ = m_lastDCValue;

        ++pPreviousLine;
        ++pPreviousLineColumn;
        if(++m_losslessPositionX == m_width)
        {
            ++m_losslessPositionY;
            m_losslessPositionX = 0;
        }
    }
    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTag
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tag::~tag()
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tag::writeLength(streamWriter* pStream, std::uint16_t length) const
{
    IMEBRA_FUNCTION_START();

    length = (std::uint16_t)(length + sizeof(length));
    pStream->adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
    pStream->write((std::uint8_t*)&length, sizeof(length));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t tag::readLength(streamReader& stream) const
{
    IMEBRA_FUNCTION_START();

    std::uint16_t length = 0;
    stream.read((std::uint8_t*)&length, sizeof(length));
    stream.adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
    if(length > 1)
        length = (std::uint16_t)(length - 2);
    return (std::uint32_t)length;

    IMEBRA_FUNCTION_END();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagUnknown
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagUnknown::writeTag(streamWriter* pStream, jpegInformation& /* information */) const
{
    IMEBRA_FUNCTION_START();

    writeLength(pStream, 0);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagUnknown::readTag(streamReader& stream, jpegInformation* /* pInformation */, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagLength = readLength(stream);
    stream.seekForward(tagLength);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagSOF
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagSOF::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Calculate the components number
    ///////////////////////////////////////////////////////////
    std::uint8_t componentsNumber = (std::uint8_t)(information.m_channelsMap.size());

    // Write the tag's length
    ///////////////////////////////////////////////////////////
    writeLength(pStream, (std::uint16_t)(6 + componentsNumber * 3));

    // Write the precision, in bits
    ///////////////////////////////////////////////////////////
    std::uint8_t precisionBits=(std::uint8_t)information.m_precision;
    pStream->write(&precisionBits, 1);

    // Write the image's size, in pixels
    ///////////////////////////////////////////////////////////
    std::uint16_t imageWidth=(std::uint16_t)information.m_imageWidth;
    std::uint16_t imageHeight=(std::uint16_t)information.m_imageHeight;
    pStream->adjustEndian((std::uint8_t*)&imageHeight, 2, streamController::highByteEndian);
    pStream->adjustEndian((std::uint8_t*)&imageWidth, 2, streamController::highByteEndian);
    pStream->write((std::uint8_t*)&imageHeight, 2);
    pStream->write((std::uint8_t*)&imageWidth, 2);

    // write the components number
    ///////////////////////////////////////////////////////////
    pStream->write((std::uint8_t*)&componentsNumber, 1);

    // Write all the components specifications
    ///////////////////////////////////////////////////////////
    std::uint8_t componentId;
    std::uint8_t componentSamplingFactor;
    std::uint8_t componentQuantTable;

    for(jpeg::jpegInformation::tChannelsMap::const_iterator channelsIterator = information.m_channelsMap.begin(); channelsIterator != information.m_channelsMap.end(); ++channelsIterator)
    {
        ptrChannel pChannel=channelsIterator->second;

        componentId = channelsIterator->first;
        componentSamplingFactor = (std::uint8_t)( ((pChannel->m_samplingFactorX & 0xf) << 4) | (pChannel->m_samplingFactorY & 0xf) );
        componentQuantTable = (std::uint8_t)pChannel->m_quantTable;

        pStream->write((std::uint8_t*)&componentId, 1);
        pStream->write((std::uint8_t*)&componentSamplingFactor, 1);
        pStream->write((std::uint8_t*)&componentQuantTable, 1);
    }

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the tag's content
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOF::readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(stream);
    std::shared_ptr<streamReader> tagReader(stream.getReader(tagLength));

    pInformation->m_bLossless = (tagEntry==0xc3) || (tagEntry==0xc7);
    pInformation->m_process = (std::uint8_t)(tagEntry - 0xc0);

    // Read the precision, in bits
    ///////////////////////////////////////////////////////////
    std::uint8_t precisionBits;
    tagReader->read(&precisionBits, 1);
    pInformation->m_precision = precisionBits;

    // Read the image's size, in pixels
    ///////////////////////////////////////////////////////////
    std::uint16_t imageWidth, imageHeight;
    tagReader->read((std::uint8_t*)&imageHeight, 2);
    tagReader->read((std::uint8_t*)&imageWidth, 2);
    tagReader->adjustEndian((std::uint8_t*)&imageHeight, 2, streamController::highByteEndian);
    tagReader->adjustEndian((std::uint8_t*)&imageWidth, 2, streamController::highByteEndian);

    if(
            precisionBits < 8 ||
            precisionBits > 16 ||
            imageWidth > codecFactory::getCodecFactory()->getMaximumImageWidth() ||
            imageHeight > codecFactory::getCodecFactory()->getMaximumImageHeight())
    {
        IMEBRA_THROW(CodecImageTooBigError, "The factory settings prevented the loading of this image. Consider using codecFactory::setMaximumImageSize() to modify the settings");
    }

    pInformation->m_imageWidth = imageWidth;
    pInformation->m_imageHeight = imageHeight;

    // Read the components number
    ///////////////////////////////////////////////////////////
    pInformation->eraseChannels();
    std::uint8_t componentsNumber;
    tagReader->read(&componentsNumber, 1);

    // Get all the components specifications
    ///////////////////////////////////////////////////////////
    std::uint8_t componentId;
    std::uint8_t componentSamplingFactor;
    std::uint8_t componentQuantTable;
    for(std::uint8_t scanComponents(0); scanComponents<componentsNumber; ++scanComponents)
    {
        tagReader->read(&componentId, 1);
        tagReader->read(&componentSamplingFactor, 1);
        tagReader->read(&componentQuantTable, 1);

        ptrChannel pChannel(std::make_shared<jpeg::jpegChannel>());
        pChannel->m_quantTable = (int)componentQuantTable;
        if(pChannel->m_quantTable >= 16)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Corrupted quantization table index in SOF tag");
        }
        pChannel->m_samplingFactorX = componentSamplingFactor >> 4;
        pChannel->m_samplingFactorY = componentSamplingFactor & 0x0f;
        if(
                (pChannel->m_samplingFactorX != 1 &&
                 pChannel->m_samplingFactorX != 2 &&
                 pChannel->m_samplingFactorX != 4) ||
                (pChannel->m_samplingFactorY != 1 &&
                 pChannel->m_samplingFactorY != 2 &&
                 pChannel->m_samplingFactorY != 4)
                )
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Wrong sampling factor in SOF tag");
        }
        pInformation->m_channelsMap[componentId] = pChannel;
    }

    // Recalculate the MCUs' attributes
    ///////////////////////////////////////////////////////////
    pInformation->allocChannels();

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDHT
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the DHT entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDHT::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Used to write bytes into the stream
    /////////////////////////////////////////////////////////////////
    std::uint8_t byte;

    // There are two phases:
    //  the first phase calculates the tag's length, the second one
    //  write the tables' definitions
    /////////////////////////////////////////////////////////////////
    std::uint16_t tagLength = 0;
    for(int phase = 0; phase < 2; ++phase)
    {
        // Write the tag's length
        /////////////////////////////////////////////////////////////////
        if(phase == 1)
            writeLength(pStream, tagLength);

        // Scan all the used tables
        /////////////////////////////////////////////////////////////////
        for(int tableNum = 0; tableNum < 16; ++tableNum)
        {
            // Scan for DC and AC tables
            /////////////////////////////////////////////////////////////////
            for(int DcAc = 0; DcAc < 2; ++DcAc)
            {
                // bAdd is true if the huffman table is used by a channel
                /////////////////////////////////////////////////////////////////
                bool bAdd=false;

                for(jpeg::jpegInformation::tChannelsMap::const_iterator channelsIterator = information.m_channelsMap.begin(); !bAdd && channelsIterator != information.m_channelsMap.end(); ++channelsIterator)
                {
                    ptrChannel pChannel=channelsIterator->second;
                    bAdd= DcAc==0 ? (tableNum == pChannel->m_huffmanTableDC) : (tableNum == pChannel->m_huffmanTableAC);
                }

                // If the table is used by at least one channel, then write
                //  its definition
                /////////////////////////////////////////////////////////////////
                if(!bAdd)
                {
                    continue;
                }
                std::shared_ptr<huffmanTable> pHuffman;

                if(DcAc==0)
                {
                    pHuffman = information.m_pHuffmanTableDC[tableNum];
                }
                else
                {
                    pHuffman = information.m_pHuffmanTableAC[tableNum];
                }

                // Calculate the tag's length
                /////////////////////////////////////////////////////////////////
                if(phase == 0)
                {
                    pHuffman->incValueFreq(0x100);
                    pHuffman->calcHuffmanCodesLength(16);
                    // Remove the value 0x100 now
                    pHuffman->removeLastCode();

                    pHuffman->calcHuffmanTables();
                    tagLength = (std::uint16_t)(tagLength + 17);
                    for(std::uint32_t scanLength(0); scanLength != 16;)
                    {
                        tagLength = (std::uint16_t)(tagLength + (pHuffman->getValuesPerLength(++scanLength)));
                    }
                    continue;
                }

                // Write the huffman table
                /////////////////////////////////////////////////////////////////

                // Write the table ID
                /////////////////////////////////////////////////////////////////
                std::uint8_t tableID=(std::uint8_t)((DcAc<<4) | tableNum);
                pStream->write(&tableID, 1);

                // Write the values per length.
                /////////////////////////////////////////////////////////////////
                for(std::uint32_t scanLength(0); scanLength != 16;)
                {
                    byte = (std::uint8_t)(pHuffman->getValuesPerLength(++scanLength));
                    pStream->write(&byte, 1);
                }

                // Write the table values
                /////////////////////////////////////////////////////////////////
                std::uint32_t valueIndex(0);
                for(std::uint32_t scanLength(0); scanLength != 16; ++scanLength)
                {
                    for(std::uint32_t scanValues = 0; scanValues < pHuffman->getValuesPerLength(scanLength+1); ++scanValues)
                    {
                        byte = (std::uint8_t)(pHuffman->getOrderedValue(valueIndex++));
                        pStream->write(&byte, 1);
                    }
                }
            } // DcAc
        } // tableNum
    } // phase

    IMEBRA_FUNCTION_END();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DHT entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDHT::readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(stream);
    std::shared_ptr<streamReader> tagReader(stream.getReader(tagLength));

    // Used to read bytes from the stream
    /////////////////////////////////////////////////////////////////
    std::uint8_t byte;

        // Read all the defined tables
        /////////////////////////////////////////////////////////////////
        while(!tagReader->endReached())
        {
            // Read the table's ID
            /////////////////////////////////////////////////////////////////
            tagReader->read(&byte, 1);

            // Get a pointer to the right table
            /////////////////////////////////////////////////////////////////
            std::shared_ptr<huffmanTable> pHuffman;
            if((byte & 0xf0) == 0)
                pHuffman = pInformation->m_pHuffmanTableDC[byte & 0xf];
            else
                pHuffman = pInformation->m_pHuffmanTableAC[byte & 0xf];

            // Reset the table
            /////////////////////////////////////////////////////////////////
            pHuffman->reset();

            // Read the number of codes per length
            /////////////////////////////////////////////////////////////////
            for(std::uint32_t scanLength=0; scanLength != 16; )
            {
                tagReader->read(&byte, 1);
                pHuffman->setValuesPerLength(++scanLength, (std::uint32_t)byte);
            }

            // Used to store the values into the table
            /////////////////////////////////////////////////////////////////
            std::uint32_t valueIndex = 0;

            // Read all the values and store them into the huffman table
            /////////////////////////////////////////////////////////////////
            for(std::uint32_t scanLength = 0; scanLength != 16; ++scanLength)
            {
                for(std::uint32_t scanValues = 0; scanValues != pHuffman->getValuesPerLength(scanLength + 1); ++scanValues)
                {
                    tagReader->read(&byte, 1);
                    pHuffman->addOrderedValue(valueIndex++, (std::uint32_t)byte);
                }
            }

            // Calculate the huffman tables
            /////////////////////////////////////////////////////////////////
            pHuffman->calcHuffmanTables();
        }

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}




/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagSOS
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the SOS entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOS::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Calculate the components number
    /////////////////////////////////////////////////////////////////
    std::uint8_t componentsNumber((std::uint8_t)information.m_channelsList.size());

    // Write the tag's length
    /////////////////////////////////////////////////////////////////
    writeLength(pStream, (std::uint16_t)(4 + 2 * componentsNumber));

    // Write the component's number
    /////////////////////////////////////////////////////////////////
    pStream->write(&componentsNumber, 1);

    // Scan all the channels in the current scan
    /////////////////////////////////////////////////////////////////
    for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: information.m_channelsList)
    {
        std::uint8_t channelId(0);

        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        // Find the channel's ID
        /////////////////////////////////////////////////////////////////
        for(jpeg::jpegInformation::tChannelsMap::const_iterator mapIterator = information.m_channelsMap.begin(); mapIterator != information.m_channelsMap.end(); ++mapIterator)
        {
            if(mapIterator->second == pChannel)
            {
                channelId=mapIterator->first;
                break;
            }
        }

        // Write the channel's ID
        /////////////////////////////////////////////////////////////////
        pStream->write(&channelId, 1);

        // Write the ac/dc tables ID
        /////////////////////////////////////////////////////////////////
        std::uint8_t acdc = (std::uint8_t)(((pChannel->m_huffmanTableDC & 0xf)<<4) | (pChannel->m_huffmanTableAC & 0xf));

        pStream->write(&acdc, 1);
    }

    std::uint8_t byte;

    // Write the spectral index start
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)information.m_spectralIndexStart;
    pStream->write(&byte, 1);

    // Write the spectral index end
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)information.m_spectralIndexEnd;
    pStream->write(&byte, 1);

    // Write the hi/lo bit
    /////////////////////////////////////////////////////////////////
    byte = 0;
    pStream->write(&byte, 1);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the SOS entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOS::readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(stream);
    std::shared_ptr<streamReader> tagReader(stream.getReader(tagLength));

    pInformation->m_eobRun = 0;
    pInformation->m_channelsList.clear();

    std::uint8_t componentsNumber;
    tagReader->read(&componentsNumber, 1);

    if(componentsNumber == 0)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Jpeg image with zero color components");
    }

    std::uint8_t byte;
    for(std::uint8_t scanComponents = 0; scanComponents != componentsNumber; ++scanComponents)
    {
        tagReader->read(&byte, 1);

        jpeg::jpegInformation::tChannelsMap::const_iterator findChannel = pInformation->m_channelsMap.find(byte);
        if(findChannel == pInformation->m_channelsMap.end())
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Corrupted SOS tag found (scan component not specified by the SOF tag)");
        }
        ptrChannel pChannel = findChannel->second;

        pChannel->processUnprocessedAmplitudes();

        tagReader->read(&byte, 1);

        pChannel->m_huffmanTableDC=byte>>4;
        pChannel->m_huffmanTableAC=byte & 0xf;
        pChannel->m_pActiveHuffmanTableDC = pInformation->m_pHuffmanTableDC[pChannel->m_huffmanTableDC].get();
        pChannel->m_pActiveHuffmanTableAC = pInformation->m_pHuffmanTableAC[pChannel->m_huffmanTableAC].get();

        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        pInformation->m_channelsList.push_back(pChannel);

    }

    tagReader->read(&byte, 1);
    pInformation->m_spectralIndexStart = byte;

    tagReader->read(&byte, 1);
    pInformation->m_spectralIndexEnd = byte;

    tagReader->read(&byte, 1);
    if(byte != 0)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Progressive JPEG not supported");
    }

    pInformation->findMcuSize();

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDRI
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the DRI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDRI::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Write the tag's length
    /////////////////////////////////////////////////////////////////
    writeLength(pStream, 2);

    // Write the MCU per restart interval
    /////////////////////////////////////////////////////////////////
    std::uint16_t unitsPerRestartInterval = information.m_mcuPerRestartInterval;
    pStream->adjustEndian((std::uint8_t*)&unitsPerRestartInterval, 2, streamController::highByteEndian);
    pStream->write((std::uint8_t*)&unitsPerRestartInterval, 2);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DRI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDRI::readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(stream);
    std::shared_ptr<streamReader> tagReader(stream.getReader(tagLength));

    std::uint16_t unitsPerRestartInterval;
    tagReader->read((std::uint8_t*)&unitsPerRestartInterval, 2);
    tagReader->adjustEndian((std::uint8_t*)&unitsPerRestartInterval, 2, streamController::highByteEndian);
    pInformation->m_mcuPerRestartInterval=unitsPerRestartInterval;

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagRST
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the RST tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagRST::writeTag(streamWriter* /* pStream */, jpegInformation& /* information */) const
{
    return;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the RST tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagRST::readTag(streamReader&, jpegInformation* pInformation, std::uint8_t tagEntry) const
{
    IMEBRA_FUNCTION_START();

    // Reset the channels last dc value
    /////////////////////////////////////////////////////////////////
    for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: pInformation->m_channelsList)
    {
        pChannel->processUnprocessedAmplitudes();
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;
    }

    // Calculate the mcu processed counter
    /////////////////////////////////////////////////////////////////
    if(pInformation->m_mcuPerRestartInterval > 0)
    {
        std::uint32_t doneRestartInterval=(pInformation->m_mcuProcessed + pInformation->m_mcuPerRestartInterval-1) / pInformation->m_mcuPerRestartInterval-1;
        std::uint8_t doneRestartIntervalID=(std::uint8_t)(doneRestartInterval & 0x7);
        std::uint8_t foundRestartIntervalID=tagEntry & 0x7;
        if(foundRestartIntervalID<doneRestartIntervalID)
        doneRestartInterval += 8;
        doneRestartInterval -= doneRestartIntervalID;
        doneRestartInterval += foundRestartIntervalID;
        pInformation->m_mcuProcessed=(doneRestartInterval+1) * pInformation->m_mcuPerRestartInterval;
        pInformation->m_mcuProcessedY = pInformation->m_mcuProcessed / pInformation->m_mcuNumberX;
        pInformation->m_mcuProcessedX = pInformation->m_mcuProcessed - (pInformation->m_mcuProcessedY * pInformation->m_mcuNumberX);
        pInformation->m_mcuLastRestart = pInformation->m_mcuProcessed;

        // Update the lossless pixel's counter in the channels
        /////////////////////////////////////////////////////////////////
        for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: pInformation->m_channelsList)
        {
            pChannel->m_losslessPositionX = pInformation->m_mcuProcessedX / pChannel->m_blockMcuX;
            pChannel->m_losslessPositionY = pInformation->m_mcuProcessedY / pChannel->m_blockMcuY;
        }
    }

    pInformation->m_eobRun = 0;

    IMEBRA_FUNCTION_END();
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagEOI
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the EOI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagEOI::writeTag(streamWriter* pStream, jpegInformation& /* information */) const
{
    IMEBRA_FUNCTION_START();

    writeLength(pStream, 0);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the EOI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagEOI::readTag(streamReader&, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    pInformation->m_bEndOfImage=true;
}

} // namespace jpeg



///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
jpegStreamReader::jpegStreamReader(std::shared_ptr<streamReader> pStreamReader):
    m_pStreamReader(pStreamReader),
    m_inBitsBuffer(0),
    m_inBitsNum(0)
{
}


} // namespace codecs

} // namespace implementation

} // namespace imebra



