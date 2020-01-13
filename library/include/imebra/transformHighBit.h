/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transformHighBit.h
    \brief Declaration of TransformHighBit.

*/

#if !defined(imebraTransformHighBit__INCLUDED_)
#define imebraTransformHighBit__INCLUDED_

#include <memory>
#include "definitions.h"
#include "transform.h"

namespace imebra
{

namespace implementation
{
namespace transforms
{
    class transformHighBit;
}
}

///
/// \brief The TransformHighBit transform shifts the input image's values
///        so they match the high-bit settings of the Transform output image.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TransformHighBit: public Transform
{

public:
    /// \brief Constructor.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TransformHighBit();

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source TransformHighBit object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TransformHighBit(const TransformHighBit& source);

    TransformHighBit& operator=(const TransformHighBit& source) = delete;

    virtual ~TransformHighBit();

};

}

#endif // imebraTransformHighBit__INCLUDED_
