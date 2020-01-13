/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file ageImpl.cpp
    \brief Implementation of the class age.

*/

#include "VOIDescriptionImpl.h"
#include <codecvt>
#include <locale>

namespace imebra
{

namespace implementation
{

VOIDescription::VOIDescription(double center, double width, dicomVOIFunction_t function, const std::string& description):
    m_center(center), m_width(width), m_function(function), m_description(description)
{
}

VOIDescription::VOIDescription(double center, double width, dicomVOIFunction_t function, const std::wstring& description):
    m_center(center),
    m_width(width),
    m_function(function),
    m_description(std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(description))
{
}

double VOIDescription::getCenter() const
{
    return m_center;
}

double VOIDescription::getWidth() const
{
    return m_width;
}

dicomVOIFunction_t VOIDescription::getFunction() const
{
    return m_function;
}

std::string VOIDescription::getDescription() const
{
    return m_description;
}

std::wstring VOIDescription::getUnicodeDescription() const
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(m_description);
}

} // namespace implementation

} // namespace imebra

