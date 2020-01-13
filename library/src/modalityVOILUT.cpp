/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file modalityVOILUT.cpp
    \brief Implementation of the ModalityVOILUT class..

*/


#include "../include/imebra/modalityVOILUT.h"
#include "../implementation/modalityVOILUTImpl.h"

namespace imebra
{

ModalityVOILUT::ModalityVOILUT(const DataSet& dataset):
    Transform(std::make_shared<imebra::implementation::transforms::modalityVOILUT>(getDataSetImplementation(dataset)))
{
}

ModalityVOILUT::ModalityVOILUT(const ModalityVOILUT& source): Transform(source)
{
}

ModalityVOILUT::~ModalityVOILUT()
{
}

}
