/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file stream.h
    \brief Declaration of the stream class.

*/

#if !defined(imebraStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "baseStreamImpl.h"

#include <ios>
#include <stdio.h>
#include <mutex>


namespace imebra
{

namespace implementation
{

class fileStream
{
public:
    enum class openMode: std::uint8_t
    {
        read = 0,
        write = 1
    };

    fileStream(const std::wstring& fileName, openMode mode);
    fileStream(const std::string& fileName, openMode mode);

    virtual ~fileStream();

protected:
    FILE* m_openFile;
    mutable std::mutex m_mutex;
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class derives from the baseStream
///         class and implements a file stream.
///
/// This class can be used to read/write on physical files
///  in the mass storage.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class fileStreamInput : public baseStreamInput, public fileStream
{
public:
    fileStreamInput(const std::string& fileName);
    fileStreamInput(const std::wstring& fileName);

    ///////////////////////////////////////////////////////////
    //
    // Virtual stream's functions
    //
    ///////////////////////////////////////////////////////////
    virtual size_t read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength) override;

    virtual void terminate() override;

    virtual bool seekable() const override;

    ///////////////////////////////////////////////////////////
    //
    // Returns the file size
    //
    ///////////////////////////////////////////////////////////
    size_t getSize() const;
};

class fileStreamOutput : public baseStreamOutput, public fileStream
{
public:
    fileStreamOutput(const std::string& fileName);

    fileStreamOutput(const std::wstring& fileName);

    ///////////////////////////////////////////////////////////
    //
    // Virtual stream's functions
    //
    ///////////////////////////////////////////////////////////
    virtual void write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength) override;

};

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
