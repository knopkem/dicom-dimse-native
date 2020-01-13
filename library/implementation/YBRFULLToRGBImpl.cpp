/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file YBRFULLToRGB.cpp
    \brief Implementation of the class YBRFULLToRGB.

*/

#include "exceptionImpl.h"
#include "YBRFULLToRGBImpl.h"
#include "dataHandlerImpl.h"
#include "dataSetImpl.h"
#include "imageImpl.h"


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
//
//
// Return the initial color space (YBR_FULL)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string YBRFULLToRGB::getInitialColorSpace() const
{
    return "YBR_FULL";
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the final color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string YBRFULLToRGB::getFinalColorSpace() const
{
    return "RGB";
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the initial color space (YBR_YCT)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string YBRICTToRGB::getInitialColorSpace() const
{
    return "YBR_ICT";
}


} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

