/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file randomUidGeneratorImpl.cpp
    \brief Implementation of the UID generator using a random number generator

*/

#include "randomUidGeneratorImpl.h"
#include "dataHandlerStringUIImpl.h"
#include <cstdint>
#include <sstream>
#include <random>
#include <mutex>

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{

randomUidGenerator::randomUidGenerator(const std::string& root, std::uint32_t departmentId, std::uint32_t modelId)
{
    std::ostringstream buildRootUID;

    std::random_device randomDevice;

    std::uniform_int_distribution<std::uint16_t> dist(0, 65535);

    buildRootUID << handlers::normalizeUid(root) << "." << departmentId << "." << modelId << "." <<
                    dist(randomDevice) << "." <<
                    dist(randomDevice) << "." <<
                    dist(randomDevice) << ".";

    m_root = buildRootUID.str();

}

std::string randomUidGenerator::getUid()
{
    std::string timeUID(getTimeBasedPartialUID());

    std::ostringstream buildUID;
    buildUID << m_root << timeUID << ".";

    std::unique_lock<std::mutex> lock(m_lockGenerator);

    if(timeUID != m_lastUsedTime)
    {
        m_lastUsedTime = timeUID;
        m_counter = 0;
    }

    buildUID << m_counter++;

    return buildUID.str();
}


} // namespace uidGenerators

} // namespace implementation

} // namespace imebra


