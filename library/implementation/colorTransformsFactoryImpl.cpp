/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file colorTransformsFactory.cpp
    \brief Implementation of the colorTransformsFactory class.

*/

#include "exceptionImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "transformsChainImpl.h"
#include "imageImpl.h"
#include <locale>
#include "../include/imebra/exceptions.h"

#include "MONOCHROME1ToMONOCHROME2Impl.h"
#include "MONOCHROME1ToRGBImpl.h"
#include "MONOCHROME2ToRGBImpl.h"
#include "MONOCHROME2ToYBRFULLImpl.h"
#include "PALETTECOLORToRGBImpl.h"
#include "RGBToMONOCHROME2Impl.h"
#include "RGBToYBRFULLImpl.h"
#include "RGBToYBRPARTIALImpl.h"
#include "RGBToYBRRCTImpl.h"
#include "YBRFULLToMONOCHROME2Impl.h"
#include "YBRFULLToRGBImpl.h"
#include "YBRPARTIALToRGBImpl.h"
#include "YBRRCTToRGBImpl.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{

namespace colorTransforms
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// colorTransformsFactory
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Force the construction of the factory before main()
//  starts
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static colorTransformsFactory::forceColorTransformsFactoryConstruction forceConstruction;

colorTransformsFactory::colorTransformsFactory()
{
    IMEBRA_FUNCTION_START();

    registerTransform(std::make_shared<MONOCHROME1ToMONOCHROME2>());
    registerTransform(std::make_shared<MONOCHROME2ToMONOCHROME1>());
    registerTransform(std::make_shared<MONOCHROME1ToRGB>());
    registerTransform(std::make_shared<MONOCHROME2ToRGB>());
    registerTransform(std::make_shared<MONOCHROME2ToYBRFULL>());
    registerTransform(std::make_shared<MONOCHROME2ToYBRICT>());
    registerTransform(std::make_shared<PALETTECOLORToRGB>());
    registerTransform(std::make_shared<RGBToMONOCHROME2>());
    registerTransform(std::make_shared<RGBToYBRFULL>());
    registerTransform(std::make_shared<RGBToYBRICT>());
    registerTransform(std::make_shared<RGBToYBRRCT>());
    registerTransform(std::make_shared<RGBToYBRPARTIAL>());
    registerTransform(std::make_shared<YBRFULLToMONOCHROME2>());
    registerTransform(std::make_shared<YBRICTToMONOCHROME2>());
    registerTransform(std::make_shared<YBRFULLToRGB>());
    registerTransform(std::make_shared<YBRICTToRGB>());
    registerTransform(std::make_shared<YBRRCTToRGB>());
    registerTransform(std::make_shared<YBRPARTIALToRGB>());

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a color transform
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void colorTransformsFactory::registerTransform(std::shared_ptr<colorTransform> newColorTransform)
{
    IMEBRA_FUNCTION_START();

    m_transformsList.push_back(newColorTransform);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a pointer to the colorTransformsFactory instance
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<colorTransformsFactory> colorTransformsFactory::getColorTransformsFactory()
{
    IMEBRA_FUNCTION_START();

    // Violation to requirement REQ_MAKE_SHARED due to protected constructor
    static std::shared_ptr<colorTransformsFactory> m_transformFactory(new colorTransformsFactory());
    return m_transformFactory;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Normalize the color space name
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////;
std::string colorTransformsFactory::normalizeColorSpace(const std::string& colorSpace)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedColorSpace;

    size_t c42position = colorSpace.find("_42");
    if(c42position != colorSpace.npos)
        normalizedColorSpace=colorSpace.substr(0, c42position);
    else
        normalizedColorSpace=colorSpace;

    // Colorspace transformed to uppercase
    ///////////////////////////////////////////////////////////
    for(size_t adjustColorSpace = 0; adjustColorSpace != normalizedColorSpace.size(); ++adjustColorSpace)
    {
        normalizedColorSpace[adjustColorSpace] = std::toupper<char>(normalizedColorSpace[adjustColorSpace], std::locale());
    }

    return normalizedColorSpace;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the color space name represents a
//  monochrome color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool colorTransformsFactory::isMonochrome(const std::string& colorSpace)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedColorSpace = normalizeColorSpace(colorSpace);
    return (normalizedColorSpace == "MONOCHROME1" || normalizedColorSpace == "MONOCHROME2");

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the color space name represents an
//  horizontally subsampled color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool colorTransformsFactory::isSubsampledX(const std::string& colorSpace)
{
    return (colorSpace.find("_42") != colorSpace.npos);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the color space name represents a
//  vertically subsampled color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool colorTransformsFactory::isSubsampledY(const std::string& colorSpace)
{
    return (colorSpace.find("_420")!=colorSpace.npos);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the specified color space can be
//  subsampled
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool colorTransformsFactory::canSubsample(const std::string& colorSpace)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedColorSpace = normalizeColorSpace(colorSpace);
    return normalizedColorSpace.find("YBR_") == 0;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Subsample a color space name
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string colorTransformsFactory::makeSubsampled(const std::string& colorSpace, bool bSubsampleX, bool bSubsampleY)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedColorSpace = normalizeColorSpace(colorSpace);
    if(!canSubsample(normalizedColorSpace))
    {
        return normalizedColorSpace;
    }
    if(bSubsampleY)
    {
        return normalizedColorSpace + "_420";
    }
    if(bSubsampleX)
    {
        return normalizedColorSpace + "_422";
    }
    return normalizedColorSpace;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the number of channels for the specified
//  color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t colorTransformsFactory::getNumberOfChannels(const std::string& colorSpace)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedColorSpace = normalizeColorSpace(colorSpace);

    struct sColorSpace
    {
        sColorSpace(const char* colorSpace, std::uint8_t channelsNumber): m_colorSpace(colorSpace), m_channelsNumber(channelsNumber){}
        std::string m_colorSpace;
        std::uint8_t m_channelsNumber;
    };

    static const sColorSpace imbxColorSpaces[]=
    {
        sColorSpace("RGB", 0x3),
        sColorSpace("YBR_FULL", 0x3),
        sColorSpace("YBR_PARTIAL", 0x3),
        sColorSpace("YBR_RCT", 0x3),
        sColorSpace("YBR_ICT", 0x3),
        sColorSpace("PALETTE COLOR", 0x1),
        sColorSpace("CMYK", 0x4),
        sColorSpace("CMY", 0x3),
        sColorSpace("MONOCHROME2", 0x1),
        sColorSpace("MONOCHROME1", 0x1),
        sColorSpace("", 0x0)
    };

    for(std::uint8_t findColorSpace = 0; imbxColorSpaces[findColorSpace].m_channelsNumber != 0x0; ++findColorSpace)
    {
        if(imbxColorSpaces[findColorSpace].m_colorSpace == normalizedColorSpace)
        {
            return imbxColorSpaces[findColorSpace].m_channelsNumber;
        }
    }

    return 0;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns a transform that can convert a color space into
//  another
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<transform> colorTransformsFactory::getTransform(const std::string& startColorSpace, const std::string& endColorSpace)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedStartColorSpace = normalizeColorSpace(startColorSpace);
    std::string normalizedEndColorSpace = normalizeColorSpace(endColorSpace);

    if(normalizedStartColorSpace == normalizedEndColorSpace)
    {
        return std::shared_ptr<colorTransform>(0);
    }

    for(tTransformsList::iterator scanSingleTransform = m_transformsList.begin(); scanSingleTransform != m_transformsList.end(); ++scanSingleTransform)
    {
        if( (*scanSingleTransform)->getInitialColorSpace() == normalizedStartColorSpace &&
            (*scanSingleTransform)->getFinalColorSpace() == normalizedEndColorSpace)
        {
            std::shared_ptr<colorTransform> newTransform = *scanSingleTransform;
            return newTransform;
        }
    }

    for(tTransformsList::iterator scanMultipleTransforms = m_transformsList.begin(); scanMultipleTransforms != m_transformsList.end(); ++scanMultipleTransforms)
    {
        if( (*scanMultipleTransforms)->getInitialColorSpace() != normalizedStartColorSpace)
        {
            continue;
        }

        for(tTransformsList::iterator secondTransform = m_transformsList.begin(); secondTransform != m_transformsList.end(); ++secondTransform)
        {
            if( (*secondTransform)->getFinalColorSpace() != normalizedEndColorSpace ||
                (*secondTransform)->getInitialColorSpace() != (*scanMultipleTransforms)->getFinalColorSpace())
            {
                continue;
            }

            std::shared_ptr<colorTransform> newTransform0 = *scanMultipleTransforms;
            std::shared_ptr<colorTransform> newTransform1 = *secondTransform;

            std::shared_ptr<transformsChain> chain = std::make_shared<transformsChain>();
            chain->addTransform(newTransform0);
            chain->addTransform(newTransform1);

            return chain;
        }
    }

    IMEBRA_THROW(ColorTransformsFactoryNoTransformError, "There isn't any transform that can convert between the color space " << startColorSpace << " and " << endColorSpace);

    IMEBRA_FUNCTION_END();
}

} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra
