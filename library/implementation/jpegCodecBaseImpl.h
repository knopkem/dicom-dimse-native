/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegCodecBaseImpl.h
    \brief Declaration of the class jpegCodecBaseImpl.

*/

#if !defined(imebraJpegCodecBase_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_)
#define imebraJpegCodecBase_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_

#include <map>
#include <list>
#include "imageCodecImpl.h"
#include "streamReaderImpl.h"

// Bits used to left shift the values before they are passed to the
// DCT
#define JPEG_DECOMPRESSION_BITS_PRECISION 14


namespace imebra
{

class huffmanTable;

namespace implementation
{

namespace codecs
{

/// \addtogroup group_codecs
///
/// @{

// The following classes are used in the jpegCodec
//  declaration
///////////////////////////////////////////////////////////
namespace jpeg
{
    class tag;

    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    //
    //
    // CImbsJpegCodecChannel
    // An image's channel
    //
    //
    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    class jpegChannel: public channel
    {
    public:
        // Constructor
        ///////////////////////////////////////////////////////////
        jpegChannel();
        ~jpegChannel();

        // Quantization table's id
        ///////////////////////////////////////////////////////////
        int m_quantTable;

        // Blocks per MCU
        ///////////////////////////////////////////////////////////
        std::uint32_t m_blockMcuX;
        std::uint32_t m_blockMcuY;
        std::uint32_t m_blockMcuXY;

        // Last DC value
        ///////////////////////////////////////////////////////////
        std::int32_t m_lastDCValue;

        // Default DC value
        ///////////////////////////////////////////////////////////
        std::int32_t m_defaultDCValue;

        // Lossless position
        ///////////////////////////////////////////////////////////
        std::uint32_t m_losslessPositionX;
        std::uint32_t m_losslessPositionY;

        std::int32_t m_unprocessedAmplitudesBuffer[1024];
        std::uint32_t m_unprocessedAmplitudesCount;
        std::uint32_t m_unprocessedAmplitudesPredictor;

        // Huffman tables' id
        ///////////////////////////////////////////////////////////
        int m_huffmanTableDC;
        int m_huffmanTableAC;
        huffmanTable* m_pActiveHuffmanTableDC;
        huffmanTable* m_pActiveHuffmanTableAC;

        std::int32_t m_valuesMask;

        inline void addUnprocessedAmplitude(std::int32_t unprocessedAmplitude, std::uint32_t predictor, bool bMcuRestart)
        {
            if(bMcuRestart ||
                predictor != m_unprocessedAmplitudesPredictor ||
                m_unprocessedAmplitudesCount == sizeof(m_unprocessedAmplitudesBuffer) / sizeof(m_unprocessedAmplitudesBuffer[0]))
            {
                processUnprocessedAmplitudes();
                if(bMcuRestart)
                {
                    m_unprocessedAmplitudesPredictor = 0;
                    m_unprocessedAmplitudesBuffer[0] = unprocessedAmplitude + m_defaultDCValue;
                }
                else
                {
                    m_unprocessedAmplitudesPredictor = predictor;
                    m_unprocessedAmplitudesBuffer[0] = unprocessedAmplitude;
                }
                ++m_unprocessedAmplitudesCount;
                return;
            }
            m_unprocessedAmplitudesBuffer[m_unprocessedAmplitudesCount++] = unprocessedAmplitude;
        }

        void processUnprocessedAmplitudes();
    };

    struct jpegInformation
    {
        jpegInformation();

        void reset(imageQuality_t compQuality);

        // Erase the allocated channels
        ///////////////////////////////////////////////////////////
        void eraseChannels();

        // Allocate the image's channels
        ///////////////////////////////////////////////////////////
        void allocChannels();

        // Find the mcu's size
        ///////////////////////////////////////////////////////////
        void findMcuSize();

        // Recalculate the tables for dequantization/quantization
        void recalculateQuantizationTables(int table);

        // The image's size, in pixels
        ///////////////////////////////////////////////////////////
        std::uint32_t m_imageWidth;
        std::uint32_t m_imageHeight;

        // Encoding process
        ///////////////////////////////////////////////////////////
        std::uint8_t  m_process;

        // The bits per color component
        ///////////////////////////////////////////////////////////
        std::uint32_t m_precision;
        std::int32_t m_valuesMask;

        // true when the end of the image has been reached
        ///////////////////////////////////////////////////////////
        bool m_bEndOfImage;

        // The allocated channels
        ///////////////////////////////////////////////////////////
        typedef std::shared_ptr<jpeg::jpegChannel> ptrChannel;
        typedef std::map<std::uint8_t, ptrChannel> tChannelsMap;
        tChannelsMap m_channelsMap;

        // The list of the channels in the active scan
        ///////////////////////////////////////////////////////////
        std::list<ptrChannel> m_channelsList;

        // Huffman tables
        ///////////////////////////////////////////////////////////
        std::shared_ptr<huffmanTable> m_pHuffmanTableDC[16];
        std::shared_ptr<huffmanTable> m_pHuffmanTableAC[16];

        //
        // Quantization tables
        //
        ///////////////////////////////////////////////////////////
        std::uint32_t m_quantizationTable[16][64];

        // The number of MCUs per restart interval
        ///////////////////////////////////////////////////////////
        std::uint16_t m_mcuPerRestartInterval;

        // The number of processed MCUs
        ///////////////////////////////////////////////////////////
        std::uint32_t m_mcuProcessed;
        std::uint32_t m_mcuProcessedX;
        std::uint32_t m_mcuProcessedY;

        // The length of the EOB run
        ///////////////////////////////////////////////////////////
        std::uint32_t m_eobRun;

        // The last found restart interval
        ///////////////////////////////////////////////////////////
        std::uint32_t m_mcuLastRestart;

        // Spectral index and progressive bits reading
        ///////////////////////////////////////////////////////////
        std::uint32_t m_spectralIndexStart;
        std::uint32_t m_spectralIndexEnd;

        // true if we are reading a lossless jpeg image
        ///////////////////////////////////////////////////////////
        bool m_bLossless;

        // The maximum sampling factor
        ///////////////////////////////////////////////////////////
        std::uint32_t m_maxSamplingFactorX;
        std::uint32_t m_maxSamplingFactorY;

        // The number of MCUs (horizontal, vertical, total)
        ///////////////////////////////////////////////////////////
        std::uint32_t m_mcuNumberX;
        std::uint32_t m_mcuNumberY;
        std::uint32_t m_mcuNumberTotal;


        // The image's size, rounded to accomodate all the MCUs
        ///////////////////////////////////////////////////////////
        std::uint32_t m_jpegImageWidth;
        std::uint32_t m_jpegImageHeight;

        long long m_decompressionQuantizationTable[16][64];
        float m_compressionQuantizationTable[16][64];

    };
}


class jpegCodecBase
{
public:
    jpegCodecBase();
    virtual ~jpegCodecBase();

    /// \internal
    /// \brief This enumeration contains the tags used by
    ///         the jpeg codec
    ///////////////////////////////////////////////////////////
    enum tTagId
    {
        unknown = 0xff,

        sof0 = 0xc0,
        sof1 = 0xc1,
        sof2 = 0xc2,
        sof3 = 0xc3,

        dht = 0xc4,

        sof5 = 0xc5,
        sof6 = 0xc6,
        sof7 = 0xc7,

        sof9 = 0xc9,
        sofA = 0xca,
        sofB = 0xcb,

        sofD = 0xcd,
        sofE = 0xce,
        sofF = 0xcf,

        rst0 = 0xd0,
        rst1 = 0xd1,
        rst2 = 0xd2,
        rst3 = 0xd3,
        rst4 = 0xd4,
        rst5 = 0xd5,
        rst6 = 0xd6,
        rst7 = 0xd7,

        eoi = 0xd9,
        sos = 0xda,
        dqt = 0xdb,

        dri = 0xdd
    };

    // Register a tag in the jpeg codec
    ///////////////////////////////////////////////////////////
    void registerTag(tTagId tagId, std::shared_ptr<jpeg::tag> pTag);

    void writeTag(streamWriter* pDestinationStream, tTagId tagId, jpeg::jpegInformation& information) const;

    // Map of the available Jpeg tags
    ///////////////////////////////////////////////////////////
    typedef std::shared_ptr<jpeg::tag> ptrTag;
    typedef std::map<std::uint8_t, ptrTag> tTagsMap;
    tTagsMap m_tagsMap;

};


/// @}

namespace jpeg
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// jpegCodecTag
//
// The base class for all the jpeg tags
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tag
{
public:
    typedef std::shared_ptr<jpeg::jpegChannel> ptrChannel;

public:
    virtual ~tag();

    // Write the tag's content.
    // The function should call WriteLength first.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const = 0;

    // Read the tag's content. The function should call
    //  ReadLength first.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const = 0;

protected:
    // Write the tag's length
    ///////////////////////////////////////////////////////////
    void writeLength(streamWriter* pStream, std::uint16_t length) const;

    // Read the tag's length
    ///////////////////////////////////////////////////////////
    std::uint32_t readLength(streamReader& stream) const;
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// jpegCodecTagUnknown
//
// Read/write an unknown tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagUnknown: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// jpegCodecTagSOF
//
// Read/write a SOF tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagSOF: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read/write a DHT tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagDHT: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read/write a SOS tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagSOS: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read/write a DQT tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagDQT: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read/write a DRI tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagDRI: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read/write a RST tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagRST: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read/write an EOI tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class tagEOI: public tag
{
public:
    // Write the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void writeTag(streamWriter* pStream, jpegInformation& information) const override;

    // Read the tag's content.
    ///////////////////////////////////////////////////////////
    virtual void readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t tagEntry) const override;

};

} // namespace jpegCodec


class jpegStreamReader
{
public:
    /// \brief Build a streamReader and connect it to an
    ///         existing stream.
    ///
    ///////////////////////////////////////////////////////////
    jpegStreamReader(std::shared_ptr<streamReader> pStreamReader);

    /// \brief Read the specified amount of bits from the
    ///         stream.
    ///
    /// The functions uses a special bit pointer to keep track
    ///  of the bytes that haven't been completly read.
    ///
    /// The function throws a streamExceptionRead exception if
    ///  an error occurs.
    ///
    /// @param bitsNum   the number of bits to read.
    ///                  The function can read 32 bits maximum
    /// @return an integer containing the fetched bits, right
    ///                   aligned
    ///
    ///////////////////////////////////////////////////////////
    inline std::uint32_t readBits(size_t bitsNum)
    {
        IMEBRA_FUNCTION_START();

        const size_t bufferSize(8);

        // All the requested bits are already in the buffer.
        // Just return them.
        ///////////////////////////////////////////////////////////
        if(bitsNum <= m_inBitsNum)
        {
            std::uint32_t returnValue((m_inBitsBuffer & 0x00ff) >> (bufferSize - bitsNum));
            m_inBitsBuffer <<= bitsNum;
            m_inBitsNum -= bitsNum;
            return returnValue;
        }

        // Fill a local variable with the read bits
        ///////////////////////////////////////////////////////////
        std::uint32_t returnValue(0);

        // Some bits are in the bits buffer, copy them and reset
        //  the bits buffer
        ///////////////////////////////////////////////////////////
        if(m_inBitsNum != 0)
        {
            bitsNum -= m_inBitsNum;
            returnValue = ((std::uint32_t)((m_inBitsBuffer & 0xff) >> (bufferSize - m_inBitsNum))) << bitsNum;
        }

        // Read the requested number of bits
        ///////////////////////////////////////////////////////////
        for(;;)
        {
            if(bitsNum <= 8)
            {
                m_inBitsBuffer = readByte();
                returnValue |= (m_inBitsBuffer >> (bufferSize - bitsNum));
                m_inBitsBuffer <<= bitsNum;
                m_inBitsNum = 8 - bitsNum;
                return returnValue;
            }

            bitsNum -= 8;
            returnValue |= ((std::uint32_t)readByte()) << bitsNum;
        }

        IMEBRA_FUNCTION_END();
    }

    /// \brief Read one bit from the stream.
    ///
    /// The returned buffer will store the value 0 or 1,
    ///  depending on the value of the read bit.
    ///
    /// The function throws a streamExceptionRead if an error
    ///  occurs.
    ///
    /// @return the value of the read bit (1 or 0)
    ///
    ///////////////////////////////////////////////////////////
    inline std::uint32_t readBit()
    {
        IMEBRA_FUNCTION_START();

        if(m_inBitsNum == 0)
        {
            m_inBitsBuffer = readByte();
            m_inBitsNum = 8;
        }
        --m_inBitsNum;
        m_inBitsBuffer <<= 1;
        return (m_inBitsBuffer >> 8) & 1;

        IMEBRA_FUNCTION_END();
    }


    /// \brief Read one bit from the stream and add its value
    ///         to the specified buffer.
    ///
    /// The buffer pointed by the pBuffer parameter is
    ///  left-shifted before the read bit is inserted in the
    ///  least significant bit of the buffer.
    ///
    /// The function throws a streamExceptionRead if an error
    ///  occurs.
    ///
    /// @param pBuffer   a pointer to a std::uint32_t value that
    ///                   will be left shifted and filled
    ///                   with the read bit.
    ///
    ///////////////////////////////////////////////////////////
    inline void addBit(std::uint32_t* const pBuffer)
    {
        IMEBRA_FUNCTION_START();

        (*pBuffer) <<= 1;

        if(m_inBitsNum == 0)
        {
            m_inBitsBuffer = readByte();
            m_inBitsNum = 8;
        }
        m_inBitsBuffer <<= 1;
        --m_inBitsNum;
        *pBuffer |= (m_inBitsBuffer >> 8) & 1;

        IMEBRA_FUNCTION_END();
    }

    /// \brief Reset the bit pointer used by readBits(),
    ///         readBit() and addBit().
    ///
    /// A subsequent call to readBits(), readBit and
    ///  addBit() will read data from a byte-aligned boundary.
    ///
    ///////////////////////////////////////////////////////////
    inline void resetInBitsBuffer()
    {
        m_inBitsNum = 0;
    }

    /// \brief Read a single byte from the stream, parsing it
    ///         if m_pTagByte is not zero.
    ///
    /// The read byte is stored in the buffer pointed by the
    ///  parameter pBuffer.
    ///
    /// If m_pTagByte is zero, then the function reads a byte
    ///  and returns true.
    ///
    /// If m_pTagByte is not zero, then the function skips
    ///  all the bytes in the stream that have the value 0xFF.
    /// If the function doesn't have to skip any 0xFF bytes,
    ///  then the function just read a byte and returns true.
    ///
    /// If one or more 0xFF bytes have been skipped, then
    ///  the function returns a value depending on the byte
    ///  that follows the 0xFF run:
    /// - if the byte is 0, then the function fill the pBuffer
    ///    parameter with a value 0xFF and returns true
    /// - if the byte is not 0, then that value is stored in
    ///    the location pointed by m_pTagByte and the function
    ///    returns false.
    ///
    /// This mechanism is used to parse the jpeg tags in a
    ///  stream.
    ///
    /// @return          the read byte
    ///
    ///////////////////////////////////////////////////////////
    inline std::uint8_t readByte()
    {
        IMEBRA_FUNCTION_START();

        // Read one byte. Return immediatly if the tags are not
        //  activated
        ///////////////////////////////////////////////////////////
        std::uint8_t byte;
        m_pStreamReader->read(&byte, 1);
        if(byte != 0xff)
        {
            return byte;
        }
        while(byte == 0xff)
        {
            m_pStreamReader->read(&byte, 1);
        }

        if(byte == 0)
        {
            return 0xff;
        }

        IMEBRA_THROW(CodecCorruptedFileError, "Corrupted jpeg stream (tag in data stream)");

        IMEBRA_FUNCTION_END();
    }

private:
    std::shared_ptr<streamReader> m_pStreamReader;

    unsigned int m_inBitsBuffer;
    size_t m_inBitsNum;

};

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraJpegCodecBase_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_)
