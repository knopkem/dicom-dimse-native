/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tcpAddress.cpp
    \brief Implementation of the TCPAddress class.

*/

#include "../include/imebra/tcpAddress.h"
#include "../implementation/tcpSequenceStreamImpl.h"

namespace imebra
{

TCPAddress::TCPAddress(const std::shared_ptr<implementation::tcpAddress>& pAddress):
    m_pAddress(pAddress)
{
}

TCPAddress::TCPAddress(const TCPAddress& source): m_pAddress(getTCPAddressImplementation(source))
{
}

const std::shared_ptr<imebra::implementation::tcpAddress>& getTCPAddressImplementation(const TCPAddress& tcpAddress)
{
    return tcpAddress.m_pAddress;
}

std::string TCPAddress::getNode() const
{
    IMEBRA_FUNCTION_START();

    return m_pAddress->getNode();

    IMEBRA_FUNCTION_END_LOG();
}

std::string TCPAddress::getService() const
{
    IMEBRA_FUNCTION_START();

    return m_pAddress->getService();

    IMEBRA_FUNCTION_END_LOG();
}

TCPAddress::~TCPAddress()
{
}


TCPActiveAddress::TCPActiveAddress(const std::string& node, const std::string& service):
    TCPAddress(std::make_shared<implementation::tcpAddress>(node, service, implementation::tcpAddress::passiveSocket_t::active))
{
}

TCPActiveAddress::TCPActiveAddress(const TCPActiveAddress &source): TCPAddress(getTCPAddressImplementation(source))
{
}

TCPActiveAddress::~TCPActiveAddress()
{
}

TCPPassiveAddress::TCPPassiveAddress(const std::string& node, const std::string& service):
    TCPAddress(std::make_shared<implementation::tcpAddress>(node, service, implementation::tcpAddress::passiveSocket_t::passive))
{
}

TCPPassiveAddress::TCPPassiveAddress(const TCPPassiveAddress &source): TCPAddress(getTCPAddressImplementation(source))
{
}

TCPPassiveAddress::~TCPPassiveAddress()
{
}

}
