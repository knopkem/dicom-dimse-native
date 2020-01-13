/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/


/*! \file acseImpl.h
    \brief Defines the classes that implement the ACSE

*/

#if !defined(CImbxAcse_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
#define CImbxAcse_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_

#include <functional>
#include <string>
#include <memory>
#include <list>
#include <vector>
#include <set>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "configurationImpl.h"
#include "streamReaderImpl.h"

namespace imebra
{

namespace implementation
{

class streamWriter;
class streamReader;
class memory;
class dataSet;

///
/// @brief Base class for the Items encoded in the ACSE messages
///
//////////////////////////////////////////////////////////////////
class acseItem
{
public:
    ///
    /// @brief Enumerates the items embedded into PDUs
    ///
    //////////////////////////////////////////////////////////////////
    enum class itemType_t: std::uint8_t
    {
        applicationContext = 0x10,            ///< Application context item
        presentationContextRQ = 0x20,         ///< Presentation context request item
        presentationContextAC = 0x21,         ///< Presentation context acceptance item
        abstractSyntax = 0x30,                ///< Abstract syntax item
        transferSyntax = 0x40,                ///< Transfer syntax item
        userInformation = 0x50,               ///< User information item

        // User information sub items
        maximumLength = 0x51,                 ///< PDU maximum length subitem
        implementationClassUID = 0x52,        ///< Software implementationclass UID subitem
        implementationVersionName = 0x55,     ///< Software implementation version name subitem
        asynchronousOperationsWindow = 0x53,  ///< Asynchronous operations windows subitem
        SCPSCURoleSelection = 0x54,           ///< SCP/SCU role selection subitem
        //SOPClassExtendedNegotiation = 0x56,
        //SOPClassCommonExtendedNegotiation = 0x57,
        //userIdentityNegotiation = 0x58,

        // Part of user Identity negotiation
        //userIdentityNegotiationField = 0x59

    };

    virtual ~acseItem();

    ///
    /// \brief Decode an item from a streamReader
    ///
    /// \param pReader the streamReader from which the item is read
    /// \return the decoded item
    ///
    //////////////////////////////////////////////////////////////////
    static std::shared_ptr<acseItem> decodeItem(std::shared_ptr<streamReader> pReader);

    ///
    /// \brief Encode the item into a streamWriter
    ///
    /// \param pWriter the streamWriter into which the item is written
    ///
    //////////////////////////////////////////////////////////////////
    void encodeItem(std::shared_ptr<streamWriter> pWriter) const;

    ///
    /// \brief Returns the item's type
    ///
    /// \return the item's type
    ///
    //////////////////////////////////////////////////////////////////
    virtual itemType_t getItemType() const = 0;

protected:
    ///
    /// \brief Encode the item and write it into the streamWriter.
    ///
    /// \param pWriter the writer into which the item is encoded
    ///
    //////////////////////////////////////////////////////////////////
    virtual void encodeItemPayload(std::shared_ptr<streamWriter> pWriter) const = 0;

    ///
    /// \brief Decode an item from data received from a streamReader.
    ///
    /// \param pReader the streamReader used to read the data
    ///
    //////////////////////////////////////////////////////////////////
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) = 0;

};


///
/// \brief Base class for the items that read a name (UID).
///
//////////////////////////////////////////////////////////////////
class acseItemName: public acseItem
{
public:
    ///
    /// \brief Default constructor.
    ///
    //////////////////////////////////////////////////////////////////
    acseItemName();

    ///
    /// \brief Constructor.
    /// \param name the uid to store in the item
    ///
    //////////////////////////////////////////////////////////////////
    acseItemName(const std::string& name);

    ///
    /// \brief Return the uid stored in the item
    /// \return the uid stored in the item
    ///
    //////////////////////////////////////////////////////////////////
    const std::string& getName() const;

protected:
    virtual void encodeItemPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::string m_name;
};


///
/// \brief Acse item for the Application Context UID.
///
//////////////////////////////////////////////////////////////////
class acseItemApplicationContext: public acseItemName
{
public:
    using acseItemName::acseItemName;

    virtual itemType_t getItemType() const override;
};


///
/// \brief Acse item for the Abstract Syntax UID.
///
//////////////////////////////////////////////////////////////////
class acseItemAbstractSyntax: public acseItemName
{
public:
    using acseItemName::acseItemName;

    virtual itemType_t getItemType() const override;
};


///
/// \brief Acse item for the transfer syntax.
///
//////////////////////////////////////////////////////////////////
class acseItemTransferSyntax: public acseItemName
{
public:
    using acseItemName::acseItemName;

    virtual itemType_t getItemType() const override;
};


///
/// \brief Acse item for transferring the class UID
///
//////////////////////////////////////////////////////////////////
class acseItemImplementationClassUID: public acseItemName
{
public:
    using acseItemName::acseItemName;

    virtual itemType_t getItemType() const override;
};


///
/// \brief Acse item for transferring the version name
///
//////////////////////////////////////////////////////////////////
class acseItemImplementationVersionName: public acseItem
{
public:
    acseItemImplementationVersionName();
    acseItemImplementationVersionName(const std::string& name);

    virtual itemType_t getItemType() const override;

    ///
    /// \brief Return the version name stored in the item
    /// \return the version name stored in the item
    ///
    //////////////////////////////////////////////////////////////////
    const std::string& getName() const;

protected:

    virtual void encodeItemPayload(std::shared_ptr<streamWriter> pWriter) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::string m_name;
};


///
/// \brief Acse item for the PDU max length.
///
//////////////////////////////////////////////////////////////////
class acseItemMaxLength: public acseItem
{
public:
    acseItemMaxLength();

    ///
    /// \brief Constructor.
    /// \param maxLength the maximum PDU length to store in the item
    ///
    //////////////////////////////////////////////////////////////////
    acseItemMaxLength(std::uint32_t maxLength);

    ///
    /// \brief Returns the maximum PDU length stored in the item.
    /// \return the maximum PDU length
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getMaxLength() const;

    virtual itemType_t getItemType() const override;

protected:
    virtual void encodeItemPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::uint32_t m_maxLength;
};


///
/// \brief Acse item for the maximum number of overlapped
///        operations.
///
//////////////////////////////////////////////////////////////////
class acseItemAsynchronousOperationsWindow: public acseItem
{
public:
    acseItemAsynchronousOperationsWindow();

    ///
    /// \brief Constructor.
    ///
    /// \param maxOperationsInvoked   maximum number of overlapped
    ///                               operations that the service will
    ///                               invoke
    /// \param maxOperationsPerformed maximum number of overlapped
    ///                               operations that the service can
    ///                               perform
    ///
    //////////////////////////////////////////////////////////////////
    acseItemAsynchronousOperationsWindow(std::uint16_t maxOperationsInvoked, std::uint16_t maxOperationsPerformed);

    ///
    /// \brief Return the maximum number of overlapped operations that
    ///        the service will invoke.
    ///
    /// \return the maximum number of overlapped operations that the
    ///         service will invoke
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getMaxOperationsInvoked() const;

    ///
    /// \brief Return the maximum number of overlapped operations that
    ///        the service can perform.
    ///
    /// \return the maximum number of overlapped operations that the
    ///         service can perform
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getMaxOperationsPerformed() const;

    virtual itemType_t getItemType() const override;

protected:
    virtual void encodeItemPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::uint16_t m_maxOperationsInvoked;
    std::uint16_t m_maxOperationsPerformed;

};


///
/// \brief Acse item for the SCP/SCU role selection.
///
//////////////////////////////////////////////////////////////////
class acseItemSCPSCURoleSelection: public acseItem
{
public:
    acseItemSCPSCURoleSelection();

    ///
    /// \brief Constructor.
    ///
    /// \param sopClassUID UID for which the role is being negotiated
    /// \param bSCU        true if the service acts as an SCU
    /// \param bSCP        true if the service acts as an SCP
    ///
    //////////////////////////////////////////////////////////////////
    acseItemSCPSCURoleSelection(const std::string& sopClassUID, bool bSCU, bool bSCP);

    ///
    /// \brief Returns to presentation context UID.
    ///
    /// \return the presentation context UID
    ///
    //////////////////////////////////////////////////////////////////
    std::string getSopClassUID() const;

    ///
    /// \brief Returns true if the association initiator acts as
    ///        SCU for the specified presentation context.
    ///
    /// \return true if the association initiator acts as an SCU
    ///         for the presentation context, false otherwise
    ///
    //////////////////////////////////////////////////////////////////
    bool getSCU() const;

    ///
    /// \brief Returns true if the association initiator acts as
    ///        SCP for the specified presentation context.
    ///
    /// \return true if the association initiator acts as an SCP
    ///         for the presentation context, false otherwise
    ///
    //////////////////////////////////////////////////////////////////
    bool getSCP() const;

    virtual itemType_t getItemType() const override;

protected:
    virtual void encodeItemPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::string m_sopClassUID;
    bool m_bSCU;
    bool m_bSCP;
};


///
/// \brief Acse item for the user information.
///
//////////////////////////////////////////////////////////////////
class acseItemUserInformation: public acseItem
{
public:
    acseItemUserInformation();

    ///
    /// \brief List of SCU/SCP role selection items.
    ///
    //////////////////////////////////////////////////////////////////
    typedef std::list<std::shared_ptr<acseItemSCPSCURoleSelection> > scpScuSelectionList_t;

    ///
    /// \brief Constructor.
    ///
    /// \param maximumLength             maximum supported PDU length
    /// \param implementationClassUID    implementation class UID
    /// \param implementationVersionName implementation version
    /// \param maxOperationsInvoked      maximum number of
    ///                                  simultaneous operations
    ///                                  invoked
    /// \param maxOperationsPerformed    maximum number of
    ///                                  simultaneous operations that
    ///                                  can be performed
    /// \param scpScuRoles               list of SCU/SCP role
    ///                                  selection items
    ///
    //////////////////////////////////////////////////////////////////
    acseItemUserInformation(
            std::uint32_t maximumLength,
            const std::string& implementationClassUID,
            const std::string& implementationVersionName,
            std::uint16_t maxOperationsInvoked,
            std::uint16_t maxOperationsPerformed,
            const scpScuSelectionList_t& scpScuRoles);

    ///
    /// \brief Get the maximum PDU length.
    ///
    /// \return the maximum PDU length
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getMaximumPDULength() const;

    ///
    /// \brief Get the maximum number of simultaneous operations
    ///        invoked.
    ///
    /// \return the maximum number of simultaneous operations
    ///         invoked. 0 means no limits
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getMaxOperationsInvoked() const;

    ///
    /// \brief Get the maximum number of simultaneous operations that
    ///        can be performed.
    ///
    /// \return the maximum number of simultaneous operations that
    ///         can be performed. 0 means no limits
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getMaxOperationsPerformed() const;

    ///
    /// \brief Get the list of SCU/SCP role selection items.
    ///
    /// \return the list of SCU/SCP role selection items
    ///
    //////////////////////////////////////////////////////////////////
    const scpScuSelectionList_t& getScpScuRoles();

    virtual itemType_t getItemType() const override;

protected:
    virtual void encodeItemPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::shared_ptr<acseItemMaxLength> m_maximumLength;
    std::shared_ptr<acseItemImplementationClassUID> m_implementationClassUID;
    std::shared_ptr<acseItemAsynchronousOperationsWindow> m_asyncOperationsWindow;
    std::shared_ptr<acseItemImplementationVersionName> m_implementationVersionName;
    scpScuSelectionList_t m_scuScpRoleSelections;
};


///
/// \brief Base class for the presentation context item.
///
//////////////////////////////////////////////////////////////////
class acseItemPresentationContextBase: public acseItem
{
public:

    ///
    /// \brief Presentation context status.
    ///
    //////////////////////////////////////////////////////////////////
    enum class result_t: std::uint8_t
    {
        acceptance = 0,                  ///< Accepted
        userRejection = 1,               ///< Rejected by upper layer
        noReason = 2,                    ///< Rejected (no reason given)
        abstractSyntaxNotSupported = 3,  ///< Rejected (abstract syntax not supported)
        transferSyntaxesNotSupported = 4 ///< Rejected (transfer syntax not supported)
    };

    acseItemPresentationContextBase();

    ///
    /// \brief List of transfer syntaxes, in order of preference
    ///        (preferred transfer syntax first).
    ///
    /// When the presentation context is used in an acceptance
    /// message then can be empty or can contain only one
    /// transfer syntax.
    ///
    //////////////////////////////////////////////////////////////////
    typedef std::list<std::string> transferSyntaxes_t;

    ///
    /// \brief Returns the abstract syntax.
    ///
    /// \return the abstract syntax
    ///
    //////////////////////////////////////////////////////////////////
    const std::string& getAbstractSyntax() const;

    ///
    /// \brief Return the presentation context ID (an odd number
    ///        starting from 1).
    ///
    /// \return the presentation context ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint8_t getId() const;

    ///
    /// \brief Return the presentation context status (relevant only
    ///        for association acceptance messages).
    ///
    /// \return the presentation context status
    ///
    //////////////////////////////////////////////////////////////////
    result_t getResult() const;

    ///
    /// \brief Return a list of transfer syntaxes.
    ///
    /// For acceptance message can be either empty or contain just
    /// one item.
    ///
    /// \return the list of transfer syntaxes
    ///
    //////////////////////////////////////////////////////////////////
    transferSyntaxes_t getTransferSyntaxes() const;

protected:
    acseItemPresentationContextBase(std::uint8_t id, const std::string& abstractSyntax, const transferSyntaxes_t& transferSyntaxes);
    acseItemPresentationContextBase(std::uint8_t id, result_t result, const std::string& transferSyntax);

    virtual void encodeItemPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodeItemPayload(std::shared_ptr<streamReader> pReader) override;

    std::uint8_t m_id;

    result_t m_result;

    bool m_bCheckResultCorrectness;

    std::shared_ptr<acseItemAbstractSyntax> m_pAbstractSyntax;

    typedef std::list<std::shared_ptr<acseItemTransferSyntax> > transferSyntaxesList_t;
    transferSyntaxesList_t m_transferSyntaxes;
};


///
/// \brief Acse item for the presentation context (used for
///        the association request).
///
//////////////////////////////////////////////////////////////////
class acseItemPresentationContextRQ: public acseItemPresentationContextBase
{
public:
    acseItemPresentationContextRQ();

    ///
    /// \brief Constructor.
    ///
    /// \param id               the presentation context ID (an odd
    ///                         number starting from 1)
    /// \param abstractSyntax   abstract syntax
    /// \param transferSyntaxes supported transfer syntaxes
    ///
    //////////////////////////////////////////////////////////////////
    acseItemPresentationContextRQ(std::uint8_t id, const std::string& abstractSyntax, const transferSyntaxes_t& transferSyntaxes);

    virtual itemType_t getItemType() const override;
};


///
/// \brief Acse item for the presentation context (used for
///        the association acceptance).
///
//////////////////////////////////////////////////////////////////
class acseItemPresentationContextAC: public acseItemPresentationContextBase
{
public:
    acseItemPresentationContextAC();

    ///
    /// \brief Constructor.
    ///
    /// \param id             presentation context ID
    /// \param result         presentation context status
    /// \param transferSyntax accepted transfer syntax
    ///
    acseItemPresentationContextAC(std::uint8_t id, result_t result, const std::string& transferSyntax);

    virtual itemType_t getItemType() const override;
};


///
/// \brief Acse item for a PDATA value.
///
//////////////////////////////////////////////////////////////////
class acseItemPDataValue
{
public:

    ///
    /// \brief Presentation context ID of the PDATA value.
    ///
    //////////////////////////////////////////////////////////////////
    std::uint8_t m_presentationContextId;

    ///
    /// \brief true if the PDATA contains a command, false if it
    ///        contains a payload.
    ///
    //////////////////////////////////////////////////////////////////
    bool m_bCommand;

    ///
    /// \brief true if this is the last PDATA value for the command
    ///        or payload dataset, false otherwise.
    ///
    //////////////////////////////////////////////////////////////////
    bool m_bLast;

    ///
    /// \brief Memory object containing the PDATA value.
    ///
    //////////////////////////////////////////////////////////////////
    std::shared_ptr<memory> m_pMemory;

    ///
    /// \brief Offset in m_pMemory where the PDATA value starts.
    ///
    //////////////////////////////////////////////////////////////////
    size_t m_memoryOffset;

    ///
    /// \brief Size of the PDATA value within m_pMemory.
    ///
    //////////////////////////////////////////////////////////////////
    size_t m_memorySize;
};


///
/// \brief Base class for the PDU classes.
///
//////////////////////////////////////////////////////////////////
class acsePDU
{
public:

    virtual ~acsePDU();

    ///
    /// \brief PDU types
    ///
    //////////////////////////////////////////////////////////////////
    enum class pduType_t: std::uint8_t
    {
        associateRQ = 0x01, ///< Association request
        associateAC = 0x02, ///< Association accepted
        associateRJ = 0x03, ///< Association rejected
        pData = 0x04,       ///< PData (Commands and payloads)
        aReleaseRQ = 0x05,  ///< Association release request
        aReleaseRP = 0x06,  ///< Association release response
        aAbort = 0x07       ///< Association abort
    };

    ///
    /// \brief Decode a PDU from a streamReader
    ///
    /// \param pReader   the reader from which the PDU must be read
    /// \return the decoded PDU
    ///
    //////////////////////////////////////////////////////////////////
    static std::shared_ptr<acsePDU> decodePDU(std::shared_ptr<streamReader> pReader);

    ///
    /// \brief Encode the PDU into a streamWriter.
    ///
    /// \param pWriter the streamWriter into which the PDU must
    ///                be encoded
    ///
    //////////////////////////////////////////////////////////////////
    void encodePDU(std::shared_ptr<streamWriter> pWriter) const;

    ///
    /// \brief Return the PDU type.
    ///
    /// \return the PDU type
    ///
    //////////////////////////////////////////////////////////////////
    virtual pduType_t getPDUType() const = 0;

protected:
    virtual void encodePDUPayload(std::shared_ptr<streamWriter>) const = 0;
    virtual void decodePDUPayload(std::shared_ptr<streamReader> pReader) = 0;

    template<size_t readSize>
    static std::string readFixedLengthString(std::shared_ptr<streamReader> pReader)
    {
        char buffer[readSize];
        pReader->read((std::uint8_t*)buffer, readSize);

        size_t start(0), end(readSize);

        while(start < end && (buffer[start] == 0 || buffer[start] == ' '))
        {
            ++start;
        }

        while(end > start && (buffer[end - 1] == 0 || buffer[end - 1] == ' '))
        {
            --end;
        }

        return std::string(&(buffer[start]), end - start);

    }
};


///
/// \brief Base class for the association request/accept classes.
///
//////////////////////////////////////////////////////////////////
class acsePDUAssociateBase: public acsePDU
{
public:
    acsePDUAssociateBase();

    ///
    /// \brief List of presentation contexts and associated
    ///        transfer syntaxes.
    ///
    //////////////////////////////////////////////////////////////////
    typedef std::list<std::shared_ptr<acseItemPresentationContextBase> > presentationContexts_t;

    ///
    /// \brief Constructor.
    ///
    /// \param callingAETitle       calling AE Title
    /// \param calledAETitle        called AE Title
    /// \param applicationContext   application context (must be the
    ///                             DICOM application context)
    /// \param presentationContexts list of presentation contexts
    /// \param pUserInformation     user information item
    ///
    //////////////////////////////////////////////////////////////////
    acsePDUAssociateBase(
            const std::string& callingAETitle,
            const std::string& calledAETitle,
            const std::string& applicationContext,
            const presentationContexts_t& presentationContexts,
            std::shared_ptr<acseItemUserInformation> pUserInformation
            );

    ///
    /// \brief Return the calling AE title.
    ///
    /// \return the calling AE title
    ///
    //////////////////////////////////////////////////////////////////
    const std::string& getCallingAETitle() const;

    ///
    /// \brief Return the called AE title.
    ///
    /// \return the called AE title
    ///
    //////////////////////////////////////////////////////////////////
    const std::string& getCalledAETitle() const;

    ///
    /// \brief Return the application context item.
    ///
    /// \return the application context item
    ///
    //////////////////////////////////////////////////////////////////
    const std::shared_ptr<acseItemApplicationContext>& getApplicationContext() const;

    ///
    /// \brief Return the presentation contexts.
    ///
    /// \return presentation contexts
    ///
    const presentationContexts_t& getPresentationContexts() const;

    ///
    /// \brief Return the user information item.
    ///
    /// \return user information item
    ///
    //////////////////////////////////////////////////////////////////
    const std::shared_ptr<acseItemUserInformation>& getItemUserInformation() const;

protected:
    virtual void encodePDUPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodePDUPayload(std::shared_ptr<streamReader> pReader) override;

    std::string m_callingAETitle;
    std::string m_calledAETitle;
    std::string m_reserved;

    std::shared_ptr<acseItemApplicationContext> m_pApplicationContext;

    presentationContexts_t m_presentationContexts;

    std::shared_ptr<acseItemUserInformation> m_pUserInformation;
};


///
/// \brief Association request PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUAssociateRQ: public acsePDUAssociateBase
{
public:

    using acsePDUAssociateBase::acsePDUAssociateBase;

    virtual pduType_t getPDUType() const override;
};


///
/// \brief Association acceptance PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUAssociateAC: public acsePDUAssociateBase
{
public:

    using acsePDUAssociateBase::acsePDUAssociateBase;

    virtual pduType_t getPDUType() const override;
};


///
/// \brief Association rejection PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUAssociateRJ: public acsePDU
{
public:

    ///
    /// \brief Rejection type.
    ///
    //////////////////////////////////////////////////////////////////
    enum class result_t: std::uint8_t
    {
        rejectedPermanent = 1, ///< Permanent rejection
        rejectedTransient = 2  ///< Temporary rejection
    };

    ///
    /// \brief Rejection reason.
    ///
    //////////////////////////////////////////////////////////////////
    enum class reason_t: std::uint16_t
    {
        serviceUserNoReasonGiven = 0x0101,                       ///< Upper layer rejection: no reason given
        serviceUserApplicationContextNameNotSupported = 0x0102,  ///< Application context name not supported
        serviceUserCallingAETitleNotRecognized = 0x0103,         ///< Calling AE title not recognized
        serviceUserCalledAETitleNotRecognized = 0x0107,          ///< Called AE title not recognized

        serviceProviderAcseNoReasonGiven = 0x0201,               ///< Acse service rejection: no reason given
        serviceProviderAcseProtocolVersionNotSupported = 0x0202, ///< Acse protocol not supported

        serviceProviderPresentationReserved = 0x0300,            ///< Presentation context reserved
        serviceProviderPresentationTemporaryCongestion = 0x0301, ///< Temporary congestion
        serviceProviderPresentationLocalLimitExceded = 0x0302,   ///< Local limit exceeded
    };

    acsePDUAssociateRJ();

    ///
    /// \brief Constructor.
    ///
    /// \param result rejection type
    /// \param reason rejection reason
    ///
    //////////////////////////////////////////////////////////////////
    acsePDUAssociateRJ(result_t result, reason_t reason);

    ///
    /// \brief Return the rejection type.
    ///
    /// \return rejection type
    ///
    //////////////////////////////////////////////////////////////////
    result_t getResult() const;

    ///
    /// \brief Return the rejection reason.
    ///
    /// \return rejection reason
    ///
    //////////////////////////////////////////////////////////////////
    reason_t getReason() const;

    virtual pduType_t getPDUType() const override;

protected:
    virtual void encodePDUPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodePDUPayload(std::shared_ptr<streamReader> pReader) override;

    result_t m_result;
    reason_t m_reason;
};


///
/// \brief PDATA PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUPData: public acsePDU
{
public:
    virtual pduType_t getPDUType() const override;

    ///
    /// \brief Add data to the PDU.
    ///
    /// Add the data in the memory object, starting from the
    /// specified offset and terminating either at the end of the
    /// memory object or when the PDU size reaches the maximum
    /// allowd PDU size.
    ///
    /// The memory object is kept referenced by the PDU.
    ///
    /// \param presentationContextId presentation context
    /// \param pData      memory containing the data to add.
    ///                   The PDU keeps a reference to this object
    /// \param offset     data offset in memory
    /// \param maxPDUSize maximum PDU size
    /// \param bCommand   true if the memory refers to a command
    /// \return           the number of added bytes. add
    ///                   this value to the offset for the
    ///                   next call
    ///
    //////////////////////////////////////////////////////////////////
    size_t addItem(std::uint8_t presentationContextId, std::shared_ptr<memory> pData, size_t offset, size_t maxPDUSize, bool bCommand);

    ///
    /// \brief List of PDATA value items.
    ///
    //////////////////////////////////////////////////////////////////
    typedef std::list<std::shared_ptr<acseItemPDataValue> > pdataValues_t;

    ///
    /// \brief Return the list of PDATA values.
    ///
    /// \return PDATA values
    ///
    //////////////////////////////////////////////////////////////////
    const pdataValues_t& getValues() const;

protected:
    virtual void encodePDUPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodePDUPayload(std::shared_ptr<streamReader> pReader) override;

    pdataValues_t m_values;
};


///
/// \brief Base class for association release PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUARelease: public acsePDU
{
protected:
    virtual void encodePDUPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodePDUPayload(std::shared_ptr<streamReader> pReader) override;
};


///
/// \brief Association release request PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUAReleaseRQ: public acsePDUARelease
{
public:
    virtual pduType_t getPDUType() const override;
};


///
/// \brief Association release response PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUAReleaseRP: public acsePDUARelease
{
public:
    virtual pduType_t getPDUType() const override;
};


///
/// \brief Association abort PDU.
///
//////////////////////////////////////////////////////////////////
class acsePDUAAbort: public acsePDU
{
public:
    acsePDUAAbort();

    ///
    /// \brief Abort reason.
    ///
    //////////////////////////////////////////////////////////////////
    enum class reason_t: std::uint16_t
    {
        serviceUser = 0x0000,                             ///< Upper layer
        serviceProviderReasonNotSpecified = 0x0200,       ///< No reason
        serviceProviderUnrecognizedPDU = 0x0201,          ///< Unrecognized PDU
        serviceProviderUnexpectedPDU = 0x0202,            ///< Unexpected PDU
        serviceProviderUnrecognizedPDUParameter = 0x0204, ///< Unrecognized PDU parameter
        serviceProviderUnexpectedPDUParameter = 0x0205,   ///< Unexpected PDU parameter
        serviceProviderInvalidPDUParameterValue = 0x0206, ///< Invalid PDU parameter
    };

    ///
    /// \brief Constructor.
    ///
    /// \param reason abort reason
    ///
    //////////////////////////////////////////////////////////////////
    acsePDUAAbort(reason_t reason);

    ///
    /// \brief Return the abort reason.
    ///
    /// \return abort reason
    ///
    //////////////////////////////////////////////////////////////////
    reason_t getReason() const;

    virtual pduType_t getPDUType() const override;

protected:
    virtual void encodePDUPayload(std::shared_ptr<streamWriter>) const override;
    virtual void decodePDUPayload(std::shared_ptr<streamReader> reader) override;

    reason_t m_reason;
};


///
/// \brief Presentation context class
///
//////////////////////////////////////////////////////////////////
class presentationContext
{
public:
    presentationContext(const std::string& abstractSyntax);
    presentationContext(const std::string& abstractSyntax, bool bRequestorIsSCU, bool bRequestorIsSCP);
    presentationContext(const presentationContext& right);

    void addTransferSyntax(const std::string& transferSyntax);

    const std::string m_abstractSyntax;
    typedef std::list<std::string> transferSyntaxes_t;
    transferSyntaxes_t m_proposedTransferSyntaxes;

    bool m_bRequestorIsSCU;
    bool m_bRequestorIsSCP;
};


///
/// \brief List of presentation contexts.
///
//////////////////////////////////////////////////////////////////
class presentationContexts
{
public:
    presentationContexts();
    presentationContexts(const presentationContexts& right);

    typedef std::list<std::shared_ptr<presentationContext> > presentationContexts_t;
    presentationContexts_t m_presentationContexts;
};


///
/// \brief A message sent through an association.
///
//////////////////////////////////////////////////////////////////
class associationMessage
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax the message's abstract syntax
    ///
    //////////////////////////////////////////////////////////////////
    associationMessage(const std::string& abstractSyntax);

    associationMessage(const std::string& abstractSyntax, std::shared_ptr<dataSet> pCommand);

    associationMessage(const std::string& abstractSyntax, std::shared_ptr<dataSet> pCommand, std::shared_ptr<dataSet> pPayload);

    virtual ~associationMessage();

    void addDataset(const std::shared_ptr<dataSet> pDataset);

    const std::string& getAbstractSyntax() const;

    std::shared_ptr<dataSet> getCommandDataSet() const;

    std::shared_ptr<dataSet> getPayloadDataSetNoThrow() const;

    std::shared_ptr<dataSet> getPayloadDataSet() const;
    ///
    /// \brief Return true if the message is complete, false otherwise
    ///        (e.g. the check if the message includes the payload).
    ///
    /// \return true if the message is complete, false otherwise
    ///
    //////////////////////////////////////////////////////////////////
    bool isComplete() const;

protected:
    const std::string m_abstractSyntax;

    std::shared_ptr<dataSet> m_pCommand;
    std::shared_ptr<dataSet> m_pPayload;
};


///
/// \brief Base class for the association classes associationSCU
///        and associationSCP.
///
//////////////////////////////////////////////////////////////////
class associationBase
{
public:

    ///
    /// \brief The association role.
    ///
    enum class role_t
    {
        scu, ///< The SCU part of the association
        scp  ///< The SCP part of the association
    };

    ~associationBase();

    ///
    /// \brief Send a message to the connected peer.
    ///
    /// \param message the message to send
    ///
    //////////////////////////////////////////////////////////////////
    void sendMessage(std::shared_ptr<const associationMessage> message);

    ///
    /// \brief Retrieve the next received command.
    ///
    /// \return the next received command
    ///
    //////////////////////////////////////////////////////////////////
    std::shared_ptr<associationMessage> getCommand();

    ///
    /// \brief Retrieve the received response that replies to a
    ///        specific command ID.
    ///
    /// \param messageId the command's ID to which the reply is
    ///                  related
    /// \return the received response
    ///
    //////////////////////////////////////////////////////////////////
    std::shared_ptr<associationMessage> getResponse(std::uint16_t messageId);

    ///
    /// \brief Abort the association. The other peer will not send
    ///        an acknowledgement.
    ///
    /// \param reason abort reason
    ///
    //////////////////////////////////////////////////////////////////
    void abort(acsePDUAAbort::reason_t reason);

    ///
    /// \brief Release the association. Waits for the acknowledgement
    ///        response or the ACSE timeout.
    ///
    //////////////////////////////////////////////////////////////////
    void release();

    ///
    /// \brief Returns our AET.
    ///
    /// \return our AET
    ///
    //////////////////////////////////////////////////////////////////
    std::string getThisAET() const;

    ///
    /// \brief Returns the other party's AET.
    ///
    /// \return the connected peer's AET
    ///
    //////////////////////////////////////////////////////////////////
    std::string getOtherAET() const;

    std::string getPresentationContextTransferSyntax(const std::string& abstractSyntax) const;

    std::vector<std::string> getPresentationContextTransferSyntaxes(const std::string& abstractSyntax) const;

    void getMessagesThread();

protected:

    associationBase(
            role_t role,
            const std::string& thisAET,
            const std::string& otherAET,
            std::uint32_t maxOperationsWeInvoke,
            std::uint32_t maxOperationsWeCanPerform,
            std::shared_ptr<streamReader> pReader,
            std::shared_ptr<streamWriter> pWriter,
            std::uint32_t dimseTimeout);

    std::shared_ptr<associationMessage> getMessage(std::uint16_t messageId, bool bResponse);

    const role_t m_role;

    ///
    /// \brief Maps a presentation context ID to the
    ///        presentation context object and the selected
    ///        transfer syntax
    ///
    ///////////////////////////////////////////////////////////
    typedef std::map<std::uint8_t, std::pair<std::shared_ptr<presentationContext>, std::string> > presentationContextsIds_t;
    presentationContextsIds_t m_presentationContextsIds;

    const std::string m_thisAET;
    std::string m_otherAET;

    std::uint32_t m_maxOperationsInvoked;
    std::uint32_t m_maxOperationsPerformed;

    ///
    /// \brief 1 if the association is active, 0 if the
    ///        association has been aborted or released.
    ///
    ///////////////////////////////////////////////////////////
    std::atomic<int> m_bAssociated;

    std::uint32_t m_maxPDULength;

    std::shared_ptr<streamReader> m_pReader;
    std::shared_ptr<streamWriter> m_pWriter;

    std::unique_ptr<std::thread> m_readDataSetsThread;

private:

    struct receivedDataset
    {
        receivedDataset(const std::string& presentationContext, std::shared_ptr<dataSet> pDataset);
        const std::string m_presentationContext;
        std::shared_ptr<dataSet> m_pDataset;
    };

    ///
    /// \brief Decodes several PDUs until a full dataset
    ///        has been decoded
    ///
    /// \return a decoded dataSet
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<receivedDataset> decodePDU(bool bCommand, std::list<std::shared_ptr<acseItemPDataValue> >& pendingData, size_t& m_numberOfLastPData) const;

    /// Datasets ready to be retrieved by getMessage()
    ///////////////////////////////////////////////////////////
    typedef std::list<std::shared_ptr<associationMessage> > readyDatasets_t;
    readyDatasets_t m_readyDataSets;
    std::atomic<bool> m_bTerminated;
    std::mutex m_lockReadyDataSets;
    std::condition_variable m_notifyReadyDataSets;

    // Lock while writing commands (only one command at the
    // time may be sent to the other party)
    ///////////////////////////////////////////////////////////
    mutable std::mutex m_lockWrite;

    // Lock access to m_waitingResponses and
    // m_processingCommands
    ///////////////////////////////////////////////////////////
    std::mutex m_lockCommandsResponses;

    // ID of the responses that we are expecting
    ///////////////////////////////////////////////////////////
    std::set<std::uint32_t> m_waitingResponses;

    // ID of the commands that we received are still processing
    ///////////////////////////////////////////////////////////
    std::set<std::uint32_t> m_processingCommands;

    // DIMSE Timeout, in seconds (0 = infinite)
    ///////////////////////////////////////////////////////////
    std::uint32_t m_dimseTimeout;
};


///
/// \brief Manages a connection acting as SCU.
///
/// Performs an association request on an already operating
/// reading stream (e.g. a tcpSequenceStream returned by a
/// tcpListener).
///
/// After the association has been negotiated then DICOM messages
/// can be sent and received via the association.
///
//////////////////////////////////////////////////////////////////
class associationSCU: public associationBase
{
public:
    ///
    /// \brief Creates an association with an SCP.
    ///
    /// \param presentationContexts proposed presentation contexts
    /// \param thisAET              the SCU AET
    /// \param otherAET             the called SCP AET
    /// \param maxOperationsWeInvoke max number of simultaneous
    ///                             operation that this SCU will
    ///                             invoke on the SCP peer
    /// \param maxOperationsWeCanPerform maximum number of
    ///                             simultaneous operations that this
    ///                             SCP can perform (when acting as
    ///                             an SCP)
    /// \param pReader              reader on which the messages are
    ///                             received
    /// \param pWriter              writer on which the messages are
    ///                             sent
    /// \param dimseTimeoutSeconds  DIMSE timeout, in seconds. 0
    ///                             means infinite
    ///
    //////////////////////////////////////////////////////////////////
    associationSCU(
            const std::shared_ptr<const presentationContexts>& contexts,
            const std::string& thisAET,
            const std::string& otherAET,
            std::uint32_t maxOperationsWeInvoke,
            std::uint32_t maxOperationsWeCanPerform,
            std::shared_ptr<streamReader> pReader,
            std::shared_ptr<streamWriter> pWriter,
            std::uint32_t dimseTimeoutSeconds);

};


///
/// \brief Manages a connection acting as SCP.
///
/// Waits for incoming association request on an already
/// operating reading stream (e.g. a tcpSequenceStream returned
/// by a tcpListener) and replies with the connection
/// acknowledgement or rejection.
///
/// After the association has been negotiated then DICOM messages
/// can be sent and received via the association.
///
//////////////////////////////////////////////////////////////////
class associationSCP: public associationBase
{
public:
    ///
    /// \brief Waits for an association request and sets up
    ///        the association
    ///
    /// \param presentationContexts accepted presentation contexts
    /// \param thisAET              accepted called AET. Leave empty
    ///                             to accept all the called AETs
    /// \param maxOperationsWeInvoke max number of simultaneous
    ///                             operation that this SCP will
    ///                             invoke on the peer
    /// \param maxOperationsWeCanPerform maximum number of
    ///                             simultaneous operations that this
    ///                             SCP can perform
    /// \param pReader              reader on which the messages are
    ///                             received
    /// \param pWriter              writer on which the messages are
    ///                             sent
    /// \param dimseTimeoutSeconds  DIMSE timeout, in seconds. 0
    ///                             means infinite
    /// \param artimTimeoutSeconds  maximum time, in seconds, that can
    ///                             pass before an association request
    ///                             arrives
    ///
    //////////////////////////////////////////////////////////////////
    associationSCP(
            const std::shared_ptr<const presentationContexts>& contexts,
            const std::string& thisAET,
            std::uint32_t maxOperationsWeInvoke,
            std::uint32_t maxOperationsWeCanPerform,
            std::shared_ptr<streamReader> pReader,
            std::shared_ptr<streamWriter> pWriter,
            std::uint32_t dimseTimeoutSeconds,
            std::uint32_t artimTimeoutSeconds);

};

} // namespace implementation

} // namespace imebra


#endif // !defined(CImbxAcse_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
