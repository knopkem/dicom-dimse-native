/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamController.h
    \brief Declaration of the the class used to control the streams.

*/

#if !defined(imebraStreamController_00B3C824_CD0D_4D99_8436_A41FCE9E4D6B__INCLUDED_)
#define imebraStreamController_00B3C824_CD0D_4D99_8436_A41FCE9E4D6B__INCLUDED_

#include "baseStreamImpl.h"
#include <memory>
#include <string>


namespace imebra
{

/// \addtogroup group_baseclasses
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class represents a stream controller.
///        A stream controller can read or write data
///         from/to a stream.
///
/// Do not use this class directly: use
///  imebra::streamWriter or imebra::streamReader
///  instead.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class streamController
{

#if(!defined IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE)
    #define IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE 4096
#endif

public:
    /// \brief Construct the stream controller and connect it
    ///         to a stream.
    ///
    /// The stream controller can be used to control only a
    ///  portion of the connected stream.
    ///
    /// @param pControlledStream pointer to the controlled
    ///                           stream
    /// @param virtualStart      position in the stream that
    ///                           is considered as the position
    ///                           0 by the stream controller
    /// @param virtualLength     the number of bytes in the
    ///                           connected stream that the
    ///                           controller will use.
    ///                          An EOF will be issued if the
    ///                           application tries to read
    ///                           beyond the virtual length
    ///
    ///////////////////////////////////////////////////////////
    streamController(size_t virtualStart = 0, size_t virtualLength = 0);

    streamController(size_t virtualStart, size_t virtualLength,
                     std::uint8_t* pBuffer, size_t bufferSize);

    virtual ~streamController();

    /// \brief Get the stream's position relative to the
    ///         virtual start position specified in the
    ///         constructor.
    ///
    /// The position is measured in bytes from the beginning
    ///  of the stream.
    ///
    /// The position represents the byte in the stream that
    ///  the next reading or writing function will use.
    ///
    /// If the virtual start position has been set in the
    ///  constructor, then the returned position is relative
    ///  to the virtual start position.
    ///
    /// @return the stream's read position, in bytes from the
    ///                  beginning of the stream or the virtual
    ///                  start position set in the constructor
    ///
    ///////////////////////////////////////////////////////////
    size_t position();

    /// \brief Return the position in bytes from the beginning
    ///         of the stream.
    ///
    /// It acts like the function position(), but it doesn't
    ///  adjust the position to the virtual stream position
    ///  specified in the constructor.
    ///
    /// @return the stream's read position, in bytes from the
    ///                  beginning of the stream
    ///
    ///////////////////////////////////////////////////////////
    size_t getControlledStreamPosition();

    ///////////////////////////////////////////////////////////
    /// \name Byte ordering
    ///
    ///////////////////////////////////////////////////////////
    //@{

    /// \brief Specify a byte ordering
    ///
    ///////////////////////////////////////////////////////////
    enum tByteOrdering
    {
        lowByteEndian=1,  ///< the byte ordering is low endian: the least significant bytes of a word are stored first
        highByteEndian=2  ///< the byte ordering is high endian: the most significant bytes of a word are stored first
    };

    /// \brief Adjust the pBuffer's content according to the
    ///         specified byte ordering
    ///
    /// If the specified byte ordering is different from the
    ///  platform's byte ordering, then the content of
    ///  pBuffer is modified to reflect the desidered byte
    ///  ordering.
    ///
    /// @param pBuffer  a pointer to the buffer that stores
    ///                  the data to order
    /// @param wordLength the size, in bytes, of the elements
    ///                  stored in the buffer pointed by
    ///                  pBuffer.
    ///                 E.g.: if the buffer stores a collection
    ///                  of words, this parameter will be 2.
    ///                 If the buffer stores a collection of
    ///                  double words, then this parameter will
    ///                  be 4.
    /// @param endianType the desidered byte ordering.
    ///                 If it differs from the platform's byte
    ///                  ordering, then the content of the
    ///                  memory pointed by pBuffer will be
    ///                  modified.
    /// @param words    The number of elements stored in the
    ///                  buffer pointed by pBuffer.
    ///                 This value represents the number of
    ///                  element that will be byte ordered.
    ///                 The total size of the buffer should be
    ///                  equal to words*wordLength
    ///
    ///////////////////////////////////////////////////////////
    static void adjustEndian(std::uint8_t* pBuffer, const size_t wordLength, const tByteOrdering endianType, const size_t words = 1);

    static void reverseEndian(std::uint8_t* pBuffer, const size_t wordLength, const size_t words = 1);

    static std::uint16_t adjustEndian(std::uint16_t buffer, const tByteOrdering endianType);

    static std::uint32_t adjustEndian(std::uint32_t buffer, const tByteOrdering endianType);

    static std::uint64_t adjustEndian(std::uint64_t buffer, const tByteOrdering endianType);

    static tByteOrdering getPlatformEndian();

    //@}

public:

    /// \brief true writeByte() must write all 0xff as
        ///         0xff, 0x00 anf readByte() as to convert all
        ///         0xff,0x00 to 0xff, as in jpeg streams.
    ///
    ///////////////////////////////////////////////////////////
    bool m_bJpegTags;


protected:
    /// \brief Used for buffered IO
    ///
    ///////////////////////////////////////////////////////////
    std::basic_string<std::uint8_t> m_dataBuffer;

    /// \brief Byte in the stream that represents the byte 0
    ///         in the stream controller.
    ///
    ///////////////////////////////////////////////////////////
    size_t m_virtualStart;

    /// \brief Max number of bytes that the stream controller
    ///         can control in the controlled stream. An EOF
    ///         signal will be raised when trying to read
    ///         past the maximum length.
    ///
    /// If this value is 0 then there are no limits on the
    ///  maximum length.
    ///
    ///////////////////////////////////////////////////////////
    size_t m_virtualLength;

    size_t m_dataBufferStreamPosition;

    size_t m_dataBufferCurrent;
    size_t m_dataBufferEnd;
};

///@}

} // namespace imebra

#endif // !defined(imebraStreamController_00B3C824_CD0D_4D99_8436_A41FCE9E4D6B__INCLUDED_)

