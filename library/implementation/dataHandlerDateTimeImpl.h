/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerDateTime.h
    \brief Declaration of the handler for the date and time.

*/

#if !defined(imebraDataHandlerDateTime_85665C7B_8DDF_479e_8CC0_83E95CB625DC__INCLUDED_)
#define imebraDataHandlerDateTime_85665C7B_8DDF_479e_8CC0_83E95CB625DC__INCLUDED_

#include "dataHandlerDateTimeBaseImpl.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{

class readingDataHandlerDateTime : public readingDataHandlerDateTimeBase
{

public:
    readingDataHandlerDateTime(const memory& parseMemory);

    virtual std::shared_ptr<date> getDate(const size_t index) const override;

};

class writingDataHandlerDateTime: public writingDataHandlerDateTimeBase
{
public:
    writingDataHandlerDateTime(const std::shared_ptr<buffer>& pBuffer);

    virtual void setDate(const size_t index, const std::shared_ptr<const date>& pDate) override;
};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerDateTime_85665C7B_8DDF_479e_8CC0_83E95CB625DC__INCLUDED_)
