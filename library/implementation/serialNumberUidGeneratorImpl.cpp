/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file serialNumberUidGeneratorImpl.cpp
    \brief Implementation of the class serialNumberUidGenerator.

*/

#include "serialNumberUidGeneratorImpl.h"
#include "dataHandlerStringUIImpl.h"
#include <cstdint>
#include <sstream>

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{

serialNumberUidGenerator::serialNumberUidGenerator(
        const std::string& root,
        std::uint32_t departmentId,
        std::uint32_t modelId,
        std::uint32_t serialNumber):
    m_counter(0)
{
    std::ostringstream buildRootUID;

    buildRootUID << handlers::normalizeUid(root) << "." << departmentId << "." << modelId << "." << serialNumber << ".";
    m_root = buildRootUID.str();
}


std::string serialNumberUidGenerator::getUid()
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


