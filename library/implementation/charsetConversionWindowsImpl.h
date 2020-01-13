/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionWindows.h
    \brief Declaration of the class used to convert a string between different
            charsets in Windows.

*/

#if !defined(imebraCharsetConversionWindows_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraCharsetConversionWindows_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#if defined(IMEBRA_USE_WINDOWS_CHARSET)

#include "charsetConversionBaseImpl.h"
#include <windows.h>


namespace imebra
{
class charsetConversionWindows: public charsetConversionBase
{
public:
    charsetConversionWindows(const std::string& dicomName);

    virtual std::string fromUnicode(const std::wstring& unicodeString) const override;

    virtual std::wstring toUnicode(const std::string& asciiString) const override;

protected:
	unsigned long m_codePage;
	bool m_bZeroFlag;
};

typedef charsetConversionWindows defaultCharsetConversion;

} // namespace imebra

#endif // defined(IMEBRA_USE_WINDOWS_CHARSET)

#endif // !defined(imebraCharsetConversionWindows_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
