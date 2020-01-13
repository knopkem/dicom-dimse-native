/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringPN.h
    \brief Declaration of the class dataHandlerStringPN.

*/

#if !defined(imebraDataHandlerStringPN_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDataHandlerStringPN_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include "dataHandlerStringUnicodeImpl.h"
#include "patientNameImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{

/*!
\brief Handles the Dicom type "PN" (person name).

This class separates the component groups of the name.

The person name can be formed by three groups:
 one or more groups can be absent.
- the first components group contains a character
  representation of the person name
- the second components group contains an ideographic
  representation of the person name
- the third components group contains a phonetic
  representation of the patient name

Inside a components group, the name components
 (first name, middle name, surname, etc) must be
 separated by a ^.

This class doesn't insert or parse the ^ separator
 which must be inserted and handled by the calling
 application, but handles the = separator which
 separates the components groups.\n
This means that to read or set all the patient name
 you have to move the pointer to the internal element
 by using setPointer(), incPointer() or skip().

For instance, to set the name "Left^Arrow" both
 with a character and an ideographic representation you
 have to use the following code:

\code
myDataSet->getDataHandler(group, 0, tag, 0, true, "PN");
myDataSet->setSize(2);
myDataSet->setUnicodeString(L"Left^Arrow");
myDataSet->incPointer();
myDataSet->setUnicodeString(L"<-"); // :-)
\endcode

*/
class readingDataHandlerStringPN : public readingDataHandlerStringUnicode
{
public:
    readingDataHandlerStringPN(const memory& parseMemory, const std::shared_ptr<const charsetsList_t>& pCharsets);

    virtual std::shared_ptr<patientName> getPatientName(const size_t index) const;

    virtual std::shared_ptr<unicodePatientName> getUnicodePatientName(const size_t index) const;


private:

    template<class stringType_t, class patientNameType_t, typename separatorType_t> std::shared_ptr<patientNameType_t> returnPatientName(const stringType_t& string, separatorType_t separator) const
    {
        std::vector<stringType_t> groups;

        stringType_t group;

        std::basic_istringstream<separatorType_t> stream(string);
        while (std::getline(stream, group, separator))
        {
            groups.push_back(group);
        }

        if(groups.size() > 3) // Maximum 3 groups (alphabetic, ideographic, phonetic)
        {
            IMEBRA_THROW(DataHandlerCorruptedBufferError, "The Patient Name contains more than 3 groups")
        }

        groups.resize(3);

        return std::make_shared<patientNameType_t>(groups[0], groups[1], groups[2]);
    }

};

class writingDataHandlerStringPN: public writingDataHandlerStringUnicode
{
public:
    writingDataHandlerStringPN(const std::shared_ptr<buffer>& pBuffer, const std::shared_ptr<const charsetsList_t>& pCharsets);

    virtual void setPatientName(const size_t index, const std::shared_ptr<const patientName>& pPatientName) override;

    virtual void setUnicodePatientName(const size_t index, const std::shared_ptr<const unicodePatientName>& pPatientName) override;

private:

    template<class stringType_t, class patientNameType_t, typename separatorType_t> stringType_t returnPatientName(const std::shared_ptr<const patientNameType_t>& patient, separatorType_t separator)
    {
        stringType_t patientString;
        patientString.append(patient->getAlphabeticRepresentation());
        if(!patient->getIdeographicRepresentation().empty() || !patient->getPhoneticRepresentation().empty())
        {
            patientString.push_back(separator);
            patientString.append(patient->getIdeographicRepresentation());
        }
        if(!patient->getPhoneticRepresentation().empty())
        {
            patientString.push_back(separator);
            patientString.append(patient->getPhoneticRepresentation());
        }
        return patientString;
    }

};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerStringPN_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
