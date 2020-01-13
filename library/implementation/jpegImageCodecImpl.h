/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegImageCodec.h
    \brief Declaration of the class jpegImageCodec.

*/

#if !defined(imebraJpegImageCodec_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_)
#define imebraJpegImageCodec_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_

#include "imageCodecImpl.h"
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
/// This class is used to decode and encode a Jpeg image.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class jpegImageCodec : public imageCodec, public jpegCodecBase
{
public:
    // Retrieve the image from a dataset
    ///////////////////////////////////////////////////////////
    virtual std::shared_ptr<image> getImage(const std::string& transferSyntax,
                                            const std::string& colorSpace,
                                            std::uint32_t channelsNumber,
                                            std::uint32_t imageWidth,
                                            std::uint32_t imageHeight,
                                            bool bSubSampledX,
                                            bool bSubSampledY,
                                            bool bInterleaved,
                                            bool b2Complement,
                                            std::uint8_t allocatedBits,
                                            std::uint8_t storedBits,
                                            std::uint8_t highBit,
                                            std::shared_ptr<streamReader> pSourceStream) const override;

    // Return the default planar configuration
    ///////////////////////////////////////////////////////////
    virtual bool defaultInterleaved() const override;

    // Insert a jpeg compressed image into a dataset
    ///////////////////////////////////////////////////////////
    virtual void setImage(
        std::shared_ptr<streamWriter> pDestStream,
        std::shared_ptr<const image> pImage,
        const std::string& transferSyntax,
        imageQuality_t imageQuality,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool b2Complement) const override;

    // Return true if the codec can handle the transfer
    ///////////////////////////////////////////////////////////
    virtual bool canHandleTransferSyntax(const std::string& transferSyntax) const override;

    // Returns true if the transfer syntax has to be
    //  encapsulated
    //
    ///////////////////////////////////////////////////////////
    virtual bool encapsulated(const std::string& transferSyntax) const override;

    // Return the suggested allocated bits
    ///////////////////////////////////////////////////////////
    virtual std::uint32_t suggestAllocatedBits(const std::string& transferSyntax, std::uint32_t highBit) const override;

    // FDCT/IDCT
    ///////////////////////////////////////////////////////////
    void FDCT(std::int32_t* pIOMatrix, float* pDescaleFactors) const;
    void IDCT(std::int32_t* pIOMatrix, long long* pScaleFactors) const;

private:
    // Read a lossy block of pixels
    ///////////////////////////////////////////////////////////
    inline void readBlock(jpegStreamReader& stream, jpeg::jpegInformation& information, std::int32_t* pBuffer, const std::shared_ptr<jpeg::jpegChannel>& pChannel) const;

    // Write a lossy block of pixels
    ///////////////////////////////////////////////////////////
    inline void writeBlock(streamWriter* pStream, jpeg::jpegInformation& information, std::int32_t* pBuffer, const std::shared_ptr<jpeg::jpegChannel>& pChannel, bool bCalcHuffman) const;

    std::shared_ptr<image> copyJpegChannelsToImage(jpeg::jpegInformation& information, bool b2complement, const std::string& colorSpace) const;
    void copyImageToJpegChannels(jpeg::jpegInformation& information, std::shared_ptr<const image> sourceImage, bool b2complement, std::uint32_t allocatedBits, bool bSubSampledX, bool bSubSampledY) const;

    void writeScan(streamWriter* pDestinationStream, jpeg::jpegInformation& information, bool bCalcHuffman) const;

};


/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraJpegImageCodec_7F63E846_8824_42c6_A048_DD59C657AED4__INCLUDED_)
