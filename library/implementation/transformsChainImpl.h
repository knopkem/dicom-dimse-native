/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transformsChain.h
    \brief Declaration of the class transformsChain.

*/

#if !defined(imebraTransformsChain_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_)
#define imebraTransformsChain_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_

#include <vector>
#include "transformImpl.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Executes a sequence of transforms.
///
/// Before calling runTransform() specify the sequence
///  by calling addTransform().
/// Each specified transforms take the output of the 
///  previous transform as input.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class transformsChain: public transform
{
public:
	transformsChain();

	/// \brief Add a transform to the transforms chain.
	///
	/// The added transform will take the output of the 
	///  previously added transform as an input image and will
	///  supply its output to the next added transform or as
	///  an output of the transformsChain if it is the
	///  last added transform.
	///
	/// @param pTransform the transform to be added to
	///                    transformsChain
	///
	///////////////////////////////////////////////////////////
	void addTransform(std::shared_ptr<transform> pTransform);

    virtual void runTransformHandlers(
            std::shared_ptr<handlers::readingDataHandlerNumericBase> inputHandler, bitDepth_t inputDepth, std::uint32_t inputHandlerWidth, const std::string& inputHandlerColorSpace,
            std::shared_ptr<palette> inputPalette,
            std::uint32_t inputHighBit,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            std::shared_ptr<handlers::writingDataHandlerNumericBase> outputHandler, bitDepth_t outputDepth, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,
            std::shared_ptr<palette> outputPalette,
            std::uint32_t outputHighBit,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const override;

	/// \brief Returns true if the transform doesn't do
	///         anything.
	///
	/// @return false if the transform does something, or true
	///          if the transform doesn't do anything (e.g. an
	///          empty transformsChain object).
	///
	///////////////////////////////////////////////////////////
    virtual bool isEmpty() const override;

    virtual std::shared_ptr<image> allocateOutputImage(
            bitDepth_t inputDepth,
            const std::string& inputColorSpace,
            std::uint32_t inputHighBit,
            std::shared_ptr<palette> inputPalette,
            std::uint32_t outputWidth, std::uint32_t outputHeight) const override;

protected:
    typedef std::vector<std::shared_ptr<transform> > tTransformsList;
	tTransformsList m_transformsList;

};

/// @}

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraTransformsChain_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_)
