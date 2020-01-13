/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/


/*! \file dimseImpl.h
    \brief Defines the classes that implement the DIMSE

*/

#if !defined(CImbxDimse_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
#define CImbxDimse_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_

#include <atomic>
#include <string>
#include <memory>
#include "../include/imebra/definitions.h"
#include "acseImpl.h"

namespace imebra
{

namespace implementation
{

class dataSet;
class associationBase;

///
/// \brief Base class for DIMSE commands and responses.
///
//////////////////////////////////////////////////////////////////
class dimseCommandBase: public associationMessage
{
public:
    ///
    /// \brief Constructor.
    ///
    /// After calling this constructor, finish building the command
    /// by populating the dataSet returned by getCommand().
    ///
    /// \param abstractSyntax abstract syntax of the command
    /// \param pPayload       the payload for the command or response
    ///                       (optional)
    ///
    //////////////////////////////////////////////////////////////////
    dimseCommandBase(const std::string& abstractSyntax, std::shared_ptr<dataSet> pPayload);

    ///
    /// \brief Constructor.
    ///
    /// After calling this constructor, finish building the command
    /// by populating the dataSet returned by getCommand().
    ///
    /// \param abstractSyntax abstract syntax of the command
    /// \param pCommand       the command or response dataset
    /// \param pPayload       the payload for the command or response
    ///                       (optional)
    ///
    //////////////////////////////////////////////////////////////////
    dimseCommandBase(const std::string& abstractSyntax, std::shared_ptr<dataSet> pCommand, std::shared_ptr<dataSet> pPayload);

    virtual ~dimseCommandBase();

    ///
    /// \brief Validate the command.
    ///
    /// Throw if the command is not valid.
    ///
    //////////////////////////////////////////////////////////////////
    virtual void validate() const = 0;

    ///
    /// \brief Returns the affected SOP instance UID.
    ///
    /// \return affected SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    std::string getAffectedSopInstanceUid() const;

    ///
    /// \brief Returns the affected SOP class UID.
    ///
    /// \return affected SOP class UID
    ///
    //////////////////////////////////////////////////////////////////
    std::string getAffectedSopClassUid() const;

    ///
    /// \brief Returns the requested SOP instance UID.
    ///
    /// \return requested SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    std::string getRequestedSopInstanceUid() const;

    ///
    /// \brief Returns the requested SOP class UID.
    ///
    /// \return requested SOP class UID
    ///
    //////////////////////////////////////////////////////////////////
    std::string getRequestedSopClassUid() const;

};


///
/// \brief Base class for DIMSE N commands.
///
//////////////////////////////////////////////////////////////////
class dimseNCommand: public dimseCommandBase
{
public:
    ///
    /// \brief Constructor
    ///
    /// \param abstractSyntax abstract syntax for the command
    /// \param command        command type
    /// \param priority       command priority
    /// \param messageID      unique command ID
    ///
    //////////////////////////////////////////////////////////////////
    dimseNCommand(const std::string& abstractSyntax, dimseCommandType_t command, std::uint16_t messageID);

    ///
    /// \brief Constructor
    ///
    /// \param abstractSyntax abstract syntax for the command
    /// \param command        command type
    /// \param priority       command priority
    /// \param messageID      unique command ID
    /// \param pPayload       command payload
    ///
    //////////////////////////////////////////////////////////////////
    dimseNCommand(const std::string& abstractSyntax, dimseCommandType_t command, std::uint16_t messageID, std::shared_ptr<dataSet> pPayload);

    ///
    /// \brief Build a command from a received message.
    ///
    /// \param pMessage received message
    ///
    //////////////////////////////////////////////////////////////////
    dimseNCommand(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Returns the unique command ID.
    ///
    /// \return unique command ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getID() const;

    ///
    /// \brief Returns the command type.
    ///
    /// \return command type
    ///
    //////////////////////////////////////////////////////////////////
    dimseCommandType_t getCommandType() const;

    ///
    /// \brief Validate the command.
    ///
    /// Throw if the command is not valid.
    ///
    //////////////////////////////////////////////////////////////////
    virtual void validate() const override;
};


///
/// \brief Base class for DIMSE C commands.
///
//////////////////////////////////////////////////////////////////
class dimseCCommand: public dimseNCommand
{
public:
    ///
    /// \brief Constructor
    ///
    /// \param abstractSyntax abstract syntax for the command
    /// \param command        command type
    /// \param priority       command priority
    /// \param messageID      unique command ID
    ///
    //////////////////////////////////////////////////////////////////
    dimseCCommand(const std::string& abstractSyntax, dimseCommandType_t command, dimseCommandPriority_t priority, std::uint16_t messageID);

    ///
    /// \brief Constructor
    ///
    /// \param abstractSyntax abstract syntax for the command
    /// \param command        command type
    /// \param priority       command priority
    /// \param messageID      unique command ID
    /// \param pPayload       the message's payload
    ///
    //////////////////////////////////////////////////////////////////
    dimseCCommand(const std::string& abstractSyntax, dimseCommandType_t command, dimseCommandPriority_t priority, std::uint16_t messageID, std::shared_ptr<dataSet> pPayload);

    ///
    /// \brief Build a command from a received message.
    ///
    /// \param pMessage received message
    ///
    //////////////////////////////////////////////////////////////////
    dimseCCommand(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Returns the command's priority.
    ///
    /// \return command's priority
    ///
    //////////////////////////////////////////////////////////////////
    dimseCommandPriority_t getPriority() const;

    ///
    /// \brief Validate the command.
    ///
    /// Throw if the command is not valid.
    ///
    //////////////////////////////////////////////////////////////////
    virtual void validate() const override;
};


///
/// \brief Base class for DIMSE responses.
///
//////////////////////////////////////////////////////////////////
class dimseResponse: public dimseCommandBase
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     message for which the response is
    ///                     being built
    /// \param responseCode response status code
    ///
    //////////////////////////////////////////////////////////////////
    dimseResponse(std::shared_ptr<dimseNCommand> pCommand, dimseStatusCode_t responseCode);

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     message for which the response is
    ///                     being built
    /// \param responseCode response status code
    /// \param pPayload     payload dataSet to send with the response
    ///
    //////////////////////////////////////////////////////////////////
    dimseResponse(std::shared_ptr<dimseNCommand> pCommand, dimseStatusCode_t responseCode, std::shared_ptr<dataSet> pPayload);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage message containing the response
    ///
    //////////////////////////////////////////////////////////////////
    dimseResponse(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Get the ID of the message being responded to.
    ///
    /// \return Get the ID of the message being responded to
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getMessageBeingRespondedID() const;

    ///
    /// \brief Returns the meaning of the response's status code
    ///        returned by getStatusCode().
    ///
    /// \return the meaning of the response status code
    ///
    //////////////////////////////////////////////////////////////////
    dimseStatus_t getStatus() const;

    ///
    /// \brief Resturns the response's status code.
    ///
    /// \return the response's status code
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getStatusCode() const;

    ///
    /// \brief Validate the response.
    ///
    /// Throw if the command is not valid.
    ///
    //////////////////////////////////////////////////////////////////
    virtual void validate() const override;
};


///
/// \brief Base class for DIMSE responses that can be partial.
///
//////////////////////////////////////////////////////////////////
class cPartialResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               command for which this response
    ///                               is being constructed
    /// \param responseCode           response code
    /// \param remainingSubOperations number of remaining sub
    ///                               operations
    /// \param completedSubOperations number of completed sub
    ///                               operations
    /// \param failedSubOperations    number of failed sub operations
    /// \param warningSubOperations   number of sub operations
    ///                               completed with warnings
    ///
    //////////////////////////////////////////////////////////////////
    cPartialResponse(
            std::shared_ptr<dimseCCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations);

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               command for which this response
    ///                               is being constructed
    /// \param responseCode           response code
    /// \param remainingSubOperations number of remaining sub
    ///                               operations
    /// \param completedSubOperations number of completed sub
    ///                               operations
    /// \param failedSubOperations    number of failed sub operations
    /// \param warningSubOperations   number of sub operations
    ///                               completed with warnings
    /// \param pIdentifier            response payload
    ///
    //////////////////////////////////////////////////////////////////
    cPartialResponse(
            std::shared_ptr<dimseCCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor
    ///
    /// \param pMessage dimse message from which the response is built
    ///
    //////////////////////////////////////////////////////////////////
    cPartialResponse(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Return the number of remaining sub operations.
    ///
    /// \return number of remaining sub operations
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getRemainingSubOperations() const;

    ///
    /// \brief Return the number of completed sub operations.
    ///
    /// \return number of completed sub operations
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getCompletedSubOperations() const;

    ///
    /// \brief Return the number of failed sub operations.
    ///
    /// \return number of failed sub operations
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getFailedSubOperations() const;

    ///
    /// \brief Return the number of suboperations completed with
    ///        warnings.
    ///
    /// \return number of suboperations completed with warnings
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getWarningSubOperations() const;

    virtual void validate() const override;
};


///
/// \brief C-STORE command.
///
//////////////////////////////////////////////////////////////////
class cStoreCommand: public dimseCCommand
{
public:

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the abstract syntax to use
    ///                               when transmitting the command
    /// \param messageID              message ID
    /// \param priority               message priority
    /// \param affectedSopClassUid    affected SOP instance UID
    /// \param affectedSopInstanceUid affected SOP instance UID
    /// \param originatorAET          originator AET (issuer of the
    ///                               C-MOVE or C-GET command)
    /// \param originatorMessageID    message ID of the C-MOVE or
    ///                               C-GET that triggered the C-STORE
    /// \param pPayload               C-STORE payload
    ///
    //////////////////////////////////////////////////////////////////
    cStoreCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            const std::string& originatorAET,
            std::uint16_t originatorMessageID,
            std::shared_ptr<dataSet> pPayload);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-STORE command
    ///
    //////////////////////////////////////////////////////////////////
    cStoreCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

    ///
    /// \brief Return the AET of the SCU that requested the C-MOVE
    ///        or C-GET that triggered the C-STORE
    ///
    /// \return AET of the SCU that triggered the C-STORE via a
    ///         C-MOVE or a C-GET
    ///
    //////////////////////////////////////////////////////////////////
    std::string getOriginatorAET() const;

    ///
    /// \brief Return the ID of the C-MOVE or C-STORE command that
    ///        triggered the C-STORE
    ///
    /// \return ID of the C-MOVE or C-GET command that triggered the
    ///         C-STORE
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getOriginatorMessageID() const;

};


///
/// \brief C-STORE response.
///
//////////////////////////////////////////////////////////////////
class cStoreResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pStoreCommand the command for which this response is
    ///        being generated
    /// \param responseCode  the response code
    ///
    //////////////////////////////////////////////////////////////////
    cStoreResponse(std::shared_ptr<cStoreCommand> pStoreCommand, dimseStatusCode_t responseCode);

    ///
    /// \brief Constructor.
    ///
    /// \param A message containing the C-STORE response
    ///
    //////////////////////////////////////////////////////////////////
    cStoreResponse(std::shared_ptr<const associationMessage> pMessage);
};


///
/// \brief C-GET command.
///
//////////////////////////////////////////////////////////////////
class cGetCommand: public dimseCCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the abstract syntax to use
    ///                               when transmitting the command
    /// \param messageID              message ID
    /// \param priority               message priority
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param pIdentifier            the keys to use for the
    ///                               C-GET query
    ///
    //////////////////////////////////////////////////////////////////
    cGetCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-GET command
    ///
    //////////////////////////////////////////////////////////////////
    cGetCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

};


///
/// \brief C-GET response.
///
//////////////////////////////////////////////////////////////////
class cGetResponse: public cPartialResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               the command for which this
    ///                               response is being generated
    /// \param responseCode           response code
    /// \param remainingSubOperations number of remaining sub
    ///                               operations
    /// \param completedSubOperations number of completed sub
    ///                               operations
    /// \param failedSubOperations    number of failed sub operations
    /// \param warningSubOperations   number of sub operations
    ///                               completed with warnings
    /// \param pIdentifier            The identifier dataset
    ///
    //////////////////////////////////////////////////////////////////
    cGetResponse(
            std::shared_ptr<cGetCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               the command for which this
    ///                               response is being generated
    /// \param responseCode           response code
    /// \param remainingSubOperations number of remaining sub
    ///                               operations
    /// \param completedSubOperations number of completed sub
    ///                               operations
    /// \param failedSubOperations    number of failed sub operations
    /// \param warningSubOperations   number of sub operations
    ///                               completed with warnings
    ///
    //////////////////////////////////////////////////////////////////
    cGetResponse(
            std::shared_ptr<cGetCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-GET response
    ///
    //////////////////////////////////////////////////////////////////
    cGetResponse(std::shared_ptr<const associationMessage> pMessage);

};


///
/// \brief C-FIND command.
///
//////////////////////////////////////////////////////////////////
class cFindCommand: public dimseCCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax      the abstract syntax
    /// \param messageID           the message ID
    /// \param priority            the message priority
    /// \param affectedSopClassUid the affected SOP class UID
    /// \param pIdentifier         the dataset with the identifier
    ///                            (list of tags to match and their
    ///                            requested values)
    ///
    //////////////////////////////////////////////////////////////////
    cFindCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-FIND command
    ///
    //////////////////////////////////////////////////////////////////
    cFindCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

};


///
/// \brief C-FIND response.
///
//////////////////////////////////////////////////////////////////
class cFindResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor for responses that include a match.
    ///
    /// \param pCommand               the command for which this
    ///                               response is being generated
    /// \param pIdentifier            the dataset with the identifier
    ///                               (list of matched tags and their
    ///                               values)
    ///
    //////////////////////////////////////////////////////////////////
    cFindResponse(
            std::shared_ptr<cFindCommand> pCommand,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor for responses that signal a success or
    ///        a failure.
    ///
    /// \param pCommand               the command for which this
    ///                               response is being generated
    /// \param responseCode           response code
    ///
    //////////////////////////////////////////////////////////////////
    cFindResponse(
            std::shared_ptr<cFindCommand> pCommand,
            dimseStatusCode_t responseCode);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-FIND response
    ///
    //////////////////////////////////////////////////////////////////
    cFindResponse(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

};


///
/// \brief C-MOVE command.
///
//////////////////////////////////////////////////////////////////
class cMoveCommand: public dimseCCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax      the abstract syntax
    /// \param messageID           the message ID
    /// \param priority            the message priority
    /// \param affectedSopClassUid affected SOP class UID
    /// \param destinationAET      the destination AET
    /// \param pIdentifier         the dataset with the identifier
    ///                            (list of tags to match and their
    ///                            requested values)
    ///
    //////////////////////////////////////////////////////////////////
    cMoveCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            const std::string& destinationAET,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-MOVE command
    ///
    //////////////////////////////////////////////////////////////////
    cMoveCommand(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-MOVE command
    ///
    //////////////////////////////////////////////////////////////////
    std::string getDestinationAET() const;

    virtual void validate() const override;

};


///
/// \brief C-MOVE response.
///
//////////////////////////////////////////////////////////////////
class cMoveResponse: public cPartialResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               the command for which this
    ///                               response is being generated
    /// \param responseCode           response code
    /// \param remainingSubOperations number of remaining sub
    ///                               operations
    /// \param completedSubOperations number of completed sub
    ///                               operations
    /// \param failedSubOperations    number of failed sub operations
    /// \param warningSubOperations   number of sub operations
    ///                               completed with warnings
    /// \param pIdentifier            the dataset with the identifier
    ///                               (list of matched tags and their
    ///                               values)
    ///
    //////////////////////////////////////////////////////////////////
    cMoveResponse(
            std::shared_ptr<cMoveCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations,
            std::shared_ptr<dataSet> pIdentifier);

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               the command for which this
    ///                               response is being generated
    /// \param responseCode           response code
    /// \param remainingSubOperations number of remaining sub
    ///                               operations
    /// \param completedSubOperations number of completed sub
    ///                               operations
    /// \param failedSubOperations    number of failed sub operations
    /// \param warningSubOperations   number of sub operations
    ///                               completed with warnings
    ///
    //////////////////////////////////////////////////////////////////
    cMoveResponse(
            std::shared_ptr<cMoveCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-MOVE response
    ///
    //////////////////////////////////////////////////////////////////
    cMoveResponse(std::shared_ptr<const associationMessage> pMessage);

};


///
/// \brief C-ECHO command.
///
//////////////////////////////////////////////////////////////////
class cEchoCommand: public dimseCCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax      the abstract syntax
    /// \param messageID           the message ID
    /// \param priority            the priority
    /// \param affectedSopClassUid the affected SOP class UID
    ///
    //////////////////////////////////////////////////////////////////
    cEchoCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-ECHO command
    ///
    //////////////////////////////////////////////////////////////////
    cEchoCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;
};


///
/// \brief C-ECHO response
///
//////////////////////////////////////////////////////////////////
class cEchoResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     the command for which this response is
    ///                     being generated
    /// \param responseCode the response code
    ///
    //////////////////////////////////////////////////////////////////
    cEchoResponse(
            std::shared_ptr<cEchoCommand>,
            dimseStatusCode_t responseCode);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-ECHO response
    ///
    //////////////////////////////////////////////////////////////////
    cEchoResponse(std::shared_ptr<const associationMessage> pMessage);
};


///
/// \brief C-CANCEL command.
///
//////////////////////////////////////////////////////////////////
class cCancelCommand: public dimseCCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax  the abstract syntax
    /// \param messageID       the message ID
    /// \param priority        the message priority
    /// \param cancelMessageID the ID of the message to cancel
    ///
    //////////////////////////////////////////////////////////////////
    cCancelCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            std::uint16_t cancelMessageID);

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the C-CANCEL command
    ///
    //////////////////////////////////////////////////////////////////
    cCancelCommand(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Returns the ID of the message to cancel.
    ///
    /// \return the ID of the message to cancel
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getCancelMessageID() const;

    virtual void validate() const override;
};


///
/// \brief N-EVENT-REPORT command.
///
//////////////////////////////////////////////////////////////////
class nEventReportCommand: public dimseNCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the abstract syntax
    /// \param messageID              the message ID
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    /// \param eventID                the event ID
    ///
    //////////////////////////////////////////////////////////////////
    nEventReportCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            std::uint16_t eventID
            );

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the abstract syntax
    /// \param messageID              the message ID
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    /// \param eventID                the event ID
    /// \param pEventInformation      the dataset with event
    ///                               information
    ///
    //////////////////////////////////////////////////////////////////
    nEventReportCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            std::uint16_t eventID,
            std::shared_ptr<dataSet> pEventInformation
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-EVENT command
    ///
    //////////////////////////////////////////////////////////////////
    nEventReportCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

    ///
    /// \brief Returns the event ID.
    ///
    /// \return the event ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getEventID() const;

};


///
/// \brief N-EVENT-REPORT response.
///
//////////////////////////////////////////////////////////////////
class nEventReportResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor which includes a payload.
    ///        This implies a successful operation.
    ///
    /// \param pCommand    the n-event request command
    ///                    for which the response is being generated
    /// \param pEventReply the response payload
    ///
    //////////////////////////////////////////////////////////////////
    nEventReportResponse(
            std::shared_ptr<nEventReportCommand> pCommand,
            std::shared_ptr<dataSet> pEventReply
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     the n-event request command
    ///                     for which the response is being generated
    /// \param responseCode response code
    ///
    //////////////////////////////////////////////////////////////////
    nEventReportResponse(
            std::shared_ptr<nEventReportCommand> pCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-EVENT response
    ///
    //////////////////////////////////////////////////////////////////
    nEventReportResponse(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Get the event ID. The response may omit this
    ///        information.
    ///
    /// \return the event ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getEventID() const;

};


///
/// \brief Base class for N commands that need a list of
///        identifier tags.
///
//////////////////////////////////////////////////////////////////
class attributeIdentifierCommand
{
public:
    ///
    /// \brief Extract the list of identifier tags from a command
    ///        dataset.
    ///
    /// \param pDataSet the command dataset from which the list
    ///                 must be extracted
    /// \return the list of identifier tags
    ///
    //////////////////////////////////////////////////////////////////
    static attributeIdentifierList_t getAttributeList(std::shared_ptr<const dataSet> pDataSet);

    ///
    /// \brief Insert a list of identifier tags into a command
    ///        dataset.
    ///
    /// \param attributeIdentifierList list of identifier tags
    /// \param pDestinationDataSet     command dataset in which the
    ///                                identifier tags must be
    ///                                inserted
    ///
    //////////////////////////////////////////////////////////////////
    static void setAttributeIdentifierList(const attributeIdentifierList_t& attributeIdentifierList, std::shared_ptr<dataSet> pDestinationDataSet);
};


///
/// \brief Represents a N-GET command.
///
//////////////////////////////////////////////////////////////////
class nGetCommand: public dimseNCommand, protected attributeIdentifierCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax          the abstract syntax
    /// \param messageID               the message ID
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param attributeIdentifierList the list of identifier tags.
    ///                                An empty list means "all the
    ///                                tags"
    ///
    //////////////////////////////////////////////////////////////////
    nGetCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            const attributeIdentifierList_t& attributeIdentifierList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-GET command
    ///
    //////////////////////////////////////////////////////////////////
    nGetCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

    ///
    /// \brief Return the list of attributes to retrieve. an empty
    ///        list means all.
    ///
    /// \return the list of attributes to retrieve. An empty list
    ///         means "all the attributes"
    ///
    //////////////////////////////////////////////////////////////////
    attributeIdentifierList_t getAttributeList() const;
};


///
/// \brief Represents a N-GET response.
///
//////////////////////////////////////////////////////////////////
class nGetResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand       command for which the response is being
    ///                       constructed
    /// \param responseCode   response code
    /// \param pAttributeList dataset containing the list of attribute
    ///                       identifiers
    ///
    //////////////////////////////////////////////////////////////////
    nGetResponse(
            std::shared_ptr<nGetCommand> pCommand,
            dimseStatusCode_t responseCode,
            std::shared_ptr<dataSet> pAttributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand       command for which the response is being
    ///                       constructed
    /// \param responseCode   response code
    ///
    //////////////////////////////////////////////////////////////////
    nGetResponse(
            std::shared_ptr<nGetCommand> pCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-GET response
    ///
    //////////////////////////////////////////////////////////////////
    nGetResponse(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

};


///
/// \brief Represents the N-SET command
///
//////////////////////////////////////////////////////////////////
class nSetCommand: public dimseNCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax          the abstract syntax
    /// \param messageID               the message ID
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param pModificationList       dataset containing the new
    ///                                attributes values
    ///
    //////////////////////////////////////////////////////////////////
    nSetCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            std::shared_ptr<dataSet> pModificationList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-SET command
    ///
    //////////////////////////////////////////////////////////////////
    nSetCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

};


///
/// \brief Represents the N-SET response.
///
//////////////////////////////////////////////////////////////////
class nSetResponse: public dimseResponse, protected attributeIdentifierCommand
{
public:
    ///
    /// \brief Constructor of a successful response.
    ///
    /// \param pCommand           the command for which this response
    ///                           is being constructed
    /// \param modifiedAttributes list of modified attributes
    ///
    //////////////////////////////////////////////////////////////////
    nSetResponse(
            std::shared_ptr<nSetCommand> pCommand,
            attributeIdentifierList_t modifiedAttributes
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     command for which this response is being
    ///                     constructed
    /// \param responseCode response code
    ///
    //////////////////////////////////////////////////////////////////
    nSetResponse(
            std::shared_ptr<nSetCommand> pCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-SET response
    ///
    //////////////////////////////////////////////////////////////////
    nSetResponse(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Returns the list of modified attributes.
    ///
    /// \return list of modified attributes
    ///
    //////////////////////////////////////////////////////////////////
    attributeIdentifierList_t getModifiedAttributes() const;
};


///
/// \brief Represents the N-ACTION command.
///
//////////////////////////////////////////////////////////////////
class nActionCommand: public dimseNCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax          the abstract syntax
    /// \param messageID               the message ID
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param actionID                action ID
    /// \param pActionInformation      dataset containing information
    ///                                about the action
    ///
    //////////////////////////////////////////////////////////////////
    nActionCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            std::uint16_t actionID,
            std::shared_ptr<dataSet> pActionInformation
            );

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax          the abstract syntax
    /// \param messageID               the message ID
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param actionID                action ID
    ///
    //////////////////////////////////////////////////////////////////
    nActionCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            std::uint16_t actionID
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-ACTION command
    ///
    //////////////////////////////////////////////////////////////////
    nActionCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

    ///
    /// \brief Returns the action's ID.
    ///
    /// \return the action's ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getActionID() const;

};


///
/// \brief Represents the N-ACTION response.
///
//////////////////////////////////////////////////////////////////
class nActionResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor for a successful response.
    ///
    /// \param pCommand     the command for which this response is
    ///                     being constructed
    /// \param pActionReply dataSet with information about the
    ///                     action reply
    ///
    //////////////////////////////////////////////////////////////////
    nActionResponse(
            std::shared_ptr<nActionCommand> pCommand,
            std::shared_ptr<dataSet> pActionReply
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     the command for which this response is
    ///                     being constructed
    /// \param responseCode the response code
    ///
    //////////////////////////////////////////////////////////////////
    nActionResponse(
            std::shared_ptr<nActionCommand> pCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-ACTION response
    ///
    //////////////////////////////////////////////////////////////////
    nActionResponse(std::shared_ptr<const associationMessage> pMessage);

    ///
    /// \brief Returns the action's ID.
    ///
    /// \return the action's ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getActionID() const;
};


///
/// \brief The N-CREATE command.
///
//////////////////////////////////////////////////////////////////
class nCreateCommand: public dimseNCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the abstract syntax
    /// \param messageID              the message ID
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    /// \param pAttributeList         the dataset containing the
    ///                               attributes and values
    ///
    //////////////////////////////////////////////////////////////////
    nCreateCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            std::shared_ptr<dataSet> pAttributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the abstract syntax
    /// \param messageID              the message ID
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    nCreateCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-CREATE command
    ///
    //////////////////////////////////////////////////////////////////
    nCreateCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;

};


///
/// \brief Represents the N-CREATE response.
///
//////////////////////////////////////////////////////////////////
class nCreateResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param pCommand       command for which the response is being
    ///                       constructed
    /// \param pAttributeList dataset containing the attributes list
    ///
    //////////////////////////////////////////////////////////////////
    nCreateResponse(
            std::shared_ptr<nCreateCommand> pCommand,
            std::shared_ptr<dataSet> pAttributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               command for which the response
    ///                               is being constructed
    /// \param affectedSopInstanceUid affected SOP instance UID
    /// \param pAttributeList         dataset containing the
    ///                               attributes list
    ///
    //////////////////////////////////////////////////////////////////
    nCreateResponse(
            std::shared_ptr<nCreateCommand> pCommand,
            const std::string& affectedSopInstanceUid,
            std::shared_ptr<dataSet> pAttributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand     command for which the response is being
    ///                     constructed
    /// \param responseCode the response code
    ///
    //////////////////////////////////////////////////////////////////
    nCreateResponse(
            std::shared_ptr<nCreateCommand> pCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pCommand               the command for which the
    ///                               response is being constructed
    /// \param affectedSopInstanceUid created SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    nCreateResponse(
            std::shared_ptr<nCreateCommand> pCommand,
            const std::string& affectedSopInstanceUid
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-CREATE response
    ///
    //////////////////////////////////////////////////////////////////
    nCreateResponse(std::shared_ptr<const associationMessage> pMessage);

};


///
/// \brief The N-DELETE command.
///
//////////////////////////////////////////////////////////////////
class nDeleteCommand: public dimseNCommand
{
public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax          the abstract syntax
    /// \param messageID               the message ID
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    nDeleteCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-DELETE command
    ///
    //////////////////////////////////////////////////////////////////
    nDeleteCommand(std::shared_ptr<const associationMessage> pMessage);

    virtual void validate() const override;
};


///
/// \brief The N-DELETE response.
///
//////////////////////////////////////////////////////////////////
class nDeleteResponse: public dimseResponse
{
public:
    ///
    /// \brief Constructor.
    /// \param pCommand     command for which the response is being
    ///                     constructed
    /// \param responseCode response code
    ///
    //////////////////////////////////////////////////////////////////
    nDeleteResponse(
            std::shared_ptr<nDeleteCommand> pCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param pMessage dimse message containing the N-DELETE response
    ///
    //////////////////////////////////////////////////////////////////
    nDeleteResponse(std::shared_ptr<const associationMessage> pMessage);
};


///
/// \brief Class responsible for sending and receiving DIMSE
///        commands throug an associationBase derived class.
///
//////////////////////////////////////////////////////////////////
class dimseService
{
public:
    dimseService(std::shared_ptr<associationBase> pAssociation);

    ///
    /// \brief Returns the negotiated transfer syntax for a specific
    ///        abstract syntax.
    ///
    /// \param abstractSyntax the abstract syntax for which the
    ///                       transfer syntax is required
    /// \return the negotiated transfer syntax for the specified
    ///         abstract syntax
    ///
    //////////////////////////////////////////////////////////////////
    std::string getTransferSyntax(const std::string &abstractSyntax) const;

    ///
    /// \brief Retrieves an ID that can be used on the next command
    ///        sent through this DimseService object.
    ///
    /// \return an ID that can be used as command ID on a command sent
    ///         through this DimseService object
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getNextCommandID();

    ///
    /// \brief Gets the next command. Blocks until a command is
    ///        available or an exception is thrown (when the
    ///        association is closed or aborted).
    ///
    /// \return the next DIMSE command
    ///
    //////////////////////////////////////////////////////////////////
    std::shared_ptr<dimseNCommand> getCommand();

    ///
    /// \brief Gets the response for a specific command.
    ///        Blocks until the response is available or an exception
    ///        is thrown (when the association is closed or aborted).
    ///
    /// \param commandID the ID of the command for which the
    ///                  response is expected
    /// \return the response to the command with the specified ID
    ///
    //////////////////////////////////////////////////////////////////
    std::shared_ptr<dimseResponse> getResponse(std::uint16_t commandID);

    ///
    /// \brief Gets the response for a specific command.
    ///        Blocks until the response is available or an exception
    ///        is thrown (when the association is closed or aborted).
    ///
    /// \param pCommand the command for which the response is
    ///                  expected
    /// \return the response to the specific command
    ///
    //////////////////////////////////////////////////////////////////
    std::shared_ptr<dimseResponse> getResponse(std::shared_ptr<dimseNCommand> pCommand);

    ///
    /// \brief Send a command or a response.
    ///
    /// \param pCommand the command or response to send
    ///
    //////////////////////////////////////////////////////////////////
    void sendCommandOrResponse(std::shared_ptr<dimseCommandBase> pCommand);

    template<typename T> std::shared_ptr<T> getResponse(std::uint16_t commandID)
    {
        std::shared_ptr<T> pResponse(std::dynamic_pointer_cast<T>(getResponse(commandID)));

        if(pResponse == nullptr)
        {
            throw std::bad_cast();
        }
        return pResponse;
    }

    std::atomic<std::uint16_t> m_messageID;

    std::shared_ptr<associationBase> m_pAssociation;
};






} // namespace implementation

} // namespace imebra


#endif // !defined(CImbxDimse_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
