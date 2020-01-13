/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamReader.h
    \brief Declaration of the the class used to read the streams.

*/

#if !defined(imebraStreamReader_F6221390_BC44_4B83_B5BB_3485222FF1DD__INCLUDED_)
#define imebraStreamReader_F6221390_BC44_4B83_B5BB_3485222FF1DD__INCLUDED_

#include "streamControllerImpl.h"
#include "../include/imebra/exceptions.h"
#include <exception>

namespace imebra
{

namespace implementation
{

class streamWriter;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a stream reader.
///        A stream reader can read data from a stream.
///        Several stream readers can share a single
///        baseStream derived object.
///
/// The stream reader object is not multithread safe, but
///  one single stream can have several streamReader
///  objects (in different threads) connected to it.
///
/// A stream reader can also be connected only to a part
///  of a stream: when this feature is used, then the
///  streamReader's client thinks that he is using a
///  whole stream, while the reader limits its view
///  to allowed stream's bytes only.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class streamReader: public streamController
{
public:
    /// \brief Build a streamReader and connect it to an
    ///         existing stream.
    ///
    ///////////////////////////////////////////////////////////
    streamReader(std::shared_ptr<baseStreamInput> pControlledStream);

    /// \brief Build a streamReader and connect it to a part
    ///         of an existing stream.
    ///
    /// When the streamReader is connected to a part of a
    ///  stream then all the its functions will act on the
    ///  viewable stream's part only.
    ///
    /// @param pControlledStream  the stream that will be
    ///                            controlled by the reader
    /// @param virtualStart       the first stream's byte
    ///                            visible to the reader
    /// @param virtualLength      the number of bytes visible
    ///                            to the reader. A value of 0
    ///                            means that all the bytes
    ///                            are visible
    ///
    ///////////////////////////////////////////////////////////
    streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, size_t virtualLength);

    streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, size_t virtualLength, std::uint8_t* pBuffer, size_t bufferLength);

    streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, std::uint8_t* pBuffer, size_t bufferLength);

    ~streamReader();

    std::shared_ptr<baseStreamInput> getControlledStream();

    /// \brief Returns a new streamReader object that starts
    ///        at the current stream location and continues
    ///        for the specified amount of bytes.
    ///
    /// @param virtualLength the amount of bytes that can be
    ///                      read from the new streamReader.
    ///                      The called streamReader will
    ///                      advance the read position past
    ///                      the end position of the new
    ///                      streamReader
    std::shared_ptr<streamReader> getReader(size_t virtualLength);

    /// \brief Read raw data from the stream.
    ///
    /// The number of bytes specified in the parameter
    ///  bufferLength will be read from the stream and copied
    ///  into a buffer referenced by the parameter pBuffer.
    /// The buffer's size must be equal or greater than
    ///  the number of bytes to read.
    ///
    /// The functions throws a streamExceptionRead
    ///  exception if an error occurs.
    ///
    /// @param pBuffer   a pointer to the buffer where the
    ///                  read data must be copied.
    ///                  the buffer's size must be at least
    ///                  equal to the size indicated in the
    ///                  bufferLength parameter.
    /// @param bufferLength the number of bytes to read from
    ///                  the stream.
    ///
    ///////////////////////////////////////////////////////////
    void read(std::uint8_t* pBuffer, size_t bufferLength);

    size_t readSome(std::uint8_t* pBuffer, size_t bufferLength);

    /// \brief Seek the stream's read position.
    ///
    /// The read position is moved to the specified byte in the
    ///  stream.
    /// Subsequent calls to the read operations like read(),
    ///  readBits(), readBit(), addBit() and readByte() will
    ///  read data from the position specified here.
    ///
    /// @param newPosition the new position to use for read
    ///                   operations, in bytes from the
    ///                   beginning of the stream
    ///
    ///////////////////////////////////////////////////////////
    void seek(size_t newPosition);

    void seekForward(std::uint32_t newPosition);

    bool seekable() const;

    size_t getVirtualLength() const;

    ///
    /// \brief Causes subsequent and current read operations
    ///        to fail with a StreamClosedError exception
    ///
    ///////////////////////////////////////////////////////////
    void terminate();

    /// \brief Returns true if the last byte in the stream
    ///         has already been read.
    ///
    /// @return true if the last byte in the stream has already
    ///          been read
    ///
    ///////////////////////////////////////////////////////////
    bool endReached();

private:
    friend class forwardStream;

    void addForwardWriter(const std::shared_ptr<streamWriter>& pWriter);
    void removeForwardWriter(const std::shared_ptr<streamWriter>& pWriter);

    /// \brief Read data from the file into the data buffer.
    ///
    /// The function reads as many bytes as possible until the
    ///  data buffer is full or the controlled stream cannot
    ///  supply any more byte.
    ///
    ///////////////////////////////////////////////////////////
    size_t fillDataBuffer();

    /// \brief Read data from the file into the data buffer.
    ///
    ///////////////////////////////////////////////////////////
    size_t fillDataBuffer(std::uint8_t* pDestinationBuffer, size_t readLength);

    std::shared_ptr<baseStreamInput> m_pControlledStream;

    typedef std::list<std::shared_ptr<streamWriter> > forwardList_t;
    forwardList_t m_forwardStream;

};


/// \brief In the constructor adds a forward stream to a 
///        streamWriter and removes it in the destructor.
///
///////////////////////////////////////////////////////////
class forwardStream
{
public:
    forwardStream(const std::shared_ptr<streamReader>& pSource, const std::shared_ptr<streamWriter>& destination);

    ~forwardStream();

private:
    const std::shared_ptr<streamReader> m_pSource;
    const std::shared_ptr<streamWriter> m_pDestination;
};



///@}

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraStreamReader_F6221390_BC44_4B83_B5BB_3485222FF1DD__INCLUDED_)
