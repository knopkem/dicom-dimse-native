/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file colorTransformsFactory.cpp
    \brief Implementation of the class ColorTransformsFactory.
*/

#include "../include/imebra/colorTransformsFactory.h"
#include "../implementation/colorTransformsFactoryImpl.h"
#include "../include/imebra/exceptions.h"
namespace imebra
{

std::string ColorTransformsFactory::normalizeColorSpace(const std::string& colorSpace)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(colorSpace);
}

bool ColorTransformsFactory::isMonochrome(const std::string& colorSpace)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::isMonochrome(colorSpace);
}

bool ColorTransformsFactory::isSubsampledX(const std::string& colorSpace)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::isSubsampledX(colorSpace);
}

bool ColorTransformsFactory::isSubsampledY(const std::string& colorSpace)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::isSubsampledY(colorSpace);
}

bool ColorTransformsFactory::canSubsample(const std::string& colorSpace)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::canSubsample(colorSpace);
}

std::string ColorTransformsFactory::makeSubsampled(const std::string& colorSpace, bool bSubsampleX, bool bSubsampleY)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::makeSubsampled(colorSpace, bSubsampleX, bSubsampleY);
}

std::uint32_t ColorTransformsFactory::getNumberOfChannels(const std::string& colorSpace)
{
    return imebra::implementation::transforms::colorTransforms::colorTransformsFactory::getNumberOfChannels(colorSpace);
}

Transform ColorTransformsFactory::getTransform(const std::string& startColorSpace, const std::string& endColorSpace)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<imebra::implementation::transforms::colorTransforms::colorTransformsFactory> factory(imebra::implementation::transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
    Transform transform(factory->getTransform(startColorSpace, endColorSpace));
    if(transform.m_pTransform == 0)
    {
        IMEBRA_THROW(ColorTransformsFactoryNoTransformError, "There is no color transform that can convert between the specified color spaces " << startColorSpace << " and " << endColorSpace);
    }
    return transform;

    IMEBRA_FUNCTION_END_LOG();
}

}
