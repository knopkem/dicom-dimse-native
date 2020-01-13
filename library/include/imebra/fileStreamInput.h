/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file fileStreamInput.h
    \brief Declaration of the FileStreamInput class.

*/

#if !defined(imebraFileStreamInput__INCLUDED_)
#define imebraFileStreamInput__INCLUDED_

#include <string>
#include "baseStreamInput.h"
#include "definitions.h"

namespace imebra
{

///
/// \brief Represents an input file stream.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API FileStreamInput : public BaseStreamInput
{

public:
    /// \brief Constructor.
    ///
    /// \param name the path to the file to open in read mode
    ///
    ///////////////////////////////////////////////////////////////////////////////
#ifndef SWIG // Use only UTF-8 strings with SWIG
    explicit FileStreamInput(const std::wstring& name);
#endif

    /// \brief Constructor.
    ///
    /// \param name the path to the file to open in read mode, in encoded in UTF8
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit FileStreamInput(const std::string& name);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source FileStreamInput object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    FileStreamInput(const FileStreamInput& source);

    FileStreamInput& operator=(const FileStreamInput& source) = delete;

    /// \brief Destructor. Closes the file.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    ~FileStreamInput();
};

}
#endif // !defined(imebraFileStreamInput__INCLUDED_)
