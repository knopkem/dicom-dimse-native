/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transform.h
    \brief Declaration of the base class used by all the transforms.
 
*/

#if !defined(imebraTransform_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_)
#define imebraTransform_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_

#include <memory>
#include <limits>
#include "dataHandlerNumericImpl.h"
#include "imageImpl.h"

#define DEFINE_RUN_TEMPLATE_TRANSFORM \
virtual void runTransformHandlers(\
    std::shared_ptr<imebra::implementation::handlers::readingDataHandlerNumericBase> inputHandler, bitDepth_t inputDepth, std::uint32_t inputHandlerWidth, const std::string& inputHandlerColorSpace,\
    std::shared_ptr<imebra::implementation::palette> inputPalette,\
    std::uint32_t inputHighBit,\
    std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,\
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> outputHandler, bitDepth_t outputDepth, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,\
    std::shared_ptr<imebra::implementation::palette> outputPalette,\
    std::uint32_t outputHighBit,\
    std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const override\
{\
    IMEBRA_FUNCTION_START();\
    runTemplateTransform(*this, inputHandler, outputHandler, inputDepth, inputHandlerWidth, inputHandlerColorSpace, inputPalette, inputHighBit,\
            inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,\
            outputDepth, outputHandlerWidth, outputHandlerColorSpace, outputPalette, outputHighBit,\
            outputTopLeftX, outputTopLeftY);\
    IMEBRA_FUNCTION_END();\
}

namespace imebra
{

namespace implementation
{

namespace handlers
{
    class dataHandlerNumericBase;
}

class image;
class dataSet;
class lut;

/// \namespace transforms
/// \brief All the transforms are declared in this
///         namespace.
///
///////////////////////////////////////////////////////////
namespace transforms
{

/*! \addtogroup group_transforms Transforms
\brief The transform classes apply a transformation to
		one input image and return the result of the
		transformation into an output image.

Usually the transforms require that the input and the
 output images use the same color space, but the
 color transforms are able to copy the pixel data
 from the color space of the input image into the
 color space of the output image.

The application can call
 transforms::transform::allocateOutputImage() to
 allocate an output image that is compatible with the
 selected transform and input image.\n
For instance, once an image has been retrieved from
 a dataSet we can ask the modalityVOILUT transform
 to allocate an output image for us, and it will
 allocate an image with the right color space and
 bit depth;

\code
// loadedDataSet is a std::shared_ptr<dataSet> previously loaded
// Here we get the first image in the dataSet
std::shared_ptr<image> inputImage(loadedDataSet->getImage(0));

// We need to get the image's size because we have to
//  tell the transform on which area we want to apply
//  the transform (we want all the image area)
std::uint32_t width, height;
inputImage->getSize(&width, &height);

// Allocate the modality transform. The modality transform
//  gets the transformation parameters from the dataset
std::shared_ptr<transforms::modalityVOILUT> modalityTransform(new transforms::modalityVOILUT(loadedDataSet));

// We ask the transform to allocate a proper output image
std::shared_ptr<image> outputImage(modalityTransform->allocateOutputImage(inputImage, width, height));

// And now we run the transform
modalityTransform->runTransform(inputImage, 0, 0, width, height, outputImage, 0, 0);
\endcode

All the transforms but the modalityVOILUT can convert
 the result to the bit depth of the output image, so for
 instance the transform colorTransforms::YBRFULLToRGB
 can take a 16 bits per channel input image and
 write the result to a 8 bits per color channel output
 image.\n
modalityVOILUT cannot do this because its output has
 to conform to the value in the tag 0028,1054; the
 tag 0028,1054 specifies the units of the modality VOI-LUT
 transform. modalityVOILUT::allocateOutputImage() is able
 output image that can hold the result of the
 to allocate the modality transformation.

*/
/// @{

/// \brief This is the base class for the transforms.
///
/// A transform takes one input and one output image:
///  the output image is modified according to the
///  transform's type, input image's content and
///  transform's parameter.
///
///////////////////////////////////////////////////////////
class transform
{

public:
    virtual ~transform();

	/// \brief Returns true if the transform doesn't do
	///         anything.
	///
	/// @return false if the transform does something, or true
	///          if the transform doesn't do anything (e.g. an
	///          empty transformsChain object).
	///
	///////////////////////////////////////////////////////////
    virtual bool isEmpty() const;


	/// \brief Allocate an output image that is compatible with
	///         the transform given the specified input image.
	///
	/// @param pInputImage image that will be used as input
	///                     image in runTransform()
	/// @param width       the width of the output image,
	///                     in pixels
	/// @param height      the height of the output image,
	///                     in pixels
	/// @return an image suitable to be used as output image
	///          in runTransform()
	///
	///////////////////////////////////////////////////////////
    virtual std::shared_ptr<image> allocateOutputImage(
            bitDepth_t inputDepth,
            const std::string& inputColorSpace,
            std::uint32_t inputHighBit,
            std::shared_ptr<palette> inputPalette,
            std::uint32_t outputWidth, std::uint32_t outputHeight) const = 0;


	/// \brief Executes the transform.
	///
	/// @param inputImage    the input image for the transform
	/// @param inputTopLeftX the horizontal position of the
	///                       top left corner of the area to
	///                       process
	/// @param inputTopLeftY the vertical position of the top
	///                       left corner of the area to
	///                       process
	/// @param inputWidth    the width of the area to process
	/// @param inputHeight   the height of the area to process
	/// @param outputImage   the output image for the transform
	/// @param outputTopLeftX the horizontal position of the
	///                       top left corner of the output
	///                       area
	/// @param outputTopLeftY the vertical position of the top
	///                        left corner of the output area
	///
	///////////////////////////////////////////////////////////
	virtual void runTransform(
            const std::shared_ptr<const image>& inputImage,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            const std::shared_ptr<image>& outputImage,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const;

    virtual void runTransformHandlers(
            std::shared_ptr<handlers::readingDataHandlerNumericBase> inputHandler, bitDepth_t inputDepth, std::uint32_t inputHandlerWidth, const std::string& inputHandlerColorSpace,
            std::shared_ptr<palette> inputPalette,
            std::uint32_t inputHighBit,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            std::shared_ptr<handlers::writingDataHandlerNumericBase> outputHandler, bitDepth_t outputDepth, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,
            std::shared_ptr<palette> outputPalette,
            std::uint32_t outputHighBit,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const = 0;
};


template <typename dataType>
typename std::enable_if<std::is_unsigned<dataType>::value, dataType>::type getMinValue(std::uint32_t /* highBit */)
{
    return 0;
}

template <typename dataType>
typename std::enable_if<std::is_signed<dataType>::value, dataType>::type getMinValue(std::uint32_t highBit)
{
    return (dataType)((std::int64_t)-1 * ((std::int64_t)1 << highBit));
}


template <typename transformClass, typename inputType, typename... Args>
void runTemplateTransform1(
        const transformClass& transformObject,
        const inputType* pInputData,
        std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> outputHandler, Args... args)
{
    IMEBRA_FUNCTION_START();

    imebra::implementation::handlers::writingDataHandlerNumericBase* pHandler(outputHandler.get());

    if(typeid(*pHandler) == typeid(handlers::writingDataHandlerUint8))
    {
        transformObject.templateTransform(pInputData, (std::uint8_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::writingDataHandlerInt8))
    {
        transformObject.templateTransform(pInputData, (std::int8_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::writingDataHandlerUint16))
    {
        transformObject.templateTransform(pInputData, (std::uint16_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::writingDataHandlerInt16))
    {
        transformObject.templateTransform(pInputData, (std::int16_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::writingDataHandlerUint32))
    {
        transformObject.templateTransform(pInputData, (std::uint32_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::writingDataHandlerInt32))
    {
        transformObject.templateTransform(pInputData, (std::int32_t*)pHandler->getMemoryBuffer(), args...);
    }
    else
    {
        IMEBRA_THROW(std::runtime_error, "Data type not valid");\
    }

    IMEBRA_FUNCTION_END();
}


template <typename transformClass, typename... Args>
void runTemplateTransform(
        const transformClass& transformObject,
        std::shared_ptr<imebra::implementation::handlers::readingDataHandlerNumericBase> inputHandler, Args... args)
{
    IMEBRA_FUNCTION_START();

    imebra::implementation::handlers::readingDataHandlerNumericBase* pHandler(inputHandler.get());
    if(typeid(*pHandler) == typeid(handlers::readingDataHandlerUint8))
    {
        runTemplateTransform1<transformClass, std::uint8_t>(transformObject, (const std::uint8_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::readingDataHandlerInt8))
    {
        runTemplateTransform1<transformClass, std::int8_t>(transformObject, (const std::int8_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::readingDataHandlerUint16))
    {
        runTemplateTransform1<transformClass, std::uint16_t>(transformObject, (const std::uint16_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::readingDataHandlerInt16))
    {
        runTemplateTransform1<transformClass, std::int16_t>(transformObject, (const std::int16_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::readingDataHandlerUint32))
    {
        runTemplateTransform1<transformClass, std::uint32_t>(transformObject, (const std::uint32_t*)pHandler->getMemoryBuffer(), args...);
    }
    else if(typeid(*pHandler) == typeid(handlers::readingDataHandlerInt32))
    {
        runTemplateTransform1<transformClass, std::int32_t>(transformObject, (const std::int32_t*)pHandler->getMemoryBuffer(), args...);
    }
    else
    {
        IMEBRA_THROW(std::runtime_error, "Data type not valid");\
    }
    IMEBRA_FUNCTION_END();
}





/// @}

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraTransform_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_)
