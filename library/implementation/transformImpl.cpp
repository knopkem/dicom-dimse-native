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
    \brief Implementation of the base class used by the transforms.

*/

#include "exceptionImpl.h"
#include "transformImpl.h"
#include "imageImpl.h"
#include "transformHighBitImpl.h"
#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
transform::~transform()
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Declare an input parameter
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool transform::isEmpty() const
{
	return false;
}


void transform::runTransform(
            const std::shared_ptr<const image>& inputImage,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            const std::shared_ptr<image>& outputImage,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t inputImageWidth, inputImageHeight;
    inputImage->getSize(&inputImageWidth, &inputImageHeight);
    std::uint32_t outputImageWidth, outputImageHeight;
    outputImage->getSize(&outputImageWidth, &outputImageHeight);

    if(inputTopLeftX + inputWidth > inputImageWidth ||
        inputTopLeftY + inputHeight > inputImageHeight ||
        outputTopLeftX + inputWidth > outputImageWidth ||
        outputTopLeftY + inputHeight > outputImageHeight)
    {
        IMEBRA_THROW(TransformInvalidAreaError, "The input and/or output areas are invalid");
    }

    std::shared_ptr<handlers::readingDataHandlerNumericBase> inputHandler(inputImage->getReadingDataHandler());
	std::shared_ptr<palette> inputPalette(inputImage->getPalette());
    std::string inputColorSpace(inputImage->getColorSpace());
	std::uint32_t inputHighBit(inputImage->getHighBit());
    bitDepth_t inputDepth(inputImage->getDepth());

    std::shared_ptr<handlers::writingDataHandlerNumericBase> outputHandler(outputImage->getWritingDataHandler());
	std::shared_ptr<palette> outputPalette(outputImage->getPalette());
    std::string outputColorSpace(outputImage->getColorSpace());
	std::uint32_t outputHighBit(outputImage->getHighBit());
    bitDepth_t outputDepth(outputImage->getDepth());

	if(isEmpty())
	{
        std::shared_ptr<transformHighBit> emptyTransform(std::make_shared<transformHighBit>());
        emptyTransform->runTransformHandlers(inputHandler, inputDepth, inputImageWidth, inputColorSpace, inputPalette, inputHighBit,
											 inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,
                                             outputHandler, outputDepth, outputImageWidth, outputColorSpace, outputPalette, outputHighBit,
											 outputTopLeftX, outputTopLeftY);
		return;
	}

    runTransformHandlers(inputHandler, inputDepth, inputImageWidth, inputColorSpace, inputPalette, inputHighBit,
		inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,
        outputHandler, outputDepth, outputImageWidth, outputColorSpace, outputPalette, outputHighBit,
		outputTopLeftX, outputTopLeftY);

    IMEBRA_FUNCTION_END();
}




} // namespace transforms

} // namespace implementation

} // namespace imebra
