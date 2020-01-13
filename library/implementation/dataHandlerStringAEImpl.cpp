/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringAE.cpp
    \brief Implementation of the class dataHandlerStringAE.

*/

#include "dataHandlerStringAEImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataHandlerStringAE
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
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
readingDataHandlerStringAE::readingDataHandlerStringAE(const memory &parseMemory): readingDataHandlerString(parseMemory, tagVR_t::AE, '\\', 0x20)
{
}

writingDataHandlerStringAE::writingDataHandlerStringAE(const std::shared_ptr<buffer> &pBuffer): writingDataHandlerString(pBuffer, tagVR_t::AE, '\\', 0, 16)
{
}

} // namespace handlers

} // namespace implementation

} // namespace imebra
