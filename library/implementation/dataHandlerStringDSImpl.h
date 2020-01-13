/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringDS.h
    \brief Declaration of the class dataHandlerStringDS.

*/

#if !defined(imebraDataHandlerStringDS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDataHandlerStringDS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include "dataHandlerStringImpl.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Handles the Dicom type "DS" (decimal string)
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class readingDataHandlerStringDS : public readingDataHandlerString
{
public:
    readingDataHandlerStringDS(const memory& memory);

	// Overwritten to use getDouble()
	///////////////////////////////////////////////////////////
    virtual std::int32_t getSignedLong(const size_t index) const override;

	// Overwritten to use getDouble()
	///////////////////////////////////////////////////////////
    virtual std::uint32_t getUnsignedLong(const size_t index) const override;

};

class writingDataHandlerStringDS: public writingDataHandlerString
{
public:
    writingDataHandlerStringDS(const std::shared_ptr<buffer> pBuffer);

    // Overwritten to use setDouble()
    ///////////////////////////////////////////////////////////
    virtual void setSignedLong(const size_t index, const std::int32_t value) override;

    // Overwritten to use setDouble()
    ///////////////////////////////////////////////////////////
    virtual void setUnsignedLong(const size_t index, const std::uint32_t value) override;
};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerStringDS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
