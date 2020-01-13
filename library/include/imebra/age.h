/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file age.h
    \brief Declaration of the class Age.

*/

#if !defined(imebraAGE__INCLUDED_)
#define imebraAGE__INCLUDED_

#include <cstdint>
#include <memory>
#include "../include/imebra/definitions.h"

namespace imebra
{

namespace implementation
{
class age;
}

///
/// \brief Specifies an age, in days, weeks, months or years.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API Age
{
    friend class DataSet;
    friend class ReadingDataHandler;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source Date object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    Age(const Age& source);

    Age& operator=(const Age& source) = delete;

    virtual ~Age();

    /// \brief Constructor.
    ///
    /// \param initialAge the initial age to assign to the object, in days, weeks,
    ///                   months or years, depending on the parameter initialUnits
    /// \param initialUnits the units of the value in initialAge
    ///
    ///////////////////////////////////////////////////////////////////////////////
    Age(std::uint32_t initialAge, ageUnit_t initialUnits);

    /// \brief Return the age in years.
    ///
    /// \return the stored age converted to years.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    double getYears() const;

    std::uint32_t getAgeValue() const;
    ageUnit_t getAgeUnits() const;

#ifndef SWIG
protected:
    explicit Age(const std::shared_ptr<implementation::age>& pAge);

private:
    friend const std::shared_ptr<implementation::age>& getAgeImplementation(const Age& age);
    std::shared_ptr<implementation::age> m_pAge;
#endif

};

} // namespace imebra

#endif // !defined(imebraAGE__INCLUDED_)
