/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transform.cpp
    \brief Implementation of the Transform class.

*/

#include "../include/imebra/transform.h"
#include "../include/imebra/image.h"
#include "../implementation/transformImpl.h"

namespace imebra
{

Transform::~Transform()
{
}

Transform::Transform(const std::shared_ptr<imebra::implementation::transforms::transform>& pTransform): m_pTransform(pTransform)
{}

Transform::Transform(const Transform &source): m_pTransform(getTransformImplementation(source))
{
}

const std::shared_ptr<imebra::implementation::transforms::transform>& getTransformImplementation(const Transform& transform)
{
    return transform.m_pTransform;
}

bool Transform::isEmpty() const
{
    IMEBRA_FUNCTION_START();

    return m_pTransform == 0 || m_pTransform->isEmpty();

    IMEBRA_FUNCTION_END_LOG();
}

MutableImage Transform::allocateOutputImage(const Image& inputImage, std::uint32_t width, std::uint32_t height) const
{
    IMEBRA_FUNCTION_START();

    const std::shared_ptr<implementation::image> pImage(getImageImplementation(inputImage));
    return MutableImage(m_pTransform->allocateOutputImage(pImage->getDepth(),
                                                   pImage->getColorSpace(),
                                                   pImage->getHighBit(),
                                                   pImage->getPalette(),
                                                   width, height));

    IMEBRA_FUNCTION_END_LOG();
}

void Transform::runTransform(
            const Image& inputImage,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            MutableImage& outputImage,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const
{
    IMEBRA_FUNCTION_START();

    m_pTransform->runTransform(getImageImplementation(inputImage),
        inputTopLeftX,
        inputTopLeftY,
        inputWidth,
        inputHeight,
        getImageImplementation(outputImage),
        outputTopLeftX,
        outputTopLeftY);

    IMEBRA_FUNCTION_END_LOG();
}

}
