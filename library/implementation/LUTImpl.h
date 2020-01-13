/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file LUT.h
    \brief Declaration of the class lut.

*/

#if !defined(imebraLUT_C2D59748_5D38_4b12_BA16_5EC22DA7C0E7__INCLUDED_)
#define imebraLUT_C2D59748_5D38_4b12_BA16_5EC22DA7C0E7__INCLUDED_

#include <map>
#include <memory>
#include "dataHandlerNumericImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{
    class readingDataHandler;
    class writingDataHandler;
    class buffer;
}

/// \addtogroup group_image
///
/// @{

///////////////////////////////////////////////////////////
/// \brief Represents a Lookup table (LUT).
///
/// The lookup table maps a value stored into an image
///  into another value that must be used for the
///  visualization or for the analysis of the image.
///
/// 3 lookups tables can be joined together to form a
///  color palette.
///
///////////////////////////////////////////////////////////
class lut
{
public:
    // Destructor
    ///////////////////////////////////////////////////////////
    virtual ~lut();


    /// \brief Initializes the lut with the values stored in
    ///         three data handlers, usually retrieved from
    ///         a dataset.
    ///
    /// @param pDescriptor   the handler that manages the
    ///                       lut descriptor (size, first
    ///                       mapped value and number of bits)
    /// @param pData         the handler that manages the
    ///                       lut data
    /// @param description   a string that describes the
    ///                       lut
    ///
    ///////////////////////////////////////////////////////////
    lut(std::shared_ptr<handlers::readingDataHandlerNumericBase> pDescriptor, std::shared_ptr<handlers::readingDataHandlerNumericBase> pData, const std::wstring& description, bool signedData);

    std::shared_ptr<handlers::readingDataHandlerNumericBase> getReadingDataHandler() const;

    /// \brief Return the lut's description.
    ///
    /// @return the lut description
    ///
    ///////////////////////////////////////////////////////////
    std::wstring getDescription() const;

    /// \brief Return the number of bits used to store a mapped
    ///         value.
    ///
    /// @return the number of bits used to store a mapped value
    ///
    ///////////////////////////////////////////////////////////
    std::uint8_t getBits() const;

    /// \brief Return the lut's size.
    ///
    /// @return the number of mapped value stored in the lut
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getSize() const;

    std::int32_t getFirstMapped() const;

    std::uint32_t getMappedValue(std::int32_t index) const;

protected:
    // Convert a signed value in the LUT descriptor to an
    //  unsigned value.
    ///////////////////////////////////////////////////////////
    std::uint32_t descriptorSignedToUnsigned(std::int32_t signedValue);

    std::uint32_t m_size;
    std::int32_t m_firstMapped;
    std::uint8_t m_bits;

    std::wstring m_description;

    std::shared_ptr<handlers::readingDataHandlerNumericBase> m_pDataHandler;
};


/// \brief Represents an RGB color palette.
///
/// A color palette uses 3 lut objects to represent the
///  colors.
///
///////////////////////////////////////////////////////////
class palette
{
public:
    /// \brief Construct the color palette.
    ///
    /// @param red   the lut containing the red components
    /// @param green the lut containing the green components
    /// @param blue  the lut containing the blue components
    ///
    ///////////////////////////////////////////////////////////
    palette(std::shared_ptr<lut> red, std::shared_ptr<lut> green, std::shared_ptr<lut> blue);

    /// \brief Retrieve the lut containing the red components.
    ///
    /// @return the lut containing the red components
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<lut> getRed() const;

    /// \brief Retrieve the lut containing the green components.
    ///
    /// @return the lut containing the green components
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<lut> getGreen() const;

    /// \brief Retrieve the lut containing the blue components.
    ///
    /// @return the lut containing the blue components
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<lut> getBlue() const;

protected:
    std::shared_ptr<lut> m_redLut;
    std::shared_ptr<lut> m_greenLut;
    std::shared_ptr<lut> m_blueLut;
};


/// @}


} // namespace implementation

} // namespace imebra

#endif // !defined(imebraLUT_C2D59748_5D38_4b12_BA16_5EC22DA7C0E7__INCLUDED_)
