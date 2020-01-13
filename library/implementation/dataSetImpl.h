/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataSet.h
    \brief Declaration of the class dataSet.

*/

#if !defined(imebraDataSet_93F684BF_0024_4bf3_89BA_D98E82A1F44C__INCLUDED_)
#define imebraDataSet_93F684BF_0024_4bf3_89BA_D98E82A1F44C__INCLUDED_

#include "exceptionImpl.h"
#include "streamCodecImpl.h"
#include "dataImpl.h"
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <mutex>


namespace imebra
{

namespace implementation
{

//The following classes are used in this declaration file
class image;
class lut;
class date;
class age;
class VOIDescription;
class streamReader;
class streamWriter;
class overlay;

/// \addtogroup group_dataset Dicom data
/// \brief The Dicom dataset is represented by the
///         class dataSet.
///
/// @{



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief A data set is a collection of groups of tags
///        (see dataGroup).
///
/// The dataSet is usually built from a dicom stream by
///  using the codec codecs::dicomCodec.
///
/// Also the tags with the data type SQ (sequence) contains
///  one or more embedded dataSets that can be retrieved
///  by using data::getSequenceItem().
///
/// If your application creates a new dataset then it can
///  set the default dataSet's charset by calling
///  setCharsetsList(). See \ref imebra_unicode for
///  more information related to Imebra and the Dicom
///   charsets.
///
/// The dataSet and its components (all the dataGroup,
///  and data) share a common lock object:
///  this means that a lock on one of the dataSet's
///  component will lock the entire dataSet and all
///  its components.
///
/// For an introduction to the dataSet, read
///  \ref quick_tour_dataSet.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSet : public std::enable_shared_from_this<dataSet>
{
public:
    // Costructor
    ///////////////////////////////////////////////////////////
    dataSet(const std::shared_ptr<charsetsList_t>& pCharsetsList);

    // Create a sequence item dataset
    ///////////////////////////////////////////////////////////
    dataSet(const std::string& transferSyntax, const std::shared_ptr<charsetsList_t>& pCharsetsList);

    // Create a root dataset (not a sequence item)
    ///////////////////////////////////////////////////////////
    dataSet(const std::string& transferSyntax, const charsetsList_t& charsetsList);

    ///////////////////////////////////////////////////////////
    /// \name Get/set groups/tags
    ///
    ///////////////////////////////////////////////////////////
    //@{

    /// \brief Retrieve a tag object.
    ///
    /// Tag object is represented by the \ref data class.
    ///
    /// If the tag doesn't exist and the parameter bCreate is
    ///  set to false, then the function returns a null
    ///  pointer.
    /// If the tag doesn't exist and the parameter bCreate is
    ///  set to true, then an empty tag will be created and
    ///  inserted into the dataset.
    ///
    /// @param groupId The group to which the tag belongs.
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify which group
    ///                 must be retrieved.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero.
    /// @param tagId   The id of the tag to retrieve.
    /// @param bCreate When bCreate is set to true and the
    ///                 requested tag doesn't exist,
    ///                 then a new one is created and inserted
    ///                 into the dataset.
    ///                When bCreate is set to false and the
    ///                 requested tag doesn't exist, then
    ///                 a null pointer is returned.
    /// @return        A pointer to the retrieved tag.
    ///                If the requested tag doesn't exist then
    ///                 the returned value depend on the value
    ///                 of the bCreate parameter: when bCreate
    ///                 is false then a value of zero is
    ///                 returned, otherwise a pointer to the
    ///                 just created tag is returned.
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<data> getTag(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId) const;

    std::shared_ptr<data> getTagCreate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, tagVR_t tagVR);

    std::shared_ptr<data> getTagCreate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId);

    bool bufferExists(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const;

    //@}


    ///////////////////////////////////////////////////////////
    /// \name Get/set the image
    ///
    ///////////////////////////////////////////////////////////
    //@{

    /// \brief Retrieve an image from the dataset.
    ///
    /// The right codec will be automatically used to
    ///  decode the image embedded into the dataset.
    /// If multiple frames are available, then the
    ///  calling application can decide the frame to
    ///  retrieve.
    ///
    /// The function throw an exception if the requested
    ///  image doesn't exist or the image's tag is
    ///  corrupted.
    ///
    /// The retrieved image should then be processed by the
    ///  transforms::modalityVOILUT transform in order to
    ///  convert the pixels value to a meaningful space.
    /// Infact, the dicom image's pixel values saved by
    ///  other application have a meaningful value only for
    ///  the application that generated them, while the
    ///  modality VOI/LUT transformation will convert those
    ///  values to a more portable unit (e.g.: optical
    ///  density).
    ///
    /// Further transformations are applied by
    ///  the transforms::VOILUT transform, in order to
    ///  adjust the image's contrast for displaying purposes.
    ///
    /// @param frameNumber The frame number to retrieve.
    ///                    The first frame's id is 0
    /// @return            A pointer to the retrieved
    ///                     image
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<image> getImage(std::uint32_t frameNumber) const;

    /// \brief Retrieve an image from the dataset and apply the
    ///        modality transform if it is specified in the
    ///        dataset.
    ///
    /// If the dataSet does not contain any modality transform
    ///  then returns the same image returned by getImage().
    ///
    /// @param frameNumber The frame number to retrieve.
    ///                    The first frame's id is 0
    /// @return            A pointer to the retrieved
    ///                     image to which the modality
    ///                     transform has been applied
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<image> getModalityImage(std::uint32_t frameNumber) const;

    /// \brief Insert an image into the data set.
    ///
    /// The specified transfer syntax will be used to choose
    ///  the right codec for the image.
    ///
    /// @param frameNumber The frame number where the image
    ///                     must be stored.
    ///                    The first frame's id is 0.
    /// @param pImage      A pointer to the image object to
    ///                     be stored into the data set.
    /// @param quality     an enumeration that set the
    ///                     compression quality
    ///
    ///////////////////////////////////////////////////////////
    void setImage(std::uint32_t frameNumber, std::shared_ptr<image> pImage, imageQuality_t quality);

    /// \brief Retrieve an overlay from the dataset.
    ///
    /// If the dataSet does not contain the requested overlay
    ///  then throws MissingDataElementError.
    ///
    /// @param overlayNumber The number of the overlay to
    ///                      retrieve (0...127)
    /// @return              the requested overlay
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<overlay> getOverlay(std::uint32_t overlayNumber) const;

    /// \brief Stores an overlay into the dataset.
    ///
    /// @param overlayNumber The number of the overlay to
    ///                      retrieve (0...127)
    /// @param pOverlay      the overlay to store
    ///
    ///////////////////////////////////////////////////////////
    void setOverlay(std::uint32_t overlayNumber, std::shared_ptr<overlay> pOverlay);

    /// \brief Retrieve the first and the last buffers used
    ///         to store the image.
    ///
    /// This function works only with the new Dicom3 streams,
    ///  not with the old NEMA format.
    ///
    /// This function is used by setImage() and getImage().
    ///
    /// @param frameNumber the frame for which the buffers
    ///                     have to be retrieved
    /// @param pFirstBuffer a pointer to a variable that will
    ///                     contain the id of the first buffer
    ///                     used to store the image
    /// @param pEndBuffer  a pointer to a variable that will
    ///                     contain the id of the first buffer
    ///                     next to the last one used to store
    ///                     the image
    /// @return the total length of the buffers that contain
    ///          the image
    ///
    ///////////////////////////////////////////////////////////
    size_t getFrameBufferIds(std::uint32_t frameNumber, std::uint32_t* pFirstBuffer, std::uint32_t* pEndBuffer) const;

    //@}


    ///////////////////////////////////////////////////////////
    /// \name Get/set a sequence item
    ///
    ///////////////////////////////////////////////////////////
    //@{

    /// \brief Retrieve a data set embedded into a sequence
    ///        tag.
    ///
    /// Sequence tags store several binary data which can be
    ///  individually parsed as a normal dicom file
    ///  (without the preamble of 128 bytes and the DICM
    ///   signature).
    ///
    /// When using sequences an application can store several
    ///  nested dicom structures.
    ///
    /// This function parse a single item of a sequence tag
    ///  and return a data set object (represented by a
    ///  this class) which stores the retrieved
    ///  tags.
    ///
    /// If the requested tag's type is not a sequence or the
    ///  requested item in the sequence is missed, then a null
    ///  pointer will be returned.
    ///
    /// @param groupId The group to which the sequence
    ///                 tag to be parsed belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify which group
    ///                 must be retrieved.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to parse
    /// @param itemId  The id of the tag's item to parse
    ///                (zero based)
    /// @return        A pointer to the retrieved data set.
    ///                If the requested group, tag or buffer
    ///                 (sequence item) doesn't exist, or if
    ///                 the tag's type is not a sequence (SQ),
    ///                 then a null pointer is returned,
    ///                 otherwise a pointer to the retrieved
    ///                 dataset is returned
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<dataSet> getSequenceItem(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t itemId) const;

    std::shared_ptr<dataSet> appendSequenceItem(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId);

    /// \brief Retrieve a LUT.
    ///
    /// LUT are encoded into sequences.
    /// This function retrieve the sequence and build
    ///  a \ref lut object describing the LUT.
    ///
    /// To retrieve the LUTs to use in the VOILUT transform
    /// retrieve the lut from the sequence tag 0028,3010.
    ///
    /// @param groupId The group to which the LUT sequence
    ///                 belongs
    /// @param tagId   The id of the tag to containing the
    ///                 LUT
    /// @param lutId   The id of the lut inside the tag (0
    ///                 based)
    /// @return        A pointer to the retrieved LUT.
    ///                If the requested group, tag or buffer
    ///                 (sequence item) doesn't exist, or if
    ///                 the tag's type is not a sequence (SQ),
    ///                 then a null pointer is returned,
    ///                 otherwise a pointer to the retrieved
    ///                 LUT is returned
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<lut> getLut(std::uint16_t groupId, std::uint16_t tagId, size_t lutId) const;

    ///
    /// \brief Retrieve the list of VOI/LUTS that can be used
    ///        with the VOILUT transform.
    ///
    /// \return A list of VOI/LUT items that can be used with
    ///         the VOILUT transform
    ///
    ///////////////////////////////////////////////////////////
    std::list<std::shared_ptr<const VOIDescription>> getVOIs() const;

    ///
    /// \brief Return a functional group item for the requested
    ///        frame.
    ///
    /// \param frameNumber the frame for which the functional
    ///                    group is required
    /// \return a sequence item with the dataset containing the
    ///         information for the requested frame
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<dataSet> getFunctionalGroupDataSet(size_t frameNumber) const;

    //@}

    ///////////////////////////////////////////////////////////
    /// \name Get/set the tags' values
    ///
    ///////////////////////////////////////////////////////////
    //@{
public:
    /// \brief Read the value of the requested tag and return
    ///         it as a signed long.
    ///
    /// @param groupId The group to which the tag to be read
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to retrieve
    /// @param elementNumber The element's number to retrieve.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be retrieved.
    ///                The first element's number is 0
    /// @return        The tag's content, as a signed long
    ///
    ///////////////////////////////////////////////////////////
    std::int32_t getSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    std::int32_t getSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::int32_t defaultValue) const;

    /// \brief Set a tag's value as a signed long.
    ///
    /// If the specified tag doesn't exist, then a new one
    ///  will be created and inserted into the dataset.
    ///
    /// @param groupId The group to which the tag to be write
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to set
    /// @param elementNumber The element's number to set.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be set.
    ///                The first element's number is 0
    /// @param newValue the value to be written into the tag
    /// @param tagVR if the specified tag doesn't exist
    ///                 then the function will create a new
    ///                 tag with the data type specified in
    ///                 this parameter
    ///
    ///////////////////////////////////////////////////////////
    void setSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::int32_t newValue, tagVR_t tagVR);

    void setSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::int32_t newValue);

    /// \brief Retrieve a tag's value as an unsigned long.
    ///
    /// Read the value of the requested tag and return it as
    ///  an unsigned long.
    ///
    /// @param groupId The group to which the tag to be read
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to retrieve
    /// @param elementNumber The element's number to retrieve.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be retrieved.
    ///                The first element's number is 0
    /// @return        The tag's content, as an unsigned long
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    std::uint32_t getUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::uint32_t defaultValue) const;

    /// \brief Set a tag's value as an unsigned long.
    ///
    /// If the specified tag doesn't exist, then a new one
    ///  will be created and inserted into the dataset.
    ///
    /// @param groupId The group to which the tag to be write
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to set
    /// @param elementNumber The element's number to set.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be set.
    ///                The first element's number is 0
    /// @param newValue the value to be written into the tag
    /// @param tagVR if the specified tag doesn't exist
    ///                 then the function will create a new
    ///                 tag with the data type specified in
    ///                 this parameter
    ///
    ///////////////////////////////////////////////////////////
    void setUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t newValue, tagVR_t tagVR);

    void setUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t newValue);

    /// \brief Retrieve a tag's value as a double.
    ///
    /// Read the value of the requested tag and return it as
    ///  a double.
    ///
    /// @param groupId The group to which the tag to be read
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to retrieve
    /// @param elementNumber The element's number to retrieve.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be retrieved.
    ///                The first element's number is 0
    /// @return        The tag's content, as a double
    ///
    ///////////////////////////////////////////////////////////
    double getDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    double getDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, double defaultValue) const;

    /// \brief Set a tag's value as a double.
    ///
    /// If the specified tag doesn't exist, then a new one
    ///  will be created and inserted into the dataset.
    ///
    /// @param groupId The group to which the tag to be write
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to set
    /// @param elementNumber The element's number to set.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be set.
    ///                The first element's number is 0
    /// @param newValue the value to be written into the tag
    /// @param tagVR if the specified tag doesn't exist
    ///                 then the function will create a new
    ///                 tag with the data type specified in
    ///                 this parameter
    ///
    ///////////////////////////////////////////////////////////
    void setDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, double newValue, tagVR_t tagVR);

    void setDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, double newValue);

    /// \brief Retrieve a tag's value as a string.
    ///        getUnicodeString() is preferred over this
    ///         method.
    ///
    /// Read the value of the requested tag and return it as
    ///  a string.
    ///
    /// @param groupId The group to which the tag to be read
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to retrieve
    /// @param elementNumber The element's number to retrieve.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be retrieved.
    ///                The first element's number is 0
    /// @return        The tag's content, as a string
    ///
    ///////////////////////////////////////////////////////////
    std::string getString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    std::string getString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::string& defaultValue) const;

    /// \brief Retrieve a tag's value as an unicode string.
    ///
    /// Read the value of the requested tag and return it as
    ///  an unicode string.
    ///
    /// @param groupId The group to which the tag to be read
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to retrieve
    /// @param elementNumber The element's number to retrieve.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be retrieved.
    ///                The first element's number is 0
    /// @return        The tag's content, as an unicode string
    ///
    ///////////////////////////////////////////////////////////
    std::wstring getUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    std::wstring getUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::wstring& defaultValue) const;

    std::shared_ptr<patientName> getPatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;
    std::shared_ptr<patientName> getPatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::shared_ptr<patientName>& defaultValue) const;
    std::shared_ptr<unicodePatientName> getUnicodePatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;
    std::shared_ptr<unicodePatientName> getUnicodePatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::shared_ptr<unicodePatientName>& defaultValue) const;

    void setPatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const patientName>& pPatientName);
    void setUnicodePatientName(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const unicodePatientName>& pPatientName);


    /// \brief Set a tag's value as a string.
    ///        setUnicodeString() is preferred over this
    ///         method.
    ///
    /// If the specified tag doesn't exist, then a new one
    ///  will be created and inserted into the dataset.
    ///
    /// @param groupId The group to which the tag to be write
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to set
    /// @param elementNumber The element's number to set.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be set.
    ///                The first element's number is 0
    /// @param newString the value to be written into the tag
    /// @param tagVR if the specified tag doesn't exist
    ///                 then the function will create a new
    ///                 tag with the data type specified in
    ///                 this parameter
    ///
    ///////////////////////////////////////////////////////////
    void setString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::string& newString, tagVR_t tagVR);

    void setString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::string& newString);

    /// \brief Set a tag's value as an unicode string.
    ///
    /// If the specified tag doesn't exist, then a new one
    ///  will be created and inserted into the dataset.
    ///
    /// @param groupId The group to which the tag to be write
    ///                 belongs
    /// @param order   If the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the group belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   The id of the tag to set
    /// @param elementNumber The element's number to set.
    ///                A buffer can store several elements:
    ///                 this parameter specifies which element
    ///                 must be set.
    ///                The first element's number is 0
    /// @param newString the value to be written into the tag
    /// @param tagVR if the specified tag doesn't exist
    ///                 then the function will create a new
    ///                 tag with the data type specified in
    ///                 this parameter
    ///
    ///////////////////////////////////////////////////////////
    void setUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::wstring& newString, tagVR_t tagVR);

    void setUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::wstring& newString);

    void setAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const age>& pAge);

    std::shared_ptr<age> getAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    std::shared_ptr<age> getAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId,
                                  size_t elementNumber, const std::shared_ptr<age>& pDefaultAge) const;

    void setDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const date>& pDate, tagVR_t tagVR);

    void setDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::shared_ptr<const date>& pDate);

    std::shared_ptr<date> getDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const;

    std::shared_ptr<date> getDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::shared_ptr<date> defautlDate) const;

    //@}


    ///////////////////////////////////////////////////////////
    /// \name Data handlers
    ///
    ///////////////////////////////////////////////////////////
    //@{
public:

    /// \brief Return the data type of a tag
    ///
    /// @param groupId    The group to which the tag belongs
    /// @param order      When multiple groups with the same
    ///                    it are present, then use this
    ///                    parameter to specify which group
    ///                    must be used. The first group as
    ///                    an order of 0.
    /// @param tagId      The id of the tag for which the type
    ///                    must be retrieved.
    /// @return           the tag's type.
    ///
    ///////////////////////////////////////////////////////////
    tagVR_t getDataType(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId) const;

    /// \brief Return a data handler for the specified tag's
    ///         buffer.
    ///
    /// The data handler allows the application to read, write
    ///  and resize the tag's buffer.
    ///
    /// A tag can store several buffers, then the application
    ///  must specify the buffer's id it wants to deal with.
    ///
    /// @param groupId the group to which the tag belongs
    /// @param order   if the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the tag belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   the tag's id
    /// @param bufferId the buffer's id (zero based)
    /// @param bWrite  true if the application wants to write
    ///                 into the buffer
    /// @param tagVR a string with the dicom data type
    ///                 to use if the buffer doesn't exist.
    ///                If none is specified, then a default
    ///                 data type will be used
    /// @return a pointer to the data handler.
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandler> getReadingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const;

    std::shared_ptr<handlers::writingDataHandler> getWritingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR);

    std::shared_ptr<handlers::writingDataHandler> getWritingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId);

    /// \brief Return a raw data handler for the specified
    ///         tag's buffer.
    ///
    /// A raw data handler always sees the buffer as a
    ///  collection of bytes, no matter what the tag's data
    ///  type is.
    ///
    /// A tag can store several buffers, then the application
    ///  must specify the buffer's id it wants to deal with.
    ///
    /// @param groupId the group to which the tag belongs
    /// @param order   if the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the tag belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   the tag's id
    /// @param bufferId the buffer's id (zero based)
    /// @param bWrite  true if the application wants to write
    ///                 into the buffer
    /// @param tagVR a string with the dicom data type
    ///                 to use if the buffer doesn't exist.
    ///                If none is specified, then a default
    ///                 data type will be used
    /// @return a pointer to the data handler.
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandlerRaw> getReadingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const;

    std::shared_ptr<handlers::writingDataHandlerRaw> getWritingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR);

    std::shared_ptr<handlers::writingDataHandlerRaw> getWritingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId);


    std::shared_ptr<handlers::readingDataHandlerNumericBase> getReadingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const;

    std::shared_ptr<handlers::writingDataHandlerNumericBase> getWritingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR);

    std::shared_ptr<handlers::writingDataHandlerNumericBase> getWritingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId);


    /// \brief Return a streamReader connected to the specified
    ///         tag's buffer's memory.
    ///
    /// A tag can store several buffers: the application
    ///  must specify the buffer's id it wants to deal with.
    ///
    /// @param groupId the group to which the tag belongs
    /// @param order   if the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the tag belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   the tag's id
    /// @param bufferId the buffer's id (zero based)
    /// @return a pointer to the streamReader
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<streamReader> getStreamReader(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const;

    /// \brief Return a streamWriter connected to the specified
    ///         tag's buffer's memory.
    ///
    /// A tag can store several buffers, then the application
    ///  must specify the buffer's id it wants to deal with.
    ///
    /// @param groupId the group to which the tag belongs
    /// @param order   if the group is recurring in the file
    ///                 (it appears several times), then use
    ///                 this parameter to specify to which
    ///                 group the tag belongs.
    ///                This parameter is used to deal with
    ///                 old DICOM files, since the new one
    ///                 should use the sequence items to
    ///                 achieve the same result.
    ///                It should be set to zero
    /// @param tagId   the tag's id
    /// @param bufferId the buffer's id (zero based)
    /// @param dataType the datatype used to create the
    ///                 buffer if it doesn't exist already
    /// @return a pointer to the streamWriter
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<streamWriter> getStreamWriter(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t dataType);

    std::shared_ptr<streamWriter> getStreamWriter(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId);

    //@}


    ///////////////////////////////////////////////////////////
    /// \name Set/get the item offset.
    ///
    ///////////////////////////////////////////////////////////
    //@{

    /// \brief Called by codecs::dicomCodec when the dataset
    ///         is written into a stream.
    ///        Tells the dataSet the position at which it has
    ///         been written into the stream
    ///
    /// @param offset   the position at which the dataSet has
    ///                  been written into the stream
    ///
    ///////////////////////////////////////////////////////////
    void setItemOffset(std::uint32_t offset);

    /// \brief Retrieve the offset at which the dataSet is
    ///         located in the dicom stream.
    ///
    /// @return the position at which the dataSet is located
    ///          in the dicom stream
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getItemOffset() const;

    //@}

    typedef std::map<std::uint16_t, std::shared_ptr<data> > tTags;
    typedef std::vector<tTags> tGroupsList;
    typedef std::map<std::uint16_t, tGroupsList> tGroups;

    typedef std::set<std::uint16_t> tGroupsIds;

    tGroupsIds getGroups() const;

    std::uint32_t getGroupsNumber(std::uint16_t groupId) const;

    const tTags& getGroupTags(std::uint16_t groupId, size_t groupOrder) const;

    void setCharsetsList(const charsetsList_t& charsets);

private:
    /// \brief Get a frame's offset from the offset table.
    ///
    /// @param frameNumber the number of the frame for which
    ///                     the offset is requested
    /// @return the offset for the specified frame
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getFrameOffset(std::uint32_t frameNumber) const;

    /// \brief Return the first buffer's id available where
    ///         a new frame can be saved.
    ///
    /// @return the id of the first buffer available to store
    ///          a new frame
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getFirstAvailFrameBufferId() const;

    // Position of the sequence item in the stream. Used to
    //  parse DICOMDIR items
    ///////////////////////////////////////////////////////////
    std::uint32_t m_itemOffset;

    /// \brief Get the id of the buffer that starts at the
    ///         specified offset.
    ///
    /// @param offset   one offset retrieved from the frames
    ///                  offset table: see getFrameOffset()
    /// @return         the id of the buffer that starts at
    ///                  the specified offset
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getFrameBufferId(std::uint32_t offset) const;

    tGroups m_groups;

    std::shared_ptr<charsetsList_t> m_pCharsetsList;

    mutable std::recursive_mutex m_mutex;
};



/// @}



} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataSet_93F684BF_0024_4bf3_89BA_D98E82A1F44C__INCLUDED_)
