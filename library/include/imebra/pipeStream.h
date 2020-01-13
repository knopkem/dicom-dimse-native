/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file pipeStream.h
    \brief Declaration of the Pipe class.

*/

#if !defined(pipe__INCLUDED_)
#define pipe__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class pipeSequenceStream;
}

class Memory;
class MutableMemory;
class BaseStreamInput;
class BaseStreamOutput;

///
/// \brief A Pipe can be used to push and pull data to/from an Imebra codec.
///
/// This is useful when an Imebra codec must be used with a data source
/// or a data sink not supported by the library (e.g. a TLS stream).
///
/// In order to allow Imebra to read data from a custom data source:
/// - allocate a Pipe class and use it as parameter for the StreamReader
///   needed by the codec
/// - from a secondary thread feed the data to the data source by using a
///   StreamWriter
///
/// In order to allow Imebra to write data to a custom data source:
/// - allocate a Pipe class and use it as parameter for the StreamWriter
///   needed by the codec
/// - from a secondary thread read the data feed to Pipe by using a
///   StreamReader
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API PipeStream
{

public:
    /// \brief Constructor
    ///
    /// \param circularBufferSize the size of the buffer that stores the data
    ///                           fed to the Pipe until it is fetched
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit PipeStream(size_t circularBufferSize);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source Pipe object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    PipeStream(const PipeStream& source);

    virtual ~PipeStream();

    PipeStream& operator=(const PipeStream& source) = delete;

    ///
    /// \brief Wait for the specified amount of time or until the internal buffer
    ///        is empty (all the data fed has been retrieved by the StreamReader)
    ///        then call terminate().
    ///
    /// Subsequent read and write operations will fail by throwing the
    /// exception StreamClosedError.
    ///
    /// \param timeoutMilliseconds the maximum time to wait until the internal
    ///                            buffer is empty, in milliseconds
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void close(unsigned int timeoutMilliseconds);

    ///
    /// \brief Return a BaseStreamInput object able to read from the PipeStream.
    ///
    /// \return a BaseStreamInput object able to read from the PipeStream.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    BaseStreamInput getStreamInput();

    ///
    /// \brief Return a BaseStreamOutput object able to write into the PipeStream.
    ///
    /// \return a BaseStreamOutput object able to write into the PipeStream
    ///
    ///////////////////////////////////////////////////////////////////////////////
    BaseStreamOutput getStreamOutput();

#ifndef SWIG
protected:

    explicit PipeStream(const std::shared_ptr<implementation::pipeSequenceStream>& pPipeStream);

private:
    friend const std::shared_ptr<implementation::pipeSequenceStream>& getPipeStreamImplementation(const PipeStream& stream);
    std::shared_ptr<implementation::pipeSequenceStream> m_pStream;
#endif

};


}
#endif // !defined(pipe__INCLUDED_)
