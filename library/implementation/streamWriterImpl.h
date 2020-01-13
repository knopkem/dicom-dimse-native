/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamWriter.h
    \brief Declaration of the the class used to write the streams.

*/


#if !defined(imebraStreamWriter_2C008538_F046_401C_8C83_2F76E1077DB0__INCLUDED_)
#define imebraStreamWriter_2C008538_F046_401C_8C83_2F76E1077DB0__INCLUDED_

#include "streamControllerImpl.h"

namespace imebra
{

namespace implementation
{

/// \brief Use this class to write into a baseStream
///         derived class.
///
/// Like the streamReader, this class is not multithread
///  safe, but several streamWriter (in several threads) 
///  can be connected to a single stream.
///
/// A streamWriter can also be connected only to a part
///  of the original stream: when this feature is used then
///  the streamWriter will act as if only the visible bytes
///  exist.
///
///////////////////////////////////////////////////////////
class streamWriter: public streamController
{
public:

    streamWriter(std::shared_ptr<baseStreamOutput> pControlledStream);

    /// \brief Creates the streamWriter and connects it to a
	///         baseStream object.
	///
	/// @param pControlledStream   the stream used by the 
	///                             streamWriter to write
	/// @param virtualStart        the first stream's byte
	///                             visible to the streamWriter
	/// @param virtualLength       the number of stream's bytes
	///                             visible to the 
	///                             streamWriter. Set to 0 to
	///                             allow the streamWriter to
	///                             see all the bytes
	///
	///////////////////////////////////////////////////////////
    streamWriter(std::shared_ptr<baseStreamOutput> pControlledStream, size_t virtualStart, size_t virtualLength);

    /// \brief Flushes the internal buffer, disconnects the
    ///         stream and destroys the streamWriter.
    ///
    ///////////////////////////////////////////////////////////
    virtual ~streamWriter();

	/// \brief Writes the internal buffer into the connected
	///         stream. This function is automatically called
	///         when needed, but your application can call it
	///         when  syncronization between the cached data
	///         and the stream is needed.
	///
	///////////////////////////////////////////////////////////
	void flushDataBuffer();

	/// \brief Write raw data into the stream.
	///
	/// The data stored in the pBuffer parameter will be
	///  written into the stream.
	/// 
	/// The function throws a streamExceptionWrite exception
	///  if an error occurs.
	///
	/// @param pBuffer   a pointer to the buffer which stores
	///                   the data that must be written into
	///                   the stream
	/// @param bufferLength the number of bytes that must be
	///                   written to the stream
	///
	///////////////////////////////////////////////////////////
    void write(const std::uint8_t* pBuffer, size_t bufferLength);

	/// \brief Write the specified amount of bits to the
	///         stream.
	///
	/// The functions uses a special bit pointer to keep track
	///  of the bytes that haven't been completly written.
	///
	/// The function throws a streamExceptionWrite exception
	///  if an error occurs.
	///
	/// @param buffer    bits to be written.
	///                  The bits must be right aligned
	/// @param bitsNum   the number of bits to write.
	///                  The function can write max 32 bits
	///                   
	///////////////////////////////////////////////////////////
    inline void writeBits(const std::uint32_t buffer, size_t bitsNum)
	{
        IMEBRA_FUNCTION_START();

		std::uint32_t tempBuffer(buffer);

		while(bitsNum != 0)
		{
			if(bitsNum <= (8 - m_outBitsNum))
			{
                m_outBitsBuffer = (std::uint8_t)(m_outBitsBuffer | (tempBuffer << (8 - m_outBitsNum - bitsNum)));
				m_outBitsNum += bitsNum;
                if(m_outBitsNum==8)
                {
                        m_outBitsNum = 0;
                        writeByte(m_outBitsBuffer);
                        m_outBitsBuffer = 0;
                }
                return;
			}
			if(m_outBitsNum == 0 && bitsNum >= 8)
            {
                    bitsNum -= 8;
                    writeByte(std::uint8_t(tempBuffer >> bitsNum));
            }
            else
            {
                m_outBitsBuffer |= (std::uint8_t)(tempBuffer >> (bitsNum + m_outBitsNum - 8));
                bitsNum -= (8-m_outBitsNum);
                writeByte(m_outBitsBuffer);
                m_outBitsBuffer = 0;
                m_outBitsNum = 0;
            }

            tempBuffer &= (((std::uint32_t)1) << bitsNum) - 1;

        }

		IMEBRA_FUNCTION_END();
	}

	/// \brief Reset the bit pointer used by writeBits().
	///
	/// A subsequent call to writeBits() will write data to
	///  a byte-aligned boundary.
	///
	///////////////////////////////////////////////////////////
	inline void resetOutBitsBuffer()
	{
        IMEBRA_FUNCTION_START();

		if(m_outBitsNum == 0)
			return;

		writeByte(m_outBitsBuffer);
		flushDataBuffer();
		m_outBitsBuffer = 0;
		m_outBitsNum = 0;

		IMEBRA_FUNCTION_END();
	}

	/// \brief Write a single byte to the stream, parsing it
	///         if m_pTagByte is not zero.
	///
	/// The byte to be written must be stored in the buffer 
	///  pointed by the parameter pBuffer.
	///
	/// If m_pTagByte is zero, then the function writes 
	///  the byte and returns.
	///
	/// If m_pTagByte is not zero, then the function adds a
	///  byte with value 0x0 after all the bytes with value
	///  0xFF.
	/// This mechanism is used to avoid the generation of
	///  the jpeg tags in a stream.
	///
	/// @param buffer    byte to be written
	///
	///////////////////////////////////////////////////////////
	inline void writeByte(const std::uint8_t buffer)
	{
        IMEBRA_FUNCTION_START();

        if(m_dataBufferCurrent == m_dataBuffer.size())
		{
			flushDataBuffer();
		}
        m_dataBuffer[m_dataBufferCurrent++] = buffer;
		if(m_bJpegTags && buffer == (std::uint8_t)0xff)
		{
            if(m_dataBufferCurrent == m_dataBuffer.size())
			{
				flushDataBuffer();
			}
            m_dataBuffer[m_dataBufferCurrent++] = 0;
		}

        IMEBRA_FUNCTION_END();
    }

private:
    std::shared_ptr<baseStreamOutput> m_pControlledStream;

	std::uint8_t m_outBitsBuffer;
    size_t m_outBitsNum;

};

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraStreamWriter_2C008538_F046_401C_8C83_2F76E1077DB0__INCLUDED_)
