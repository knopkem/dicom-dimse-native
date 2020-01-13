/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file drawBitmap.cpp
    \brief Implementation of the class DrawBitmap.

*/

#include "../include/imebra/drawBitmap.h"
#include "../implementation/drawBitmapImpl.h"
#include "../include/imebra/image.h"
#include "../include/imebra/transform.h"

namespace imebra
{

DrawBitmap::DrawBitmap():
    m_pDrawBitmap(std::make_shared<implementation::drawBitmap>(std::shared_ptr<implementation::transforms::transform>()))
{
}

DrawBitmap::DrawBitmap(const Transform& transform):
    m_pDrawBitmap(std::make_shared<implementation::drawBitmap>(transform.m_pTransform))
{
}

DrawBitmap::DrawBitmap(const DrawBitmap& source): m_pDrawBitmap(getDrawBitmapImplementation(source))
{
}

const std::shared_ptr<implementation::drawBitmap>& getDrawBitmapImplementation(const DrawBitmap& drawBitmap)
{
    return drawBitmap.m_pDrawBitmap;
}

DrawBitmap::~DrawBitmap()
{
}

size_t DrawBitmap::getBitmap(const Image& image, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes, char* buffer, size_t bufferSize)
{
    IMEBRA_FUNCTION_START();

    return m_pDrawBitmap->getBitmap(getImageImplementation(image), drawBitmapType, rowAlignBytes, (std::uint8_t*)buffer, bufferSize);

    IMEBRA_FUNCTION_END_LOG();
}

const Memory DrawBitmap::getBitmap(const Image& image, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes)
{
    IMEBRA_FUNCTION_START();

    return Memory(m_pDrawBitmap->getBitmap(getImageImplementation(image), drawBitmapType, rowAlignBytes));

    IMEBRA_FUNCTION_END_LOG();
}

}
