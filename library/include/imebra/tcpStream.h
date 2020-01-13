/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tcpStream.h
    \brief Declaration of the TCPStream class.

*/

#if !defined(tcpStream__INCLUDED_)
#define tcpStream__INCLUDED_

#include <string>
#include "baseStreamInput.h"
#include "baseStreamOutput.h"
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class tcpSequenceStream;
}

class TCPActiveAddress;
class TCPAddress;

///
/// \brief Represents a TCP stream.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TCPStream
{

    friend class TCPListener;

public:
    ///
    /// \brief Construct a TCP socket and connects it to the destination address.
    ///
    /// This is a non-blocking operation (the connection proceed after the
    /// constructor returns). Connection errors will be reported later while
    /// the communication happens.
    ///
    /// \param address the address to which the socket has to be connected.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit TCPStream(const TCPActiveAddress& address);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source TCPStream object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TCPStream(const TCPStream& source);

    virtual ~TCPStream();

    TCPStream& operator=(const TCPStream& source) = delete;

    ///
    /// \brief Returns the address of the connected peer.
    ///
    /// \return the address of the connected peer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    const TCPAddress getPeerAddress() const;

    ///
    /// \brief Return a BaseStreamInput object able to read from the TCPStream.
    ///
    /// \return a BaseStreamInput object able to read from the TCPStream.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    BaseStreamInput getStreamInput();

    ///
    /// \brief Return a BaseStreamOutput object able to write into the TCPStream.
    ///
    /// \return a BaseStreamOutput object able to write into the TCPStream
    ///
    ///////////////////////////////////////////////////////////////////////////////
    BaseStreamOutput getStreamOutput();

#ifndef SWIG
protected:

    explicit TCPStream(const std::shared_ptr<implementation::tcpSequenceStream>& pTcpStream);

private:
    friend const std::shared_ptr<implementation::tcpSequenceStream>& getTCPStreamImplementation(const TCPStream& stream);
    std::shared_ptr<implementation::tcpSequenceStream> m_pStream;
#endif
};



}
#endif // !defined(tcpStream__INCLUDED_)
