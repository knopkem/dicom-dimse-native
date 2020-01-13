/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tag.h
    \brief Declaration of the class Tag.

*/

#if !defined(imebraTagContent__INCLUDED_)
#define imebraTagContent__INCLUDED_

#include <string>
#include <cstdint>
#include <memory>
#include <map>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class data;
}

class DataSet;
class ReadingDataHandler;
class ReadingDataHandlerNumeric;
class StreamReader;
class MutableDataSet;
class WritingDataHandler;
class WritingDataHandlerNumeric;
class StreamWriter;
class FileStreamInput;



///
/// \brief This class represents an immutable DICOM tag.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API Tag
{

    friend class DataSet;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source Tag object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    Tag(const Tag& source);

    Tag& operator=(const Tag& source) = delete;

    virtual ~Tag();

    /// \brief Returns the number of buffers in the tag.
    ///
    /// \return the number of buffers in the tag
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getBuffersCount() const;

    /// \brief Returns true if the specified buffer exists, otherwise it returns
    ///        false.
    ///
    /// \param bufferId the zero-based buffer's id the
    ///                 function has to check for
    /// \return true if the buffer exists, false otherwise
    ///
    ///////////////////////////////////////////////////////////////////////////////
    bool bufferExists(size_t bufferId) const;

    /// \brief Returns the size of a buffer, in bytes.
    ///
    /// If the buffer doesn't exist then throws MissingBufferError.
    ///
    /// \param bufferId the zero-based buffer's id the
    ///                 function has to check for
    /// \return the buffer's size in bytes
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getBufferSize(size_t bufferId) const;

    /// \brief Retrieve a ReadingDataHandler object connected to a specific
    ///        buffer.
    ///
    /// If the specified buffer does not exist then throws or MissingBufferError.
    ///
    /// \param bufferId the buffer to connect to the ReadingDataHandler object.
    ///                 The first buffer has an Id = 0
    /// \return a ReadingDataHandler object connected to the requested buffer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    ReadingDataHandler getReadingDataHandler(size_t bufferId) const;

    /// \brief Retrieve a ReadingDataHandlerNumeric object connected to the
    ///        Tag's numeric buffer.
    ///
    /// If the tag's VR is not a numeric type then throws std::bad_cast.
    ///
    /// If the specified Tag does not contain the specified buffer then
    ///  throws MissingBufferError.
    ///
    /// \param bufferId the buffer to connect to the ReadingDataHandler object.
    ///                 The first buffer has an Id = 0
    /// \return a ReadingDataHandlerNumeric object connected to the Tag's buffer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    ReadingDataHandlerNumeric getReadingDataHandlerNumeric(size_t bufferId) const;

    /// \brief Retrieve a ReadingDataHandlerNumeric object connected to the
    ///        Tag's raw data buffer (8 bit unsigned integers).
    ///
    /// If the tag's VR is not a numeric type then throws std::bad_cast.
    ///
    /// If the specified Tag does not contain the specified buffer then
    ///  throws MissingBufferError.
    ///
    /// \param bufferId the buffer to connect to the ReadingDataHandler object.
    ///                 The first buffer has an Id = 0
    /// \return a ReadingDataHandlerNumeric object connected to the Tag's buffer
    ///         (raw content represented by 8 bit unsigned integers)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    ReadingDataHandlerNumeric getReadingDataHandlerRaw(size_t bufferId) const;

    /// \brief Get a StreamReader connected to a buffer's data.
    ///
    /// \param bufferId   the id of the buffer for which the StreamReader is
    ///                    required. This parameter is usually 0
    /// \return           the StreamReader connected to the buffer's data.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    StreamReader getStreamReader(size_t bufferId) const;

    /// \brief Retrieve an embedded DataSet.
    ///
    /// Sequence tags (VR=SQ) store embedded dicom structures.
    ///
    /// @param dataSetId  the ID of the sequence item to retrieve. Several sequence
    ///                   items can be embedded in one tag.
    /// @return           the sequence DataSet
    ///
    ///////////////////////////////////////////////////////////////////////////////
    const DataSet getSequenceItem(size_t dataSetId) const;

    /// \brief Check for the existance of a sequence item.
    ///
    /// \param dataSetId the ID of the sequence item to check for
    /// \return true if the sequence item exists, false otherwise
    ///
    ///////////////////////////////////////////////////////////////////////////////
    bool sequenceItemExists(size_t dataSetId) const;

    /// \brief Get the tag's data type.
    ///
    /// @return the tag's data type
    ///
    ///////////////////////////////////////////////////////////////////////////////
    tagVR_t getDataType() const;

#ifndef SWIG
protected:
    explicit Tag(const std::shared_ptr<imebra::implementation::data>& pData);

private:
    friend const std::shared_ptr<imebra::implementation::data>& getTagImplementation(const Tag& tag);
    std::shared_ptr<imebra::implementation::data> m_pData;
#endif
};

///
/// \brief This class represents a mutable DICOM tag.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API MutableTag: public Tag
{

    friend class MutableDataSet;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source MutableTag object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    MutableTag(const MutableTag& source);

    virtual ~MutableTag();

    MutableTag& operator=(const MutableTag& source) = delete;

    /// \brief Retrieve a WritingDataHandler object connected to a specific
    ///        tag's buffer.
    ///
    /// If the specified Tag does not exist then it creates a new tag with the VR
    ///  specified in the tagVR parameter
    ///
    /// The returned WritingDataHandler is connected to a new buffer which is
    /// updated and stored in the tag when WritingDataHandler is destroyed.
    ///
    /// \param bufferId the position where the new buffer has to be stored into the
    ///                 tag. The first buffer position is 0
    /// \return a WritingDataHandler object connected to a new Tag's buffer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    WritingDataHandler getWritingDataHandler(size_t bufferId);

    /// \brief Retrieve a WritingDataHandlerNumeric object connected to the
    ///        Tag's buffer.
    ///
    /// If the tag's VR is not a numeric type then throws std::bad_cast.
    ///
    /// The returned WritingDataHandlerNumeric is connected to a new buffer which
    /// is updated and stored into the tag when WritingDataHandlerNumeric is
    /// destroyed.
    ///
    /// \param bufferId the position where the new buffer has to be stored in the
    ///                 tag. The first buffer position is 0
    /// \return a WritingDataHandlerNumeric object connected to a new Tag's buffer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    WritingDataHandlerNumeric getWritingDataHandlerNumeric(size_t bufferId);

    /// \brief Retrieve a WritingDataHandlerNumeric object connected to the
    ///        Tag's raw data buffer (8 bit unsigned integers).
    ///
    /// If the tag's VR is not a numeric type then throws std::bad_cast.
    ///
    /// The returned WritingDataHandlerNumeric is connected to a new buffer which
    /// is updated and stored into the tag when WritingDataHandlerNumeric is
    /// destroyed.
    ///
    /// \param bufferId the position where the new buffer has to be stored in the
    ///                 tag. The first buffer position is 0
    /// \return a WritingDataHandlerNumeric object connected to a new Tag's buffer
    ///         (the buffer contains raw data of 8 bit integers)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    WritingDataHandlerNumeric getWritingDataHandlerRaw(size_t bufferId);

    /// \brief Get a StreamWriter connected to a buffer's data.
    ///
    /// @param bufferId   the id of the buffer for which the StreamWriter is
    ///                    required. This parameter is usually 0
    /// @return           the StreamWriter connected to the buffer's data.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    StreamWriter getStreamWriter(size_t bufferId);

    /// \brief Append a sequence item into the Tag.
    ///
    /// Several sequence items can be nested one inside each other.
    /// When a sequence item is embedded into a Tag, then the tag will have a
    /// sequence VR (VR = SQ).
    ///
    /// \return the MutableDataSet representing the added sequence item
    ///
    ///////////////////////////////////////////////////////////////////////////////
    MutableDataSet appendSequenceItem();

    /// \brief Set the tag's content to the content of a file.
    ///
    /// The tag will just keep a reference to the file content.
    ///
    /// This is useful when embedding large objects into a dataset (e.g. a MPEG
    /// file acquired by the device).
    ///
    /// \param bufferId    the id of the buffer to which the content is added
    /// \param streamInput the file into which the tag's content is stored
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setStream(size_t bufferId, FileStreamInput& streamInput);

protected:
    explicit MutableTag(const std::shared_ptr<imebra::implementation::data>& pData);
};

}

#endif // !defined(imebraTagContent__INCLUDED_)
