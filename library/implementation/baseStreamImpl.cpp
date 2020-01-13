/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseStream.cpp
    \brief Implementation of the baseStream class.

*/

#include "baseStreamImpl.h"
#include <list>

namespace imebra
{

namespace implementation
{

baseStreamInput::~baseStreamInput()
{
}

bool baseStreamInput::seekable() const
{
    return false;
}

baseStreamOutput::~baseStreamOutput()
{
}


///////////////////////////////////////////////////////////
//
// streamTimeout
//
///////////////////////////////////////////////////////////

//
// Constructor
//
///////////////////////////////////////////////////////////
streamTimeout::streamTimeout(std::shared_ptr<baseStreamInput> pStream, std::chrono::seconds timeoutDuration):
    m_bExitTimeout(false),
    m_waitTimeoutThread(&streamTimeout::waitTimeout, this, pStream, timeoutDuration)
{
}


//
// Destructor
//
///////////////////////////////////////////////////////////
streamTimeout::~streamTimeout()
{
    {
        std::unique_lock<std::mutex> lock(m_lockFlag);
        m_bExitTimeout.store(true);
        m_flagCondition.notify_all();
    }

    m_waitTimeoutThread.join();
}


//
// Thread responsible to call terminate
//
///////////////////////////////////////////////////////////
void streamTimeout::waitTimeout(std::shared_ptr<baseStreamInput> pStream, std::chrono::seconds timeoutDuration)
{
    std::chrono::time_point<std::chrono::steady_clock> waitUntil(std::chrono::steady_clock::now() + timeoutDuration);

    std::unique_lock<std::mutex> lock(m_lockFlag);
    while(!m_bExitTimeout)
    {
        if(m_flagCondition.wait_until(lock, waitUntil) ==  std::cv_status::timeout)
        {
            pStream->terminate();
            return;
        }
    }
}





}

} // namespace imebra

