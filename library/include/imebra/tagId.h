/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file tagId.h
    \brief Declaration of the class TagId.

*/

#if !defined(imebraTagId__INCLUDED_)
#define imebraTagId__INCLUDED_

#include <cstdint>
#include <vector>
#include "definitions.h"

namespace imebra
{

///
/// \brief Represents a Dicom tag's identification.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API TagId
{
public:

    /// \brief Default constructor.
    ///
    /// Initializes the group id and the tag id to 0.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    TagId();

    /// \brief Constructor.
    ///
    /// \param groupId    the group id
    /// \param tagId      the tag id
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit TagId(std::uint16_t groupId, std::uint16_t tagId);

    /// \brief Constructor.
    ///
    /// \param groupId    the group id
    /// \param groupOrder old DICOM files may have several groups with the same id.
    ///                   This parameter specifies which of the groups with the
    ///                   same id must be taken into consideration
    /// \param tagId      the tag id
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit TagId(std::uint16_t groupId, std::uint32_t groupOrder, std::uint16_t tagId);

    TagId(const TagId& source);

    TagId& operator=(const TagId& source);

#ifndef SWIGJAVA
    /// \brief Constructor.
    ///
    /// \param id an enumeration representing a tag group and id
    ///
    /// \warning Very large enumeration classes cause an error in Java, therefore
    ///          the tagId_t enumeration is not supported in Java.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit TagId(tagId_t id);

    /// \brief Constructor.
    ///
    /// \param id         an enumeration representing a tag group and id
    /// \param groupOrder old DICOM files may have several groups with the same id.
    ///                   This parameter specifies which of the groups with the
    ///                   same id must be taken into consideration
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit TagId(tagId_t id, std::uint32_t groupOrder);
#endif

    virtual ~TagId();

    /// \brief Retrieve the group id.
    ///
    /// \return the group id
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint16_t getGroupId() const;

    /// \brief Return the group order. Old DICOM files may have several groups
    ///        with the same id. This value specifies which of the groups with the
    ///        same id has been taken into consideration.
    ///
    /// \return the group order
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint32_t getGroupOrder() const;

    /// \brief Retrieve the tag id.
    ///
    /// \return the tag id
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint16_t getTagId() const;

private:
    std::uint16_t m_groupId;
    std::uint32_t m_groupOrder;
    std::uint16_t m_tagId;
};

typedef std::vector<TagId> tagsIds_t;

}

#endif // !defined(imebraTagId__INCLUDED_)
