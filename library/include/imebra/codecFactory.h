/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file codecFactory.h
    \brief Declaration of the class used to retrieve the codec able to
        handle the requested transfer syntax.

*/

#if !defined(imebraCodecFactory__INCLUDED_)
#define imebraCodecFactory__INCLUDED_

#include <string>
#include <limits>
#include "dataSet.h"
#include "streamReader.h"
#include "streamWriter.h"
#include "definitions.h"

namespace imebra
{

///
/// \brief The CodecFactory class can load or save a DataSet or an Image object
///        using one of the codecs supplied by the Imebra library.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API CodecFactory
{
public:
    /// \brief Parses the content of the input stream and returns a DataSet
    ///        representing it.
    ///
    /// If none of the codecs supplied by Imebra is able to decode the stream's
    /// content then it throws a CodecWrongFormatError exception.
    ///
    /// The read position of the StreamReader is undefined when this method
    /// returns.
    ///
    /// \param reader            a StreamReader connected to the input stream
    /// \param maxSizeBufferLoad the maximum size of the tags that are loaded
    ///                          immediately. Tags larger than maxSizeBufferLoad
    ///                          are left on the input stream and loaded only when
    ///                          a ReadingDataHandler or a WritingDataHandler
    ///                          reference them.
    /// \return a DataSet object representing the input stream's content
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static const DataSet load(StreamReader& reader, size_t maxSizeBufferLoad = std::numeric_limits<size_t>::max());

    /// \brief Parses the content of the input file and returns a DataSet
    ///        representing it.
    ///
    /// If none of the codecs supplied by Imebra is able to decode the file's
    /// content then it throws a CodecWrongFormatError exception.
    ///
    /// \param fileName          the Unicode name of the input file to read
    /// \param maxSizeBufferLoad the maximum size of the tags that are loaded
    ///                          immediately. Tags larger than maxSizeBufferLoad
    ///                          are left on the input stream and loaded only when
    ///                          a ReadingDataHandler or a WritingDataHandler
    ///                          reference them.
    /// \return a DataSet object representing the input file's content
    ///
    ///////////////////////////////////////////////////////////////////////////////
#ifndef SWIG // Use Unicode strings only with SWIG
    static const DataSet load(const std::wstring& fileName, size_t maxSizeBufferLoad = std::numeric_limits<size_t>::max());
#endif

    /// \brief Parses the content of the input file and returns a DataSet
    ///        representing it.
    ///
    /// If none of the codecs supplied by Imebra is able to decode the file's
    /// content then it throws a CodecWrongFormatError exception.
    ///
    /// \param fileName          the Utf8 name of the input file to read
    /// \param maxSizeBufferLoad the maximum size of the tags that are loaded
    ///                          immediately. Tags larger than maxSizeBufferLoad
    ///                          are left on the input stream and loaded only when
    ///                          a ReadingDataHandler or a WritingDataHandler
    ///                          reference them.
    /// \return a DataSet object representing the input file's content
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static const DataSet load(const std::string& fileName, size_t maxSizeBufferLoad = std::numeric_limits<size_t>::max());

    static void saveImage(
            StreamWriter& destStream,
            const Image& sourceImage,
            const std::string& transferSyntax,
            imageQuality_t imageQuality_t,
            std::uint32_t allocatedBits,
            bool bSubSampledX,
            bool bSubSampledY,
            bool bInterleaved,
            bool b2Complement);

    /// \brief Saves the content of a DataSet object to an output stream using the
    ///        requested codec.
    ///
    /// \param dataSet           the DataSet object to save
    /// \param writer            a StreamWriter connected to the output stream
    /// \param codecType         the codec to use to save the DataSet
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static void save(const DataSet& dataSet, StreamWriter& writer, codecType_t codecType);

    /// \brief Saves the content of a DataSet object to an output file using the
    ///        requested codec.
    ///
    /// \param dataSet           the DataSet object to save
    /// \param fileName          the Unicode name of the output file to create
    /// \param codecType         the codec to use to save the DataSet
    ///
    ///////////////////////////////////////////////////////////////////////////////
#ifndef SWIG // Use UTF8 strings only with SWIG
    static void save(const DataSet& dataSet, const std::wstring& fileName, codecType_t codecType);
#endif

    /// \brief Saves the content of a DataSet object to an output file using the
    ///        requested codec.
    ///
    /// \param dataSet           the DataSet object to save
    /// \param fileName          the Utf8 name of the output file to create
    /// \param codecType         the codec to use to save the DataSet
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static void save(const DataSet& dataSet, const std::string& fileName, codecType_t codecType);


    /// \brief Set the maximum image's width & height accepted by Imebra.
    ///
    /// By default both the maximum width and height are set to 4096 pixels.
    ///
    /// \param maximumWidth      the maximum image's width accepted by Imebra
    /// \param maximumHeight     the maximum image's height accepted by Imebra
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static void setMaximumImageSize(const std::uint32_t maximumWidth, const std::uint32_t maximumHeight);

};

}
#endif // !defined(imebraCodecFactory__INCLUDED_)
