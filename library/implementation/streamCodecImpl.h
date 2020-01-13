/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamCodecImpl.h
    \brief Declaration of the base class used by the stream codecs.

*/

#if !defined(imebraStreamCodec_299706D7_4761_44a1_9F2D_8C38A7BD7AD5__INCLUDED_)
#define imebraStreamCodec_299706D7_4761_44a1_9F2D_8C38A7BD7AD5__INCLUDED_

#include <stdexcept>
#include <memory>
#include <limits>
#include "memoryImpl.h"
#include "../include/imebra/definitions.h"


namespace imebra
{

namespace implementation
{

// Classes used in the declaration
class streamReader;
class streamWriter;
class dataSet;

/// \namespace codecs
/// \brief This namespace is used to define the classes
///         that implement a codec and their helper
///         classes
///
///////////////////////////////////////////////////////////
namespace codecs
{

/// \addtogroup group_codecs Codecs
/// \brief The codecs can generate a dataSet structure
///         or an image from a stream or can write the
///         dataSet structure or an image into a stream.
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This is the base class for all the Imebra
///         stream codecs.
///
/// When the Imebra codecs read the content of a file, they
///  don't return a decompressed image: instead they build
///  a DICOM structure in memory, with the image(s)
///  embedded in it.
///
/// E.G:
///  A call to jpegCodec::read() will build a
///   dataSet object with a jpeg image embedded in
///   it.
///  Your application should call
///   dataSet::getImage()in order to get the
///   decompressed image.
///
/// The same concept is used when your application must
///  generate a file with a compressed image in it.
/// Your application should:
///  - allocate a dicom structure (see dataSet)
///  - insert an image into the structure by calling
///     dataSet::setImage()
///  - generate the final file using the preferred codec.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class streamCodec
{
public:

    virtual ~streamCodec();

    ///////////////////////////////////////////////////////////
    /// \name Streams read/write
    ///
    ///////////////////////////////////////////////////////////
    //@{

    /// \brief Read a stream and build an in-memory dicom
    ///        structure.
    ///
    /// The specified stream is parsed and its content is
    ///  saved into a dataSet object.
    ///
    /// The type of codec to use depends on the stream's
    ///  type (the jpeg codec jpegCodec will parse a
    ///  jpeg stream, the DICOM codec dicomCodec will
    ///  parse a DICOM stream).
    ///
    /// If you want to autodetect the file type, just
    ///  try to parse the stream with several codecs until
    ///  one of them return a valid result.
    /// This task is easier if you use the class
    ///  codecFactory, which automatically scans all the
    ///  Imebra codecs until one can decompress the stream.
    ///
    /// If the codec parses the stream correctly (no error
    ///  occurs) then the resulting dataSet object will
    ///  contain a valid dicom structure.
    ///
    /// @param pSourceStream Stream a pointer to the baseStream
    ///                 object to parse.
    ///                If the codec cannot parse the stream's
    ///                 content, then the stream is rewinded to
    ///                 its initial position.
    /// @param maxSizeBufferLoad if a loaded buffer exceedes
    ///                 the size in the parameter then it is
    ///                 not loaded immediatly but it will be
    ///                 loaded on demand. Some codecs may
    ///                 ignore this parameter.
    ///                Set to -1 to load all the buffers
    ///                 immediatly
    /// @return        a pointer to the loaded dataSet
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<dataSet> read(std::shared_ptr<streamReader> pSourceStream, std::uint32_t maxSizeBufferLoad = std::numeric_limits<std::uint32_t>::max()) const;

    /// \brief Write a dicom structure into a stream.
    ///
    /// The specified dataSet object is transformed into
    ///  the desidered kind of stream (the jpeg codec
    ///  jpegCodec will produce a jpeg stream, the DICOM
    ///  codec dicomCodec will produce a DICOM stream).
    ///
    /// @param pDestStream a pointer to the stream to use for
    ///                     writing.
    /// @param pSourceDataSet a pointer to the Dicom structure
    ///                     to write into the stream
    ///
    ///////////////////////////////////////////////////////////
    void write(std::shared_ptr<streamWriter> pDestStream, std::shared_ptr<dataSet> pSourceDataSet) const;

    //@}

protected:
    virtual void readStream(std::shared_ptr<streamReader> pInputStream, std::shared_ptr<dataSet> pDestDataSet, std::uint32_t maxSizeBufferLoad = std::numeric_limits<std::uint32_t>::max()) const = 0;
    virtual void writeStream(std::shared_ptr<streamWriter> pDestStream, std::shared_ptr<dataSet> pSourceDataSet) const = 0;
};

/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraStreamCodec_299706D7_4761_44a1_9F2D_8C38A7BD7AD5__INCLUDED_)
