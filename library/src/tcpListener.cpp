/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tcpListener.cpp
    \brief Implementation of the TCPListener class.

*/

#include "../include/imebra/tcpListener.h"
#include "../include/imebra/tcpAddress.h"
#include "../include/imebra/tcpStream.h"
#include "../implementation/tcpSequenceStreamImpl.h"

namespace imebra
{

TCPListener::TCPListener(const TCPPassiveAddress& address):
    m_pListener(std::make_shared<implementation::tcpListener>(address.m_pAddress))
{
}

TCPListener::TCPListener(const TCPListener &source): m_pListener(getTCPListenerImplementation(source))
{
}

TCPListener::~TCPListener()
{
}

const std::shared_ptr<implementation::tcpListener>& getTCPListenerImplementation(const TCPListener& source)
{
    return source.m_pListener;
}

void TCPListener::terminate()
{
    IMEBRA_FUNCTION_START();

    m_pListener->terminate();

    IMEBRA_FUNCTION_END_LOG();
}

TCPStream TCPListener::waitForConnection()
{
    IMEBRA_FUNCTION_START();

    return TCPStream(m_pListener->waitForConnection());

    IMEBRA_FUNCTION_END_LOG();
}


}

