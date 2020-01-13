/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseUidGeneratorImpl.cpp
    \brief Implementation of the base class for UID generators.

*/

#include "baseUidGeneratorImpl.h"
#include <chrono>
#include <sstream>

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{


baseUidGenerator::~baseUidGenerator()
{
}

std::string baseUidGenerator::getTimeBasedPartialUID()
{
    std::chrono::system_clock::time_point now(std::chrono::system_clock::now());
    auto sinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::uint32_t lowerEpoch = (std::uint32_t)(sinceEpoch & 0xffffffffu);
    std::uint32_t higherEpoch = (std::uint32_t)((sinceEpoch >> 32) & 0xffffffffu);

    std::ostringstream timeUID;
    timeUID << higherEpoch << "." << lowerEpoch;
    return timeUID.str();
}

} // namespace uidGenerators

} // namespace implementation

} // namespace imebra



