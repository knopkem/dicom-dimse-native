/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringCS.h
    \brief Declaration of the class dataHandlerStringCS.

*/

#if !defined(imebraDataHandlerStringCS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDataHandlerStringCS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include "dataHandlerStringImpl.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Handles the Dicom data type "CS" (code string)
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class readingDataHandlerStringCS : public readingDataHandlerString
{
public:
    readingDataHandlerStringCS(const memory& parseMemory);

};

class writingDataHandlerStringCS: public writingDataHandlerString
{
public:
    writingDataHandlerStringCS(const std::shared_ptr<buffer> pBuffer);
};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerStringCS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
