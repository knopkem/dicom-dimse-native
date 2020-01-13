/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOIDescription.cpp
    \brief Implementation of the class VOIDescription.

*/

#include "../include/imebra/VOIDescription.h"
#include "../implementation/VOIDescriptionImpl.h"

namespace imebra
{

VOIDescription::VOIDescription():
    m_pVOIDescription(std::make_shared<implementation::VOIDescription>(0.0, 1.0, dicomVOIFunction_t::linear, ""))
{
}

VOIDescription::VOIDescription(const VOIDescription& source):
    m_pVOIDescription(getVOIDescriptionImplementation(source))
{
}

VOIDescription& VOIDescription::operator=(const VOIDescription& source)
{
    m_pVOIDescription = getVOIDescriptionImplementation(source);
    return *this;
}

VOIDescription::VOIDescription(const std::shared_ptr<const implementation::VOIDescription>& pVOIDescription):
    m_pVOIDescription(pVOIDescription)
{
}

const std::shared_ptr<const implementation::VOIDescription>& getVOIDescriptionImplementation(const VOIDescription& voiDescription)
{
    return voiDescription.m_pVOIDescription;
}

VOIDescription::~VOIDescription()
{
}

VOIDescription::VOIDescription(double center, double width, dicomVOIFunction_t function, const std::string& description):
    m_pVOIDescription(std::make_shared<implementation::VOIDescription>(center, width, function, description))
{
}

VOIDescription::VOIDescription(double center, double width, dicomVOIFunction_t function, const std::wstring& description):
    m_pVOIDescription(std::make_shared<implementation::VOIDescription>(center, width, function, description))
{
}

double VOIDescription::getCenter() const
{
    return m_pVOIDescription->getCenter();
}

double VOIDescription::getWidth() const
{
    return m_pVOIDescription->getWidth();
}

dicomVOIFunction_t VOIDescription::getFunction() const
{
    return m_pVOIDescription->getFunction();
}

std::string VOIDescription::getDescription() const
{
    return m_pVOIDescription->getDescription();
}

std::wstring VOIDescription::getUnicodeDescription() const
{
    return m_pVOIDescription->getUnicodeDescription();
}

} // namespace imebra


