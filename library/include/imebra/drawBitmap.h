/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file drawBitmap.h
    \brief Declaration of the class DrawBitmap.

*/

#if !defined(imebraDrawBitmap__INCLUDED_)
#define imebraDrawBitmap__INCLUDED_

#include <memory>
#include "definitions.h"
#include "mutableMemory.h"

namespace imebra
{

namespace implementation
{
    class drawBitmap;
}

class Transform;
class Image;

///
/// \brief DrawBitmap takes care of converting an Image object into an array
///        of bytes that can be displayed by the operating system facilities.
///
/// DrawBitmap can apply several transformations to the Image before generating
/// the bitmap.
///
/// DrawBitmap applies automatically the necessary color transform and high
/// bit shift in order to obtain a 8 bits per channel RGB image.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API DrawBitmap
{

public:
    /// \brief Construct a DrawBitmap with no transforms.
    ///
    /// The getBitmap() method will not apply any Transform to the Image before
    /// generating the bitmap.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    DrawBitmap();

    /// \brief Construct a DrawBitmap object that always apply the transforms in
    ///        the specified TransformsChain before calculating the bitmap of the
    ///        Image in the getBitmap() method.
    ///
    /// \param transformsChain the transforms to apply to the Image in the
    ///        getBitmap() method
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit DrawBitmap(const Transform& transformsChain);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source DrawBitmap object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    DrawBitmap(const DrawBitmap& source);

    DrawBitmap& operator=(const DrawBitmap& source) = delete;

    /// \brief Destructor
    ///
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~DrawBitmap();

    /// \brief Apply the transforms defined in the constructor (if any) to the
    ///        input image, then calculate an array of bytes containing a bitmap
    ///        that can be rendered by the operating system.
    ///
    /// \param image          the image for which the bitmap must be calculated
    /// \param drawBitmapType the type of bitmap to generate
    /// \param rowAlignBytes  the number of bytes on which the bitmap rows are
    ///                       aligned
    /// \param destination    a pointer to the pre-allocated buffer where
    ///                       getBitmap() will store the generated bitmap
    /// \param destinationSize the size of the allocated buffer
    /// \return the number of bytes occupied by the bitmap in the pre-allocated
    ///         buffer. If the number of occupied bytes is bigger than the value
    ///         of the parameter bufferSize then the method doesn't generate
    ///         the bitmap
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getBitmap(const Image& image, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes, char* destination, size_t destinationSize);

    /// \brief Apply the transforms defined in the constructor (if any) to the
    ///        input image, then calculate an array of bytes containing a bitmap
    ///        that can be rendered by the operating system.
    ///
    /// \param image          the image for which the bitmap must be calculated
    /// \param drawBitmapType the type of bitmap to generate
    /// \param rowAlignBytes  the number of bytes on which the bitmap rows are
    ///                       aligned
    /// \return a Memory object referencing the buffer containing the
    ///         generated bitmap
    ///
    ///////////////////////////////////////////////////////////////////////////////
    const Memory getBitmap(const Image& image, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes);

#ifndef SWIG
private:
    friend const std::shared_ptr<implementation::drawBitmap>& getDrawBitmapImplementation(const DrawBitmap& drawBitmap);
    std::shared_ptr<implementation::drawBitmap> m_pDrawBitmap;
#endif

};

}

#endif // !defined(imebraDrawBitmap__INCLUDED_)
