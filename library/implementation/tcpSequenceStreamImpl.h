/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tcpSequenceStreamImpl.h
    \brief Declaration of the the TCP stream used by the imebra library.

*/

#if !defined(imebraTcpSequenceStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraTcpSequenceStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include <atomic>
#include <vector>

#include "configurationImpl.h"

#ifdef IMEBRA_WINDOWS
#pragma comment(lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <WS2tcpip.h>
#else
#include <netdb.h>
#endif

#include <condition_variable>
#include <mutex>
#include <memory>
#include "baseSequenceStreamImpl.h"

#ifndef IMEBRA_TCP_TIMEOUT_MS
#define IMEBRA_TCP_TIMEOUT_MS 1000
#endif

namespace imebra
{

namespace implementation
{

#ifdef IMEBRA_WINDOWS

class initWinsock
{
public:
    initWinsock();
    ~initWinsock();

    static std::shared_ptr<initWinsock> getWinsockInitialization();
};

#define INIT_WINSOCK std::shared_ptr<initWinsock> windowsInitWinsock(initWinsock::getWinsockInitialization())

#else

#define INIT_WINSOCK

#endif

class tcpSequenceStreamInput;
class tcpSequenceStreamOutput;


long throwTcpException(long socketOperationResult);


///
/// \brief A TCP address.
///
///////////////////////////////////////////////////////////
class tcpAddress
{
public:
    enum class passiveSocket_t: std::uint8_t
    {
        passive,
        active
    };

    tcpAddress(const std::string& node, const std::string& service, passiveSocket_t passiveSocket);
    tcpAddress(const sockaddr& address, socklen_t addressLength);
    ~tcpAddress();

    std::string getNode() const;
    std::string getService() const;
    const sockaddr* getSockAddr() const;
    socklen_t getSockAddrLen() const;
    int getFamily() const;
    int getType() const;
    int getProtocol() const;

private:
    std::string m_node;
    std::string m_service;
    std::vector<std::uint8_t> m_sockAddr;

};


///
/// \brief Base class for tcpSequenceStream and
///        tcpListener
///
///////////////////////////////////////////////////////////
class tcpBaseSocket
{
public:
    ///
    /// \brief Constructor
    ///
    /// \param socket the socket number
    ///
    ///////////////////////////////////////////////////////////
    tcpBaseSocket(int socket);

    ///
    /// \brief Destructor. Closes the socket.
    ///
    ///////////////////////////////////////////////////////////
    virtual ~tcpBaseSocket();

    ///
    /// \brief Enable or disable the blocking mode
    ///
    /// \param bBlocking true to enable the blocking mode,
    ///                  false to disable it
    ///
    ///////////////////////////////////////////////////////////
    void setBlockingMode(bool bBlocking);

    ///
    /// \brief Forces a termination of pending and subsequent
    ///        read and write operations by causing them to
    ///        throw SocketClosedException.
    ///
    ///////////////////////////////////////////////////////////
    void terminate();

    ///
    /// \brief If terminate has been called then throws
    ///        StreamClosedException, otherwise returns.
    ///
    ///////////////////////////////////////////////////////////
    void isTerminating();

    enum class pollType_t
    {
        read = 0,
        write = 1
    };

    ///
    /// \brief Execute a poll on the socket for the specified
    ///        flags. The timeout is defined by
    ///        IMEBRA_TCP_TIMEOUT_MS.
    ///
    /// \param flags flags to poll
    ///
    ///////////////////////////////////////////////////////////
    void poll(pollType_t pollType);

    ///
    /// \brief Allocate this class at the beginning of a
    ///        blocking method.
    ///
    ///////////////////////////////////////////////////////////
    class tcpTerminateWaiting
    {
    public:
        tcpTerminateWaiting(tcpBaseSocket& terminateObject);
        ~tcpTerminateWaiting();

        tcpBaseSocket& m_terminateObject;
    };

protected:
    int m_socket;

    std::atomic<bool> m_bTerminate;
    std::atomic<int> m_waiting;
    std::condition_variable m_waitingCondition;
    std::mutex m_waitingMutex;
};


///
/// \brief A TCP socket
///
///////////////////////////////////////////////////////////
class tcpSequenceStream: protected tcpBaseSocket
{
    friend class tcpSequenceStreamInput;
    friend class tcpSequenceStreamOutput;

public:

    ///
    /// \brief Constructor.
    ///
    /// \param tcpSocket a TCP socket
    /// \param pAddress  the peer address to which the socket
    ///                  is already connected
    ///
    ///////////////////////////////////////////////////////////
    tcpSequenceStream(int tcpSocket, std::shared_ptr<tcpAddress> pAddress);

    ///
    /// \brief Constructor.
    ///
    /// Creates a socket and connect it to the specified
    /// address. The connection occurs in non-blocking mode
    /// therefore the constructor returns immediately.
    ///
    /// Connection errors are reported at the first call
    /// to read() or write().
    ///
    /// \param pAddress the address to which the socket must
    ///                 be connected
    ///
    ///////////////////////////////////////////////////////////
    tcpSequenceStream(std::shared_ptr<tcpAddress> pAddress);

    ///
    /// \brief Destructor.
    ///
    /// Terminates pending read() and write() operations by
    /// throwing StreamClosedException, shutdown and closes
    /// the socket.
    ///
    ///////////////////////////////////////////////////////////
    ~tcpSequenceStream();

    ///
    /// \brief Returns the address to which the socket is
    ///        connected.
    ///
    /// \return the peer's address
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<tcpAddress> getPeerAddress() const;

    void terminate();

private:
    size_t read(std::uint8_t* pBuffer, size_t bufferLength);
    void write(const std::uint8_t* pBuffer, size_t bufferLength);

    const std::shared_ptr<tcpAddress> m_pAddress;
};


class tcpSequenceStreamInput: public baseSequenceStreamInput
{
public:
    tcpSequenceStreamInput(std::shared_ptr<tcpSequenceStream> pTcpStream);

    virtual size_t read(std::uint8_t* pBuffer, size_t bufferLength) override;

    virtual void terminate() override;

private:
    std::shared_ptr<tcpSequenceStream> m_pTcpStream;
};


class tcpSequenceStreamOutput: public baseSequenceStreamOutput
{
public:
    tcpSequenceStreamOutput(std::shared_ptr<tcpSequenceStream> pTcpStream);

    void write(const std::uint8_t* pBuffer, size_t bufferLength) override;

private:
    std::shared_ptr<tcpSequenceStream> m_pTcpStream;
};


///
/// \brief Listens for incoming TCP connections.
///
///////////////////////////////////////////////////////////
class tcpListener: public tcpBaseSocket
{
public:
    ///
    /// \brief Constructors. Creates a socket that listens for
    ///        incoming connections at the specified address.
    ///
    /// \param pAddress the address to which the socket must be
    ///                 bound
    ///
    ///////////////////////////////////////////////////////////
    tcpListener(std::shared_ptr<tcpAddress> pAddress);

    ///
    /// \brief Terminates pending waitForConnection() calls
    ///        and closes the socket.
    ///
    ///////////////////////////////////////////////////////////
    ~tcpListener();

    ///
    /// \brief Wait for an incoming connection until a
    ///        connection is accepted or the socket is
    ///        terminated by the destructor or terminate(),
    ///        in which case it throws SocketClosedError.
    ///
    /// \return an tcpSequenceStream for to the accepted
    ///         connection
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<tcpSequenceStream> waitForConnection();

};


} // namespace implementation

} // namespace imebra


#endif // !defined(imebraTcpSequenceStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
