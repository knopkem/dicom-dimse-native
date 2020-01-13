/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file colorTransformsFactory.h
    \brief Declaration of the class used to retrieve a color transform able to
	        handle the requested color spaces.

*/

#if !defined(imebraColorTransformsFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraColorTransformsFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include <list>

#include "colorTransformImpl.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{

namespace colorTransforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class maintains a list of all the available
///         colorTransform classes and retrieve the
///         most appropriate transform class (or classes)
///         when a color space conversion is needed.
///
/// One instance of this class is statically allocated
///  by the library; the application does NOT have to
///  allocate its own instance of colorTransformsFactory.
///
/// A pointer to the statically allocated 
///  colorTransformsFactory class can be obtained by 
///  calling that static function 
///  colorTransformsFactory::getColorTransformsFactory().
///
/// The class can also retrieve more information
///  from a name of a color space (in dicom standard).
/// For instance, both the Dicom color space 
///  "YBR_FULL_422" and "YBR_FULL" describe the color
///  space YBR, but the first indicates that the image
///  is subsampled both horizontally and vertically.
///
/// The colorTransformsFactory can normalize the color
///  space name (e.g.: convert "YBR_FULL_422" to
///  "YBR_FULL") and can retrieve the subsampling
///  parameters.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class colorTransformsFactory
{
    colorTransformsFactory();

public:
	/// \internal
	/// \brief Register a color transform in the 
	///         colorTransformsFactory class.
	///
	/// @parameter newColorTransform the color transform to
	///                               be registered
	///
	///////////////////////////////////////////////////////////
	void registerTransform(std::shared_ptr<colorTransform> newColorTransform);

	///////////////////////////////////////////////////////////
	/// \name Static instance
	///       
	/// Static functions that return a pointer to the
	///  statically allocated instance of 
	///  colorTransformsFactory
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Retrieve a pointer to the unique statically 
	///         allocated instance of the colorTransforsFactory
	///         class.
	///         
	/// The application must use the colorTransformsFactory
	///  referenced by this function.
	///
	/// @return a pointer to the unique instance of the
	///          colorTransformsFactory
	///
	///////////////////////////////////////////////////////////
	static std::shared_ptr<colorTransformsFactory> getColorTransformsFactory();

	
	///////////////////////////////////////////////////////////
	/// \name Static functions
	///       
	/// Static functions that operate on the color space name
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Normalize a color space name.
	///
	/// The function converts all the chars to uppercase and
	///  remove additional information from the color space.
	///
	/// For instance, the color space "ybr_full_420" is
	///  converted to "YBR_FULL".
	///
	/// @param colorSpace the color space name to be normalized
	/// @return the normalized color space name
	///
	///////////////////////////////////////////////////////////
    static std::string normalizeColorSpace(const std::string& colorSpace);

	/// \brief Returns true if the color space name specified
	///         in the parameter has only one color channel and
	///         is monochrome (it doesn't have a lookup color
	///         table).
	///
	/// At the moment, only the color space names "MONOCHROME1" 
	///  and "MONOCHROME2" indicate a monochrome color space.
	///
	/// @param colorSpace the name of the color space to be
	///                    tested
	/// @return true if the color space indicated in the 
	///                    parameter is monochrome, or false
	///                    otherwise
	///
	///////////////////////////////////////////////////////////
    static bool isMonochrome(const std::string& colorSpace);

	/// \brief Returns true if the name of the color space
	///         specified in the parameter indicates that
	///         the chrominance channels are subsampled
	///         horizontally.
	///
	/// @param colorSpace the name of the color space to be
	///                    tested
	/// @return true if the name of the color space in the
	///                    parameter colorSpace has the
	///                    chrominance channels subsampled
	///                    horizontally
	///
	///////////////////////////////////////////////////////////
    static bool isSubsampledX(const std::string& colorSpace);

	/// \brief Returns true if the name of the color space
	///         specified in the parameter indicates that
	///         the chrominance channels are subsampled
	///         vertically.
	///
	/// @param colorSpace the name of the color space to be
	///                    tested
	/// @return true if the name of the color space in the
	///                    parameter colorSpace has the
	///                    chrominance channels subsampled
	///                    vertically
	///
	///////////////////////////////////////////////////////////
    static bool isSubsampledY(const std::string& colorSpace);

	/// \brief Returns true if the color space specified 
	///         in the parameter can be subsampled.
	///
	/// For instance, the color spaces "YBR_FULL" and 
	///  "YBR_PARTIAL" can be subsampled, but the color
	///  space "RGB" cannot be subsampled.
	///
	/// @param colorSpace the name of the color space to
	///                    be tested
	/// @return true if the name of the color space in the
	///                    parameter colorSpace can be
	///                    subsampled
	///
	///////////////////////////////////////////////////////////
    static bool canSubsample(const std::string& colorSpace);

	/// \brief Add the subsamplig information to a color space
	///         name.
	///
	/// Only the color spaces for which canSubsample() returns
	///  true can have the subsampling information.
	///
	/// @param colorSpace the name of the color space to which
	///                    the subsampling information should
	///                    be added
	/// @param bSubsampleX if true, then the function will make
	///                    the color space subsampled 
	///                    horizontally. The color space will
	///                    also be subsampled vertically
	/// @param bSubsampleY if true, then the function will make
	///                    the color space subsampled 
	///                    vertically
	/// @return the color space name subsampled as specified
	///
	///////////////////////////////////////////////////////////
    static std::string makeSubsampled(const std::string& colorSpace, bool bSubsampleX, bool bSubsampleY);

	/// \brief Returns the number of channels used by the
	///         specified color space.
	///
	/// For instance, the color space "RGB" has 3 color 
	///  channels, while the "MONOCHROME2" color space has
	///  1 color channel.
	///
	/// @param colorSpace the name of the color space for
	///                    which the number of channels
	///                    must be returned
	/// @return the number of color channels in the 
	///                    specified color channel
	///
	///////////////////////////////////////////////////////////
    static std::uint32_t getNumberOfChannels(const std::string& colorSpace);
	
	//@}
	

	///////////////////////////////////////////////////////////
	/// \name Color space conversion
	/// 
	/// Return the transform that convert one color space into
	///  another
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Returns a transform that can convert the
	///         pixels from one color space to another color
	///         space.
	///
	/// If no conversion is needed then the function
	///  returns 0.
	/// If the function cannot find any suitable transform 
	///  then a colorTransformsFactoryExceptionNoTransform
	///  is thrown.
	///
	/// @param startColorSpace the color space from which the
	///                         conversion has to take
	///                         place
	/// @param endColorSpace   the color space resulting 
	///                         from the conversion
	/// @return the transform that can convert the 
	///          startColorSpace into endColorSpace, or 0 if
	///          startColorSpace and endColorSpace have the
	///          same value
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<transform> getTransform(const std::string& startColorSpace, const std::string& endColorSpace);

	//@}

protected:
	typedef std::list<std::shared_ptr<colorTransform> > tTransformsList;
	tTransformsList m_transformsList;

public:
	// Force the construction of the factory before main()
	//  starts
	///////////////////////////////////////////////////////////
	class forceColorTransformsFactoryConstruction
	{
	public:
		forceColorTransformsFactoryConstruction()
		{
			colorTransformsFactory::getColorTransformsFactory();
		}
	};


};

/// @}

} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra


#endif // !defined imebraColorTransformsFactory
