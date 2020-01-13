/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/dataSet.h"
#include "../implementation/dataSetImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/charsetConversionBaseImpl.h"
#include <typeinfo>
#include <memory>

namespace imebra
{

TagId::TagId(): m_groupId(0), m_groupOrder(0), m_tagId(0)
{
}

TagId::TagId(std::uint16_t groupId, std::uint16_t tagId):
    m_groupId(groupId), m_groupOrder(0), m_tagId(tagId)
{
}

TagId::TagId(std::uint16_t groupId, std::uint32_t groupOrder, std::uint16_t tagId):
    m_groupId(groupId), m_groupOrder(groupOrder), m_tagId(tagId)
{
}

TagId::TagId(tagId_t id):
    m_groupId((std::uint16_t)((std::uint32_t)id / (std::uint32_t)65536)),
    m_groupOrder(0),
    m_tagId((std::uint16_t)((std::uint32_t)id & (std::uint32_t)0x0000ffff))
{
}

TagId::TagId(tagId_t id, std::uint32_t groupOrder):
    m_groupId((std::uint16_t)((std::uint32_t)id / (std::uint32_t)65536)),
    m_groupOrder(groupOrder),
    m_tagId((std::uint16_t)((std::uint32_t)id & (std::uint32_t)0x0000ffff))
{
}

TagId::TagId(const TagId& source):
    m_groupId(source.m_groupId), m_groupOrder(source.m_groupOrder), m_tagId(source.m_tagId)
{
}

TagId& TagId::operator=(const TagId& source)
{
    m_groupId = source.m_groupId;
    m_groupOrder = source.m_groupOrder;
    m_tagId = source.m_tagId;

    return *this;
}


TagId::~TagId()
{
}

std::uint16_t TagId::getGroupId() const
{
    return m_groupId;
}

std::uint32_t TagId::getGroupOrder() const
{
    return m_groupOrder;
}

std::uint16_t TagId::getTagId() const
{
    return m_tagId;
}

}
