/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transformsChain.cpp
    \brief Implementation of the TransformsChain class.

*/

#include "../include/imebra/transformsChain.h"
#include "../implementation/transformsChainImpl.h"

namespace imebra
{

TransformsChain::TransformsChain(): Transform(std::make_shared<imebra::implementation::transforms::transformsChain>())
{
}

TransformsChain::TransformsChain(const TransformsChain& source): Transform(source)
{
}

TransformsChain::~TransformsChain()
{
}

void TransformsChain::addTransform(const Transform& transform)
{
    IMEBRA_FUNCTION_START();

    ((imebra::implementation::transforms::transformsChain*)m_pTransform.get())->addTransform(getTransformImplementation(transform));

    IMEBRA_FUNCTION_END_LOG();
}

}
