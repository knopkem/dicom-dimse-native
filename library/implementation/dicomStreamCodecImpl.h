/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomStreamCodec.h
    \brief Declaration of the class dicomStreamCodec.

*/

#if !defined(imebraDicomStreamCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
#define imebraDicomStreamCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_

#include "streamCodecImpl.h"
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
/// This class is used to decode and encode a DICOM stream.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dicomStreamCodec : public streamCodec
{
public:
    /// \brief Parse the dicom stream and fill the data set
    ///        with the read tags.
    ///
    /// This function doesn't parse the 128 bytes preamble and
    ///  the DICM signature, which has to be parsed by the
    ///  calling function.
    ///
    /// @param pStream    The stream do decode
    /// @param pDataSet   A pointer to the data set to fill
    ///                    with the decoded tags
    /// @param bExplicitDataType true if the stream is encoded
    ///                    with explicit data type, false
    ///                    otherwise.
    ///                   Even when this parameter is set,
    ///                    the function will automatically
    ///                    switch this parameter if a mismatch
    ///                    is detected during the decoding
    ///                    procedure
    /// @param endianType The stream's endian type.
    ///                   Even when this parameter is set,
    ///                    the function will automatically
    ///                    switch this parameter if a mismatch
    ///                    is detected during the decoding
    ///                    procedure
    /// @param subItemLength the number of bytes to parse.
    ///                   If this parameter is set to
    ///                    0xffffffff then the function will
    ///                    stop parsing at the end of the
    ///                    sequence or at the end of the file
    /// @param maxSizeBufferLoad if a loaded buffer is exceedes
    ///                    the size in the parameter then it is
    ///                    not loaded immediatly but it will be
    ///                    loaded on demand. Some codecs may
    ///                    ignore this parameter.
    ///                   Set to -1 to load all the buffers
    ///                    immediatly
    /// @param pReadSubItemLength a pointer to a std::uint32_t
    ///                    that the function will fill with
    ///                    the number of bytes read
    /// @param depth      the current dataSet depth:
    ///                    - 0 = root dataset
    ///                    - >=1 = dataset embedded into
    ///                      another dataset. This value is
    ///                      used to prevent a stack overflow
    ///
    ///////////////////////////////////////////////////////////
    static void parseStream(
        std::shared_ptr<streamReader> pStream,
        std::shared_ptr<dataSet> pDataSet,
        bool bExplicitDataType,
        streamController::tByteOrdering endianType,
        std::uint32_t maxSizeBufferLoad = 0xffffffff,
        std::uint32_t subItemLength = 0xffffffff,
        std::uint32_t* pReadSubItemLength = 0,
        std::uint32_t depth = 0);

    /// \brief Indicates the type of DICOM stream to build
    ///
    ///////////////////////////////////////////////////////////
    enum class streamType_t
    {
        normal,      ///< DICOM stream for embedded datasets
        mediaStorage ///< DICOM stream form media storage
    };

    /// \brief Write the dataSet to the specified stream
    ///         in Dicom format, without the file header and
    ///         signature.
    ///
    /// @param pStream   the destination stream
    /// @param pDataSet  the dataSet to be written
    /// @param bExplicitDataType true if the function must
    ///                   write the data type, false if
    ///                   the data type is implicit
    /// @param endianType the endian type to be generated
    /// @param streamType the type of DICOM stream to build
    ///
    ///////////////////////////////////////////////////////////
    static void buildStream(std::shared_ptr<streamWriter> pStream, std::shared_ptr<const dataSet> pDataSet, bool bExplicitDataType, streamController::tByteOrdering endianType, streamType_t streamType);

protected:
    // Write a dicom stream
    ///////////////////////////////////////////////////////////
    virtual void writeStream(std::shared_ptr<streamWriter> pStream, std::shared_ptr<dataSet> pDataSet) const;

    // Load a dicom stream
    ///////////////////////////////////////////////////////////
    virtual void readStream(std::shared_ptr<streamReader> pStream, std::shared_ptr<dataSet> pDataSet, std::uint32_t maxSizeBufferLoad = 0xffffffff) const;

protected:
    // Read a single tag
    ///////////////////////////////////////////////////////////
    static std::uint32_t readTag(std::shared_ptr<streamReader> pStream, std::shared_ptr<dataSet> pDataSet, std::uint32_t tagLengthDWord, std::uint16_t tagId, std::uint16_t order, std::uint16_t tagSubId, tagVR_t tagType, streamController::tByteOrdering endianType, size_t wordSize, std::uint32_t bufferId, std::uint32_t maxSizeBufferLoad = 0xffffffff);

    // Calculate the tag's length
    ///////////////////////////////////////////////////////////
    static std::uint32_t getTagLength(const std::shared_ptr<data>& pData, bool bExplicitDataType, std::uint32_t* pHeaderLength, bool *pbSequence);

    // Calculate the group's length
    ///////////////////////////////////////////////////////////
    static std::uint32_t getGroupLength(const dataSet::tTags& tags, bool bExplicitDataType);

    // Calculate the dataset's length
    ///////////////////////////////////////////////////////////
    static std::uint32_t getDataSetLength(std::shared_ptr<dataSet>, bool bExplicitDataType);

    // Write a single group
    ///////////////////////////////////////////////////////////
    static void writeGroup(std::shared_ptr<streamWriter> pDestStream, const dataSet::tTags& tags, std::uint16_t groupId, bool bExplicitDataType, streamController::tByteOrdering endianType);

    // Write a single tag
    ///////////////////////////////////////////////////////////
    static void writeTag(std::shared_ptr<streamWriter> pDestStream, std::shared_ptr<data> pData, std::uint16_t tagId, bool bExplicitDataType, streamController::tByteOrdering endianType);
};


/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDicomStreamCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
