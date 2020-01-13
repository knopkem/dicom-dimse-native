/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tag.h
    \brief Declaration of the class Tag.

*/

#if !defined(imebraLut__INCLUDED_)
#define imebraLut__INCLUDED_

#include <string>
#include <cstdint>
#include "readingDataHandlerNumeric.h"
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class lut;
}

///
/// \brief This class represents Lookup Table.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API LUT
{

    friend class DataSet;
    friend class MutableDataSet;
    friend class VOILUT;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source LUT object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    LUT(const LUT& source);

    LUT& operator=(const LUT& source) = delete;

    virtual ~LUT();

    /// \brief Returns the LUT's description.
    ///
    /// \return the LUT's description
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getDescription() const;

    ReadingDataHandlerNumeric getReadingDataHandler() const;

    /// \brief Return the number of bits used to store a LUT value.
    ///
    /// @return the number of bits used to store a mapped value
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getBits() const;

    /// \brief Return the lut's size (the number of stored values).
    ///
    /// @return the number of mapped values stored in the lut
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getSize() const;

    std::int32_t getFirstMapped() const;

    std::uint32_t getMappedValue(std::int32_t index) const;

#ifndef SWIG
protected:
    explicit LUT(const std::shared_ptr<imebra::implementation::lut>& pLut);

private:
    friend const std::shared_ptr<imebra::implementation::lut>& getLUTImplementation(const LUT& lut);
    std::shared_ptr<imebra::implementation::lut> m_pLut;
#endif
};

}

#endif // !defined(imebraLut__INCLUDED_)
