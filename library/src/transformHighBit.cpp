/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transformsHighBit.cpp
    \brief Implementation of the TransformsHighBit class.

*/

#include "../include/imebra/transformHighBit.h"
#include "../implementation/transformHighBitImpl.h"


namespace imebra
{

TransformHighBit::TransformHighBit(): Transform(std::make_shared<imebra::implementation::transforms::transformHighBit>())
{
}

TransformHighBit::TransformHighBit(const TransformHighBit& source): Transform(source)
{
}

TransformHighBit::~TransformHighBit()
{
}

}
