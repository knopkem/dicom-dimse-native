/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file YBRFULLToMONOCHROME2.cpp
    \brief Implementation of the class YBRFULLToMONOCHROME2.

*/

#include "exceptionImpl.h"
#include "YBRFULLToMONOCHROME2Impl.h"
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
std::string YBRFULLToMONOCHROME2::getInitialColorSpace() const
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
std::string YBRFULLToMONOCHROME2::getFinalColorSpace() const
{
    return "MONOCHROME2";
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the initial color space (YBR_ICT)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string YBRICTToMONOCHROME2::getInitialColorSpace() const
{
    return "YBR_ICT";
}



} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

