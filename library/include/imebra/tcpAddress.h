/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tcpAddress.h
    \brief Declaration of the TCPAddress class.

*/

#if !defined(tcpAddress__INCLUDED_)
#define tcpAddress__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class tcpAddress;
}

///
/// \brief Represents a TCP address.
///
/// Use TCPActiveAddress to manage an address of an active socket (a socket
/// that initiates the connection) and TCPPassiveAddress for a passive
/// socket (a socket that listens for incoming connections).
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TCPAddress
{

    friend class TCPStream;
    friend class TCPListener;

public:

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source TCPAddress object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TCPAddress(const TCPAddress& source);

    TCPAddress& operator=(const TCPAddress& source) = delete;

    virtual ~TCPAddress();

    ///
    /// \brief Returns the node part of the TCP address.
    ///
    /// The TCP address is composed by two parts:
    /// - the node which identifies the machine in the network
    /// - the service which identifies one of the ports (services) in the
    ///   machine
    ///
    /// \return the node part of the TCP address
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getNode() const;

    ///
    /// \brief Returns the service part of the TCP address.
    ///
    /// See getNode() for more information.
    ///
    /// \return the service part of the TCP address
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getService() const;

#ifndef SWIG
protected:
    explicit TCPAddress(const std::shared_ptr<imebra::implementation::tcpAddress>& pAddress);

private:
    friend const std::shared_ptr<imebra::implementation::tcpAddress>& getTCPAddressImplementation(const TCPAddress& tcpAddress);
    std::shared_ptr<implementation::tcpAddress> m_pAddress;
#endif
};


///
/// \brief Represents an address of an active socket (a socket that initiates
///        the connection with the peer).
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TCPActiveAddress: public TCPAddress
{
public:
    ///
    /// \brief Constructor.
    ///
    /// Constructs an active TCP address from a node and a service description.
    ///
    /// The node may be a host name or address (both IPv4 and IPv6) while the
    /// service name may be a port number (in string format) or name (e.g.
    /// "FTP").
    ///
    /// \param node    the host name or address (e.g. "192.168.10.20" or
    ///                "example.com"). Use an empty string to refer to the
    ///                local host.
    /// \param service the service port (in string format) or name (e.g.
    ///                "140" or "ftp").
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TCPActiveAddress(const std::string& node, const std::string& service);

    TCPActiveAddress(const TCPActiveAddress& source);

    TCPActiveAddress& operator=(const TCPActiveAddress&) = delete;

    virtual ~TCPActiveAddress();
};


///
/// \brief Represents an address of a passive socket (a socket that listens
///        for connections initiated by the peers).
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TCPPassiveAddress: public TCPAddress
{
public:
    ///
    /// \brief Constructor.
    ///
    /// Constructs an active TCP address from a node and a service description.
    ///
    /// The node may be a host name or address (both IPv4 and IPv6) while the
    /// service name may be a port number (in string format) or name (e.g.
    /// "FTP").
    ///
    /// \param node    the host name or address (e.g. "192.168.10.20" or
    ///                "example.com"). Use an empty string to refer to all the
    ///                TCP devices on the local host.
    /// \param service the service port (in string format) or name (e.g.
    ///                "140" or "ftp").
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TCPPassiveAddress(const std::string& node, const std::string& service);

    TCPPassiveAddress(const TCPPassiveAddress& source);

    TCPPassiveAddress& operator=(const TCPPassiveAddress&) = delete;

    virtual ~TCPPassiveAddress();
};


}
#endif // !defined(tcpAddress__INCLUDED_)
