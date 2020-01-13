/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tcpListener.h
    \brief Declaration of the TCPListener class.

*/

#if !defined(tcpListener__INCLUDED_)
#define tcpListener__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class tcpListener;
}

class TCPPassiveAddress;
class TCPStream;


///
/// \brief Represents listening TCP socket.
///
/// Once allocated the socket starts listening at the address declared in
/// the constructor.
///
/// A loop in the client application should call waitForConnection() in order
/// to retrieve all the connections accepted by the socket.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TCPListener
{

public:
    /// \brief Constructor.
    ///
    /// Constructs a listening socket and starts listening for incoming
    /// connections.
    ///
    /// @param address the address to which the listening socket must be bound
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit TCPListener(const TCPPassiveAddress& address);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source TCPListener object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TCPListener(const TCPListener& source);

    virtual ~TCPListener();

    TCPListener& operator=(const TCPListener& source) = delete;

    /// \brief Waits for an incoming connection on the listening socket.
    ///
    /// The method blocks until a new connection is accepted or until the socket
    /// is closed, in which case it throws the exception StreamClosedError.
    ///
    /// The socket is closed by the TCPStream's destructor or by a call to
    /// terminate().
    ///
    /// @return a new TCPStream object bound to the new accepted connection.
    ///         the returned TCPStream object will be owned by the caller
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TCPStream waitForConnection();

    ///
    /// \brief Instruct the listener to terminate any pending action.
    ///
    /// If a thread is in the method waitForConnection() then it will receive
    /// the exception StreamClosedError. StreamClosedError will be also thrown
    /// for each subsequent call to waitForConnection().
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void terminate();

#ifndef SWIG
private:
    friend const std::shared_ptr<implementation::tcpListener>& getTCPListenerImplementation(const TCPListener& source);
    std::shared_ptr<implementation::tcpListener> m_pListener;
#endif

};

}
#endif // !defined(tcpListener__INCLUDED_)
