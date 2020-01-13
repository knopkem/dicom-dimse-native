/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringUI.h
    \brief Declaration of the class dataHandlerStringUI.

*/

#if !defined(imebraDataHandlerStringUI_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDataHandlerStringUI_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include "dataHandlerStringImpl.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{

///
/// \brief Normalize a DICOM name (UID).
///
/// E.g.: "001.23.03" is normalized to "1.23.3",
/// "001.23.3." is normalized to "1.23.3".
///
/// \param uid the name to normalize
/// \return the normalized name
///
//////////////////////////////////////////////////////////////////
std::string normalizeUid(const std::string& uid);

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// dataHandlerStringUI
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class readingDataHandlerStringUI : public readingDataHandlerString
{
public:
    readingDataHandlerStringUI(const memory& parseMemory);

    virtual std::string getString(const size_t index) const;

    virtual std::int32_t getSignedLong(const size_t index) const;

    virtual std::uint32_t getUnsignedLong(const size_t index) const;

    virtual double getDouble(const size_t index) const;


};

class writingDataHandlerStringUI: public writingDataHandlerString
{
public:
    writingDataHandlerStringUI(const std::shared_ptr<buffer>& pBuffer);

    virtual void setString(const size_t index, const std::string& value);
};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerStringUI_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
