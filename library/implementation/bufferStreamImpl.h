/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

///////////////////////////////////////////////////////////
//  bufferStream.h
//  Declaration of the Class bufferStream
///////////////////////////////////////////////////////////

#if !defined(imebraBufferStream_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
#define imebraBufferStream_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_

#include "memoryStreamImpl.h"
#include "dataHandlerNumericImpl.h"

namespace imebra
{

namespace implementation
{

/// \addtogroup group_dataset
///
/// @{

class bufferStreamOutput: public memoryStreamOutput
{
public:
    bufferStreamOutput(std::shared_ptr<handlers::writingDataHandlerRaw> pDataHandler):
      memoryStreamOutput(pDataHandler->getMemory()),
	  m_pDataHandler(pDataHandler){}
protected:

    std::shared_ptr<handlers::writingDataHandlerRaw> m_pDataHandler;
};

/// @}

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraBufferStream_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
