/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file pipeImpl.h
    \brief Declaration of the the pipe stream used by the imebra library.

*/

#if !defined(imebraPipeStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraPipeStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include <atomic>

#include "configurationImpl.h"

#include <condition_variable>
#include <mutex>
#include "baseSequenceStreamImpl.h"

#ifndef IMEBRA_PIPE_TIMEOUT_MS
#define IMEBRA_PIPE_TIMEOUT_MS 500
#endif

namespace imebra
{

namespace implementation
{

class memory;

///
/// \brief A PIPE to communicate between threads
///
///////////////////////////////////////////////////////////
class pipeSequenceStream
{
    friend class pipeSequenceStreamInput;
    friend class pipeSequenceStreamOutput;
public:

    ///
    /// \brief Constructor.
    ///
    /// \param bufferSize size of the circular buffer
    ///
    ///////////////////////////////////////////////////////////
    pipeSequenceStream(size_t bufferSize);

    ///
    /// \brief Destructor.
    ///
    /// Terminates pending read() and write() operations by
    /// throwing StreamClosedException.
    ///
    ///////////////////////////////////////////////////////////
    ~pipeSequenceStream();

    ///
    /// \brief Waits for the circular buffer to become empty
    ///        and then call terminate()
    ///
    /// @param timeoutMilliseconds the timeout (in ms)
    ///                            after which terminate() is
    ///                            called even if the internal
    ///                            buffer is not empty
    ///
    ///////////////////////////////////////////////////////////
    void close(unsigned int timeoutMilliseconds);

    ///
    /// \brief Forces the exit from pending writing and reading
    ///        operations.
    ///
    ///////////////////////////////////////////////////////////
    void terminate();

private:

    size_t read(std::uint8_t* pBuffer, size_t bufferLength);
    void write(const std::uint8_t* pBuffer, size_t bufferLength);

    std::shared_ptr<memory> m_pMemory;

    std::atomic<bool> m_bTerminate;
    size_t m_writePosition;
    size_t m_readPosition;
    size_t m_availableData;
    std::mutex m_positionMutex;
    std::condition_variable m_positionConditionVariable;
};


class pipeSequenceStreamInput: public baseSequenceStreamInput
{
public:
    pipeSequenceStreamInput(std::shared_ptr<pipeSequenceStream> pPipeStream);

    virtual size_t read(std::uint8_t* pBuffer, size_t bufferLength) override;

    virtual void terminate() override;

private:
    std::shared_ptr<pipeSequenceStream> m_pPipeStream;
};


class pipeSequenceStreamOutput: public baseSequenceStreamOutput
{
public:
    pipeSequenceStreamOutput(std::shared_ptr<pipeSequenceStream> pPipeStream);

    void write(const std::uint8_t* pBuffer, size_t bufferLength) override;

private:
    std::shared_ptr<pipeSequenceStream> m_pPipeStream;
};



} // namespace implementation

} // namespace imebra


#endif // !defined(imebraPipeStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
