/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file uidGeneratorFactory.h
    \brief Declaration of the class used to manage the UID generators.

*/


#include "../include/imebra/uidGeneratorFactory.h"
#include "../include/imebra/baseUidGenerator.h"
#include "../implementation/exceptionImpl.h"
#include "../implementation/uidGeneratorFactoryImpl.h"

namespace imebra
{


void UIDGeneratorFactory::registerUIDGenerator(const std::string& name, const BaseUIDGenerator& uidGenerator)
{
    implementation::uidGenerators::uidGeneratorFactory::getUidGeneratorFactory().registerUIDGenerator(name, getBaseUidGeneratorImplementation(uidGenerator));
}

BaseUIDGenerator UIDGeneratorFactory::getUIDGenerator(const std::string& name)
{
    IMEBRA_FUNCTION_START();

    return BaseUIDGenerator(implementation::uidGenerators::uidGeneratorFactory::getUidGeneratorFactory().getUidGenerator(name));

    IMEBRA_FUNCTION_END_LOG();
}

BaseUIDGenerator UIDGeneratorFactory::getDefaultUIDGenerator()
{
    IMEBRA_FUNCTION_START();

    return BaseUIDGenerator(implementation::uidGenerators::uidGeneratorFactory::getUidGeneratorFactory().getDefaultUidGenerator());

    IMEBRA_FUNCTION_END_LOG();
}

}
