/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transformsChain.h
    \brief Declaration of TransformsChain.

*/

#if !defined(imebraTransformsChain__INCLUDED_)
#define imebraTransformsChain__INCLUDED_

#include <memory>
#include "definitions.h"
#include "transform.h"

namespace imebra
{

namespace implementation
{
namespace transforms
{
    class transformsChain;
}
}

class DrawBitmap;

///
/// \brief The TransformsChain applies a series of Transforms to the input
///        image.
///
/// Transforms can be added to the chain by using addTransform().
///
///////////////////////////////////////////////////////////////////////////////

class IMEBRA_API TransformsChain: public Transform
{

public:
    /// \brief Constructor.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TransformsChain();

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source TransformsChain object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TransformsChain(const TransformsChain& source);

    TransformsChain& operator=(const TransformsChain& source) = delete;

    virtual ~TransformsChain();

    /// \brief Add a transform to the chain of transforms that will be applied to
    ///        the input image.
    ///
    /// \param transform the Transform to add to the chain of transforms
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void addTransform(const Transform& transform);
};

}

#endif // imebraTransformsChain__INCLUDED_
