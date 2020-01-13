/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include <limits>
#include <algorithm>
#include "acseImpl.h"
#include "streamWriterImpl.h"
#include "memoryStreamImpl.h"
#include "memoryImpl.h"
#include "configurationImpl.h"
#include "dicomStreamCodecImpl.h"
#include "dataHandlerStringUIImpl.h"
#include <memory.h>
#include <cassert>

namespace imebra
{

namespace implementation
{

static const std::string m_applicationContext("1.2.840.10008.3.1.1.1");


acseItem::~acseItem()
{
}


///////////////////////////////////////////////////////////
//
// Decode an item
//
///////////////////////////////////////////////////////////
std::shared_ptr<acseItem> acseItem::decodeItem(std::shared_ptr<streamReader> pReader)
{
    IMEBRA_FUNCTION_START();

    // Get the item's type and create the proper class
    ///////////////////////////////////////////////////////////
    itemType_t itemType;
    pReader->read((std::uint8_t*)&itemType, sizeof(itemType));

    try
    {
        std::shared_ptr<acseItem> pItem;
        switch(itemType)
        {
        case itemType_t::applicationContext:
            pItem = std::make_shared<acseItemApplicationContext>();
            break;
        case itemType_t::presentationContextRQ:
            pItem = std::make_shared<acseItemPresentationContextRQ>();
            break;
        case itemType_t::presentationContextAC:
            pItem = std::make_shared<acseItemPresentationContextAC>();
            break;
        case itemType_t::abstractSyntax:
            pItem = std::make_shared<acseItemAbstractSyntax>();
            break;
        case itemType_t::transferSyntax:
            pItem = std::make_shared<acseItemTransferSyntax>();
            break;
        case itemType_t::userInformation:
            pItem = std::make_shared<acseItemUserInformation>();
            break;
        case itemType_t::maximumLength:
            pItem = std::make_shared<acseItemMaxLength>();
            break;
        case itemType_t::implementationClassUID:
            pItem = std::make_shared<acseItemImplementationClassUID>();
            break;
        case itemType_t::implementationVersionName:
            pItem = std::make_shared<acseItemImplementationVersionName>();
            break;
        case itemType_t::asynchronousOperationsWindow:
            pItem = std::make_shared<acseItemAsynchronousOperationsWindow>();
            break;
        case itemType_t::SCPSCURoleSelection:
            pItem = std::make_shared<acseItemSCPSCURoleSelection>();
            break;
        default:
            IMEBRA_THROW(AcseCorruptedMessageError, "Found unknown ACSE item");
        }

        // Read reserved byte
        ///////////////////////////////////////////////////////////
        std::uint8_t reserved;
        pReader->read(&reserved, sizeof(reserved));

        // Read item's length
        ///////////////////////////////////////////////////////////
        std::uint16_t itemLength;
        pReader->read((std::uint8_t*)&itemLength, sizeof(itemLength));
        pReader->adjustEndian((std::uint8_t*)&itemLength, 2, streamController::highByteEndian);

        // Get reader for virtual stream with size = item's length
        ///////////////////////////////////////////////////////////
        std::shared_ptr<streamReader> pItemStream(pReader->getReader(itemLength));

        // Decode the item and return it
        ///////////////////////////////////////////////////////////
        pItem->decodeItemPayload(pItemStream);
        return pItem;
    }
    catch(const StreamEOFError&)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Corrupted ACSE item");
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Encode an item
//
///////////////////////////////////////////////////////////
void acseItem::encodeItem(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    // Write the item's type
    ///////////////////////////////////////////////////////////
    itemType_t itemType = getItemType();
    pWriter->write((std::uint8_t*)&itemType, 1);

    // Write the reserved byte
    ///////////////////////////////////////////////////////////
    static const std::uint8_t zero(0);
    pWriter->write(&zero, sizeof(zero));

    // Encode the item in a memory buffer
    ///////////////////////////////////////////////////////////
    std::shared_ptr<memory> pMemory(std::make_shared<memory>());
    {
        std::shared_ptr<memoryStreamOutput> pMemoryStream(std::make_shared<memoryStreamOutput>(pMemory));
        std::shared_ptr<streamWriter> pMemoryWriter(std::make_shared<streamWriter>(pMemoryStream));
        encodeItemPayload(pMemoryWriter);
    }

    // Check the memory size
    ///////////////////////////////////////////////////////////
    if(pMemory->size() > std::numeric_limits<std::uint16_t>::max())
    {
        IMEBRA_THROW(std::logic_error, "A PDU item cannot be longer than 65535 bytes");
    }

    // Write the item's length
    ///////////////////////////////////////////////////////////
    std::uint16_t itemLength((std::uint16_t)pMemory->size());
    pWriter->adjustEndian((std::uint8_t*)&itemLength, 2, streamController::highByteEndian);
    pWriter->write((std::uint8_t*)&itemLength, sizeof(itemLength));

    // Write the item's payload
    ///////////////////////////////////////////////////////////
    pWriter->write(pMemory->data(), pMemory->size());

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// acseItemName (stores a string) default constructor
//
///////////////////////////////////////////////////////////
acseItemName::acseItemName()
{
}


///////////////////////////////////////////////////////////
//
// acseItemName (stores a string)
//
///////////////////////////////////////////////////////////
acseItemName::acseItemName(const std::string &name):
    m_name(name)
{
}


///////////////////////////////////////////////////////////
//
// Returns the string stored in the item
//
///////////////////////////////////////////////////////////
const std::string& acseItemName::getName() const
{
    return m_name;
}


///////////////////////////////////////////////////////////
//
// Encode the string
//
///////////////////////////////////////////////////////////
void acseItemName::encodeItemPayload(std::shared_ptr<streamWriter> writer) const
{
    IMEBRA_FUNCTION_START();

    std::string name(handlers::normalizeUid(m_name));
    writer->write((const std::uint8_t*)name.c_str(), name.size());

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Decode the string
//
///////////////////////////////////////////////////////////
void acseItemName::decodeItemPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    std::uint8_t buffer[128];

    std::string name;
    while(!reader->endReached())
    {
        size_t readSize(reader->readSome(buffer, sizeof(buffer)));
        name.append(std::string((char*)buffer, readSize));
    }

    m_name = handlers::normalizeUid(name);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Application Context Item
//
///////////////////////////////////////////////////////////
acseItem::itemType_t acseItemApplicationContext::getItemType() const
{
    return itemType_t::applicationContext;
}


///////////////////////////////////////////////////////////
//
// Presentation Context Request Item
//
///////////////////////////////////////////////////////////

acseItemPresentationContextRQ::acseItemPresentationContextRQ()
{
    m_bCheckResultCorrectness = false;
}

acseItemPresentationContextRQ::acseItemPresentationContextRQ(std::uint8_t id, const std::string& abstractSyntax, const transferSyntaxes_t& transferSyntaxes):
    acseItemPresentationContextBase(id, abstractSyntax, transferSyntaxes)
{
}


acseItem::itemType_t acseItemPresentationContextRQ::getItemType() const
{
    return itemType_t::presentationContextRQ;
}


///////////////////////////////////////////////////////////
//
// Presentation Context Acceptance Item
//
///////////////////////////////////////////////////////////

acseItemPresentationContextAC::acseItemPresentationContextAC()
{

}

acseItemPresentationContextAC::acseItemPresentationContextAC(std::uint8_t id, result_t result, const std::string& transferSyntax):
    acseItemPresentationContextBase(id, result, transferSyntax)
{
}

acseItem::itemType_t acseItemPresentationContextAC::getItemType() const
{
    return itemType_t::presentationContextAC;
}


///////////////////////////////////////////////////////////
//
// Presentation Context Item (base for request and
//  acceptance)
//
///////////////////////////////////////////////////////////

acseItemPresentationContextBase::acseItemPresentationContextBase():
    m_id(0), m_result(result_t::acceptance), m_bCheckResultCorrectness(true)
{
}

acseItemPresentationContextBase::acseItemPresentationContextBase(std::uint8_t id, const std::string& abstractSyntax, const transferSyntaxes_t& transferSyntaxes):
    m_id(id), m_result(result_t::acceptance), m_bCheckResultCorrectness(true)
{
    IMEBRA_FUNCTION_START();

    m_pAbstractSyntax = std::make_shared<acseItemAbstractSyntax>(abstractSyntax);
    for(transferSyntaxes_t::const_iterator scanTransferSyntaxes(transferSyntaxes.begin()), endTransferSyntaxes(transferSyntaxes.end());
        scanTransferSyntaxes != endTransferSyntaxes;
        ++scanTransferSyntaxes)
    {
        m_transferSyntaxes.push_back(std::make_shared<acseItemTransferSyntax>(*scanTransferSyntaxes));
    }

    IMEBRA_FUNCTION_END();
}

acseItemPresentationContextBase::acseItemPresentationContextBase(std::uint8_t id, result_t result, const std::string& transferSyntax):
    m_id(id), m_result(result), m_bCheckResultCorrectness(true)
{
    IMEBRA_FUNCTION_START();

    if(!transferSyntax.empty())
    {
        m_transferSyntaxes.push_back(std::make_shared<acseItemTransferSyntax>(transferSyntax));
    }

    IMEBRA_FUNCTION_END();
}


const std::string& acseItemPresentationContextBase::getAbstractSyntax() const
{
    if(m_pAbstractSyntax.get() == 0)
    {
        static const std::string empty;
        return empty;
    }

    return m_pAbstractSyntax->getName();
}

std::uint8_t acseItemPresentationContextBase::getId() const
{
    return m_id;
}

acseItemPresentationContextBase::result_t acseItemPresentationContextBase::getResult() const
{
    return m_result;
}

acseItemPresentationContextBase::transferSyntaxes_t acseItemPresentationContextBase::getTransferSyntaxes() const
{
    IMEBRA_FUNCTION_START();

    transferSyntaxes_t transferSyntaxes;

    for(transferSyntaxesList_t::const_iterator scanTransferSyntaxes(m_transferSyntaxes.begin()), endTransferSyntaxes(m_transferSyntaxes.end());
        scanTransferSyntaxes != endTransferSyntaxes;
        ++scanTransferSyntaxes)
    {
        transferSyntaxes.push_back((*scanTransferSyntaxes)->getName());
    }

    return transferSyntaxes;

    IMEBRA_FUNCTION_END();
}


void acseItemPresentationContextBase::encodeItemPayload(std::shared_ptr<streamWriter> writer) const
{
    IMEBRA_FUNCTION_START();

    // Write the presentation context
    ///////////////////////////////////////////////////////////
    IMEBRA_LOG_INFO("  -- Presentation context ID = " << +m_id);
    writer->write(&m_id, sizeof(m_id));

    // Write the reserved byte
    ///////////////////////////////////////////////////////////
    static const std::uint8_t zero(0);
    writer->write(&zero, 1);

    // Write the result code and the reserved byte
    ///////////////////////////////////////////////////////////
    IMEBRA_LOG_INFO("     Presentation context acceptance = " << (int)m_result << " (0 = acceptance/proposal, 1 = user rejection, 2 = no reason, 3 = abstract syntax not supported, 4 = transfer syntax not supported)");
    writer->write((std::uint8_t*)&m_result, sizeof(m_result));
    writer->write(&zero, 1);

    if(m_pAbstractSyntax != 0)
    {
        IMEBRA_LOG_INFO("     Abstract syntax = " << m_pAbstractSyntax->getName());
        m_pAbstractSyntax->encodeItem(writer);
    }

    for(transferSyntaxesList_t::const_iterator scanSyntaxes(m_transferSyntaxes.begin()), endSyntaxes(m_transferSyntaxes.end());
        scanSyntaxes != endSyntaxes;
        ++scanSyntaxes)
    {
        IMEBRA_LOG_INFO("     Transfer syntax = " << (*scanSyntaxes)->getName());
        (*scanSyntaxes)->encodeItem(writer);
    }

    IMEBRA_FUNCTION_END();
}


void acseItemPresentationContextBase::decodeItemPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    reader->read(&m_id, sizeof(m_id));
    if((m_id & 0x01) == 0)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Presentation context ID is not an odd number");
    }

    std::uint8_t zero;
    reader->read(&zero, 1);
    reader->read((std::uint8_t*)&m_result, sizeof(m_result));
    if(
            m_result != result_t::acceptance &&
            m_result != result_t::abstractSyntaxNotSupported &&
            m_result != result_t::transferSyntaxesNotSupported&&
            m_result != result_t::userRejection &&
            m_result != result_t::noReason)
    {
        if(m_bCheckResultCorrectness)
        {
            IMEBRA_THROW(AcseCorruptedMessageError, "Presentation context rejection code not recognized");
        }
        m_result = result_t::acceptance; // Necessary because DCMTK does not set this to 0 in requests
    }
    reader->read(&zero, 1);

    try
    {
        for(;;)
        {
            std::shared_ptr<acseItem> item(acseItem::decodeItem(reader));
            switch(item->getItemType())
            {
            case acseItem::itemType_t::abstractSyntax:
                m_pAbstractSyntax = std::dynamic_pointer_cast<acseItemAbstractSyntax>(item);
                break;
            case acseItem::itemType_t::transferSyntax:
                if(m_result == result_t::acceptance)
                {
                    m_transferSyntaxes.push_back(std::dynamic_pointer_cast<acseItemTransferSyntax>(item));
                }
                break;
            default:
                // ignore
                break;
            }
        }
    }
    catch(const StreamEOFError&)
    {
        // Ignore (means no more items)
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Abstract Syntax Item
//
///////////////////////////////////////////////////////////
acseItem::itemType_t acseItemAbstractSyntax::getItemType() const
{
    return itemType_t::abstractSyntax;
}


///////////////////////////////////////////////////////////
//
// Transfer Syntax Item
//
///////////////////////////////////////////////////////////
acseItem::itemType_t acseItemTransferSyntax::getItemType() const
{
    return itemType_t::transferSyntax;
}


///////////////////////////////////////////////////////////
//
// User Information Item
//
///////////////////////////////////////////////////////////

acseItemUserInformation::acseItemUserInformation()
{
}


acseItemUserInformation::acseItemUserInformation(
        std::uint32_t maximumLength,
        const std::string& implementationClassUID,
        const std::string& implementationVersionName,
        std::uint16_t maxOperationsInvoked,
        std::uint16_t maxOperationsPerformed,
        const scpScuSelectionList_t& scpScuRoles):
    m_scuScpRoleSelections(scpScuRoles)
{
    IMEBRA_FUNCTION_START();

    if(maximumLength != 0)
    {
        m_maximumLength = std::make_shared<acseItemMaxLength>(maximumLength);
    }

    if(!implementationClassUID.empty())
    {
        m_implementationClassUID = std::make_shared<acseItemImplementationClassUID>(implementationClassUID);
    }

    if(!implementationVersionName.empty())
    {
        m_implementationVersionName = std::make_shared<acseItemImplementationVersionName>(implementationVersionName);
    }

    if(maxOperationsInvoked != 0 || maxOperationsPerformed != 0)
    {
        m_asyncOperationsWindow = std::make_shared<acseItemAsynchronousOperationsWindow>(maxOperationsInvoked, maxOperationsPerformed);
    }

    IMEBRA_FUNCTION_END();
}


acseItem::itemType_t acseItemUserInformation::getItemType() const
{
    return itemType_t::userInformation;
}


std::uint32_t acseItemUserInformation::getMaximumPDULength() const
{
    if(m_maximumLength.get() == 0)
    {
        return 0;
    }
    return m_maximumLength->getMaxLength();
}


std::uint16_t acseItemUserInformation::getMaxOperationsInvoked() const
{
    if(m_asyncOperationsWindow.get() == 0)
    {
        return 1;
    }
    return m_asyncOperationsWindow->getMaxOperationsInvoked();
}


std::uint16_t acseItemUserInformation::getMaxOperationsPerformed() const
{
    if(m_asyncOperationsWindow.get() == 0)
    {
        return 1;
    }
    return m_asyncOperationsWindow->getMaxOperationsPerformed();
}


const acseItemUserInformation::scpScuSelectionList_t& acseItemUserInformation::getScpScuRoles()
{
    return m_scuScpRoleSelections;
}


void acseItemUserInformation::encodeItemPayload(std::shared_ptr<streamWriter> writer) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- User information");
    if(m_maximumLength.get() != 0)
    {
        IMEBRA_LOG_INFO("     Max PDU length = " << m_maximumLength->getMaxLength());
        m_maximumLength->encodeItem(writer);
    }

    if(m_implementationClassUID.get() != 0)
    {
        IMEBRA_LOG_INFO("     Implementation class UID = " << m_implementationClassUID->getName());
        m_implementationClassUID->encodeItem(writer);
    }

    if(m_asyncOperationsWindow.get() != 0)
    {
        IMEBRA_LOG_INFO("     Async operations window: invoked = " << m_asyncOperationsWindow->getMaxOperationsInvoked() << " performed = " << m_asyncOperationsWindow->getMaxOperationsPerformed());
        m_asyncOperationsWindow->encodeItem(writer);
    }

    if(m_implementationVersionName.get() != 0)
    {
        IMEBRA_LOG_INFO("     Implementation version name = " << m_implementationVersionName->getName());
        m_implementationVersionName->encodeItem(writer);
    }

    for(scpScuSelectionList_t::const_iterator scanItems(m_scuScpRoleSelections.begin()), end(m_scuScpRoleSelections.end());
        scanItems != end;
        ++scanItems)
    {
        IMEBRA_LOG_INFO("     SCP/SCU selection: class UID = " << (*scanItems)->getSopClassUID() << " SCP: " << ((*scanItems)->getSCP() ? "Yes" : "No") << " SCU: " << ((*scanItems)->getSCU() ? "Yes" : "No"));
        (*scanItems)->encodeItem(writer);
    }

    IMEBRA_FUNCTION_END();
}


void acseItemUserInformation::decodeItemPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    try
    {
        for(;;)
        {
            std::shared_ptr<acseItem> item(acseItem::decodeItem(reader));

            itemType_t itemType(item->getItemType());
            switch(itemType)
            {
            case itemType_t::maximumLength:
                m_maximumLength = std::static_pointer_cast<acseItemMaxLength>(item);
                break;
            case itemType_t::implementationClassUID:
                m_implementationClassUID = std::static_pointer_cast<acseItemImplementationClassUID>(item);
                break;
            case itemType_t::implementationVersionName:
                m_implementationVersionName = std::static_pointer_cast<acseItemImplementationVersionName>(item);
                break;
            case itemType_t::asynchronousOperationsWindow:
                m_asyncOperationsWindow = std::static_pointer_cast<acseItemAsynchronousOperationsWindow>(item);
                break;
            case itemType_t::SCPSCURoleSelection:
                m_scuScpRoleSelections.push_back(std::static_pointer_cast<acseItemSCPSCURoleSelection>(item));
                break;
            default:
                break;
            }
        }
    }
    catch(const StreamEOFError&)
    {
        // Ignore (means no more items)
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Max Length Item
//
///////////////////////////////////////////////////////////

acseItemMaxLength::acseItemMaxLength():
    m_maxLength(0)
{
}


acseItemMaxLength::acseItemMaxLength(std::uint32_t maxLength):
    m_maxLength(maxLength)
{
}


std::uint32_t acseItemMaxLength::getMaxLength() const
{
    return m_maxLength;
}


acseItem::itemType_t acseItemMaxLength::getItemType() const
{
    return itemType_t::maximumLength;
}


void acseItemMaxLength::encodeItemPayload(std::shared_ptr<streamWriter> writer) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t maxLength = writer->adjustEndian(m_maxLength, streamController::highByteEndian);
    writer->write((std::uint8_t*)&maxLength, sizeof(maxLength));

    IMEBRA_FUNCTION_END();
}


void acseItemMaxLength::decodeItemPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    std::uint32_t maxLength;
    reader->read((std::uint8_t*)&maxLength, sizeof(maxLength));
    m_maxLength = reader->adjustEndian(maxLength, streamController::highByteEndian);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Implementation Class UID Item
//
///////////////////////////////////////////////////////////
acseItem::itemType_t acseItemImplementationClassUID::getItemType() const
{
    return itemType_t::implementationClassUID;
}


///////////////////////////////////////////////////////////
//
// Implementation Version Item constructor
//
///////////////////////////////////////////////////////////
acseItemImplementationVersionName::acseItemImplementationVersionName()
{
}

acseItemImplementationVersionName::acseItemImplementationVersionName(const std::string& name):
    m_name(name)
{
}


///////////////////////////////////////////////////////////
//
// Implementation Version Item
//
///////////////////////////////////////////////////////////
acseItem::itemType_t acseItemImplementationVersionName::getItemType() const
{
    return itemType_t::implementationVersionName;
}

const std::string& acseItemImplementationVersionName::getName() const
{
    return m_name;
}

///////////////////////////////////////////////////////////
//
// Encode the string
//
///////////////////////////////////////////////////////////
void acseItemImplementationVersionName::encodeItemPayload(std::shared_ptr<streamWriter> writer) const
{
    IMEBRA_FUNCTION_START();

    writer->write((const std::uint8_t*)m_name.c_str(), m_name.size());

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Decode the string
//
///////////////////////////////////////////////////////////
void acseItemImplementationVersionName::decodeItemPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    std::uint8_t buffer[128];

    std::string name;
    while(!reader->endReached())
    {
        size_t readSize(reader->readSome(buffer, sizeof(buffer)));
        name.append(std::string((char*)buffer, readSize));
    }

    m_name = name;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Asynchronous Operations Window Item
//
///////////////////////////////////////////////////////////

acseItemAsynchronousOperationsWindow::acseItemAsynchronousOperationsWindow():
    m_maxOperationsInvoked(0), m_maxOperationsPerformed(0)
{
}


acseItemAsynchronousOperationsWindow::acseItemAsynchronousOperationsWindow(std::uint16_t maxOperationsInvoked, std::uint16_t maxOperationsPerformed):
    m_maxOperationsInvoked(maxOperationsInvoked), m_maxOperationsPerformed(maxOperationsPerformed)
{
}


std::uint16_t acseItemAsynchronousOperationsWindow::getMaxOperationsInvoked() const
{
    return m_maxOperationsInvoked;
}


std::uint16_t acseItemAsynchronousOperationsWindow::getMaxOperationsPerformed() const
{
    return m_maxOperationsPerformed;
}



acseItem::itemType_t acseItemAsynchronousOperationsWindow::getItemType() const
{
    return itemType_t::asynchronousOperationsWindow;
}


void acseItemAsynchronousOperationsWindow::encodeItemPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    std::uint16_t maxOperationsInvoked(pWriter->adjustEndian(m_maxOperationsInvoked, streamController::highByteEndian));
    pWriter->write((std::uint8_t*)&maxOperationsInvoked, sizeof(maxOperationsInvoked));

    std::uint16_t maxOperationsPerformed(pWriter->adjustEndian(m_maxOperationsPerformed, streamController::highByteEndian));
    pWriter->write((std::uint8_t*)&maxOperationsPerformed, sizeof(maxOperationsPerformed));

    IMEBRA_FUNCTION_END();
}


void acseItemAsynchronousOperationsWindow::decodeItemPayload(std::shared_ptr<streamReader> pReader)
{
    IMEBRA_FUNCTION_START();

    std::uint16_t maxOperationsInvoked;
    pReader->read((std::uint8_t*)&maxOperationsInvoked, sizeof(maxOperationsInvoked));
    m_maxOperationsInvoked = pReader->adjustEndian(maxOperationsInvoked, streamController::highByteEndian);

    std::uint16_t maxOperationsPerformed;
    pReader->read((std::uint8_t*)&maxOperationsPerformed, sizeof(maxOperationsPerformed));
    m_maxOperationsPerformed = pReader->adjustEndian(maxOperationsPerformed, streamController::highByteEndian);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// SCP/SCU Role Item
//
///////////////////////////////////////////////////////////

acseItemSCPSCURoleSelection::acseItemSCPSCURoleSelection():
    m_bSCU(false), m_bSCP(false)
{
}


acseItemSCPSCURoleSelection::acseItemSCPSCURoleSelection(const std::string& sopClassUID, bool bSCU, bool bSCP):
    m_sopClassUID(sopClassUID), m_bSCU(bSCU), m_bSCP(bSCP)
{
}


std::string acseItemSCPSCURoleSelection::getSopClassUID() const
{
    return m_sopClassUID;
}


bool acseItemSCPSCURoleSelection::getSCU() const
{
    return m_bSCU;
}


bool acseItemSCPSCURoleSelection::getSCP() const
{
    return m_bSCP;
}


acseItem::itemType_t acseItemSCPSCURoleSelection::getItemType() const
{
    return itemType_t::SCPSCURoleSelection;
}


void acseItemSCPSCURoleSelection::encodeItemPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    std::string sopClassUID(handlers::normalizeUid(m_sopClassUID));

    std::uint16_t sopClassLength((std::uint16_t)sopClassUID.size());
    pWriter->adjustEndian((std::uint8_t*)&sopClassLength, sizeof(sopClassLength), streamController::highByteEndian);
    pWriter->write((const std::uint8_t*)&sopClassLength, sizeof(sopClassLength));

    pWriter->write((const std::uint8_t*)&(sopClassUID[0]), sopClassUID.size());

    std::uint8_t scu(m_bSCU ? 1 : 0), scp(m_bSCP ? 1 : 0);
    pWriter->write(&scu, 1);
    pWriter->write(&scp, 1);

    IMEBRA_FUNCTION_END();
}


void acseItemSCPSCURoleSelection::decodeItemPayload(std::shared_ptr<streamReader> pReader)
{
    IMEBRA_FUNCTION_START();

    std::uint16_t sopClassLength;
    pReader->read((std::uint8_t*)&sopClassLength, sizeof(sopClassLength));

    std::string sopClassUID((size_t)pReader->adjustEndian(sopClassLength, streamController::highByteEndian), ' ');
    pReader->read((std::uint8_t*)&(sopClassUID[0]), sopClassUID.size());
    m_sopClassUID = handlers::normalizeUid(sopClassUID);

    std::uint8_t scu, scp;

    pReader->read(&scu, 1);
    m_bSCU = (scu != 0);

    pReader->read(&scp, 1);
    m_bSCP = (scp != 0);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// PDU
//
///////////////////////////////////////////////////////////

acsePDU::~acsePDU()
{
}


std::shared_ptr<acsePDU> acsePDU::decodePDU(std::shared_ptr<streamReader> pReader)
{
    IMEBRA_FUNCTION_START();

    pduType_t pduType;
    pReader->read((std::uint8_t*)&pduType, 1);

    try
    {
        std::shared_ptr<acsePDU> pPDU;
        switch(pduType)
        {
        case pduType_t::associateRQ:
            pPDU = std::make_shared<acsePDUAssociateRQ>();
            break;
        case pduType_t::associateAC:
            pPDU = std::make_shared<acsePDUAssociateAC>();
            break;
        case pduType_t::associateRJ:
            pPDU = std::make_shared<acsePDUAssociateRJ>();
            break;
        case pduType_t::pData:
            pPDU = std::make_shared<acsePDUPData>();
            break;
        case pduType_t::aReleaseRQ:
            pPDU = std::make_shared<acsePDUAReleaseRQ>();
            break;
        case pduType_t::aReleaseRP:
            pPDU = std::make_shared<acsePDUAReleaseRP>();
            break;
        case pduType_t::aAbort:
            pPDU = std::make_shared<acsePDUAAbort>();
            break;
        default:
            IMEBRA_THROW(AcseCorruptedMessageError, "Invalid PDU code");

        }

        std::uint8_t reserved;
        pReader->read(&reserved, sizeof(reserved));

        std::uint32_t pduLength;
        pReader->read((std::uint8_t*)&pduLength, sizeof(pduLength));
        pReader->adjustEndian((std::uint8_t*)&pduLength, 4, streamController::highByteEndian);

        std::shared_ptr<streamReader> pPDUStream(pReader->getReader(pduLength)); //

        pPDU->decodePDUPayload(pPDUStream);

        return pPDU;
    }
    catch(const StreamEOFError&)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Corrupted ACSE PDU");
    }

    IMEBRA_FUNCTION_END();
}


void acsePDU::encodePDU(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    pduType_t pduType = getPDUType();
    pWriter->write((std::uint8_t*)&pduType, 1);

    static const std::uint8_t zero(0);
    pWriter->write(&zero, sizeof(zero));

    std::shared_ptr<memory> pMemory(std::make_shared<memory>());
    {
        std::shared_ptr<memoryStreamOutput> pMemoryStream(std::make_shared<memoryStreamOutput>(pMemory));
        std::shared_ptr<streamWriter> pMemoryWriter(std::make_shared<streamWriter>(pMemoryStream));
        encodePDUPayload(pMemoryWriter);
    }

    if(pMemory->size() > std::numeric_limits<std::uint32_t>::max())
    {
        IMEBRA_THROW(std::logic_error, "The PDU's size is too big");
    }

    std::uint32_t pduLength = (std::uint32_t)pMemory->size();
    pWriter->adjustEndian((std::uint8_t*)&pduLength, sizeof(pduLength), streamController::highByteEndian);
    pWriter->write((std::uint8_t*)&pduLength, sizeof(pduLength));

    pWriter->write(pMemory->data(), pMemory->size());

    pWriter->flushDataBuffer();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Associate PDU (Base class)
//
///////////////////////////////////////////////////////////

acsePDUAssociateBase::acsePDUAssociateBase()
{
}


acsePDUAssociateBase::acsePDUAssociateBase(
        const std::string& callingAETitle,
        const std::string& calledAETitle,
        const std::string& applicationContext,
        const presentationContexts_t& presentationContexts,
        std::shared_ptr<acseItemUserInformation> pUserInformation
        ):
    m_callingAETitle(callingAETitle),
    m_calledAETitle(calledAETitle),
    m_pApplicationContext(std::make_shared<acseItemApplicationContext>(applicationContext)),
    m_presentationContexts(presentationContexts),
    m_pUserInformation(pUserInformation)
{
}


const std::string& acsePDUAssociateBase::getCallingAETitle() const
{
    return m_callingAETitle;
}


const std::string& acsePDUAssociateBase::getCalledAETitle() const
{
    return m_calledAETitle;
}


const std::shared_ptr<acseItemApplicationContext>& acsePDUAssociateBase::getApplicationContext() const
{
    return m_pApplicationContext;
}


const acsePDUAssociateBase::presentationContexts_t& acsePDUAssociateBase::getPresentationContexts() const
{
    return m_presentationContexts;
}


const std::shared_ptr<acseItemUserInformation>& acsePDUAssociateBase::getItemUserInformation() const
{
    return m_pUserInformation;
}


void acsePDUAssociateBase::encodePDUPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Sending Association PDU")

    IMEBRA_LOG_INFO("     Protocol version = 1");
    std::uint16_t protocolVersion(pWriter->adjustEndian(std::uint16_t(1), streamController::highByteEndian));
    pWriter->write((std::uint8_t*)&protocolVersion, sizeof(protocolVersion));

    const std::uint16_t zero(0);
    pWriter->write((std::uint8_t*)&zero, sizeof(zero));

    std::string calledAETitle(m_calledAETitle + std::string(size_t(16) - m_calledAETitle.size(), ' '));
    IMEBRA_LOG_INFO("     Called AET = " << calledAETitle);
    pWriter->write((std::uint8_t*)calledAETitle.c_str(), calledAETitle.size());

    std::string callingAETitle(m_callingAETitle + std::string(size_t(16) - m_callingAETitle.size(), ' '));
    IMEBRA_LOG_INFO("     Calling AET = " << callingAETitle);
    pWriter->write((std::uint8_t*)callingAETitle.c_str(), callingAETitle.size());
    std::string reserved(m_reserved + std::string(size_t(32) - m_reserved.size(), ' '));
    pWriter->write((std::uint8_t*)reserved.c_str(), reserved.size());

    m_pApplicationContext->encodeItem(pWriter);

    for(presentationContexts_t::const_iterator scanContexts(m_presentationContexts.begin()), endContexts(m_presentationContexts.end());
        scanContexts != endContexts;
        ++scanContexts)
    {
        (*scanContexts)->encodeItem(pWriter);
    }

    m_pUserInformation->encodeItem(pWriter);

    IMEBRA_FUNCTION_END();
}


void acsePDUAssociateBase::decodePDUPayload(std::shared_ptr<streamReader> pReader)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Received Association PDU")

    std::uint16_t protocolVersion;
    pReader->read((std::uint8_t*)&protocolVersion, sizeof(protocolVersion));
    pReader->adjustEndian((std::uint8_t*)&protocolVersion, sizeof(protocolVersion), streamController::highByteEndian);
    IMEBRA_LOG_INFO("     Protocol version = " << +protocolVersion);
    if((protocolVersion & 0x0001) == 0)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Wrong protocol version");
    }

    std::uint16_t zero;
    pReader->read((std::uint8_t*)&zero, sizeof(zero));

    m_calledAETitle = readFixedLengthString<16>(pReader);
    IMEBRA_LOG_INFO("     Called AET = " << m_calledAETitle);
    m_callingAETitle = readFixedLengthString<16>(pReader);
    IMEBRA_LOG_INFO("     Calling AET = " << m_callingAETitle);
    m_reserved = readFixedLengthString<32>(pReader);

    try
    {
        for(;;)
        {
            std::shared_ptr<acseItem> item(acseItem::decodeItem(pReader));
            if(item != 0)
            {
                acseItem::itemType_t itemType(item->getItemType());
                switch(itemType)
                {
                case acseItem::itemType_t::applicationContext:
                    m_pApplicationContext = std::dynamic_pointer_cast<acseItemApplicationContext>(item);
                    break;
                case acseItem::itemType_t::presentationContextRQ:
                    m_presentationContexts.push_back(std::dynamic_pointer_cast<acseItemPresentationContextRQ>(item));
                    break;
                case acseItem::itemType_t::presentationContextAC:
                    m_presentationContexts.push_back(std::dynamic_pointer_cast<acseItemPresentationContextAC>(item));
                    break;
                case acseItem::itemType_t::userInformation:
                    m_pUserInformation = std::dynamic_pointer_cast<acseItemUserInformation>(item);
                    break;
                default:
                    // ignore
                    break;
                }
            }
        }
    }
    catch(const StreamEOFError&)
    {
        // Ignore (means no more items)
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Association Request PDU
//
///////////////////////////////////////////////////////////

acsePDU::pduType_t acsePDUAssociateRQ::getPDUType() const
{
    return pduType_t::associateRQ;
}


///////////////////////////////////////////////////////////
//
// Association Acceptance PDU
//
///////////////////////////////////////////////////////////

acsePDU::pduType_t acsePDUAssociateAC::getPDUType() const
{
    return pduType_t::associateAC;
}


///////////////////////////////////////////////////////////
//
// Association Rejection PDU
//
///////////////////////////////////////////////////////////

acsePDUAssociateRJ::acsePDUAssociateRJ()
{
}

acsePDUAssociateRJ::acsePDUAssociateRJ(result_t result, reason_t reason):
    m_result(result), m_reason(reason)
{
}


acsePDU::pduType_t acsePDUAssociateRJ::getPDUType() const
{
    return pduType_t::associateRJ;
}


acsePDUAssociateRJ::result_t acsePDUAssociateRJ::getResult() const
{
    return m_result;
}


acsePDUAssociateRJ::reason_t acsePDUAssociateRJ::getReason() const
{
    return m_reason;
}


void acsePDUAssociateRJ::encodePDUPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Sending Association Rejection PDU");

    const std::uint8_t zero(0);
    pWriter->write(&zero, sizeof(zero));
    pWriter->write((std::uint8_t*)&m_result, sizeof(m_result));

    std::uint16_t reason(pWriter->adjustEndian((std::uint16_t)m_reason, streamController::highByteEndian));
    pWriter->write((std::uint8_t*)&reason, sizeof(reason));

    IMEBRA_FUNCTION_END();
}


void acsePDUAssociateRJ::decodePDUPayload(std::shared_ptr<streamReader> pReader)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Received Association Rejection PDU");

    std::uint8_t zero;
    pReader->read(&zero, sizeof(zero));
    pReader->read((std::uint8_t*)&m_result, sizeof(m_result));
    if(m_result != result_t::rejectedPermanent && m_result != result_t::rejectedTransient)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Unrecognized association rejection code");
    }

    std::uint16_t reason;
    pReader->read((std::uint8_t*)&reason, sizeof(reason));
    m_reason = (reason_t)(pReader->adjustEndian(reason, streamController::highByteEndian));
    if(
            m_reason != reason_t::serviceProviderAcseNoReasonGiven &&
            m_reason != reason_t::serviceProviderAcseProtocolVersionNotSupported &&
            m_reason != reason_t::serviceProviderPresentationLocalLimitExceded &&
            m_reason != reason_t::serviceProviderPresentationReserved &&
            m_reason != reason_t::serviceProviderPresentationTemporaryCongestion &&
            m_reason != reason_t::serviceUserApplicationContextNameNotSupported &&
            m_reason != reason_t::serviceUserCalledAETitleNotRecognized &&
            m_reason != reason_t::serviceUserCallingAETitleNotRecognized &&
            m_reason != reason_t::serviceUserNoReasonGiven)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Unrecognized association rejection reason");
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Data PDU
//
///////////////////////////////////////////////////////////

acsePDU::pduType_t acsePDUPData::getPDUType() const
{
    return pduType_t::pData;
}


size_t acsePDUPData::addItem(std::uint8_t presentationContextId, std::shared_ptr<memory> pMemory, size_t offset, size_t maxPDUSize, bool bCommand)
{
    IMEBRA_FUNCTION_START();

    // Get the current PDU size
    size_t pduSize(0);
    for(pdataValues_t::const_iterator scanPValues(m_values.begin()), endPValues(m_values.end()); scanPValues != endPValues; ++scanPValues)
    {
        pduSize += 6; // item size + context id + pdv header
        pduSize += (*scanPValues)->m_memorySize;
    }

    assert(maxPDUSize >= pduSize);
    size_t remainingPDUSize(maxPDUSize - pduSize);
    if(remainingPDUSize < 8) // compare with 8 because 1 byte is rounded up
    {
        return 0;
    }
    remainingPDUSize -= 6;

    std::shared_ptr<acseItemPDataValue> pData(std::make_shared<acseItemPDataValue>());
    pData->m_presentationContextId = presentationContextId;
    pData->m_bCommand = bCommand;
    pData->m_pMemory = pMemory;
    pData->m_memoryOffset = offset;

    size_t remainingMemorySize = pMemory->size() - offset;
    if(remainingPDUSize >= remainingMemorySize)
    {
        pData->m_bLast = true;
        pData->m_memorySize = remainingMemorySize;
        m_values.push_back(pData);
        return remainingMemorySize;
    }
    if((remainingPDUSize & 0x1) != 0)
    {
        --remainingPDUSize;
    }
    pData->m_bLast = false;
    pData->m_memorySize = remainingPDUSize;
    m_values.push_back(pData);
    return remainingPDUSize;

    IMEBRA_FUNCTION_END();
}


const acsePDUPData::pdataValues_t& acsePDUPData::getValues() const
{
    return m_values;
}


void acsePDUPData::encodePDUPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Sending Data PDU");

    for(pdataValues_t::const_iterator scanPValues(m_values.begin()), endPValues(m_values.end()); scanPValues != endPValues; ++scanPValues)
    {
        IMEBRA_LOG_INFO("     -- PValue");
        IMEBRA_LOG_INFO("        size = " << (*scanPValues)->m_memorySize << " bytes");

        std::uint32_t length(pWriter->adjustEndian((std::uint32_t)((*scanPValues)->m_memorySize + 2), streamController::highByteEndian));
        pWriter->write((std::uint8_t*)&length, sizeof(length));
        pWriter->write((std::uint8_t*)&(*scanPValues)->m_presentationContextId, sizeof(acseItemPDataValue::m_presentationContextId));

        IMEBRA_LOG_INFO("        type = " << ((*scanPValues)->m_bCommand ? "command" : "payload"));
        IMEBRA_LOG_INFO("        last = " << ((*scanPValues)->m_bLast ? "yes" : "no"));
        std::uint8_t pdvHeader = (std::uint8_t)(((*scanPValues)->m_bCommand ? 1 : 0) | ((*scanPValues)->m_bLast ? 2 : 0));
        pWriter->write(&pdvHeader, 1);
        pWriter->write((*scanPValues)->m_pMemory->data() + (*scanPValues)->m_memoryOffset, (*scanPValues)->m_memorySize);
    }

    IMEBRA_FUNCTION_END();
}


void acsePDUPData::decodePDUPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Received Data PDU");

    while(!reader->endReached())
    {
        std::uint32_t length;

        try
        {
            reader->read((std::uint8_t*)&length, sizeof(length));
            reader->adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
        }
        catch(const StreamEOFError&)
        {
            IMEBRA_THROW(AcseCorruptedMessageError, "Could not read the PDATA item length");
        }


        std::shared_ptr<acseItemPDataValue> pDataValue(std::make_shared<acseItemPDataValue>());
        reader->read(&(pDataValue->m_presentationContextId), sizeof(acseItemPDataValue::m_presentationContextId));
        if((pDataValue->m_presentationContextId & 0x01) == 0)
        {
            IMEBRA_THROW(AcseCorruptedMessageError, "PDU PDATA presentation context ID is not an odd number");
        }
        std::uint8_t pdvHeader;
        reader->read(&pdvHeader, 1);
        pDataValue->m_bCommand = (pdvHeader & 1) == 0 ? false : true;
        pDataValue->m_bLast = (pdvHeader & 2) == 0 ? false : true;
        pDataValue->m_pMemory = std::make_shared<memory>(length - 2);
        pDataValue->m_memoryOffset = 0;
        pDataValue->m_memorySize = length - 2;
        reader->read(pDataValue->m_pMemory->data(), length - 2);

        IMEBRA_LOG_INFO("     -- PValue");
        IMEBRA_LOG_INFO("        size = " << pDataValue->m_memorySize << " bytes");
        IMEBRA_LOG_INFO("        type = " << (pDataValue->m_bCommand ? "command" : "payload"));
        IMEBRA_LOG_INFO("        last = " << (pDataValue->m_bLast ? "yes" : "no"));

        m_values.push_back(pDataValue);
    }

    IMEBRA_FUNCTION_END();
}



///////////////////////////////////////////////////////////
//
// Association Release PDU (Base class)
//
///////////////////////////////////////////////////////////

void acsePDUARelease::encodePDUPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Sending Association Release PDU");

    const std::uint32_t zero(0);
    pWriter->write((std::uint8_t*)&zero, sizeof(zero));

    IMEBRA_FUNCTION_END();
}


void acsePDUARelease::decodePDUPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Receiving Association Release PDU");

    std::uint32_t zero(0);
    reader->read((std::uint8_t*)&zero, sizeof(zero));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Association Release Request PDU
//
///////////////////////////////////////////////////////////

acsePDU::pduType_t acsePDUAReleaseRQ::getPDUType() const
{
    return pduType_t::aReleaseRQ;
}


///////////////////////////////////////////////////////////
//
// Association Release Response PDU
//
///////////////////////////////////////////////////////////

acsePDU::pduType_t acsePDUAReleaseRP::getPDUType() const
{
    return pduType_t::aReleaseRP;
}


///////////////////////////////////////////////////////////
//
// Abort PDU
//
///////////////////////////////////////////////////////////

acsePDUAAbort::acsePDUAAbort():
    m_reason(reason_t::serviceProviderReasonNotSpecified)
{
}


acsePDUAAbort::acsePDUAAbort(reason_t reason):
    m_reason(reason)
{
}


acsePDUAAbort::reason_t acsePDUAAbort::getReason() const
{
    return m_reason;
}


acsePDU::pduType_t acsePDUAAbort::getPDUType() const
{
    return pduType_t::aAbort;
}


void acsePDUAAbort::encodePDUPayload(std::shared_ptr<streamWriter> pWriter) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Sending Association Abort PDU");

    const std::uint16_t zero(0);
    pWriter->write((std::uint8_t*)&zero, sizeof(zero));

    std::uint16_t reason(pWriter->adjustEndian((std::uint16_t)m_reason, streamController::highByteEndian));
    pWriter->write((std::uint8_t*)&reason, sizeof(reason));

    IMEBRA_FUNCTION_END();
}


void acsePDUAAbort::decodePDUPayload(std::shared_ptr<streamReader> reader)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("  -- Receiving Association Abort PDU");

    std::uint16_t zero(0);
    reader->read((std::uint8_t*)&zero, sizeof(zero));

    std::uint16_t reason;
    reader->read((std::uint8_t*)&reason, sizeof(reason));
    m_reason = (reason_t)(reader->adjustEndian(reason, streamController::highByteEndian));
    if(
            (reason_t)((std::uint16_t)m_reason & (std::uint16_t)0xff00) != reason_t::serviceUser &&
            m_reason != reason_t::serviceProviderReasonNotSpecified &&
            m_reason != reason_t::serviceProviderUnrecognizedPDU &&
            m_reason != reason_t::serviceProviderUnexpectedPDU &&
            m_reason != reason_t::serviceProviderUnrecognizedPDUParameter &&
            m_reason != reason_t::serviceProviderUnexpectedPDUParameter &&
            m_reason != reason_t::serviceProviderInvalidPDUParameterValue)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "A-ABORT unrecognized abort reason");
    }

    IMEBRA_FUNCTION_END();
}


presentationContext::presentationContext(const std::string& abstractSyntax):
    m_abstractSyntax(abstractSyntax), m_bRequestorIsSCU(true), m_bRequestorIsSCP(false)
{
}

presentationContext::presentationContext(const std::string& abstractSyntax, bool bRequestorIsSCU, bool bRequestorIsSCP):
    m_abstractSyntax(abstractSyntax), m_bRequestorIsSCU(bRequestorIsSCU), m_bRequestorIsSCP(bRequestorIsSCP)
{
}

presentationContext::presentationContext(const presentationContext& right):
    m_abstractSyntax(right.m_abstractSyntax),
    m_proposedTransferSyntaxes(right.m_proposedTransferSyntaxes),
    m_bRequestorIsSCU(right.m_bRequestorIsSCU),
    m_bRequestorIsSCP(right.m_bRequestorIsSCP)
{
}

void presentationContext::addTransferSyntax(const std::string& transferSyntax)
{
    m_proposedTransferSyntaxes.push_back(transferSyntax);
}


presentationContexts::presentationContexts()
{

}


presentationContexts::presentationContexts(const presentationContexts &right)
{
    for(const std::shared_ptr<presentationContext>& context: right.m_presentationContexts)
    {
        m_presentationContexts.push_back(std::make_shared<presentationContext>(*context));
    }
}


associationMessage::associationMessage(const std::string& abstractSyntax):
    associationMessage(abstractSyntax, nullptr)
{
}


associationMessage::associationMessage(const std::string& abstractSyntax, std::shared_ptr<dataSet> pCommand):
    associationMessage(abstractSyntax, pCommand, nullptr)
{
}


associationMessage::associationMessage(const std::string& abstractSyntax, std::shared_ptr<dataSet> pCommand, std::shared_ptr<dataSet> pPayload):
    m_abstractSyntax(abstractSyntax), m_pCommand(pCommand), m_pPayload(pPayload)
{
}


associationMessage::~associationMessage()
{
}


void associationMessage::addDataset(const std::shared_ptr<dataSet> pDataset)
{
    IMEBRA_FUNCTION_START();

    if(m_pCommand == nullptr)
    {
        m_pCommand = pDataset;
        return;
    }

    if(m_pPayload == nullptr)
    {
        m_pPayload = pDataset;
        return;
    }

    IMEBRA_THROW(std::logic_error, "Too many datasets being added to the message");

    IMEBRA_FUNCTION_END();
}

const std::string& associationMessage::getAbstractSyntax() const
{
    return m_abstractSyntax;
}

std::shared_ptr<dataSet> associationMessage::getCommandDataSet() const
{
    if(m_pCommand == nullptr)
    {
        IMEBRA_THROW(AcseCorruptedMessageError, "Missing command dataset");
    }
    return m_pCommand;
}

std::shared_ptr<dataSet> associationMessage::getPayloadDataSetNoThrow() const
{
    return m_pPayload;
}

std::shared_ptr<dataSet> associationMessage::getPayloadDataSet() const
{
    IMEBRA_FUNCTION_START();

    if(m_pPayload == nullptr)
    {
        IMEBRA_THROW(MissingItemError, "Missing payload dataset");
    }

    return m_pPayload;

    IMEBRA_FUNCTION_END();
}




bool associationMessage::isComplete() const
{
    return m_pCommand != nullptr &&
            m_pCommand->bufferExists(0, 0, 0x100, 0) &&
            (m_pCommand->getUnsignedLong(0, 0, 0x800, 0, 0, 0x0101) == 0x0101 || m_pPayload != nullptr);
}



associationBase::associationBase(
        role_t role,
        const std::string& thisAET,
        const std::string& otherAET,
        std::uint32_t maxOperationsWeInvoke,
        std::uint32_t maxOperationsWeCanPerform,
        std::shared_ptr<streamReader> pReader,
        std::shared_ptr<streamWriter> pWriter,
        std::uint32_t dimseTimeout):
    m_role(role),
    m_thisAET(thisAET),
    m_otherAET(otherAET),
    m_maxOperationsInvoked(maxOperationsWeInvoke),
    m_maxOperationsPerformed(maxOperationsWeCanPerform),
    m_bAssociated(1),
    m_maxPDULength(MAXIMUM_PDU_SIZE),
    m_pReader(pReader),
    m_pWriter(pWriter),
    m_bTerminated(false),
    m_dimseTimeout(dimseTimeout)
{
}


associationBase::~associationBase()
{
    m_pReader->terminate();
    if(m_readDataSetsThread != nullptr)
    {
        m_readDataSetsThread->join();
    }
}


void associationBase::sendMessage(std::shared_ptr<const associationMessage> message)
{
    IMEBRA_FUNCTION_START();

    // Find the payload's transfer syntax
    std::string transferSyntax;
    const std::shared_ptr<const dataSet> pPayload(message->getPayloadDataSetNoThrow());
    if(pPayload != nullptr)
    {
		// This seems to be a bug, we need to use the negotiated transfer syntax, defaulting to implicit vr for now
        //transferSyntax = pPayload->getString(0x2, 0, 0x10, 0, 0, "");
        transferSyntax = "1.2.840.10008.1.2";
    }

    // Find the transfer syntax negotiated for the requested
    // presentation context
    ///////////////////////////////////////////////////////////
    std::uint8_t presentationContextId(0);
    std::shared_ptr<const presentationContext> pPresentationContext;
    for(presentationContextsIds_t::const_iterator scanPresentationContexts(m_presentationContextsIds.begin()), endPresentationContexts(m_presentationContextsIds.end());
        scanPresentationContexts != endPresentationContexts;
        ++scanPresentationContexts)
    {
        if(message->getAbstractSyntax() == scanPresentationContexts->second.first->m_abstractSyntax &&
                (transferSyntax.empty() || transferSyntax == scanPresentationContexts->second.second))
        {
            transferSyntax = scanPresentationContexts->second.second;
            presentationContextId = scanPresentationContexts->first;
            pPresentationContext = scanPresentationContexts->second.first;
            break;
        }
        else {
            std::cout << message->getAbstractSyntax() << " | " << scanPresentationContexts->second.first->m_abstractSyntax << std::endl;
            std::cout << transferSyntax << " | " << scanPresentationContexts->second.second << std::endl;
        }
    }

    if(presentationContextId == 0)
    {
        IMEBRA_THROW(AcsePresentationContextNotRequestedError, "The message's presentation context was not requested during the association negotiation");
    }
    if(transferSyntax.empty())
    {
        IMEBRA_THROW(AcseNoTransferSyntaxError, "No transfer syntax for the selected presentation context with abstract syntax " << message->getAbstractSyntax());
    }

    // Serialize all the datasets (command and payload)
    ///////////////////////////////////////////////////////////
    std::shared_ptr<acsePDUPData> pData;

    std::unique_lock<std::mutex> lockWrite(m_lockWrite);

    for(size_t dataSetCount(0); dataSetCount != 2; ++dataSetCount)
    {
        bool bExplicitDataType(false);
        streamController::tByteOrdering endianType(streamController::lowByteEndian);

        if(dataSetCount != 0)
        {
            // Adjust the transfer syntax flags
            ///////////////////////////////////////////////////////////
            bExplicitDataType = (transferSyntax != "1.2.840.10008.1.2");        // Implicit VR little endian

            // Explicit VR big endian
            ///////////////////////////////////////////////////////////
            endianType = (transferSyntax == "1.2.840.10008.1.2.2") ? streamController::highByteEndian : streamController::lowByteEndian;
        }

        std::shared_ptr<const dataSet> pDataSet(dataSetCount == 0 ? message->getCommandDataSet() : message->getPayloadDataSetNoThrow());
        if(pDataSet == nullptr)
        {
            break;
        }

        if(pDataSet->bufferExists(0, 0, 0x100, 0))
        {
            // Check if the role is correct for the selected
            // presentation context
            ///////////////////////////////////////////////////////////
            const bool bResponse( (pDataSet->getUnsignedLong(0x0, 0, 0x100, 0, 0, 0) & 0x00008000) != 0);
            if(m_role == role_t::scu)
            {
                if(!bResponse && !pPresentationContext->m_bRequestorIsSCU)
                {
                    IMEBRA_THROW(AcseWrongRoleError, "Wrong role for the selected presentation context");
                }
            }
            else
            {
                if(!bResponse && !pPresentationContext->m_bRequestorIsSCP)
                {
                    IMEBRA_THROW(AcseWrongRoleError, "Wrong role for the selected presentation context");
                }
            }

            std::unique_lock<std::mutex> lockCommandsResponses(m_lockCommandsResponses);
            if(bResponse)
            {
                std::uint32_t responseId = pDataSet->getUnsignedLong(0x0, 0, 0x120, 0, 0, 0);
                if((pDataSet->getUnsignedLong(0x0, 0, 0x900, 0, 0, 0) & 0xfff0) == 0xff00)
                {
                    // partial response
                    if(m_processingCommands.find(responseId) == m_processingCommands.end())
                    {
                        IMEBRA_THROW(AcseWrongResponseIdError, "Sending a partial response with an ID that does not correspond to any received command");
                    }
                }
                else if(m_processingCommands.erase(responseId) == 0)
                {
                    IMEBRA_THROW(AcseWrongResponseIdError, "Sending a response with an ID that does not correspond to any received command");
                }
            }
            else
            {
                if(pDataSet->getUnsignedLong(0x0, 0, 0x100, 0, 0, 0) != 0x0fff)
                {
                    const std::uint32_t commandId(pDataSet->getUnsignedLong(0x0, 0, 0x110, 0, 0, 0));
                    if(m_waitingResponses.count(commandId) != 0)
                    {
                        IMEBRA_THROW(AcseWrongCommandIdError, "Sending a command with the same ID of a command still being processed");
                    }
                    if(m_maxOperationsInvoked != 0 && m_waitingResponses.size() == m_maxOperationsInvoked)
                    {
                        IMEBRA_THROW(AcseTooManyOperationsInvokedError, "Invoking too many operations (max is " << m_maxOperationsInvoked << ")");
                    }
                    m_waitingResponses.insert(commandId);
                }
            }
        }

        if(dataSetCount == 1)
        {
            // Commands are written in a separate PDU
            pData->encodePDU(m_pWriter);
            pData.reset();
        }
        if(pData == 0)
        {
            pData = std::make_shared<acsePDUPData>();
        }

        // Encode the dataset into a memory object
        //////////////////////////////////////////
        std::shared_ptr<memory> pEncodedDataSet(std::make_shared<memory>());
        {
            std::shared_ptr<memoryStreamOutput> pWriteCommand(std::make_shared<memoryStreamOutput>(pEncodedDataSet));
            std::shared_ptr<streamWriter> pCommandWriter(std::make_shared<streamWriter>(pWriteCommand));
            codecs::dicomStreamCodec::buildStream(pCommandWriter, pDataSet, bExplicitDataType, endianType, codecs::dicomStreamCodec::streamType_t::normal);
        }

        if((pEncodedDataSet->size() & 0x1) != 0)
        {
            IMEBRA_THROW(std::logic_error, "The data size should be aligned on 2 bytes boundary");
        }

        for(size_t offset(0); offset != pEncodedDataSet->size(); )
        {
            size_t added(pData->addItem(presentationContextId, pEncodedDataSet, offset, m_maxPDULength, dataSetCount == 0));
            if(added == 0)
            {
                pData->encodePDU(m_pWriter);
                pData = std::make_shared<acsePDUPData>();
            }
            offset += added;
        }
    }
    pData->encodePDU(m_pWriter);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<associationMessage> associationBase::getCommand()
{
    return getMessage(0, false);
}


///////////////////////////////////////////////////////////
//
// Get a response dataset
//
///////////////////////////////////////////////////////////
std::shared_ptr<associationMessage> associationBase::getResponse(std::uint16_t messageId)
{
    return getMessage(messageId, true);
}


///////////////////////////////////////////////////////////
//
// Get a message (command + payload)
//
///////////////////////////////////////////////////////////
std::shared_ptr<associationMessage> associationBase::getMessage(std::uint16_t messageId, bool bResponse)
{
    IMEBRA_FUNCTION_START();

    std::chrono::time_point<std::chrono::steady_clock> endTime(
                std::chrono::steady_clock::now() + std::chrono::seconds(m_dimseTimeout));

    std::unique_lock<std::mutex> lock(m_lockReadyDataSets);

    // Loop until all the message's dataset have been found
    ///////////////////////////////////////////////////////////
    while(!m_bTerminated)
    {
        for(readyDatasets_t::iterator scanDatasets(m_readyDataSets.begin()), endDatasets(m_readyDataSets.end());
            scanDatasets != endDatasets;
            /* increased in the loop */)
        {
            readyDatasets_t::iterator nextDataset(scanDatasets);
            ++nextDataset;

            std::shared_ptr<dataSet> commandDataset((*scanDatasets)->getCommandDataSet());

            if(
                    (*scanDatasets)->isComplete() &&
                    (((commandDataset->getUnsignedLong(0x0, 0, 0x100, 0, 0, 0)) & 0x00008000) != 0) == bResponse && // Check if this is a response
                    (!bResponse || (std::uint16_t)commandDataset->getUnsignedLong(0, 0, 0x0120, 0, 0) == messageId)) // If is a response check the message id
            {
                std::shared_ptr<associationMessage> pMessage(*scanDatasets);
                m_readyDataSets.erase(scanDatasets);
                return pMessage;
            }

            scanDatasets = nextDataset;
        }

        if(m_dimseTimeout != 0 && std::chrono::steady_clock::now() > endTime)
        {
            abort(acsePDUAAbort::reason_t::serviceUser);
            m_pReader->terminate();
            IMEBRA_THROW(StreamClosedError, "DIMSE Timeout, closing association");
        }

        if(m_dimseTimeout == 0)
        {
            m_notifyReadyDataSets.wait(lock);
        }
        else
        {
            m_notifyReadyDataSets.wait_until(lock, endTime);
        }
    }

    IMEBRA_THROW(StreamClosedError, "The input stream has been closed");

    IMEBRA_FUNCTION_END();
}


associationBase::receivedDataset::receivedDataset(const std::string& presentationContext, std::shared_ptr<dataSet> pDataset):
    m_presentationContext(presentationContext),
    m_pDataset(pDataset)
{
}


///////////////////////////////////////////////////////////
//
// Decode and return a complete dataset
//
///////////////////////////////////////////////////////////
std::shared_ptr<associationBase::receivedDataset> associationBase::decodePDU(bool bCommand, std::list<std::shared_ptr<acseItemPDataValue> >& pendingPData, size_t& numberOfLastPData) const
{
    IMEBRA_FUNCTION_START();

    // Loop until a dataset is complete
    ///////////////////////////////////////////////////////////
    for(;;)
    {
        // Check if a complete dataset is available
        ///////////////////////////////////////////////////////////
        if(numberOfLastPData != 0)
        {
            // Decode all the pdata values until the last one of a
            // dataset is found
            ///////////////////////////////////////////////////////////
            size_t datasetSize(0);
            std::string abstractSyntax;
            std::string transferSyntax;
            for(std::shared_ptr<acseItemPDataValue> pData: pendingPData)
            {
                datasetSize += pData->m_memorySize;
                if(pData->m_bLast)
                {
                    presentationContextsIds_t::const_iterator findPresentationContext(
                                m_presentationContextsIds.find(pData->m_presentationContextId));
                    if(findPresentationContext == m_presentationContextsIds.end())
                    {
                        IMEBRA_THROW(AcseCorruptedMessageError, "Presentation context ID " << pData->m_presentationContextId << " not valid");
                    }
                    abstractSyntax = findPresentationContext->second.first->m_abstractSyntax;
                    transferSyntax = findPresentationContext->second.second;
                    break;
                }
            }

            std::shared_ptr<memory> datasetMemory(std::make_shared<memory>(datasetSize));
            size_t memoryOffset(0);
            for(;;)
            {
                std::shared_ptr<acseItemPDataValue> pData(pendingPData.front());
                pendingPData.pop_front();
                ::memcpy(datasetMemory->data() + memoryOffset,
                         pData->m_pMemory->data() + pData->m_memoryOffset,
                         pData->m_memorySize);
                memoryOffset += pData->m_memorySize;
                if(pData->m_bLast)
                {
                    break;
                }
            }
            --numberOfLastPData;

            bool bExplicitDataType(false);
            streamController::tByteOrdering endianType(streamController::lowByteEndian);

            if(!bCommand)
            {
                // Adjust the transfer syntax flags
                ///////////////////////////////////////////////////////////
                bExplicitDataType = (transferSyntax != "1.2.840.10008.1.2");        // Implicit VR little endian

                // Explicit VR big endian
                ///////////////////////////////////////////////////////////
                endianType = (transferSyntax == "1.2.840.10008.1.2.2") ? streamController::highByteEndian : streamController::lowByteEndian;
            }

            std::shared_ptr<memoryStreamInput> dataSetStream(std::make_shared<memoryStreamInput>(datasetMemory));
            std::shared_ptr<streamReader> dataSetStreamReader(std::make_shared<streamReader>(dataSetStream));
            std::shared_ptr<dataSet> pDataset(std::make_shared<dataSet>(transferSyntax, charsetsList_t()));
            codecs::dicomStreamCodec::parseStream(dataSetStreamReader, pDataset, bExplicitDataType, endianType);

            // Return the dataset
            ///////////////////////////////////////////////////////////
            return std::make_shared<receivedDataset>(abstractSyntax, pDataset);
        }

        // Get pdata values
        ///////////////////////////////////////////////////////////
        std::shared_ptr<acsePDU> pdu(acsePDU::decodePDU(m_pReader));

        switch(pdu->getPDUType())
        {
        case acsePDU::pduType_t::aReleaseRQ:
            // release request. Send a release response and
            // throw a StreamClosedError exception
            {
                std::unique_lock<std::mutex> lock(m_lockWrite);
                std::shared_ptr<acsePDUAReleaseRP> releaseRP(std::make_shared<acsePDUAReleaseRP>());
                releaseRP->encodePDU(m_pWriter);
                IMEBRA_THROW(StreamClosedError, "The association has been released");
            }
            break;
        case acsePDU::pduType_t::aReleaseRP:
            // release response received
            IMEBRA_THROW(StreamClosedError, "The association has been released");
        case acsePDU::pduType_t::aAbort:
            // association aborted
            IMEBRA_THROW(StreamClosedError, "The association has been aborted");
        case acsePDU::pduType_t::pData:
            {

                std::shared_ptr<acsePDUPData> pData(std::static_pointer_cast<acsePDUPData>(pdu));

                // Add the pdata values to the pending pdata
                ///////////////////////////////////////////////////////////
                for(std::shared_ptr<acseItemPDataValue> pDataValue: pData->getValues())
                {
                    if(pDataValue->m_memorySize != 0)
                    {
                        pendingPData.push_back(pDataValue);
                        if(pDataValue->m_bLast)
                        {
                            ++numberOfLastPData;
                        }
                    }
                }
            }
            break;
        default:
            IMEBRA_THROW(AcseCorruptedMessageError, "Unexpected association request message (association already negotiated)");
        }

    }

    IMEBRA_FUNCTION_END();
}


void associationBase::abort(acsePDUAAbort::reason_t reason)
{
    IMEBRA_FUNCTION_START();

    if(m_bAssociated.fetch_and(0))
    {
        {
            std::unique_lock<std::mutex> lock(m_lockWrite);
            std::shared_ptr<acsePDUAAbort> abort(std::make_shared<acsePDUAAbort>(reason));
            abort->encodePDU(m_pWriter);
        }
        m_pReader->terminate();
    }

    IMEBRA_FUNCTION_END();
}

void associationBase::release()
{
    IMEBRA_FUNCTION_START();

    if(m_bAssociated.fetch_and(0))
    {
        {
            std::unique_lock<std::mutex> lock(m_lockWrite);
            std::shared_ptr<acsePDUAReleaseRQ> release(std::make_shared<acsePDUAReleaseRQ>());
            release->encodePDU(m_pWriter);
        }

        // Wait for release response
        std::unique_lock<std::mutex> lock(m_lockReadyDataSets);
        while(!m_bTerminated)
        {
            m_notifyReadyDataSets.wait(lock);
        }
    }

    IMEBRA_FUNCTION_END();
}

std::string associationBase::getThisAET() const
{
    IMEBRA_FUNCTION_START();

    return m_thisAET;

    IMEBRA_FUNCTION_END();
}

std::string associationBase::getOtherAET() const
{
    IMEBRA_FUNCTION_START();

    return m_otherAET;

    IMEBRA_FUNCTION_END();
}

std::string associationBase::getPresentationContextTransferSyntax(const std::string& abstractSyntax) const
{
    IMEBRA_FUNCTION_START();

    for(const presentationContextsIds_t::value_type& scanContexts: m_presentationContextsIds)
    {
        if(scanContexts.second.first->m_abstractSyntax == abstractSyntax)
        {
            if(scanContexts.second.second.empty())
            {
                IMEBRA_THROW(AcseNoTransferSyntaxError, "None of the proposed transfer syntax was accepted during the negotiation for the abstract syntax " << abstractSyntax);
            }
            return scanContexts.second.second;
        }
    }

    IMEBRA_THROW(AcsePresentationContextNotRequestedError, "The abstract syntax " << abstractSyntax << " was not negotiated");

    IMEBRA_FUNCTION_END();
}

std::vector<std::string> associationBase::getPresentationContextTransferSyntaxes(const std::string& abstractSyntax) const
{
    IMEBRA_FUNCTION_START();

    std::vector<std::string> transferSyntaxes;

    bool bAbstractSyntaxFound(false);
    for(const presentationContextsIds_t::value_type& scanContexts: m_presentationContextsIds)
    {
        if(scanContexts.second.first->m_abstractSyntax == abstractSyntax)
        {
            bAbstractSyntaxFound = true;
            if(!scanContexts.second.second.empty())
            {
                transferSyntaxes.push_back(scanContexts.second.second);
            }
        }
    }

    if(!transferSyntaxes.empty())
    {
        return transferSyntaxes;
    }

    if(bAbstractSyntaxFound)
    {
        IMEBRA_THROW(AcseNoTransferSyntaxError, "None of the proposed transfer syntax was accepted during the negotiation for the abstract syntax " << abstractSyntax);
    }
    else
    {
        IMEBRA_THROW(AcsePresentationContextNotRequestedError, "The abstract syntax " << abstractSyntax << " was not negotiated");
    }

    IMEBRA_FUNCTION_END();
}


void associationBase::getMessagesThread()
{
    std::shared_ptr<associationMessage> pMessage;

    std::list<std::shared_ptr<acseItemPDataValue> > pendingData;
    size_t numberOfLastPData(0);

    try
    {
        // Loop until terminated
        ///////////////////////////////////////////////////////////
        for(;;)
        {
            std::shared_ptr<receivedDataset> pReceivedDataset(decodePDU(pMessage == nullptr, pendingData, numberOfLastPData));

            if(pReceivedDataset->m_pDataset->bufferExists(0, 0, 0x100, 0))
            {
                {
                    std::unique_lock<std::mutex> lockCommandsResponses(m_lockCommandsResponses);

                    // We received a command or response dataset
                    ///////////////////////////////////////////////////////////
                    if( (pReceivedDataset->m_pDataset->getUnsignedLong(0x0, 0, 0x100, 0, 0, 0) & 0x00008000) != 0)
                    {
                        // We received a response
                        ///////////////////////////////////////////////////////////

                        // Check for a partial response
                        ///////////////////////////////////////////////////////////
                        if( (pReceivedDataset->m_pDataset->getUnsignedLong(0x0, 0, 0x900, 0, 0, 0) & 0x0000fff0) == 0xff00)
                        {
                            // We received a partial response
                            ///////////////////////////////////////////////////////////
                            if(m_waitingResponses.find(pReceivedDataset->m_pDataset->getUnsignedLong(0, 0, 0x0120, 0, 0)) == m_waitingResponses.end())
                            {
                                abort(acsePDUAAbort::reason_t::serviceProviderInvalidPDUParameterValue);
                                IMEBRA_THROW(AcseWrongResponseIdError, "Received a partial response with a wrong ID");
                            }
                        }
                        else if(m_waitingResponses.erase(pReceivedDataset->m_pDataset->getUnsignedLong(0, 0, 0x0120, 0, 0)) == 0)
                        {
                            abort(acsePDUAAbort::reason_t::serviceProviderInvalidPDUParameterValue);
                            IMEBRA_THROW(AcseWrongResponseIdError, "Received a response with a wrong ID");
                        }
                    }
                    else
                    {
                        // We received a command (not cancel)
                        ///////////////////////////////////////////////////////////
                        if(pReceivedDataset->m_pDataset->getUnsignedLong(0x0, 0, 0x100, 0, 0, 0) != 0x0fff)
                        {
                            if(m_processingCommands.count(pReceivedDataset->m_pDataset->getUnsignedLong(0, 0, 0x0110, 0, 0)) != 0)
                            {
                                abort(acsePDUAAbort::reason_t::serviceProviderInvalidPDUParameterValue);
                                IMEBRA_THROW(AcseWrongCommandIdError, "Received a command with an ID from a command still being processed");
                            }
                            if(m_maxOperationsPerformed != 0 && m_processingCommands.size() == m_maxOperationsPerformed)
                            {
                                IMEBRA_THROW(AcseTooManyOperationsPerformedError, "Performing too many operations (max is " << m_maxOperationsPerformed << ")");
                            }
                            m_processingCommands.insert(pReceivedDataset->m_pDataset->getUnsignedLong(0, 0, 0x0110, 0, 0));
                        }
                    }
                }

                // We already have an incomplete message for which
                // the payload hasn't arrived yet
                ///////////////////////////////////////////////////////////
                if(pMessage != nullptr)
                {
                    abort(acsePDUAAbort::reason_t::serviceProviderUnexpectedPDU);
                    IMEBRA_THROW(AcseCorruptedMessageError, "Payload expected");
                }

                // Create the new message
                ///////////////////////////////////////////////////////////
                pMessage = std::make_shared<associationMessage>(pReceivedDataset->m_presentationContext, pReceivedDataset->m_pDataset);

            }
            else
            {
                // We received the payload
                ///////////////////////////////////////////////////////////
                if(pMessage == nullptr)
                {
                    abort(acsePDUAAbort::reason_t::serviceProviderUnexpectedPDU);
                    IMEBRA_THROW(AcseCorruptedMessageError, "Payload received before a command");
                }
                if(pMessage->getAbstractSyntax() != pReceivedDataset->m_presentationContext)
                {
                    abort(acsePDUAAbort::reason_t::serviceProviderInvalidPDUParameterValue);
                    IMEBRA_THROW(AcseCorruptedMessageError, "The payload has an abstract syntax different from the command");
                }
                pMessage->addDataset(pReceivedDataset->m_pDataset);
            }

            // If the message is complete then add it to the list of
            // received messages
            if(pMessage != nullptr && pMessage->isComplete())
            {
                std::unique_lock<std::mutex> lock(m_lockReadyDataSets);
                m_readyDataSets.push_back(pMessage);
                pMessage.reset();
                m_notifyReadyDataSets.notify_all();
            }

        }
    }
    catch(const StreamEOFError&)
    {
        // Set the terminated flag, release current getMessage()
        // operations
        std::unique_lock<std::mutex> lock(m_lockReadyDataSets);
        m_bTerminated = true;
        m_notifyReadyDataSets.notify_all();
    }

}

associationSCU::associationSCU(
        const std::shared_ptr<const presentationContexts>& contexts,
        const std::string& thisAET,
        const std::string& otherAET,
        std::uint32_t maxOperationsWeInvoke,
        std::uint32_t maxOperationsWeCanPerform,
        std::shared_ptr<streamReader> pReader,
        std::shared_ptr<streamWriter> pWriter,
        std::uint32_t dimseTimeout):
    associationBase(role_t::scu, thisAET, otherAET, maxOperationsWeInvoke, maxOperationsWeCanPerform, pReader, pWriter, dimseTimeout)

{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("-- Starting SCU association negotiation");

    acsePDUAssociateRQ::presentationContexts_t contextsPDUs;
    acseItemUserInformation::scpScuSelectionList_t scpScuRoles;
    typedef std::map<std::string, std::shared_ptr<acseItemSCPSCURoleSelection> > scpScuRolesMap_t;
    scpScuRolesMap_t scpScuRolesMap;

    // Assign an ID to the presentation contexts
    // (ID starts from 1, only odd numbers)
    ///////////////////////////////////////////////////////////
    std::uint8_t contextId(1);
    presentationContexts contextsCopy(*contexts);
    for(const std::shared_ptr<presentationContext>& context: contextsCopy.m_presentationContexts)
    {
        contextsPDUs.push_back(std::make_shared<acseItemPresentationContextRQ>(contextId, context->m_abstractSyntax, context->m_proposedTransferSyntaxes));
        m_presentationContextsIds[contextId] = std::pair<std::shared_ptr<presentationContext>, std::string>(context, "");
        contextId = (std::uint8_t)(contextId + 2);

        // If the role is not the default one (only SCU) then
        // send also a SCP/SCU role selection item
        ///////////////////////////////////////////////////////////
        scpScuRolesMap_t::iterator findScpScuRole = scpScuRolesMap.find(context->m_abstractSyntax);
        if(findScpScuRole == scpScuRolesMap.end())
        {
            std::shared_ptr<acseItemSCPSCURoleSelection> role(std::make_shared<acseItemSCPSCURoleSelection>(
                                                                  context->m_abstractSyntax,
                                                                  context->m_bRequestorIsSCU,
                                                                  context->m_bRequestorIsSCP));
            scpScuRolesMap[context->m_abstractSyntax] = role;
        }
        else
        {
            std::shared_ptr<acseItemSCPSCURoleSelection> role(std::make_shared<acseItemSCPSCURoleSelection>(
                                                                  context->m_abstractSyntax,
                                                                  findScpScuRole->second->getSCU() || context->m_bRequestorIsSCU,
                                                                  findScpScuRole->second->getSCP() || context->m_bRequestorIsSCP));
            findScpScuRole->second = role;
        }
    }
    for(const auto& requestedRole: scpScuRolesMap)
    {
        if(!requestedRole.second->getSCU() || requestedRole.second->getSCP())
        {
            scpScuRoles.push_back(requestedRole.second);
        }
    }

    const std::string implementationUid(IMEBRA_IMPLEMENTATION_CLASS_UID);
    const std::string implementationName(IMEBRA_IMPLEMENTATION_NAME);
    std::shared_ptr<acseItemUserInformation> pUserInformation(
                std::make_shared<acseItemUserInformation>(
                    m_maxPDULength,
                    implementationUid,
                    implementationName,
                    m_maxOperationsInvoked,
                    m_maxOperationsPerformed,
                    scpScuRoles));

    acsePDUAssociateRQ associationRequest(m_thisAET, m_otherAET, m_applicationContext, contextsPDUs, pUserInformation);

    // Send the association request PDU
    ///////////////////////////////////////////////////////////
    associationRequest.encodePDU(m_pWriter);

    // Wait for the response PDU
    ///////////////////////////////////////////////////////////
    std::shared_ptr<acsePDU> response(acsePDU::decodePDU(m_pReader));

    // Any response other than AssociateAC means that the
    // association failed
    ///////////////////////////////////////////////////////////
    switch(response->getPDUType())
    {
    case acsePDU::pduType_t::associateAC:
    {
        // Cast the response PDU to accepted association
        ///////////////////////////////////////////////////////////
        std::shared_ptr<acsePDUAssociateAC> responseAC(std::dynamic_pointer_cast<acsePDUAssociateAC>(response));

        // Get the max PDU length accepted by the SCP
        ///////////////////////////////////////////////////////////
        std::shared_ptr<acseItemUserInformation> pResponseUserInformation(responseAC->getItemUserInformation());
        m_maxPDULength = pResponseUserInformation->getMaximumPDULength();

        // Fix the maximum number of operations invoked/performed
        ///////////////////////////////////////////////////////////
        std::uint16_t responseMaxInvoked(pResponseUserInformation->getMaxOperationsInvoked());
        if(responseMaxInvoked != 0 && responseMaxInvoked < m_maxOperationsInvoked)
        {
            m_maxOperationsInvoked = responseMaxInvoked;
        }
        std::uint16_t responseMaxPerformed(pResponseUserInformation->getMaxOperationsPerformed());
        if(responseMaxPerformed != 0 && responseMaxPerformed < m_maxOperationsPerformed)
        {
            m_maxOperationsPerformed = responseMaxPerformed;
        }

        // Put the accepted scp/scu roles in a map
        ///////////////////////////////////////////////////////////
        typedef std::map<std::string, std::shared_ptr<acseItemSCPSCURoleSelection> > scpScuRolesMap_t;
        scpScuRolesMap_t acceptedScpScuRoles;
        for(const std::shared_ptr<acseItemSCPSCURoleSelection> acceptedRole: pResponseUserInformation->getScpScuRoles())
        {
            acceptedScpScuRoles[acceptedRole->getSopClassUID()] = acceptedRole;
        }

        // Get the accepted presentation contexts
        ///////////////////////////////////////////////////////////
        for(const std::shared_ptr<acseItemPresentationContextBase>& context: responseAC->getPresentationContexts())
        {
            std::uint8_t id(context->getId());
            presentationContextsIds_t::iterator findContext(m_presentationContextsIds.find(id));
            if(findContext == m_presentationContextsIds.end())
            {
                // The returned presentation context wasn't in the
                // requested ones
                ///////////////////////////////////////////////////////////
                IMEBRA_THROW(AcseCorruptedMessageError, "Presentation context " << id << " was not proposed");
            }

            // Check supported transfer syntaxes
            ///////////////////////////////////////////////////////////
            acseItemPresentationContextBase::transferSyntaxes_t syntaxes(context->getTransferSyntaxes());
            if(syntaxes.size() != 0)
            {
                if(std::find(
                            findContext->second.first->m_proposedTransferSyntaxes.begin(),
                            findContext->second.first->m_proposedTransferSyntaxes.end(),
                            syntaxes.front()) != findContext->second.first->m_proposedTransferSyntaxes.end())
                {
                    findContext->second.second = syntaxes.front();

                    // Check the scp/scu role
                    scpScuRolesMap_t::const_iterator findRole(acceptedScpScuRoles.find(findContext->second.first->m_abstractSyntax));
                    if(findRole == acceptedScpScuRoles.end())
                    {
                        findContext->second.first->m_bRequestorIsSCP = false;
                    }
                    else
                    {
                        findContext->second.first->m_bRequestorIsSCU &= findRole->second->getSCU();
                        findContext->second.first->m_bRequestorIsSCP &= findRole->second->getSCP();
                    }
                }
                else
                {
                    //responded with a not supported transfer syntax
                    ///////////////////////////////////////////////////////////
                    IMEBRA_THROW(AcseCorruptedMessageError, "Unsupported transfer syntax " << syntaxes.front() << " for ID " << id);
                }
            }
            else if(syntaxes.size() > 1)
            {
                // Only one transfer syntax can be accepted
                ///////////////////////////////////////////////////////////
                IMEBRA_THROW(AcseCorruptedMessageError, "Only one transfer syntax can be accepted");
            }
        }

        break;
    }
    case acsePDU::pduType_t::associateRJ:
    {
        // Cast the response PDU to rejected association
        ///////////////////////////////////////////////////////////
        std::shared_ptr<acsePDUAssociateRJ> responseRJ(std::dynamic_pointer_cast<acsePDUAssociateRJ>(response));

        const bool bPermanent(responseRJ->getResult() == acsePDUAssociateRJ::result_t::rejectedPermanent);

        switch(responseRJ->getReason())
        {
        case acsePDUAssociateRJ::reason_t::serviceUserNoReasonGiven:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUNoReasonGivenError, "Service user no reason given", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceUserApplicationContextNameNotSupported:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUApplicationContextNameNotSupportedError, "Application context " << m_applicationContext << " name not supported", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceUserCallingAETitleNotRecognized:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUCallingAETNotRecognizedError, "Calling AET " << thisAET << " not recognized", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceUserCalledAETitleNotRecognized:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUCalledAETNotRecognizedError, "Called AET " << otherAET << " not recognized", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceProviderAcseNoReasonGiven:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCPNoReasonGivenError, "Service provider no reason given", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceProviderAcseProtocolVersionNotSupported:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCPAcseProtocolVersionNotSupportedError, "Protocol version not supported", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceProviderPresentationReserved:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCPPresentationReservedError, "Presentation reserved", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceProviderPresentationTemporaryCongestion:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCPPresentationTemporaryCongestionError, "Temporary congestion", bPermanent);
        case acsePDUAssociateRJ::reason_t::serviceProviderPresentationLocalLimitExceded:
            IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCPPresentationLocalLimitExcededError, "Presentation local limit exceeded", bPermanent);
        }

        IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUNoReasonGivenError, "Service user no reason given", bPermanent);

    }
    default:
        IMEBRA_THROW(AcseCorruptedMessageError, "Unexpected message type");
    }

    IMEBRA_LOG_INFO("-- Terminated SCU association negotiation");

    m_readDataSetsThread.reset(new std::thread(&associationBase::getMessagesThread, this));

    IMEBRA_FUNCTION_END();
}


associationSCP::associationSCP(
        const std::shared_ptr<const presentationContexts>& contexts,
        const std::string& thisAET,
        std::uint32_t maxOperationsWeInvoke,
        std::uint32_t maxOperationsWeCanPerform,
        std::shared_ptr<streamReader> pReader,
        std::shared_ptr<streamWriter> pWriter,
        std::uint32_t dimseTimeout,
        std::uint32_t artimTimeoutSeconds):
    associationBase(role_t::scp, thisAET, "", maxOperationsWeInvoke, maxOperationsWeCanPerform, pReader, pWriter, dimseTimeout)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_LOG_INFO("-- Starting SCP association negotiation");

    // Wait for association request PDU
    ///////////////////////////////////////////////////////////
    std::shared_ptr<acsePDU> request;
    {
        streamTimeout artim(pReader->getControlledStream(), std::chrono::seconds(artimTimeoutSeconds));
        request = acsePDU::decodePDU(m_pReader);

        if(request->getPDUType() != acsePDU::pduType_t::associateRQ)
        {
            IMEBRA_THROW(AcseCorruptedMessageError, "Unexpected message type (was expecting an association request)");
        }
    }

    // Cast received PDU to association request
    ///////////////////////////////////////////////////////////
    std::shared_ptr<acsePDUAssociateRQ> associationRQ(std::dynamic_pointer_cast<acsePDUAssociateRQ>(request));

    // Only the DICOM syntax is supported: reject if
    // different
    ///////////////////////////////////////////////////////////
    if(associationRQ->getApplicationContext()->getName() != m_applicationContext)
    {
        std::shared_ptr<acsePDUAssociateRJ> reject(
                    std::make_shared<acsePDUAssociateRJ>(
                        acsePDUAssociateRJ::result_t::rejectedPermanent,
                        acsePDUAssociateRJ::reason_t::serviceUserApplicationContextNameNotSupported));
        reject->encodePDU(m_pWriter);
        IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUApplicationContextNameNotSupportedError, "Application context " << m_applicationContext << " not supported", true);
    }

    // Check the requested AET. Reject if different from
    // the SCP AET
    ///////////////////////////////////////////////////////////
    std::string requestedAET(associationRQ->getCalledAETitle());
    if(!m_thisAET.empty() && m_thisAET != requestedAET)
    {
        std::shared_ptr<acsePDUAssociateRJ> reject(
                    std::make_shared<acsePDUAssociateRJ>(
                        acsePDUAssociateRJ::result_t::rejectedPermanent,
                        acsePDUAssociateRJ::reason_t::serviceUserCalledAETitleNotRecognized));
        reject->encodePDU(m_pWriter);
        IMEBRA_THROW_ADDITIONAL_PARAM(AcseSCUCalledAETNotRecognizedError, "SCP AET is " << thisAET << " but " << requestedAET << " was requested instead", true);
    }

    // Get the SCU AET
    ///////////////////////////////////////////////////////////
    m_otherAET = associationRQ->getCallingAETitle();

    // Get the SCU maximum PDU size
    ///////////////////////////////////////////////////////////
    std::shared_ptr<acseItemUserInformation> pUserInformation(associationRQ->getItemUserInformation());
    m_maxPDULength = pUserInformation->getMaximumPDULength();

    // Get the operations invoked and performed
    ///////////////////////////////////////////////////////////
    std::uint16_t requestedMaxInvoked(pUserInformation->getMaxOperationsInvoked());
    if(requestedMaxInvoked != 0 && requestedMaxInvoked < m_maxOperationsPerformed)
    {
        m_maxOperationsPerformed = requestedMaxInvoked;
    }
    std::uint16_t requestedMaxPerformed(pUserInformation->getMaxOperationsPerformed());
    if(requestedMaxPerformed != 0 && requestedMaxPerformed < m_maxOperationsInvoked)
    {
        m_maxOperationsInvoked = requestedMaxPerformed;
    }

    // Put the requested scp/scu roles in a map
    ///////////////////////////////////////////////////////////
    typedef std::map<std::string, std::shared_ptr<acseItemSCPSCURoleSelection> > scpScuRolesMap_t;
    scpScuRolesMap_t requestedScpScuRoles;
    for(const std::shared_ptr<acseItemSCPSCURoleSelection> requestedRole: pUserInformation->getScpScuRoles())
    {
        requestedScpScuRoles[requestedRole->getSopClassUID()] = requestedRole;
    }

    // Match the requested presentation contexts with the ones
    // we support
    ///////////////////////////////////////////////////////////
    acsePDUAssociateAC::presentationContexts_t acceptedContexts;
    scpScuRolesMap_t scpScuRoles;

    // Scan all the requested presentation contexts
    for(const std::shared_ptr<acseItemPresentationContextBase>& context: associationRQ->getPresentationContexts())
    {
        std::uint8_t id(context->getId());

        // Build a presentationContext object for the requested presentation context
        std::shared_ptr<presentationContext> pPresentationContext(std::make_shared<presentationContext>(context->getAbstractSyntax()));
        for(const std::string& transferSyntax: context->getTransferSyntaxes())
        {
            pPresentationContext->addTransferSyntax(transferSyntax);
        }

        // Store the created presentation context with its ID
        m_presentationContextsIds[id] =
                std::pair<std::shared_ptr<presentationContext>, std::string>
                (pPresentationContext, "");

        // Check if we support the context and one of the transfer
        //  syntaxes
        ///////////////////////////////////////////////////////////
        bool bAbstractSyntaxSupported(false);
        for(const std::shared_ptr<const presentationContext> pFindContext: contexts->m_presentationContexts)
        {
            if(pFindContext->m_abstractSyntax == pPresentationContext->m_abstractSyntax)
            {
                bAbstractSyntaxSupported = true;

                // We support the abstract syntax, now look for the transfer syntaxes
                for(const std::string& scpTransferSyntax: m_presentationContextsIds[id].first->m_proposedTransferSyntaxes)
                {
                    if(std::find(pFindContext->m_proposedTransferSyntaxes.begin(), pFindContext->m_proposedTransferSyntaxes.end(), scpTransferSyntax) != pFindContext->m_proposedTransferSyntaxes.end())
                    {
                        m_presentationContextsIds[id].second = scpTransferSyntax;

                        scpScuRolesMap_t::const_iterator findRole(requestedScpScuRoles.find(pPresentationContext->m_abstractSyntax));
                        if(findRole != requestedScpScuRoles.end())
                        {
                            pPresentationContext->m_bRequestorIsSCU = pFindContext->m_bRequestorIsSCU && findRole->second->getSCU();
                            pPresentationContext->m_bRequestorIsSCP = pFindContext->m_bRequestorIsSCP && findRole->second->getSCP();
                            scpScuRolesMap_t::iterator declaredScpScuRole = scpScuRoles.find(pPresentationContext->m_abstractSyntax);
                            if(declaredScpScuRole != scpScuRoles.end())
                            {
                                std::shared_ptr<acseItemSCPSCURoleSelection> updateScpScuRole = std::make_shared<acseItemSCPSCURoleSelection>(
                                                pPresentationContext->m_abstractSyntax,
                                                pPresentationContext->m_bRequestorIsSCU || declaredScpScuRole->second->getSCU(),
                                                pPresentationContext->m_bRequestorIsSCP || declaredScpScuRole->second->getSCP());
                                scpScuRoles[pPresentationContext->m_abstractSyntax] = updateScpScuRole;
                            }
                            else
                            {
                                scpScuRoles[pPresentationContext->m_abstractSyntax] = std::make_shared<acseItemSCPSCURoleSelection>(
                                                pPresentationContext->m_abstractSyntax,
                                                pPresentationContext->m_bRequestorIsSCU,
                                                pPresentationContext->m_bRequestorIsSCP);

                            }
                        }
                        break;
                    }
                }
            }
        }

        if(!m_presentationContextsIds[id].second.empty())
        {
            acceptedContexts.push_back(
                        std::make_shared<acseItemPresentationContextAC>(
                            id,
                            acseItemPresentationContextAC::result_t::acceptance,
                            m_presentationContextsIds[id].second));
        }
        else if(bAbstractSyntaxSupported)
        {
            acceptedContexts.push_back(
                        std::make_shared<acseItemPresentationContextAC>(
                            id,
                            acseItemPresentationContextAC::result_t::transferSyntaxesNotSupported,
                            ""));
        }
        else
        {
            acceptedContexts.push_back(
                        std::make_shared<acseItemPresentationContextAC>(
                            id,
                            acseItemPresentationContextAC::result_t::abstractSyntaxNotSupported,
                            ""));
        }
    }

    acseItemUserInformation::scpScuSelectionList_t scpScuRolesList;
    for(const auto& scpScuRole: scpScuRoles)
    {
        if(!scpScuRole.second->getSCU() || scpScuRole.second->getSCP())
        {
            scpScuRolesList.push_back(scpScuRole.second);
        }
    }
    std::shared_ptr<acseItemUserInformation> pUserInformationAC(std::make_shared<acseItemUserInformation>(
                                                                    m_maxPDULength,
                                                                    IMEBRA_IMPLEMENTATION_CLASS_UID,
                                                                    IMEBRA_IMPLEMENTATION_NAME,
                                                                    m_maxOperationsPerformed,
                                                                    m_maxOperationsInvoked,
                                                                    scpScuRolesList));
    std::shared_ptr<acsePDUAssociateAC> responseAC(std::make_shared<acsePDUAssociateAC>(
                                                       associationRQ->getCalledAETitle(),
                                                       associationRQ->getCallingAETitle(),
                                                       m_applicationContext,
                                                       acceptedContexts,
                                                       pUserInformationAC));
    responseAC->encodePDU(m_pWriter);

    IMEBRA_LOG_INFO("-- Terminated SCP association negotiation");

    m_readDataSetsThread.reset(new std::thread(&associationBase::getMessagesThread, this));

    IMEBRA_FUNCTION_END_MODIFY(CodecCorruptedFileError, AcseCorruptedMessageError);
}



}

}
