/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegStreamCodec.h
    \brief Declaration of the class jpegStreamCodec.

*/

#if !defined(imebraJpegStreamCodec_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_)
#define imebraJpegStreamCodec_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_

#include "streamCodecImpl.h"
#include "jpegCodecBaseImpl.h"
#include <map>
#include <list>


namespace imebra
{

namespace implementation
{

namespace codecs
{

/// \addtogroup group_codecs
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief The Jpeg codec.
///
/// This class is used to decode and encode a Jpeg stream.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class jpegStreamCodec : public streamCodec, public jpegCodecBase
{
public:
	// Constructor
	///////////////////////////////////////////////////////////
    jpegStreamCodec();

protected:
	// Read a jpeg stream and build a Dicom dataset
	///////////////////////////////////////////////////////////
    virtual void readStream(std::shared_ptr<streamReader> pSourceStream, std::shared_ptr<dataSet> pDataSet, std::uint32_t maxSizeBufferLoad = 0xffffffff) const override;

	// Write a Dicom dataset as a Jpeg stream
	///////////////////////////////////////////////////////////
    virtual void writeStream(std::shared_ptr<streamWriter> pSourceStream, std::shared_ptr<dataSet> pDataSet) const override;
};


/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraJpegStreamCodec_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_)
