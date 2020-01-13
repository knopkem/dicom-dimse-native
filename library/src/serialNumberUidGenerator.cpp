/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/



#include "../include/imebra/serialNumberUidGenerator.h"
#include "../implementation/serialNumberUidGeneratorImpl.h"

namespace imebra
{

SerialNumberUIDGenerator::SerialNumberUIDGenerator(const std::string& root, std::uint32_t departmentId, std::uint32_t modelId, std::uint32_t serialNumber):
    BaseUIDGenerator(std::make_shared<implementation::uidGenerators::serialNumberUidGenerator>(root, departmentId, modelId, serialNumber))
{
}

SerialNumberUIDGenerator::SerialNumberUIDGenerator(const SerialNumberUIDGenerator& source):
    BaseUIDGenerator(source)
{
}

SerialNumberUIDGenerator::~SerialNumberUIDGenerator()
{
}


}
