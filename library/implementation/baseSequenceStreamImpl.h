/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseSequenceStreamImpl.h
    \brief Declaration of the the base class for the sequence streams (TCP)
            used by the imebra library.

*/

#if !defined(imebraBaseSequenceStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraBaseSequenceStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "exceptionImpl.h"
#include "baseStreamImpl.h"
#include <mutex>
#include <stdexcept>


namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class represents a sequential input stream.
///
/// Specialized input stream that cannot seek its
///  read position.
///
/// \warning This class is not thread safe, nor are the
/// streamController derived classes using this class.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class baseSequenceStreamInput: public baseStreamInput
{
public:
    baseSequenceStreamInput();

    virtual size_t read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength) override;

    virtual size_t read(std::uint8_t* pBuffer, size_t bufferLength) = 0;

private:
    size_t m_currentPosition;

    std::mutex m_mutex;
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class represents a sequential output
///        stream.
///
/// Specialized output stream that cannot seek its
///  write position.
///
/// \warning This class is not thread safe, nor are the
/// streamController derived classes using this class.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class baseSequenceStreamOutput: public baseStreamOutput
{
public:
    baseSequenceStreamOutput();

    virtual void write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength) override;

    virtual void write(const std::uint8_t* pBuffer, size_t bufferLength) = 0;

private:
    size_t m_currentPosition;

    std::mutex m_mutex;
};

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraBaseSequenceStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
