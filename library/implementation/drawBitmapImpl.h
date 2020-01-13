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
    \brief Declaration of the a class that draw an image into a bitmap.

This file is not included automatically by implementation.h

*/

#if !defined(imebraDrawBitmap_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraDrawBitmap_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "memoryImpl.h"
#include "transformImpl.h"
#include "../include/imebra/exceptions.h"
#include "../include/imebra/definitions.h"


#include <memory>
#include <string.h>

namespace imebra
{

	namespace implementation
	{

		/// \addtogroup group_helpers Helpers
		///
		/// @{

		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		/// \brief This class takes an image as an input and
		///         returns an 8 bit RGB bitmap of the requested
		///         image's area.
		///
		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
        class drawBitmap
		{
		public:
			/// \brief Constructor.
			///
			/// @param sourceImage  the input image that has to be
			///                      rendered
			/// @param transformsChain the list of transforms to be
			///                      applied to the image before it
			///                      is rendered. Can be null.
			///                     The transformation to RGB and
			///                      high bit shift are applied
			///                      automatically by this class
			///
			///////////////////////////////////////////////////////////
            drawBitmap(std::shared_ptr<transforms::transform> transformsChain);

			/// \brief Renders the image specified in the constructor
			///         into an RGB or BGR buffer.
			///
			/// The caller can pass to the function a memory object
			///  that will be used to store the RGB/BGR buffer,
			///  otherwise the function will allocate a new one.
			///
			/// Each row of pixels in the RGB/BGR buffer will be
			///  aligned according to the template parameter
			///  rowAlignBytes
			///
			/// \image html drawbitmap.png "drawbitmap"
			/// \image latex drawbitmap.png "drawbitmap" width=15cm
			///
			/// The figure illustrates how the getBitmap() method
			///  works:
			/// -# the image is resized according to the parameters
			///        totalWidthPixels and totalHeightPixels
			/// -# the area specified by visibleTopLeftX,
			///    visibleTopLeftY - visibleBottomRightX,
			///    visibleBottomRightY is rendered into the buffer
			///
			/// Please note that the rendering algorithm achieves the
			///  described results without actually resizing the image.
			///
            /// @param drawBitmapType The RGB order. Must be
			///                         drawBitmapBGR for BMP images
            /// @param rowAlignBytes  the boundary alignment of each
			///                         row. Must be 4 for BMP images
			/// @return the memory object in which the output buffer
            ///          is stored.
			///
			///////////////////////////////////////////////////////////
            std::shared_ptr<memory> getBitmap(const std::shared_ptr<const image>& sourceImage, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes);

            size_t getBitmap(const std::shared_ptr<const image>& sourceImage, drawBitmapType_t drawBitmapType, std::uint32_t rowAlignBytes, std::uint8_t* pBuffer, size_t bufferSize);

		protected:
            // Transform that calculates an 8 bit per channel RGB image
            std::shared_ptr<transforms::transform> m_userTransforms;
		};

		/// @}

	} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDrawBitmap_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
