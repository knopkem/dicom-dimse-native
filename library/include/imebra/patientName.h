/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file patientName.h
    \brief Declaration of the classes PatientName and UnicodePatientName.

*/

#if !defined(imebraPATIENTNAME__INCLUDED_)
#define imebraPATIENTNAME__INCLUDED_

#include <memory>
#include "../include/imebra/definitions.h"

namespace imebra
{

namespace implementation
{
class patientName;
class unicodePatientName;
}

///
/// \brief Stores a patient name (in alphabetic, ideographic and phonetic
///        forms).
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API PatientName
{
    friend class DataSet;
    friend class ReadingDataHandler;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source PatientName object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    PatientName(const PatientName& source);

    PatientName& operator=(const PatientName& source) = delete;

    virtual ~PatientName();

    /// \brief Constructor.
    ///
    /// \param alphabeticRepresentation  the alphabetic representation of the
    ///                                  patient name (UTF8 encoded)
    /// \param ideographicRepresentation the ideographic representation of the
    ///                                  patient name (UTF8 encoded)
    /// \param phoneticRepresentation    the phonetic representation of the
    ///                                  patient name (UTF8 encoded)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    PatientName(const std::string& alphabeticRepresentation, const std::string& ideographicRepresentation, const std::string& phoneticRepresentation);

    /// \brief Returns the alphabetic form of the patient name (UTF8 encoded).
    ///
    /// \return alphabetic representation of the patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getAlphabeticRepresentation() const;

    /// \brief Returns the ideographic form of the patient name (UTF8 encoded).
    ///
    /// \return ideographic representation of the patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getIdeographicRepresentation() const;

    /// \brief Returns the phonetic form of the patient name (UTF8 encoded).
    ///
    /// \return phonetic representation of the patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getPhoneticRepresentation() const;

#ifndef SWIG
protected:
    explicit PatientName(const std::shared_ptr<implementation::patientName>& pPatientName);

private:
    friend const std::shared_ptr<implementation::patientName>& getPatientNameImplementation(const PatientName& patientName);
    std::shared_ptr<implementation::patientName> m_pPatientName;
#endif

};

#ifndef SWIG // Use UTF8 strings only with SWIG

///
/// \brief Stores a patient name with Unicode chars (in alphabetic,
///        ideographic and phonetic forms).
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API UnicodePatientName
{
    friend class DataSet;
    friend class ReadingDataHandler;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source PatientName object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    UnicodePatientName(const UnicodePatientName& source);

    UnicodePatientName& operator=(const UnicodePatientName& source) = delete;

    virtual ~UnicodePatientName();

    /// \brief Constructor.
    ///
    /// \param alphabeticRepresentation  the alphabetic representation of the
    ///                                  patient name
    /// \param ideographicRepresentation the ideographic representation of the
    ///                                  patient name
    /// \param phoneticRepresentation    the phonetic representation of the
    ///                                  patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    UnicodePatientName(const std::wstring& alphabeticRepresentation, const std::wstring& ideographicRepresentation, const std::wstring& phoneticRepresentation);

    /// \brief Returns the alphabetic form of the patient name.
    ///
    /// \return alphabetic representation of the patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getAlphabeticRepresentation() const;

    /// \brief Returns the ideographic form of the patient name.
    ///
    /// \return ideographic representation of the patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getIdeographicRepresentation() const;

    /// \brief Returns the phonetic form of the patient name.
    ///
    /// \return phonetic representation of the patient name
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getPhoneticRepresentation() const;

protected:
    explicit UnicodePatientName(const std::shared_ptr<implementation::unicodePatientName>& pPatientName);

private:
    friend const std::shared_ptr<implementation::unicodePatientName>& getUnicodePatientNameImplementation(const UnicodePatientName& patientName);
    std::shared_ptr<implementation::unicodePatientName> m_pPatientName;

};

#endif

} // namespace imebra

#endif // !defined(imebraPATIENTNAME__INCLUDED_)
