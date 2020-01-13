/*
Copyright 2005 - 2019 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file overlayImpl.h
    \brief Declaration of the overlay class

*/

#if !defined(imebraOverlay_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraOverlay_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include <memory>
#include <../include/imebra/definitions.h>

namespace imebra
{

namespace implementation
{

class image;
class buffer;

namespace handlers
{
class readingDataHandlerNumericBase;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///
/// \brief Stores information about an overlay
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class overlay
{

public:
    ///
    /// \brief Constructor.
    ///
    /// \param width                       the images width
    /// \param height                      the images height
    /// \param firstFrame                  first frame related
    ///                                     to the overlay
    /// \param framesCount                 number of frames in
    ///                                     this overlay
    /// \param zeroBasedOriginX            origin X (zero based)
    /// \param zeroBasedOriginY            origin Y (zero based)
    /// \param type                        overlay type
    /// \param subType                     overlay subtype
    /// \param label                       label
    /// \param description                 description
    /// \param roiArea                     ROI area
    /// \param roiAreaPresent              true if ROI area is
    ///                                     valid
    /// \param roiMean                     ROI mean
    /// \param roiMeanPresent              true if ROI mean is
    ///                                     valid
    /// \param roiStandardDeviation        ROI standard
    ///                                     deviation
    /// \param roiStandardDeviationPresent true if ROI standard
    ///                                     deviation is valid
    /// \param pBuffer                     contains the overlay
    ///                                     imagea
    ///
    ///////////////////////////////////////////////////////////
    overlay(
            std::uint32_t width,
            std::uint32_t height,
            std::uint32_t firstFrame,
            std::uint32_t framesCount,
            std::int32_t zeroBasedOriginX,
            std::int32_t zeroBasedOriginY,
            overlayType_t type,
            const std::string& subType,
            const std::wstring& label,
            const std::wstring& description,
            std::uint32_t roiArea,
            bool roiAreaPresent,
            double roiMean,
            bool roiMeanPresent,
            double roiStandardDeviation,
            bool roiStandardDeviationPresent,
            const std::shared_ptr<buffer>& pBuffer);

    ///
    /// \brief Constructor.
    ///
    /// \param type    the overlay type
    /// \param subType the overlay subtype
    ///
    ///////////////////////////////////////////////////////////
    overlay(
            overlayType_t type,
            const std::string& subType,
            std::uint32_t firstFrame,
            std::int32_t zeroBasedOriginX, std::int32_t zeroBasedOriginY,
            const std::wstring& label,
            const std::wstring& description);

    overlay(
            overlayType_t type,
            const std::string& subType,
            std::uint32_t firstFrame,
            std::int32_t zeroBasedOriginX, std::int32_t zeroBasedOriginY,
            const std::string& label,
            const std::string& description);

    ///
    /// \brief Set the first frame related to the overlay
    ///
    /// \param firstFrame first frame (zero based)
    ///
    ///////////////////////////////////////////////////////////
    void setFirstFrame(std::uint32_t firstFrame);
    void setZeroBasedOrigin(std::int32_t x, std::int32_t y);
    void setOneBasedOrigin(std::int32_t x, std::int32_t y);
    void setUnicodeLabel(const std::wstring& label);
    void setLabel(const std::string& label);
    void setUnicodeDescription(const std::wstring& description);
    void setDescription(const std::string& description);
    void setROIArea(std::uint32_t roiArea);
    void setROIMean(double roiMean);
    void setROIStandardDeviation(double roiStandardDeviation);
    void setImage(std::uint32_t frame, const std::shared_ptr<const image>& pImage);

    std::uint32_t getFirstFrame() const;
    std::uint32_t getFramesCount() const;
    std::int32_t getZeroBasedOriginX() const;
    std::int32_t getZeroBasedOriginY() const;
    std::int32_t getOneBasedOriginX() const;
    std::int32_t getOneBasedOriginY() const;
    overlayType_t getType() const;
    std::string getSubType() const;
    std::wstring getUnicodeLabel() const;
    std::string getLabel() const;
    std::wstring getUnicodeDescription() const;
    std::string getDescription() const;
    std::uint32_t getROIArea() const;
    bool getROIAreaPresent() const;
    double getROIMean() const;
    bool getROIMeanPresent() const;
    double getROIStandardDeviation() const;
    bool getROIStandardDeviationPresent() const;
    std::shared_ptr<image> getImage(std::uint32_t frame) const;
    std::shared_ptr<buffer> getBuffer() const;

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::uint32_t m_firstFrame;
    std::uint32_t m_framesCount;
    std::int32_t m_originX;
    std::int32_t m_originY;
    overlayType_t m_type;
    std::string m_subType;
    std::wstring m_label;
    std::wstring m_description;
    std::uint32_t m_roiArea;
    bool m_roiAreaPresent;
    double m_roiMean;
    bool m_roiMeanPresent;
    double m_roiStandardDeviation;
    bool m_roiStandardDeviationPresent;

    std::shared_ptr<buffer> m_pBuffer;
    tagVR_t m_dataType;
};

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraOverlay_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
