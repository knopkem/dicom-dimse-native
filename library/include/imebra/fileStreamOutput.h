/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file fileStreamOutput.h
    \brief Declaration of the file output stream class.

*/

#if !defined(imebraFileStreamOutput__INCLUDED_)
#define imebraFileStreamOutput__INCLUDED_

#include <string>
#include "baseStreamOutput.h"
#include "definitions.h"

namespace imebra
{

///
/// \brief Represents an output file stream.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API FileStreamOutput : public BaseStreamOutput
{

public:
    /// \brief Constructor.
    ///
    /// \param name the path to the file to open in write mode
    ///
    ///////////////////////////////////////////////////////////////////////////////
#ifndef SWIG // Use only UTF-8 strings with SWIG
    explicit FileStreamOutput(const std::wstring& name);
#endif

    /// \brief Constructor.
    ///
    /// \param name the path to the file to open in write mode
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit FileStreamOutput(const std::string& name);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source FileStreamOutput object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    FileStreamOutput(const FileStreamOutput& source);

    FileStreamOutput& operator=(const FileStreamOutput& source) = delete;

    /// \brief Destructor. Closes the file
    ///
    ///////////////////////////////////////////////////////////////////////////////
    ~FileStreamOutput();

};

}
#endif // !defined(imebraFileStreamOutput__INCLUDED_)
