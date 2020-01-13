/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transformsChain.cpp
    \brief Implementation of the class transformsChain.

*/

#include "exceptionImpl.h"
#include "transformsChainImpl.h"
#include "imageImpl.h"
#include "dataSetImpl.h"
#include "transformHighBitImpl.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{



transformsChain::transformsChain()
{}

///////////////////////////////////////////////////////////
//
// Add a new transform to the chain
//
///////////////////////////////////////////////////////////
void transformsChain::addTransform(std::shared_ptr<transform> pTransform)
{
    IMEBRA_FUNCTION_START();

        if(pTransform != 0 && !pTransform->isEmpty())
        {
            m_transformsList.push_back(pTransform);
        }

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Returns true if no transform has been defined
//
///////////////////////////////////////////////////////////
bool transformsChain::isEmpty() const
{
	return m_transformsList.empty();
}


void transformsChain::runTransformHandlers(
        std::shared_ptr<handlers::readingDataHandlerNumericBase> inputHandler, bitDepth_t inputDepth, std::uint32_t inputHandlerWidth, const std::string& inputHandlerColorSpace,
        std::shared_ptr<palette> inputPalette,
        std::uint32_t inputHighBit,
        std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
        std::shared_ptr<handlers::writingDataHandlerNumericBase> outputHandler, bitDepth_t outputDepth, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,
        std::shared_ptr<palette> outputPalette,
        std::uint32_t outputHighBit,
        std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const
{
    IMEBRA_FUNCTION_START();

    if(isEmpty())
    {
        std::shared_ptr<transformHighBit> highBit(std::make_shared<transformHighBit>());
        highBit->runTransformHandlers(inputHandler, inputDepth, inputHandlerWidth, inputHandlerColorSpace,
                                      inputPalette,
                                      inputHighBit,
                                      inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,
                                      outputHandler, outputDepth, outputHandlerWidth, outputHandlerColorSpace,
                                      outputPalette,
                                      outputHighBit,
                                      outputTopLeftX, outputTopLeftY);
        return;
    }

    if(m_transformsList.size() == 1)
    {
        m_transformsList.front()->runTransformHandlers(inputHandler, inputDepth, inputHandlerWidth, inputHandlerColorSpace,
                                               inputPalette,
                                               inputHighBit,
                                               inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,
                                               outputHandler, outputDepth, outputHandlerWidth, outputHandlerColorSpace,
                                               outputPalette,
                                               outputHighBit,
                                               outputTopLeftX, outputTopLeftY);
        return;
    }

    std::uint32_t allocateRows = 65536 / inputWidth;
    if(allocateRows == 0)
    {
        allocateRows = 1;
    }
    if(allocateRows > inputHeight)
    {
        allocateRows = inputHeight;
    }

    // Allocate temporary images
    ///////////////////////////////////////////////////////////
    typedef std::vector<std::shared_ptr<image> > tTemporaryImagesList;
    tTemporaryImagesList temporaryImages;

    temporaryImages.push_back(m_transformsList.at(0)->allocateOutputImage(inputDepth,
                                                                          inputHandlerColorSpace,
                                                                          inputHighBit,
                                                                          inputPalette,
                                                                          inputWidth, allocateRows));

    for(size_t scanTransforms(1); scanTransforms != m_transformsList.size() - 1; ++scanTransforms)
    {
        std::shared_ptr<image> inputTemporaryImage = temporaryImages.at(scanTransforms - 1);
        temporaryImages.push_back(m_transformsList.at(scanTransforms)->allocateOutputImage(inputTemporaryImage->getDepth(),
                                                                                           inputTemporaryImage->getColorSpace(),
                                                                                           inputTemporaryImage->getHighBit(),
                                                                                           inputTemporaryImage->getPalette(),
                                                                                           inputWidth, allocateRows));
    }



    // Run all the transforms. Split the images into several
    //  parts
    ///////////////////////////////////////////////////////////
    while(inputHeight != 0)
    {
        std::uint32_t rows = allocateRows;
        if(rows > inputHeight)
        {
            rows = inputHeight;
        }
        inputHeight -= rows;

        m_transformsList.at(0)->runTransformHandlers(inputHandler, inputDepth, inputHandlerWidth, inputHandlerColorSpace,
                                                     inputPalette,
                                                     inputHighBit,
                                                     inputTopLeftX, inputTopLeftY, inputWidth, rows,
                                                     temporaryImages.front()->getWritingDataHandler(),
                                                     temporaryImages.front()->getDepth(), inputWidth,
                                                     temporaryImages.front()->getColorSpace(),
                                                     temporaryImages.front()->getPalette(),
                                                     temporaryImages.front()->getHighBit(),
                                                     0, 0);
        inputTopLeftY += rows;

        for(size_t scanTransforms(1); scanTransforms != m_transformsList.size() - 1; ++scanTransforms)
        {
            m_transformsList.at(scanTransforms)->runTransform(temporaryImages.at(scanTransforms - 1), 0, 0, inputWidth, rows, temporaryImages.at(scanTransforms), 0, 0);
        }

        m_transformsList.back()->runTransformHandlers(temporaryImages.back()->getReadingDataHandler(),
                                                      temporaryImages.back()->getDepth(), inputWidth,
                                                      temporaryImages.back()->getColorSpace(),
                                                      temporaryImages.back()->getPalette(),
                                                      temporaryImages.back()->getHighBit(),
                                                      0, 0, inputWidth, rows,
                                                      outputHandler, outputDepth, outputHandlerWidth, outputHandlerColorSpace,
                                                      outputPalette,
                                                      outputHighBit,
                                                      outputTopLeftX, outputTopLeftY);
        outputTopLeftY += rows;
    }

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<image> transformsChain::allocateOutputImage(
        bitDepth_t inputDepth,
        const std::string& inputColorSpace,
        std::uint32_t inputHighBit,
        std::shared_ptr<palette> inputPalette,
        std::uint32_t outputWidth, std::uint32_t outputHeight) const
{
    IMEBRA_FUNCTION_START();

    if(isEmpty())
	{
        return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, inputHighBit);
	}

	if(m_transformsList.size() == 1)
	{
        return m_transformsList.front()->allocateOutputImage(inputDepth,
                                                             inputColorSpace,
                                                             inputHighBit,
                                                             inputPalette,
                                                             outputWidth, outputHeight);
	}

	// Get the position of the last transform
	///////////////////////////////////////////////////////////
    tTransformsList::const_iterator lastTransform(m_transformsList.end());
	--lastTransform;

	std::shared_ptr<image> temporaryImage;

    for(tTransformsList::const_iterator scanTransforms(m_transformsList.begin()); scanTransforms != lastTransform; ++scanTransforms)
	{
		if(scanTransforms == m_transformsList.begin())
		{
            temporaryImage = (*scanTransforms)->allocateOutputImage(inputDepth,
                                                                    inputColorSpace,
                                                                    inputHighBit,
                                                                    inputPalette,
                                                                    1, 1);
		}
		else
		{
            std::shared_ptr <image> newImage( (*scanTransforms)->allocateOutputImage(temporaryImage->getDepth(),
                                                                                     temporaryImage->getColorSpace(),
                                                                                     temporaryImage->getHighBit(),
                                                                                     temporaryImage->getPalette(),
                                                                                     1, 1) );
			temporaryImage = newImage;
		}
	}
    return (*lastTransform)->allocateOutputImage(temporaryImage->getDepth(),
                                                 temporaryImage->getColorSpace(),
                                                 temporaryImage->getHighBit(),
                                                 temporaryImage->getPalette(),
                                                 outputWidth, outputHeight);

    IMEBRA_FUNCTION_END();
}


} // namespace transforms

} // namespace implementation

} // namespace imebra
