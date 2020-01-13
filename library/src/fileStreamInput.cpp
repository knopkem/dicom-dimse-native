/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file fileStreamInput.cpp
    \brief Implementation of the file input stream class.

*/

#include "../include/imebra/fileStreamInput.h"
#include "../implementation/fileStreamImpl.h"

namespace imebra
{

FileStreamInput::~FileStreamInput()
{
}

FileStreamInput::FileStreamInput(const std::wstring& name): BaseStreamInput(std::make_shared<implementation::fileStreamInput>(name))
{
}

FileStreamInput::FileStreamInput(const std::string& name): BaseStreamInput(std::make_shared<implementation::fileStreamInput>(name))
{
}

FileStreamInput::FileStreamInput(const FileStreamInput& source): BaseStreamInput(source)
{
}

}
