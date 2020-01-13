/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseStreamOutput.cpp
    \brief Implementation of the the base class for the streams (memory, file, ...).
*/

#include "../include/imebra/baseStreamOutput.h"
#include "../implementation/baseStreamImpl.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

BaseStreamOutput::BaseStreamOutput(const BaseStreamOutput &source):
    m_pOutputStream(getBaseStreamOutputImplementation(source))
{
}

BaseStreamOutput::BaseStreamOutput(const std::shared_ptr<implementation::baseStreamOutput>& pOutputStream): m_pOutputStream(pOutputStream)
{
}

BaseStreamOutput::~BaseStreamOutput()
{
}

const std::shared_ptr<implementation::baseStreamOutput>& getBaseStreamOutputImplementation(const BaseStreamOutput& baseStreamOutput)
{
    return baseStreamOutput.m_pOutputStream;
}

}
