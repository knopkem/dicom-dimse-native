/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file age.cpp
    \brief Implementation of the class Age.

*/

#include "../include/imebra/age.h"
#include "../implementation/ageImpl.h"

namespace imebra
{

Age::Age(const Age& source):
    m_pAge(getAgeImplementation(source))
{
}

Age::Age(const std::shared_ptr<implementation::age>& pAge):
    m_pAge(pAge)
{
}

const std::shared_ptr<implementation::age>& getAgeImplementation(const Age& age)
{
    return age.m_pAge;
}

Age::~Age()
{
}

Age::Age(std::uint32_t initialAge, ageUnit_t initialUnits):
    m_pAge(std::make_shared<implementation::age>(initialAge, initialUnits))
{
}

double Age::getYears() const
{
    return m_pAge->getYears();
}

std::uint32_t Age::getAgeValue() const
{
    return m_pAge->getAgeValue();
}

ageUnit_t Age::getAgeUnits() const
{
    return m_pAge->getAgeUnits();
}


} // namespace imebra


