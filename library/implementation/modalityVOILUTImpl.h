/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file modalityVOILUT.h
    \brief Declaration of the class modalityVOILUT.

*/

#if !defined(imebraModalityVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
#define imebraModalityVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_

#include "transformImpl.h"
#include "imageImpl.h"
#include "dataSetImpl.h"
#include "LUTImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "../include/imebra/exceptions.h"


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
/// \brief This class transforms the pixel values of the
///         image retrieved from the dataset into values
///         that are meaningful to th application.
///
/// For instance, the original pixel values could store
///  a device specific value that has a meaning only when
///  used by the device that generated it: this transform
///  uses the modality VOI/LUT defined in the dataset to
///  convert the original values into optical density
///  or other known measure units.
///
/// If the dataset doesn't define any modality VOI/LUT
///  transformation, then the input image is simply copied
///  into the output image.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class modalityVOILUT: public transform
{
public:
	/// \brief Constructor.
	///
	/// @param pDataSet the dataSet from which the input
	///         images come from
	///
	///////////////////////////////////////////////////////////
    modalityVOILUT(std::shared_ptr<const dataSet> pDataSet);

	DEFINE_RUN_TEMPLATE_TRANSFORM;

	template <class inputType, class outputType>
			void templateTransform(
                    const inputType* inputHandlerData,
                    outputType* outputHandlerData,
                    bitDepth_t /* inputDepth */, std::uint32_t inputHandlerWidth, const std::string& inputHandlerColorSpace,
                    std::shared_ptr<palette> /* inputPalette */,
                    std::uint32_t /* inputHighBit */,
                    std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
                    bitDepth_t /* outputDepth */, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,
                    std::shared_ptr<palette> /* outputPalette */,
                    std::uint32_t /* outputHighBit */,
                    std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const
	{
        IMEBRA_FUNCTION_START();

		if(!colorTransforms::colorTransformsFactory::isMonochrome(inputHandlerColorSpace) || !colorTransforms::colorTransformsFactory::isMonochrome(outputHandlerColorSpace))
		{
            IMEBRA_THROW(ModalityVOILUTError, "modalityVOILUT can process only monochromatic images");
		}
        const inputType* pInputMemory(inputHandlerData);
		outputType* pOutputMemory(outputHandlerData);

		pInputMemory += inputTopLeftY * inputHandlerWidth + inputTopLeftX;
		pOutputMemory += outputTopLeftY * outputHandlerWidth + outputTopLeftX;

		//
		// Modality LUT found
		//
		///////////////////////////////////////////////////////////
        if(m_voiLut != 0 && m_voiLut->getSize() != 0)
		{
			for(; inputHeight != 0; --inputHeight)
			{
                for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
				{
                    *(pOutputMemory++) = (outputType) ( m_voiLut->getMappedValue((std::int32_t)*(pInputMemory++)));
				}
				pInputMemory += (inputHandlerWidth - inputWidth);
				pOutputMemory += (outputHandlerWidth - inputWidth);
			}
			return;
		}

		//
		// Modality LUT not found
		//
		///////////////////////////////////////////////////////////

		// Retrieve the intercept/scale pair
		///////////////////////////////////////////////////////////
		for(; inputHeight != 0; --inputHeight)
		{
            for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
			{
                *(pOutputMemory++) = (outputType)((double)(*(pInputMemory++)) * m_rescaleSlope + m_rescaleIntercept);
			}
			pInputMemory += (inputHandlerWidth - inputWidth);
			pOutputMemory += (outputHandlerWidth - inputWidth);
		}
		IMEBRA_FUNCTION_END();
	}

    virtual bool isEmpty() const override;

    virtual std::shared_ptr<image> allocateOutputImage(
            bitDepth_t inputDepth,
            const std::string& inputColorSpace,
            std::uint32_t inputHighBit,
            std::shared_ptr<palette> inputPalette,
            std::uint32_t outputWidth, std::uint32_t outputHeight) const override;

private:
    std::shared_ptr<const dataSet> m_pDataSet;
    std::shared_ptr<const lut> m_voiLut;
    double m_rescaleIntercept;
    double m_rescaleSlope;
	bool m_bEmpty;
};


/// @}

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraModalityVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
