/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dimse.h
    \brief Declaration of the the DIMSE classes.
*/

#if !defined(imebraDIMSE__INCLUDED_)
#define imebraDIMSE__INCLUDED_

#include <memory>
#include <vector>
#include <string>
#include "acse.h"
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class dimseCommandBase;
    class dimseService;
    class dimseNCommand;
    class dimseCCommand;
    class dimseResponse;
    class cPartialResponse;
    class cStoreResponse;
    class cStoreCommand;
    class cGetCommand;
    class cGetResponse;
    class cFindCommand;
    class cFindResponse;
    class cMoveCommand;
    class cMoveResponse;
    class cEchoCommand;
    class cEchoResponse;
    class cCancelCommand;
    class nEventReportCommand;
    class nEventReportResponse;
    class nGetCommand;
    class nGetResponse;
    class nSetCommand;
    class nSetResponse;
    class nActionCommand;
    class nActionResponse;
    class nCreateCommand;
    class nCreateResponse;
    class nDeleteCommand;
    class nDeleteResponse;
}

class DataSet;
class CStoreCommand;
class CMoveCommand;
class CGetCommand;
class CFindCommand;
class CEchoCommand;
class CCancelCommand;
class NActionCommand;
class NEventReportCommand;
class NCreateCommand;
class NDeleteCommand;
class NSetCommand;
class NGetCommand;


///
/// \brief Base class for the DICOM commands and responses.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API DimseCommandBase
{

public:
    virtual ~DimseCommandBase();

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source DimseCommandBase object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    DimseCommandBase(const DimseCommandBase& source);

    DimseCommandBase& operator=(const DimseCommandBase& source) = delete;

    ///
    /// \brief Returns the data set containing the command (tags in group 0)
    ///
    /// \return the DataSet containing the command tags (tags in group 0)
    ///
    //////////////////////////////////////////////////////////////////
    const DataSet getCommandDataSet() const;

    ///
    /// \brief Returns the data set containing the command's payload.
    ///
    /// \return the DataSet containing the command's payload
    ///
    //////////////////////////////////////////////////////////////////
    const DataSet getPayloadDataSet() const;

    ///
    /// \brief Returns the command's abstract syntax.
    ///
    /// \return the command's abstract syntax
    ///
    //////////////////////////////////////////////////////////////////
    std::string getAbstractSyntax() const;

    ///
    /// \brief Returns the affected SOP instance UID.
    ///
    /// \return the affected SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    std::string getAffectedSopInstanceUid() const;

    ///
    /// \brief Returns the affected SOP Class UID.
    ///
    /// \return the affected SOP Class UID
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

#ifndef SWIG
protected:
    explicit DimseCommandBase(const std::shared_ptr<implementation::dimseCommandBase>& pCommand);

private:
    friend const std::shared_ptr<implementation::dimseCommandBase>& getDimseCommandBaseImplementation(const DimseCommandBase& dimseCommandBase);
    std::shared_ptr<implementation::dimseCommandBase> m_pCommand;
#endif
};


///
/// \brief Base class for the DICOM commands.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API DimseCommand: public DimseCommandBase
{

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source DimseCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    DimseCommand(const DimseCommand& source);

    virtual ~DimseCommand();

    DimseCommand& operator=(const DimseCommand& source) = delete;

    ///
    /// \brief Returns the command ID.
    ///
    /// \return the command ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getID() const;

    ///
    /// \brief Returns the command type.
    ///
    /// \return the command type
    ///
    //////////////////////////////////////////////////////////////////
    dimseCommandType_t getCommandType() const;

    ///
    /// \brief Downcast the DimseCommand to a CStoreCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a CStoreCommand
    ///
    //////////////////////////////////////////////////////////////////
    const CStoreCommand getAsCStoreCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a CMoveCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a CMoveCommand
    ///
    //////////////////////////////////////////////////////////////////
    const CMoveCommand getAsCMoveCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a CGetCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a CGetCommand
    ///
    //////////////////////////////////////////////////////////////////
    const CGetCommand getAsCGetCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a CFindCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a CFindCommand
    ///
    //////////////////////////////////////////////////////////////////
    const CFindCommand getAsCFindCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a CEchoCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a CEchoCommand
    ///
    //////////////////////////////////////////////////////////////////
    const CEchoCommand getAsCEchoCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a CCancelCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a CCancelCommand
    ///
    //////////////////////////////////////////////////////////////////
    const CCancelCommand getAsCCancelCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a NActionCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a NActionCommand
    ///
    //////////////////////////////////////////////////////////////////
    const NActionCommand getAsNActionCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a NEventReportCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a NEventReportCommand
    ///
    //////////////////////////////////////////////////////////////////
    const NEventReportCommand getAsNEventReportCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a NCreateCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a NCreateCommand
    ///
    //////////////////////////////////////////////////////////////////
    const NCreateCommand getAsNCreateCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a NDeleteCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a NDeleteCommand
    ///
    //////////////////////////////////////////////////////////////////
    const NDeleteCommand getAsNDeleteCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a NSetCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a NSetCommand
    ///
    //////////////////////////////////////////////////////////////////
    const NSetCommand getAsNSetCommand() const;

    ///
    /// \brief Downcast the DimseCommand to a NGetCommand,
    ///        unless the command cannot be casted in which case
    ///        a std::bad_cast exception is thrown.
    ///
    /// This method can be used with wrapper languages that don't
    /// support downcast (e.g. Golang).
    /// Other languages can still use the downcast functionalities
    /// embedded into the language.
    ///
    /// \return the DimseCommand downcasted to a NGetCommand
    ///
    //////////////////////////////////////////////////////////////////
    const NGetCommand getAsNGetCommand() const;

protected:
    explicit DimseCommand(const std::shared_ptr<implementation::dimseNCommand>& pCommand);

};


///
/// \brief Base class for the DICOM responses.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API DimseResponse: public DimseCommandBase
{

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source DimseResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    DimseResponse(const DimseResponse& source);

    virtual ~DimseResponse();

    DimseResponse& operator=(const DimseResponse& source) = delete;

    ///
    /// \brief Return the status reported by the DIMSE response.
    ///
    /// \return an enumerated status reported by the DIMSE response
    ///
    ///////////////////////////////////////////////////////////////////////////////
    dimseStatus_t getStatus() const;

    ///
    /// \brief Return the status code as reported by the DIMSE response.
    ///
    /// \return a status code reported by the DIMSE response
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::uint16_t getStatusCode() const;

protected:
    explicit DimseResponse(const std::shared_ptr<implementation::dimseResponse>& pResponse);
};


///
/// \brief Base class for all the partial responses
///        (responses to C-GET, C-MOVE and C-FIND).
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CPartialResponse: public DimseResponse
{

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CPartialResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CPartialResponse(const CPartialResponse& source);

    virtual ~CPartialResponse();

    CPartialResponse& operator=(const CPartialResponse& source) = delete;

    ///
    /// \brief Returns the number of remaining C-STORE sub operations.
    ///
    /// \return number of remaining C-STORE sub operations
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getRemainingSubOperations() const;

    ///
    /// \brief Returns the number of completed C-STORE sub operations.
    ///
    /// \return number of completed C-STORE sub operations
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getCompletedSubOperations() const;

    ///
    /// \brief Returns the number of failed C-STORE sub operations.
    ///
    /// \return number of failed C-STORE sub operations
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getFailedSubOperations() const;

    ///
    /// \brief Returns the number of C-STORE sub operations completed
    ///        with warnings
    ///
    /// \return number of C-STORE sub operations completed with
    ///         warnings
    ///
    //////////////////////////////////////////////////////////////////
    std::uint32_t getWarningSubOperations() const;

protected:
    explicit CPartialResponse(const std::shared_ptr<implementation::cPartialResponse>& pResponse);

};


///
/// \brief Represents a C-STORE request.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CStoreCommand: public DimseCommand
{
    friend class DimseCommand;
    friend class DimseService;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param priority               message priority
    /// \param affectedSopClassUid    affected SOP instance UID
    /// \param affectedSopInstanceUid affected SOP instance UID
    /// \param originatorAET          originator AET (issuer of the
    ///                               C-MOVE or C-GET command)
    /// \param originatorMessageID    message ID of the C-MOVE or
    ///                               C-GET that triggered the C-STORE
    /// \param payload                C-STORE payload
    ///
    //////////////////////////////////////////////////////////////////
    explicit CStoreCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            const std::string& originatorAET,
            std::uint16_t originatorMessageID,
            const DataSet& payload);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CStoreCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CStoreCommand(const CStoreCommand& source);

    virtual ~CStoreCommand();

    CStoreCommand& operator=(const CStoreCommand& source) = delete;

    ///
    /// \brief Returns the AET of the entity that triggered this
    ///        C-STORE request either via a C-MOVE or a C-GET
    ///        request.
    ///
    /// \return the AET for the entity that triggered this C-STORE
    ///         request via a C-MOVE or a C-GET request. Can be empty.
    ///
    //////////////////////////////////////////////////////////////////
    std::string getOriginatorAET() const;

    ///
    /// \brief Returns the message ID of the C-MOVE or C-GET request
    ///        that triggered this C-STORE request.
    ///
    /// \return the message ID of the C-MOVE or C-GET request that
    ///         triggered this C-STORE request. It is zero if it
    ///         wasn't set
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getOriginatorMessageID() const;

protected:
    explicit CStoreCommand(const std::shared_ptr<implementation::cStoreCommand>& pCommand);
};


///
/// \brief Represents a C-STORE response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CStoreResponse: public DimseResponse
{
    friend class DimseService;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param command      the C-STORE command for which this
    ///                     response is being constructed
    /// \param responseCode the response code.
    ///
    //////////////////////////////////////////////////////////////////
    explicit CStoreResponse(const CStoreCommand& command, dimseStatusCode_t responseCode);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CStoreResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CStoreResponse(const CStoreResponse& source);

    virtual ~CStoreResponse();

    CStoreResponse& operator=(const CStoreResponse& source) = delete;

protected:
    explicit CStoreResponse(const std::shared_ptr<implementation::cStoreResponse>& pResponse);
};


///
/// \brief Represents a C-GET command.
///
/// C-GET triggers a C-STORE command that is issued through
/// the same association used for the C-GET, therefore the
/// SCU must have negotiated the SCP role for the abstract
/// syntax (see PresentationContexts and AssociationSCU).
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CGetCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param priority               message priority
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param identifier             the keys to use for the
    ///                               C-GET query
    ///
    //////////////////////////////////////////////////////////////////
    explicit CGetCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            const DataSet& identifier);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CGetCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CGetCommand(const CGetCommand& source);

    virtual ~CGetCommand();

    CGetCommand& operator=(const CGetCommand& source) = delete;

protected:
    explicit CGetCommand(const std::shared_ptr<implementation::cGetCommand>& pCommand);
};


///
/// \brief Represents a C-GET response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CGetResponse: public CPartialResponse
{

    friend class DimseService;

public:

    ///
    /// \brief Constructor to use when failed C-STORE operations
    ///        are present.
    ///
    /// This constructor should be used when the response code
    /// identifies a canceled operation, a failure, a refused
    /// operation or a warning.
    ///
    /// \param receivedCommand        the C-GET command for which
    ///                               this response is being
    ///                               constructed
    /// \param responseCode           the response code.
    /// \param remainingSubOperations number of remaining C-STORE
    ///                               operations
    /// \param completedSubOperations number of completed C-STORE
    ///                               operations
    /// \param failedSubOperations    number of failed C-STORE
    ///                               operations
    /// \param warningSubOperations   number of C-STORE operations
    ///                               completed with warnings
    /// \param identifier             DataSet with the list of failed
    ///                               instance UIDs in the tag
    ///                               FailedSOPInstanceUIDList
    ///                               (0008,0058)
    ///
    //////////////////////////////////////////////////////////////////
    explicit CGetResponse(
            const CGetCommand& receivedCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations,
            const DataSet& identifier);

    ///
    /// \brief Constructor to use when all the C-STORE operations
    ///        succeeded.
    ///
    /// \param receivedCommand        the C-GET command for which
    ///                               this response is being
    ///                               constructed
    /// \param responseCode           the response code.
    /// \param remainingSubOperations number of remaining C-STORE
    ///                               operations
    /// \param completedSubOperations number of completed C-STORE
    ///                               operations
    /// \param failedSubOperations    number of failed C-STORE
    ///                               operations
    /// \param warningSubOperations   number of C-STORE operations
    ///                               completed with warnings
    ///
    //////////////////////////////////////////////////////////////////
    explicit CGetResponse(
            const CGetCommand& receivedCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CGetResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CGetResponse(const CGetResponse& source);

    virtual ~CGetResponse();

    CGetResponse& operator=(const CGetResponse& source) = delete;

protected:
    explicit CGetResponse(const std::shared_ptr<implementation::cGetResponse>& pResponse);
};


///
/// \brief Represents a C-FIND command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CFindCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax      the message's abstract syntax
    ///                            (previously negotiated via the
    ///                            PresentationContexts parameter
    ///                            of the AssociationSCP or
    ///                            AssociationSCU constructors)
    /// \param messageID           message ID (can be retrieved
    ///                            with
    ///                            DimseService::getNextCommandID()
    /// \param priority            the message priority
    /// \param affectedSopClassUid the affected SOP class UID
    /// \param identifier          the dataset with the identifier
    ///                            (list of tags to match and their
    ///                            requested values)
    ///
    //////////////////////////////////////////////////////////////////
    explicit CFindCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            const DataSet& identifier);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CFindCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CFindCommand(const CFindCommand& source);

    virtual ~CFindCommand();

    CFindCommand& operator=(const CFindCommand& source) = delete;

protected:
    explicit CFindCommand(const std::shared_ptr<implementation::cFindCommand>& pCommand);
};


///
/// \brief Represents a C-FIND response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CFindResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor for responses with pending status.
    ///
    /// Responses with pending status carry a DataSet with a single
    /// item from the C-FIND query results.
    ///
    /// \param receivedCommand command for which the reponse is being
    ///                        constructed
    /// \param identifier      one item from the C-FIND query results
    ///
    //////////////////////////////////////////////////////////////////
    explicit CFindResponse(
            const CFindCommand& receivedCommand,
            const DataSet& identifier);

    ///
    /// \brief Constructor for final C-FIND response (success or
    ///        failure).
    ///
    /// \param receivedCommand command for which the response is being
    ///                        constructed
    /// \param responseCode    response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit CFindResponse(
            const CFindCommand& receivedCommand,
            dimseStatusCode_t responseCode);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CFindResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CFindResponse(const CFindResponse& source);

    virtual ~CFindResponse();

    CFindResponse& operator=(const CFindResponse& source) = delete;

protected:
    explicit CFindResponse(const std::shared_ptr<implementation::cFindResponse>& pResponse);
};


///
/// \brief Represents a C-MOVE command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CMoveCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax      the message's abstract syntax
    ///                            (previously negotiated via the
    ///                            PresentationContexts parameter
    ///                            of the AssociationSCP or
    ///                            AssociationSCU constructors)
    /// \param messageID           message ID (can be retrieved
    ///                            with
    ///                            DimseService::getNextCommandID()
    /// \param priority            the message priority
    /// \param affectedSopClassUid affected SOP class UID
    /// \param destinationAET      the destination AET
    /// \param identifier          the dataset with the identifier
    ///                            (list of tags to match and their
    ///                            requested values)
    ///
    //////////////////////////////////////////////////////////////////
    explicit CMoveCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid,
            const std::string& destinationAET,
            const DataSet& identifier);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CMoveCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CMoveCommand(const CMoveCommand& source);

    virtual ~CMoveCommand();

    CMoveCommand& operator=(const CMoveCommand& source) = delete;

    ///
    /// \brief Returns the destination AET.
    ///
    /// \return the destination AET
    ///
    //////////////////////////////////////////////////////////////////
    std::string getDestinationAET() const;

protected:
    explicit CMoveCommand(const std::shared_ptr<implementation::cMoveCommand>& pCommand);
};


///
/// \brief Represents the C-MOVE response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CMoveResponse: public CPartialResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor to use when failed C-STORE operations
    ///        are present.
    ///
    /// This constructor should be used when the response code
    /// identifies a canceled operation, a failure, a refused
    /// operation or a warning.
    ///
    /// \param receivedCommand        the C-GET command for which
    ///                               this response is being
    ///                               constructed
    /// \param responseCode           the response code.
    /// \param remainingSubOperations number of remaining C-STORE
    ///                               operations
    /// \param completedSubOperations number of completed C-STORE
    ///                               operations
    /// \param failedSubOperations    number of failed C-STORE
    ///                               operations
    /// \param warningSubOperations   number of C-STORE operations
    ///                               completed with warnings
    /// \param identifier             DataSet with the list of failed
    ///                               instance UIDs in the tag
    ///                               FailedSOPInstanceUIDList
    ///                               (0008,0058)
    ///
    //////////////////////////////////////////////////////////////////
    explicit CMoveResponse(
            const CMoveCommand& receivedCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations,
            const DataSet& identifier);

    ///
    /// \brief Constructor to use when all the C-STORE operations
    ///        succeeded.
    ///
    /// \param receivedCommand        the C-GET command for which
    ///                               this response is being
    ///                               constructed
    /// \param responseCode           the response code.
    /// \param remainingSubOperations number of remaining C-STORE
    ///                               operations
    /// \param completedSubOperations number of completed C-STORE
    ///                               operations
    /// \param failedSubOperations    number of failed C-STORE
    ///                               operations
    /// \param warningSubOperations   number of C-STORE operations
    ///                               completed with warnings
    ///
    //////////////////////////////////////////////////////////////////
    explicit CMoveResponse(
            const CMoveCommand& receivedCommand,
            dimseStatusCode_t responseCode,
            std::uint32_t remainingSubOperations,
            std::uint32_t completedSubOperations,
            std::uint32_t failedSubOperations,
            std::uint32_t warningSubOperations);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CMoveResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CMoveResponse(const CMoveResponse& source);

    virtual ~CMoveResponse();

    CMoveResponse& operator=(const CMoveResponse& source) = delete;

protected:
    explicit CMoveResponse(const std::shared_ptr<implementation::cMoveResponse>& pResponse);
};


///
/// \brief Represents a C-ECHO command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CEchoCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax      the message's abstract syntax
    ///                            (previously negotiated via the
    ///                            PresentationContexts parameter
    ///                            of the AssociationSCP or
    ///                            AssociationSCU constructors)
    /// \param messageID           message ID (can be retrieved
    ///                            with
    ///                            DimseService::getNextCommandID()
    /// \param priority            the priority
    /// \param affectedSopClassUid the affected SOP class UID
    ///
    //////////////////////////////////////////////////////////////////
    explicit CEchoCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            const std::string& affectedSopClassUid);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CEchoCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CEchoCommand(const CEchoCommand& source);

    virtual ~CEchoCommand();

    CEchoCommand& operator=(const CEchoCommand& source) = delete;

protected:
    explicit CEchoCommand(const std::shared_ptr<implementation::cEchoCommand>& pCommand);
};


///
/// \brief Represents a C-ECHO response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CEchoResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is being
    ///                        constructed
    /// \param responseCode    the response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit CEchoResponse(
            const CEchoCommand& receivedCommand,
            dimseStatusCode_t responseCode);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CEchoResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CEchoResponse(const CEchoResponse& source);

    virtual ~CEchoResponse();

    CEchoResponse& operator=(const CEchoResponse& source) = delete;

protected:
    explicit CEchoResponse(const std::shared_ptr<implementation::cEchoResponse>& pResponse);
};


///
/// \brief Represents the C-CANCEL command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API CCancelCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param priority               the message priority
    /// \param cancelMessageID        the ID of the message to cancel
    ///
    //////////////////////////////////////////////////////////////////
    explicit CCancelCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            dimseCommandPriority_t priority,
            std::uint16_t cancelMessageID);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source CCancelCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    CCancelCommand(const CCancelCommand& source);

    virtual ~CCancelCommand();

    CCancelCommand& operator=(const CCancelCommand& source) = delete;

    ///
    /// \brief Returns the ID of the message to cancel.
    ///
    /// \return the ID of the message to cancel
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getCancelMessageID() const;

protected:
    explicit CCancelCommand(const std::shared_ptr<implementation::cCancelCommand>& pCommand);

};


///
/// \brief Represents a N-EVENT-REPORT command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NEventReportCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    /// \param eventID                the event ID
    ///
    //////////////////////////////////////////////////////////////////
    explicit NEventReportCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            std::uint16_t eventID
            );

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    /// \param eventID                the event ID
    /// \param pEventInformation      the dataset with event
    ///                               information
    ///
    //////////////////////////////////////////////////////////////////
    explicit NEventReportCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            std::uint16_t eventID,
            const DataSet& eventInformation
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NEventReportCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NEventReportCommand(const NEventReportCommand& source);

    virtual ~NEventReportCommand();

    NEventReportCommand& operator=(const NEventReportCommand& source) = delete;

    ///
    /// \brief Returns the event ID.
    ///
    /// \return the event ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getEventID() const;

protected:
    explicit NEventReportCommand(const std::shared_ptr<implementation::nEventReportCommand>& pCommand);
};


///
/// \brief N-EVENT-REPORT response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NEventReportResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor which includes a payload.
    ///        This implies a successful operation.
    ///
    /// \param receivedCommand the N-EVENT request command for which
    ///                        the response is being constructed
    /// \param eventReply      the response payload
    ///
    //////////////////////////////////////////////////////////////////
    explicit NEventReportResponse(
            const NEventReportCommand& receivedCommand,
            const DataSet& eventReply
            );

    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand the N-EVENT request command for which
    ///                        the response is being constructed
    /// \param responseCode    the response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit NEventReportResponse(
            const NEventReportCommand& receivedCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NEventReportResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NEventReportResponse(const NEventReportResponse& source);

    virtual ~NEventReportResponse();

    NEventReportResponse& operator=(const NEventReportResponse& source) = delete;

    ///
    /// \brief Get the event ID. The response may omit this
    ///        information.
    ///
    /// \return the event ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getEventID() const;

protected:
    explicit NEventReportResponse(const std::shared_ptr<implementation::nEventReportResponse>& pResponse);
};


///
/// \brief Represents a N-GET command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NGetCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param attributeIdentifierList the list of identifier tags.
    ///                                An empty list means "all the
    ///                                tags"
    ///
    //////////////////////////////////////////////////////////////////
    explicit NGetCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            const attributeIdentifierList_t& attributeIdentifierList
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NGetCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NGetCommand(const NGetCommand& source);

    virtual ~NGetCommand();

    NGetCommand& operator=(const NGetCommand& source) = delete;

    ///
    /// \brief Return the list of attributes to retrieve. an empty
    ///        list means all.
    ///
    /// \return the list of attributes to retrieve. An empty list
    ///         means "all the attributes"
    ///
    //////////////////////////////////////////////////////////////////
    attributeIdentifierList_t getAttributeList() const;

protected:
    explicit NGetCommand(const std::shared_ptr<implementation::nGetCommand>& pCommand);
};


///
/// \brief Represents a N-GET response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NGetResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is
    ///                        being constructed
    /// \param responseCode    response code
    /// \param attributeList   dataset containing the list of
    ///                        attribute identifiers
    ///
    //////////////////////////////////////////////////////////////////
    explicit NGetResponse(
            const NGetCommand& receivedCommand,
            dimseStatusCode_t responseCode,
            const DataSet& attributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is being
    ///                        constructed
    /// \param responseCode    response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit NGetResponse(
            const NGetCommand& receivedCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NGetResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NGetResponse(const NGetResponse& source);

    virtual ~NGetResponse();

    NGetResponse& operator=(const NGetResponse& source) = delete;

protected:
    explicit NGetResponse(const std::shared_ptr<implementation::nGetResponse>& pResponse);
};


///
/// \brief Represents the N-SET command
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NSetCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param modificationList        dataset containing the new
    ///                                attributes values
    ///
    //////////////////////////////////////////////////////////////////
    explicit NSetCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            const DataSet& modificationList
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NSetCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NSetCommand(const NSetCommand& source);

    virtual ~NSetCommand();

    NSetCommand& operator=(const NSetCommand& source) = delete;

protected:
    explicit NSetCommand(const std::shared_ptr<implementation::nSetCommand>& pCommand);
};


///
/// \brief Represents the N-SET response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NSetResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor of a successful response.
    ///
    /// \param receivedCommand    the command for which this response
    ///                           is being constructed
    /// \param modifiedAttributes list of modified attributes
    ///
    //////////////////////////////////////////////////////////////////
    explicit NSetResponse(
            const NSetCommand& receivedCommand,
            attributeIdentifierList_t modifiedAttributes
            );

    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand    the command for which this response
    ///                           is being constructed
    /// \param responseCode       response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit NSetResponse(
            const NSetCommand& receivedCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NSetResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NSetResponse(const NSetResponse& source);

    virtual ~NSetResponse();

    NSetResponse& operator=(const NSetResponse& source) = delete;

    ///
    /// \brief Returns the list of modified attributes.
    ///
    /// \return list of modified attributes
    ///
    //////////////////////////////////////////////////////////////////
    attributeIdentifierList_t getModifiedAttributes() const;

protected:
    explicit NSetResponse(const std::shared_ptr<implementation::nSetResponse>& pResponse);
};


///
/// \brief Represents the N-ACTION command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NActionCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param actionID                action ID
    /// \param actionInformation       dataset containing information
    ///                                about the action
    ///
    //////////////////////////////////////////////////////////////////
    explicit NActionCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            std::uint16_t actionID,
            const DataSet& actionInformation
            );

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance
    ///                                UID
    /// \param actionID                action ID
    ///
    //////////////////////////////////////////////////////////////////
    explicit NActionCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid,
            std::uint16_t actionID
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NActionCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NActionCommand(const NActionCommand& source);

    virtual ~NActionCommand();

    NActionCommand& operator=(const NActionCommand& source) = delete;

    ///
    /// \brief Returns the action's ID.
    ///
    /// \return the action's ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getActionID() const;

protected:
    explicit NActionCommand(const std::shared_ptr<implementation::nActionCommand>& pCommand);
};


///
/// \brief Represents the N-ACTION response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NActionResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor for a successful reply.
    ///
    /// \param receivedCommand the command for which this response is
    ///                        being constructed
    /// \param actionReply     dataSet with information about the
    ///                        action reply
    ///
    //////////////////////////////////////////////////////////////////
    explicit NActionResponse(
            const NActionCommand& receivedCommand,
            const DataSet& actionReply
            );

    ///
    /// \brief Constructor for a successful reply.
    ///
    /// \param receivedCommand the command for which this response is
    ///                        being constructed
    /// \param responseCode    the response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit NActionResponse(
            const NActionCommand& receivedCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NActionResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NActionResponse(const NActionResponse& source);

    virtual ~NActionResponse();

    NActionResponse& operator=(const NActionResponse& source) = delete;

    ///
    /// \brief Returns the action's ID.
    ///
    /// \return the action's ID
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getActionID() const;

protected:
    explicit NActionResponse(const std::shared_ptr<implementation::nActionResponse>& pResponse);
};


///
/// \brief The N-CREATE command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NCreateCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    /// \param attributeList          the dataset containing the
    ///                               attributes and values
    ///
    //////////////////////////////////////////////////////////////////
    explicit NCreateCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid,
            const DataSet& attributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param affectedSopClassUid    the affected SOP class UID
    /// \param affectedSopInstanceUid the affected SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    explicit NCreateCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& affectedSopClassUid,
            const std::string& affectedSopInstanceUid
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NCreateCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NCreateCommand(const NCreateCommand& source);

    virtual ~NCreateCommand();

    NCreateCommand& operator=(const NCreateCommand& source) = delete;

protected:
    explicit NCreateCommand(const std::shared_ptr<implementation::nCreateCommand>& pCommand);
};


///
/// \brief Represents the N-CREATE response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NCreateResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is
    ///                        being constructed
    /// \param attributeList   dataset containing the attributes list
    ///
    //////////////////////////////////////////////////////////////////
    NCreateResponse(
            const NCreateCommand& receivedCommand,
            const DataSet& attributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is
    ///                        being constructed
    /// \param affectedSopInstanceUid affected SOP instance UID
    /// \param attributeList   dataset containing the attributes list
    ///
    //////////////////////////////////////////////////////////////////
    explicit NCreateResponse(
            const NCreateCommand& receivedCommand,
            const std::string& affectedSopInstanceUid,
            const DataSet& attributeList
            );

    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is
    ///                        being constructed
    /// \param responseCode the response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit NCreateResponse(
            const NCreateCommand& receivedCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand       command for which the response is
    ///                              being constructed
    /// \param affectedSopInstanceUid created SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    explicit NCreateResponse(
            const NCreateCommand& receivedCommand,
            const std::string& affectedSopInstanceUid
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NCreateResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NCreateResponse(const NCreateResponse& source);

    virtual ~NCreateResponse();

    NCreateResponse& operator=(const NCreateResponse& source) = delete;

protected:
    explicit NCreateResponse(const std::shared_ptr<implementation::nCreateResponse>& pResponse);
};


///
/// \brief The N-DELETE command.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NDeleteCommand: public DimseCommand
{

    friend class DimseService;
    friend class DimseCommand;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param abstractSyntax         the message's abstract syntax
    ///                               (previously negotiated via the
    ///                               PresentationContexts parameter
    ///                               of the AssociationSCP or
    ///                               AssociationSCU constructors)
    /// \param messageID              message ID (can be retrieved
    ///                               with
    ///                               DimseService::getNextCommandID()
    /// \param requestedSopClassUid    the requested SOP class UID
    /// \param requestedSopInstanceUid the requested SOP instance UID
    ///
    //////////////////////////////////////////////////////////////////
    explicit NDeleteCommand(
            const std::string& abstractSyntax,
            std::uint16_t messageID,
            const std::string& requestedSopClassUid,
            const std::string& requestedSopInstanceUid
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NDeleteCommand object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NDeleteCommand(const NDeleteCommand& source);

    virtual ~NDeleteCommand();

    NDeleteCommand& operator=(const NDeleteCommand& source) = delete;

protected:
    explicit NDeleteCommand(const std::shared_ptr<implementation::nDeleteCommand>& pCommand);
};


///
/// \brief The N-DELETE response.
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API NDeleteResponse: public DimseResponse
{

    friend class DimseService;

public:
    ///
    /// \brief Constructor.
    ///
    /// \param receivedCommand command for which the response is
    ///                        being constructed
    /// \param responseCode    response code
    ///
    //////////////////////////////////////////////////////////////////
    explicit NDeleteResponse(
            NDeleteCommand& receivedCommand,
            dimseStatusCode_t responseCode
            );

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source NDeleteResponse object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    NDeleteResponse(const NDeleteResponse& source);

    virtual ~NDeleteResponse();

    NDeleteResponse& operator=(const NDeleteResponse& source) = delete;

protected:
    explicit NDeleteResponse(const std::shared_ptr<implementation::nDeleteResponse>& pResponse);
};


///
/// \brief Sends and receives DICOM commands and responses through
///        a AssociationSCU or an AssociationSCP.
///
/// The DimseService supplies getNextCommandID() in order to
/// obtain the ID for the commands sent through the DimseService
/// object.
///
/// \param association the AssociationBase derived class through
///                    which the DICOM commands and responses are
///                    sent and received
///
//////////////////////////////////////////////////////////////////
class IMEBRA_API DimseService
{

public:
    ///
    /// \brief Constructor.
    ///
    /// \param association the AssociationBase derived class through
    ///                    which the DICOM commands and responses are
    ///                    sent and received
    ///
    //////////////////////////////////////////////////////////////////
    explicit DimseService(AssociationBase& association);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source DimseService object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    DimseService(const DimseService& source);

    virtual ~DimseService();

    DimseService& operator=(const DimseService& source) = delete;

    ///
    /// \brief Returns the negotiated transfer syntax for a specific
    ///        abstract syntax.
    ///
    /// This method is multithread safe.
    ///
    /// \param abstractSyntax the abstract syntax for which the
    ///                       transfer syntax is needed
    /// \return the negotiated transfer syntax for the specified
    ///         abstract syntax
    ///
    //////////////////////////////////////////////////////////////////
    std::string getTransferSyntax(const std::string& abstractSyntax) const;

    ///
    /// \brief Retrieves an ID that can be used on the next command
    ///        sent through this DimseService object.
    ///
    /// This method is multithread safe.
    ///
    /// \return an ID that can be used as command ID on a command sent
    ///         through this DimseService object
    ///
    //////////////////////////////////////////////////////////////////
    std::uint16_t getNextCommandID();

    ///
    /// \brief Retrieves the next incoming DICOM command.
    ///
    /// The command blocks until the command is available or until
    /// the association is closed, either by the connected peer or
    /// by other means (e.g because of the DIMSE timeout), in which
    /// case the exception StreamEOFError is thrown.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// This method can be called by several threads at once:
    /// each thread will receive a DIMSE command and can reply
    /// to the command via sendCommandOrResponse().
    ///
    /// The underlying AssociationBase object will take care of
    /// limiting the number of incoming command according to the
    /// maximum number of performed operation negotiated for
    /// the association.
    ///
    /// Throws StreamEOFError if the association is closed while the
    /// method is waiting for an incoming command.
    ///
    /// \return the next incoming DICOM command
    ///
    //////////////////////////////////////////////////////////////////
    const DimseCommand getCommand();

    ///
    /// \brief Sends a DICOM command or response through the
    ///        association.
    ///
    /// This method is multithread safe.
    ///
    /// \param command the command or response to send
    ///
    //////////////////////////////////////////////////////////////////
    void sendCommandOrResponse(const DimseCommandBase& command);

    ///
    /// \brief Wait for the response for the specified C-STORE
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent C-CSTORE command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const CStoreResponse getCStoreResponse(const CStoreCommand& command);

    ///
    /// \brief Wait for the response for the specified C-GET
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent C-GET command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const CGetResponse getCGetResponse(const CGetCommand& command);

    ///
    /// \brief Wait for the response for the specified C-FIND
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent C-FIND command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const CFindResponse getCFindResponse(const CFindCommand& command);

    ///
    /// \brief Wait for the response for the specified C-MOVE
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent C-MOVE command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const CMoveResponse getCMoveResponse(const CMoveCommand& command);

    ///
    /// \brief Wait for the response for the specified C-ECHO
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent C-ECHO command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const CEchoResponse getCEchoResponse(const CEchoCommand& command);

    ///
    /// \brief Wait for the response for the specified N-EVENT-REPORT
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent N-EVENT-REPORT command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const NEventReportResponse getNEventReportResponse(const NEventReportCommand& command);

    ///
    /// \brief Wait for the response for the specified N-GET
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent N-GET command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const NGetResponse getNGetResponse(const NGetCommand& command);

    ///
    /// \brief Wait for the response for the specified N-SET
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent N-SET command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const NSetResponse getNSetResponse(const NSetCommand& command);

    ///
    /// \brief Wait for the response for the specified N-ACTION
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent N-ACTION command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const NActionResponse getNActionResponse(const NActionCommand& command);

    ///
    /// \brief Wait for the response for the specified N-CREATE
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent N-CREATE command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const NCreateResponse getNCreateResponse(const NCreateCommand& command);

    ///
    /// \brief Wait for the response for the specified N-DELETE
    ///        command and returns it.
    ///
    /// Other threads can wait for other commands or responses from
    /// the same DIMSE service. All the commands and responses are
    /// received by a secondary thread and distributed to all the
    /// listener waiting for a command or a response.
    ///
    /// \param command the sent N-DELETE command for which to
    ///                wait for a response
    /// \return the response for the specified command
    ///
    //////////////////////////////////////////////////////////////////
    const NDeleteResponse getNDeleteResponse(const NDeleteCommand& command);

#ifndef SWIG
private:
    friend const std::shared_ptr<implementation::dimseService>& getDimseServiceImplementation(const DimseService& service);
    std::shared_ptr<implementation::dimseService> m_pDimseService;
#endif
};


}

#endif // !defined(imebraDIMSE__INCLUDED_)
