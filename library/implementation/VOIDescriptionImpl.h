/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOIDescriptionImpl.h
    \brief Declaration of the class VOIDescription.

*/

#if !defined(imebraVOIDescription_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraVOIDescription_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include <string>
#include "../include/imebra/definitions.h"

namespace imebra
{

namespace implementation
{

class VOIDescription
{
public:
    VOIDescription(double center, double width, dicomVOIFunction_t function, const std::string& description);
    VOIDescription(double center, double width, dicomVOIFunction_t function, const std::wstring& description);

    double getCenter() const;
    double getWidth() const;
    dicomVOIFunction_t getFunction() const;
    std::string getDescription() const;
    std::wstring getUnicodeDescription() const;

private:
    const double m_center;
    const double m_width;
    const dicomVOIFunction_t m_function;
    const std::string m_description;
};

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraVOIDescription_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
