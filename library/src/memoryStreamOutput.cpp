/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/memoryStreamOutput.h"
#include "../include/imebra/mutableMemory.h"
#include "../implementation/memoryStreamImpl.h"
#include <memory>

namespace imebra
{

MemoryStreamOutput::MemoryStreamOutput(const MutableMemory& memory):
    BaseStreamOutput(std::make_shared<implementation::memoryStreamOutput>(std::const_pointer_cast<implementation::memory>(getMemoryImplementation(memory))))
{
}

MemoryStreamOutput::MemoryStreamOutput(const MemoryStreamOutput& source): BaseStreamOutput(source)
{
}

MemoryStreamOutput::~MemoryStreamOutput()
{
}

}
