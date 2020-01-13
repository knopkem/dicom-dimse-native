/*
Copyright 2005 - 2019 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file overlay.h
    \brief Declaration of the class Overlay.

*/

#if !defined(imebraOverlay__INCLUDED_)
#define imebraOverlay__INCLUDED_

#include <memory>
#include <string>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class overlay;
}

class Image;

///
/// \brief Represents an immutable DICOM overlay.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API Overlay
{
    friend class DataSet;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source Image object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    Overlay(const Overlay& source);

    virtual ~Overlay();

    Overlay& operator=(const Overlay& source) = delete;

    ///
    /// \brief Retrieve the number of the first frame related
    ///        to this overlay.
    ///
    /// \return the zero based number of the first frame (Image) related to this
    ///         overlay
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint32_t getFirstFrame() const;

    ///
    /// \brief Returns the number of bitmaps stored in this overlay.
    ///
    /// \return the number of bitmaps stored in the overlay
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint32_t getFramesCount() const;

    ///
    /// \brief Returns the horizontal position of the overlay bitmap in the DICOM
    ///        image.
    ///
    /// \return the zero-based horizontal position of the overlay bitmap into the
    ///         related DICOM image
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::int32_t getZeroBasedOriginX() const;

    ///
    /// \brief Returns the vertical position of the overlay bitmap in the DICOM
    ///        image.
    ///
    /// \return the zero-based vertical position of the overlay bitmap into the
    ///         related DICOM image
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::int32_t getZeroBasedOriginY() const;

    ///
    /// \brief Returns the horizontal position of the overlay bitmap in the DICOM
    ///        image.
    ///
    /// \return the one-based horizontal position of the overlay bitmap into the
    ///         related DICOM image
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::int32_t getOneBasedOriginX() const;

    ///
    /// \brief Returns the vertical position of the overlay bitmap in the DICOM
    ///        image.
    ///
    /// \return the one-based vertical position of the overlay bitmap into the
    ///         related DICOM image
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::int32_t getOneBasedOriginY() const;

    ///
    /// \brief Return the overlay type.
    ///
    /// \return the overlay type.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    overlayType_t getType() const;

    ///
    /// \brief Return the overlay subtype.
    ///
    /// Defined Term that identifies  the intended purpose of the Overlay Type.
    /// See Section C.9.2.1.3 for further explanation.
    ///
    /// \return the overlay subtype.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getSubType() const;

#ifndef SWIG // Use UTF8 strings only with SWIG
    ///
    /// \brief Returns the unicode overlay label.
    ///
    /// \return Unicode overlay label
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getUnicodeLabel() const;

    ///
    /// \brief Return the unicode overlay description.
    ///
    /// \return Unicode overlay description
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getUnicodeDescription() const;
#endif

    ///
    /// \brief Return the UTF-8 encoded overlay label.
    ///
    /// \return UTF-8 encoded overlay label
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getLabel() const;

    ///
    /// \brief Return the UTF-8 encoded overlay description.
    ///
    /// \return UTF-8 encoded overlay description
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getDescription() const;

    ///
    /// \brief Return the ROI area if present, otherwise throws
    ///        MissingTagError
    ///
    /// \return the ROI area in pixels
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint32_t getROIArea() const;

    ///
    /// \brief Return the ROI mean if present, otherwise throws
    ///        MissingTagError
    ///
    /// \return the ROI mean
    ///
    ///////////////////////////////////////////////////////////////////////////////
    double getROIMean() const;

    ///
    /// \brief Return the ROI standard deviation if present, otherwise throws
    ///        MissingTagError
    ///
    /// \return the ROI standard deviation
    ///
    ///////////////////////////////////////////////////////////////////////////////
    double getROIStandardDeviation();

    ///
    /// \brief Return one of the overlay images.
    ///
    /// \param frame zero based frame to retrieve
    ///
    /// \return the overlay image
    ///
    ///////////////////////////////////////////////////////////////////////////////
    const Image getImage(std::uint32_t frame) const;

#ifndef SWIG
protected:
    explicit Overlay(const std::shared_ptr<imebra::implementation::overlay>& pOverlay);

private:
    friend const std::shared_ptr<implementation::overlay>& getOverlayImplementation(const Overlay& overlay);
    std::shared_ptr<implementation::overlay> m_pOverlay;
#endif
};

///
/// \brief Represents a mutable DICOM overlay.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API MutableOverlay: public Overlay
{

public:

#ifndef SWIG // Use UTF8 strings only with SWIG
    /// \brief Constructor.
    ///
    /// \param overlayType    the overlay type
    /// \param overlaySubType the overlay sub-type (defined Term that identifies
    ///                        the intended purpose of the Overlay Type. See
    ///                        Section C.9.2.1.3 for further explanation).
    /// \param firstFrame     the first DICOM image frame related to the overlay
    /// \param zeroBasedOriginX the overlay zero-based horizontal position within
    ///                        the DICOM image
    /// \param zeroBasedOriginY the overlay zero-based vertical position within
    ///                        the DICOM image
    /// \param label           the unicode overlay label
    /// \param description     the unicode overlay description
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit MutableOverlay(
            overlayType_t overlayType,
            const std::string& overlaySubType,
            std::uint32_t firstFrame,
            std::int32_t zeroBasedOriginX, std::int32_t zeroBasedOriginY,
            const std::wstring& label,
            const std::wstring& description);
#endif

    /// \brief Constructor.
    ///
    /// \param overlayType    the overlay type
    /// \param overlaySubType the overlay sub-type (defined Term that identifies
    ///                        the intended purpose of the Overlay Type. See
    ///                        Section C.9.2.1.3 for further explanation).
    /// \param firstFrame     the first DICOM image frame related to the overlay
    /// \param zeroBasedOriginX the overlay zero-based horizontal position within
    ///                        the DICOM image
    /// \param zeroBasedOriginY the overlay zero-based vertical position within
    ///                        the DICOM image
    /// \param label           the UTF-8 encoded overlay label
    /// \param description     the UTF-8 encoded overlay description
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit MutableOverlay(
            overlayType_t overlayType,
            const std::string& overlaySubType,
            std::uint32_t firstFrame,
            std::int32_t zeroBasedOriginX, std::int32_t zeroBasedOriginY,
            const std::string& label,
            const std::string& description);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source MutableImage object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    MutableOverlay(const MutableOverlay& source);

    virtual ~MutableOverlay();

    MutableOverlay& operator=(const MutableOverlay& source) = delete;

    ///
    /// \brief Set the number of pixels in the ROI area.
    ///
    /// \param pixels number of pixels in the ROI area
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setROIArea(std::uint32_t pixels);

    ///
    /// \brief Set the mean value of the pixels in the ROI area.
    ///
    /// \param mean mean value of the pixels in the ROI area
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setROIMean(double mean);

    ///
    /// \brief Set the standard deviation of the value of the pixels in the ROI
    ///        area.
    ///
    /// \param standardDeviation standard deviation of the value of the pixels in
    ///                          the ROI area
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setROIStandardDeviation(double standardDeviation);

    ///
    /// \brief Add a bitmap to the overlay.
    ///
    /// \param frame a zero based frame (relative to the overlay frames)
    /// \param image the image for the overlay
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setImage(std::uint32_t frame, const Image& image);

protected:
    explicit MutableOverlay(std::shared_ptr<imebra::implementation::overlay> pOverlay);

};

}

#endif // !defined(imebraOverlay__INCLUDED_)
