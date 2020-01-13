/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/


#include "../include/imebra/randomUidGenerator.h"
#include "../implementation/randomUidGeneratorImpl.h"

namespace imebra
{

RandomUIDGenerator::RandomUIDGenerator(const std::string& root, std::uint32_t departmentId, std::uint32_t modelId):
    BaseUIDGenerator(std::make_shared<implementation::uidGenerators::randomUidGenerator>(root, departmentId, modelId))
{
}

RandomUIDGenerator::RandomUIDGenerator(const RandomUIDGenerator& source):
    BaseUIDGenerator(source)
{
}

RandomUIDGenerator::~RandomUIDGenerator()
{
}


}

