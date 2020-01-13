/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOILUT.cpp
    \brief Implementation of VOILUT.

*/


#include "../include/imebra/VOILUT.h"
#include "../include/imebra/dataSet.h"
#include "../include/imebra/VOIDescription.h"
#include "../implementation/VOILUTImpl.h"
#include "../include/imebra/lut.h"

namespace imebra
{

VOILUT::VOILUT(const VOILUT& source): Transform(source)
{
}

VOILUT::VOILUT(const VOIDescription &voiDescription):
    Transform(std::make_shared<imebra::implementation::transforms::VOILUT>(voiDescription.getCenter(), voiDescription.getWidth(), voiDescription.getFunction()))
{
}


VOILUT::VOILUT(const LUT &lut):
    Transform(std::make_shared<imebra::implementation::transforms::VOILUT>(getLUTImplementation(lut)))
{
}


VOILUT::~VOILUT()
{
}

VOIDescription VOILUT::getOptimalVOI(const Image& inputImage, std::uint32_t topLeftX, std::uint32_t topLeftY, std::uint32_t width, std::uint32_t height)
{
    IMEBRA_FUNCTION_START();

    return VOIDescription(imebra::implementation::transforms::VOILUT::getOptimalVOI(getImageImplementation(inputImage), topLeftX, topLeftY, width, height));

    IMEBRA_FUNCTION_END_LOG();
}

}
