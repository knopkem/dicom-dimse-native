/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerDate.h
    \brief Declaration of the data handler able to handle the dicom tags
            of type "DA" (date).

*/

#if !defined(imebraDataHandlerDate_BAA5E237_A37C_40bc_96EF_460B2D53DC12__INCLUDED_)
#define imebraDataHandlerDate_BAA5E237_A37C_40bc_96EF_460B2D53DC12__INCLUDED_

#include "dataHandlerDateTimeBaseImpl.h"


namespace imebra
{

namespace implementation
{

class date;

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This %data handler is returned by the class
///         buffer when the application wants to deal
///         with a dicom tag of type "DA" (date)
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class readingDataHandlerDate : public readingDataHandlerDateTimeBase
{
public:
    readingDataHandlerDate(const memory& parseMemory);

    virtual std::shared_ptr<date> getDate(const size_t index) const override;
};

class writingDataHandlerDate: public writingDataHandlerDateTimeBase
{
public:
    writingDataHandlerDate(const std::shared_ptr<buffer>& pBuffer);

    virtual void setDate(const size_t index, const std::shared_ptr<const date>& pDate) override;
};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerDate_BAA5E237_A37C_40bc_96EF_460B2D53DC12__INCLUDED_)
