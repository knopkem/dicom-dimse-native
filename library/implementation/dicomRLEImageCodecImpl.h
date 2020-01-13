/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomRLEImageCodec.h
    \brief Declaration of the class dicomRLEImageCodec.

*/

#if !defined(imebraDicomRLEImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
#define imebraDicomRLEImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_

#include "imageCodecImpl.h"
#include "dataImpl.h"
#include "dataSetImpl.h"
#include "streamControllerImpl.h"

/// \def IMEBRA_DATASET_MAX_DEPTH
/// \brief Max number of datasets embedded into each
///        others.
///
/// This value is used to prevent a stack when reading a
///  malformed dataset that has too many datasets embedded
///   into each others.
///
///////////////////////////////////////////////////////////
#define IMEBRA_DATASET_MAX_DEPTH 16


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
/// \brief The Dicom codec.
///
/// This class is used to decode and encode a DICOM image.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dicomRLEImageCodec : public imageCodec
{
public:
    // Get an image from a dicom structure
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

    std::shared_ptr<image> getRLEImage(const dataSet& dataset, std::shared_ptr<streamReader> pStream, tagVR_t dataType) const;

    // Write an image into a dicom structure
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

    // Returns true if the codec can handle the transfer
    //  syntax
    ///////////////////////////////////////////////////////////
    virtual bool canHandleTransferSyntax(const std::string& transferSyntax) const override;

    // Returns true if the transfer syntax has to be
    //  encapsulated
    //
    ///////////////////////////////////////////////////////////
    virtual bool encapsulated(const std::string& transferSyntax) const override;

    // Returns the suggested allocated bits
    ///////////////////////////////////////////////////////////
    virtual std::uint32_t suggestAllocatedBits(const std::string& transferSyntax, std::uint32_t highBit) const override;

protected:
    // Write an RLE compressed image
    ///////////////////////////////////////////////////////////
    static void writeRLECompressed(
            const std::vector<std::shared_ptr<channel>>& channels,
            std::uint32_t imageWidth,
            std::uint32_t imageHeight,
            std::uint32_t channelsNumber,
            streamWriter* pDestStream,
            std::uint8_t allocatedBits,
            std::uint32_t mask
            );

    // Write RLE sequence of different bytes
    ///////////////////////////////////////////////////////////
    static size_t writeRLEDifferentBytes(std::vector<std::uint8_t>* pDifferentBytes, streamWriter* pDestStream, bool bWrite);

    // Read an RLE compressed image
    ///////////////////////////////////////////////////////////
    static void readRLECompressed(
            const std::vector<std::shared_ptr<channel>>& information,
            std::uint32_t imageWidth,
            std::uint32_t imageHeight,
            std::uint32_t channelsNumber,
            streamReader* pSourceStream,
            std::uint8_t allocatedBits,
            std::uint32_t mask);


    // Flush the unwritten bytes of an uncompressed image
    ///////////////////////////////////////////////////////////
    static void flushUnwrittenPixels(const std::vector<std::shared_ptr<channel>>& information, streamWriter* pDestStream, std::uint8_t* pBitPointer, std::uint32_t wordSizeBytes);

};


/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDicomRLEImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
