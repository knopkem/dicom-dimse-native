/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOILUT.h
    \brief Declaration of the class VOILUT.

*/

#if !defined(imebraVOILUT__INCLUDED_)
#define imebraVOILUT__INCLUDED_

#include "transform.h"
#include "definitions.h"

namespace imebra
{

class DataSet;
class LUT;

///
/// \brief A VOILUT transform enhances the visibility of a specific range of
///        brightness in an image.
///
/// The client can use VOILUT in three ways:
/// - by declaring the minimum and maximum values of the pixels that must be
///   visible via the method setCenterWidth()
/// - by declaring a LUT via the method setLUT()
/// - by letting the transform calculate the most appropriate center/width
///   values via the method applyOptimalVOI()
///
/// The DataSet may already supply suitable values for the methods
/// setCenterWidth() and setLUT().
///
/// In order to retrieve from the DataSet the suggested center/width values,
/// call DataSet.getVOIs().
///
/// To get from the DataSet a list of LUTs that can be used with the VOILUT
/// transform use DataSet.getLUT(TagId(tagId_t::VOILUTSequence_0028_3010), X)
/// where X is a 0 base index (the DataSet may provide more than one LUT for
/// the VOILUT transform).
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API VOILUT: public Transform
{

public:

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source VOILUT object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    VOILUT(const VOILUT& source);

    VOILUT& operator=(const VOILUT& source) = delete;

    ///
    /// \brief Construct a VOILUT transform and specifies the LUT (lookup table)
    ///        to use in the transform.
    ///
    /// \param lut the lookup table to use during the transform
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit VOILUT (const LUT& lut);

    ///
    /// \brief Construct a VOILUT transform and specifies the window center, width
    ///        and function to use during the transform
    ///
    /// \param voiDescription a VOIDescription object specifying the center,
    ///                       width and function
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit VOILUT (const VOIDescription& voiDescription);

    virtual ~VOILUT();

    /// \brief Find the optimal VOI settings for a specific image's area and apply
    ///        it with setCenterWidth().
    ///
    /// \param inputImage   the image to analyze
    /// \param topLeftX     the horizontal coordinate of the top-left angle of the
    ///                     area to analyze
    /// \param topLeftY     the vertical coordinate of the top-left angle of the
    ///                     area to analyze
    /// \param width        the width of the area to analyze
    /// \param height       the height of the area to analyze
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static VOIDescription getOptimalVOI(const Image& inputImage, std::uint32_t topLeftX, std::uint32_t topLeftY, std::uint32_t width, std::uint32_t height);

};

}

#endif // imebraVOILUT__INCLUDED_
