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

#if !defined(imebraStreamWriter__INCLUDED_)
#define imebraStreamWriter__INCLUDED_

#include <memory>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class streamWriter;
}

class BaseStreamOutput;
class CodecFactory;
class Memory;

///
/// \brief A StreamWriter is used to write data into a BaseStreamOutput
///        object.
///
/// A StreamWriter can be mapped to only a portion of the BaseStreamOutput it
/// manages.
///
/// \warning: The StreamWriter object IS NOT THREAD-SAFE: however, several
///           StreamWriter objects from different threads can be connected to
///           the same BaseStreamOutput object.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API StreamWriter
{

    friend class CodecFactory;
    friend class MutableTag;
    friend class AssociationSCU;
    friend class AssociationSCP;
    friend class MutableDataSet;

public:
    /// \brief Constructor.
    ///
    /// \param stream the BaseStreamOutput object on which the StreamWriter will
    ///               write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit StreamWriter(const BaseStreamOutput& stream);

    /// \brief Constructor.
    ///
    /// This version of the constructor limits the portion of the stream that
    /// the StreamWriter will be able to use.
    ///
    /// \param stream        the BaseStreamOutput object on which the StreamWriter
    ///                      will write
    /// \param virtualStart  the first visible byte of the managed stream
    /// \param virtualLength the number of visible bytes in the managed stream
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit StreamWriter(const BaseStreamOutput& stream, size_t virtualStart, size_t virtualLength);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source StreamWriter object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    StreamWriter(const StreamWriter& source);

    StreamWriter& operator=(const StreamWriter& source) = delete;

    ///
    /// \brief Write raw data into the stream.
    ///
    /// \param data         a pointer to the buffer which stores the data that
    ///                     must be written intothe stream
    /// \param bufferLength the number of bytes that must be written to the stream
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void write(const char* data, size_t dataSize);

    ///
    /// \brief Write raw data into the stream.
    ///
    /// \param memory a Memory object containing the data to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void write(const Memory& memory);

    ///
    /// \brief Flush all the unwritten data into the controlled stream
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void flush();

    virtual ~StreamWriter();

#ifndef SWIG
protected:
    explicit StreamWriter(const std::shared_ptr<implementation::streamWriter>& pWriter);

private:
    friend const std::shared_ptr<implementation::streamWriter>& getStreamWriterImplementation(const StreamWriter& streamWriter);
    std::shared_ptr<implementation::streamWriter> m_pWriter;
#endif
};

}

#endif // !defined(imebraStreamWriter__INCLUDED_)
