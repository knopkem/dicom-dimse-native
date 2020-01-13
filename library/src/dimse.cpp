/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dimse.cpp
    \brief Implementation of the the DIMSE classes.
*/

#include "../include/imebra/dimse.h"
#include "../include/imebra/dataSet.h"
#include "../implementation/dimseImpl.h"
#include <typeinfo>

namespace imebra
{

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// DimseCommandBase
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
DimseCommandBase::DimseCommandBase(const std::shared_ptr<implementation::dimseCommandBase>& pCommand):
    m_pCommand(pCommand)
{
}

const std::shared_ptr<implementation::dimseCommandBase>& getDimseCommandBaseImplementation(const DimseCommandBase& commandBase)
{
    return commandBase.m_pCommand;
}


DimseCommandBase::DimseCommandBase(const DimseCommandBase& source):
    m_pCommand(getDimseCommandBaseImplementation(source))
{
}


//////////////////////////////////////////////////////////////////
//
// Destructor
//
//////////////////////////////////////////////////////////////////
DimseCommandBase::~DimseCommandBase()
{
}


//////////////////////////////////////////////////////////////////
//
// Return the command DataSet.
//
//////////////////////////////////////////////////////////////////
const DataSet DimseCommandBase::getCommandDataSet() const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pCommand->getCommandDataSet());

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Return the command payload.
//
//////////////////////////////////////////////////////////////////
const DataSet DimseCommandBase::getPayloadDataSet() const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pCommand->getPayloadDataSet());

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Return the abstract syntax
//
//////////////////////////////////////////////////////////////////
std::string DimseCommandBase::getAbstractSyntax() const
{
    IMEBRA_FUNCTION_START();

    return m_pCommand->getAbstractSyntax();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the affected SOP instance UID
//
//////////////////////////////////////////////////////////////////
std::string DimseCommandBase::getAffectedSopInstanceUid() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::dimseCCommand>(m_pCommand))->getAffectedSopInstanceUid();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the affected SOP class UID
//
//////////////////////////////////////////////////////////////////
std::string DimseCommandBase::getAffectedSopClassUid() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::dimseCCommand>(m_pCommand))->getAffectedSopClassUid();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the requested SOP instance UID
//
//////////////////////////////////////////////////////////////////
std::string DimseCommandBase::getRequestedSopInstanceUid() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::dimseCCommand>(m_pCommand))->getRequestedSopInstanceUid();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the requested SOP class UID
//
//////////////////////////////////////////////////////////////////
std::string DimseCommandBase::getRequestedSopClassUid() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::dimseCCommand>(m_pCommand))->getRequestedSopClassUid();

    IMEBRA_FUNCTION_END_LOG();
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// DimseCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
DimseCommand::DimseCommand(const std::shared_ptr<implementation::dimseNCommand>& pCommand):
    DimseCommandBase(pCommand)
{
}


DimseCommand::DimseCommand(const DimseCommand &source): DimseCommandBase(source)
{
}


DimseCommand::~DimseCommand()
{
}


//////////////////////////////////////////////////////////////////
//
// Get the command ID
//
//////////////////////////////////////////////////////////////////
std::uint16_t DimseCommand::getID() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::dimseCCommand>(getDimseCommandBaseImplementation(*this)))->getID();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command type
//
//////////////////////////////////////////////////////////////////
dimseCommandType_t DimseCommand::getCommandType() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::dimseNCommand>(getDimseCommandBaseImplementation(*this)))->getCommandType();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a CStoreCommand
//
//////////////////////////////////////////////////////////////////
const CStoreCommand DimseCommand::getAsCStoreCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::cStoreCommand> pCommand(std::dynamic_pointer_cast<implementation::cStoreCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return CStoreCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a CMoveCommand
//
//////////////////////////////////////////////////////////////////
const CMoveCommand DimseCommand::getAsCMoveCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::cMoveCommand> pCommand(std::dynamic_pointer_cast<implementation::cMoveCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return CMoveCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a CGetCommand
//
//////////////////////////////////////////////////////////////////
const CGetCommand DimseCommand::getAsCGetCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::cGetCommand> pCommand(std::dynamic_pointer_cast<implementation::cGetCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return CGetCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a CFindCommand
//
//////////////////////////////////////////////////////////////////
const CFindCommand DimseCommand::getAsCFindCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::cFindCommand> pCommand(std::dynamic_pointer_cast<implementation::cFindCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return CFindCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a CEchoCommand
//
//////////////////////////////////////////////////////////////////
const CEchoCommand DimseCommand::getAsCEchoCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::cEchoCommand> pCommand(std::dynamic_pointer_cast<implementation::cEchoCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return CEchoCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a CCancelCommand
//
//////////////////////////////////////////////////////////////////
const CCancelCommand DimseCommand::getAsCCancelCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::cCancelCommand> pCommand(std::dynamic_pointer_cast<implementation::cCancelCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return CCancelCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a NActionCommand
//
//////////////////////////////////////////////////////////////////
const NActionCommand DimseCommand::getAsNActionCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::nActionCommand> pCommand(std::dynamic_pointer_cast<implementation::nActionCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return NActionCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a NEventReportCommand
//
//////////////////////////////////////////////////////////////////
const NEventReportCommand DimseCommand::getAsNEventReportCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::nEventReportCommand> pCommand(std::dynamic_pointer_cast<implementation::nEventReportCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return NEventReportCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a NCreate
//
//////////////////////////////////////////////////////////////////
const NCreateCommand DimseCommand::getAsNCreateCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::nCreateCommand> pCommand(std::dynamic_pointer_cast<implementation::nCreateCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return NCreateCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a NDeleteCommand
//
//////////////////////////////////////////////////////////////////
const NDeleteCommand DimseCommand::getAsNDeleteCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::nDeleteCommand> pCommand(std::dynamic_pointer_cast<implementation::nDeleteCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return NDeleteCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a NSetCommand
//
//////////////////////////////////////////////////////////////////
const NSetCommand DimseCommand::getAsNSetCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::nSetCommand> pCommand(std::dynamic_pointer_cast<implementation::nSetCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return NSetCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the command as a NGetCommand
//
//////////////////////////////////////////////////////////////////
const NGetCommand DimseCommand::getAsNGetCommand() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::nGetCommand> pCommand(std::dynamic_pointer_cast<implementation::nGetCommand>(getDimseCommandBaseImplementation(*this)));
    if(pCommand.get() == nullptr)
    {
        throw std::bad_cast();
    }
    return NGetCommand(pCommand);

    IMEBRA_FUNCTION_END_LOG();
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// DimseResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
DimseResponse::DimseResponse(const std::shared_ptr<implementation::dimseResponse>& pResponse):
    DimseCommandBase(pResponse)
{
}


DimseResponse::DimseResponse(const DimseResponse &source): DimseCommandBase(getDimseCommandBaseImplementation(source))
{
}


DimseResponse::~DimseResponse()
{
}


//////////////////////////////////////////////////////////////////
//
// Get the response status
//
//////////////////////////////////////////////////////////////////
dimseStatus_t DimseResponse::getStatus() const
{
    IMEBRA_FUNCTION_START();

    return std::static_pointer_cast<implementation::dimseResponse>(getDimseCommandBaseImplementation(*this))->getStatus();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the response status code
//
//////////////////////////////////////////////////////////////////
std::uint16_t DimseResponse::getStatusCode() const
{
    IMEBRA_FUNCTION_START();

    return std::static_pointer_cast<implementation::dimseResponse>(getDimseCommandBaseImplementation(*this))->getStatusCode();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CStoreCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CStoreCommand::CStoreCommand(const std::shared_ptr<implementation::cStoreCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CStoreCommand::CStoreCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        dimseCommandPriority_t priority,
        const std::string& affectedSopClassUid,
        const std::string& affectedSopInstanceUid,
        const std::string& originatorAET,
        std::uint16_t originatorMessageID,
        const DataSet& payload):
    DimseCommand(std::make_shared<implementation::cStoreCommand>(
                     abstractSyntax,
                     messageID,
                     priority,
                     affectedSopClassUid,
                     affectedSopInstanceUid,
                     originatorAET,
                     originatorMessageID,
                     getDataSetImplementation(payload)))
{
}


CStoreCommand::CStoreCommand(const CStoreCommand &source): DimseCommand(std::static_pointer_cast<implementation::dimseCCommand>(getDimseCommandBaseImplementation(source)))
{
}


CStoreCommand::~CStoreCommand()
{
}


//////////////////////////////////////////////////////////////////
//
// Get the originator AET
//
//////////////////////////////////////////////////////////////////
std::string CStoreCommand::getOriginatorAET() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cStoreCommand>(getDimseCommandBaseImplementation(*this)))->getOriginatorAET();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the originator message ID
//
//////////////////////////////////////////////////////////////////
std::uint16_t CStoreCommand::getOriginatorMessageID() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cStoreCommand>(getDimseCommandBaseImplementation(*this)))->getOriginatorMessageID();

    IMEBRA_FUNCTION_END_LOG();
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CStoreCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
CStoreResponse::CStoreResponse(const std::shared_ptr<implementation::cStoreResponse>& pResponse):
    DimseResponse(pResponse)
{
}

CStoreResponse::CStoreResponse(const CStoreCommand& command, dimseStatusCode_t responseCode):
    DimseResponse(std::make_shared<implementation::cStoreResponse>(
                      std::static_pointer_cast<implementation::cStoreCommand>(getDimseCommandBaseImplementation(command)), responseCode))
{
}


CStoreResponse::CStoreResponse(const CStoreResponse &source): DimseResponse(source)
{
}


CStoreResponse::~CStoreResponse()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CGetCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CGetCommand::CGetCommand(const std::shared_ptr<implementation::cGetCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CGetCommand::CGetCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        dimseCommandPriority_t priority,
        const std::string& affectedSopClassUid,
        const DataSet& identifier):
    DimseCommand(std::make_shared<implementation::cGetCommand>(
                     abstractSyntax,
                     messageID,
                     priority,
                     affectedSopClassUid,
                     getDataSetImplementation(identifier)))
{
}


CGetCommand::CGetCommand(const CGetCommand &source): DimseCommand(std::static_pointer_cast<implementation::dimseCCommand>(getDimseCommandBaseImplementation(source)))
{
}


CGetCommand::~CGetCommand()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CPartialResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CPartialResponse::CPartialResponse(const std::shared_ptr<implementation::cPartialResponse>& pResponse):
    DimseResponse(pResponse)
{
}


CPartialResponse::CPartialResponse(const CPartialResponse &source): DimseResponse(std::static_pointer_cast<implementation::dimseResponse>(getDimseCommandBaseImplementation(source)))
{
}


CPartialResponse::~CPartialResponse()
{
}


//////////////////////////////////////////////////////////////////
//
// Get the number of remaining sub operations
//
//////////////////////////////////////////////////////////////////
std::uint32_t CPartialResponse::getRemainingSubOperations() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cPartialResponse>(getDimseCommandBaseImplementation(*this)))->getRemainingSubOperations();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the number of completed sub operations
//
//////////////////////////////////////////////////////////////////
std::uint32_t CPartialResponse::getCompletedSubOperations() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cPartialResponse>(getDimseCommandBaseImplementation(*this)))->getCompletedSubOperations();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the number of failed sub operations
//
//////////////////////////////////////////////////////////////////
std::uint32_t CPartialResponse::getFailedSubOperations() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cPartialResponse>(getDimseCommandBaseImplementation(*this)))->getFailedSubOperations();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Get the number of sub operations completed with warnings
//
//////////////////////////////////////////////////////////////////
std::uint32_t CPartialResponse::getWarningSubOperations() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cPartialResponse>(getDimseCommandBaseImplementation(*this)))->getWarningSubOperations();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CGetResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
CGetResponse::CGetResponse(const std::shared_ptr<implementation::cGetResponse>& pResponse):
    CPartialResponse(pResponse)
{
}

CGetResponse::CGetResponse(
        const CGetCommand& receivedCommand,
        dimseStatusCode_t responseCode,
        std::uint32_t remainingSubOperations,
        std::uint32_t completedSubOperations,
        std::uint32_t failedSubOperations,
        std::uint32_t warningSubOperations,
        const DataSet& identifier):
    CGetResponse(
        std::make_shared<implementation::cGetResponse>(
            std::static_pointer_cast<implementation::cGetCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode,
            remainingSubOperations,
            completedSubOperations,
            failedSubOperations,
            warningSubOperations,
            getDataSetImplementation(identifier)
            ))
{
}

CGetResponse::CGetResponse(
        const CGetCommand& receivedCommand,
        dimseStatusCode_t responseCode,
        std::uint32_t remainingSubOperations,
        std::uint32_t completedSubOperations,
        std::uint32_t failedSubOperations,
        std::uint32_t warningSubOperations):
    CGetResponse(
        std::make_shared<implementation::cGetResponse>(
            std::static_pointer_cast<implementation::cGetCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode,
            remainingSubOperations,
            completedSubOperations,
            failedSubOperations,
            warningSubOperations,
            nullptr
            ))
{
}


CGetResponse::CGetResponse(const CGetResponse &source): CPartialResponse(source)
{
}


CGetResponse::~CGetResponse()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CFindCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CFindCommand::CFindCommand(const std::shared_ptr<implementation::cFindCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CFindCommand::CFindCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        dimseCommandPriority_t priority,
        const std::string& affectedSopClassUid,
        const DataSet& identifier):
    DimseCommand(std::make_shared<implementation::cFindCommand>(
                     abstractSyntax,
                     messageID,
                     priority,
                     affectedSopClassUid,
                     getDataSetImplementation(identifier)))
{
}


CFindCommand::CFindCommand(const CFindCommand &source): DimseCommand(source)
{
}


CFindCommand::~CFindCommand()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CFindResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
CFindResponse::CFindResponse(const std::shared_ptr<implementation::cFindResponse>& pResponse):
    DimseResponse(pResponse)
{
}

CFindResponse::CFindResponse(
        const CFindCommand& receivedCommand,
        const DataSet& identifier):
    CFindResponse(
        std::make_shared<implementation::cFindResponse>(
            std::static_pointer_cast<implementation::cFindCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            getDataSetImplementation(identifier)
            ))
{
}


CFindResponse::CFindResponse(
        const CFindCommand& receivedCommand,
        dimseStatusCode_t responseCode):
    CFindResponse(
        std::make_shared<implementation::cFindResponse>(
            std::static_pointer_cast<implementation::cFindCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode
            ))
{
}


CFindResponse::CFindResponse(const CFindResponse &source): DimseResponse(source)
{
}


CFindResponse::~CFindResponse()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CMoveCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CMoveCommand::CMoveCommand(const std::shared_ptr<implementation::cMoveCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CMoveCommand::CMoveCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        dimseCommandPriority_t priority,
        const std::string& affectedSopClassUid,
        const std::string& destinationAET,
        const DataSet& identifier):
    DimseCommand(std::make_shared<implementation::cMoveCommand>(
                     abstractSyntax,
                     messageID,
                     priority,
                     affectedSopClassUid,
                     destinationAET,
                     getDataSetImplementation(identifier)))
{
}


CMoveCommand::CMoveCommand(const CMoveCommand &source): DimseCommand(std::static_pointer_cast<implementation::dimseCCommand>(getDimseCommandBaseImplementation(source)))
{
}


CMoveCommand::~CMoveCommand()
{
}


//////////////////////////////////////////////////////////////////
//
// Return the destination AET
//
//////////////////////////////////////////////////////////////////
std::string CMoveCommand::getDestinationAET() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cMoveCommand>(getDimseCommandBaseImplementation(*this)))->getDestinationAET();

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CMoveResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
CMoveResponse::CMoveResponse(const std::shared_ptr<implementation::cMoveResponse>& pResponse):
    CPartialResponse(pResponse)
{
}

CMoveResponse::CMoveResponse(
        const CMoveCommand& receivedCommand,
        dimseStatusCode_t responseCode,
        std::uint32_t remainingSubOperations,
        std::uint32_t completedSubOperations,
        std::uint32_t failedSubOperations,
        std::uint32_t warningSubOperations,
        const DataSet& identifier):
    CMoveResponse(
        std::make_shared<implementation::cMoveResponse>(
            std::static_pointer_cast<implementation::cMoveCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode,
            remainingSubOperations,
            completedSubOperations,
            failedSubOperations,
            warningSubOperations,
            getDataSetImplementation(identifier)
            ))
{
}


CMoveResponse::CMoveResponse(
        const CMoveCommand& receivedCommand,
        dimseStatusCode_t responseCode,
        std::uint32_t remainingSubOperations,
        std::uint32_t completedSubOperations,
        std::uint32_t failedSubOperations,
        std::uint32_t warningSubOperations):
    CMoveResponse(
        std::make_shared<implementation::cMoveResponse>(
            std::static_pointer_cast<implementation::cMoveCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode,
            remainingSubOperations,
            completedSubOperations,
            failedSubOperations,
            warningSubOperations,
            nullptr
            ))
{
}


CMoveResponse::CMoveResponse(const CMoveResponse& source): CPartialResponse(source)
{
}


CMoveResponse::~CMoveResponse()
{
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CEchoCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CEchoCommand::CEchoCommand(const std::shared_ptr<implementation::cEchoCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CEchoCommand::CEchoCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        dimseCommandPriority_t priority,
        const std::string& affectedSopClassUid):
    DimseCommand(std::make_shared<implementation::cEchoCommand>(
                     abstractSyntax,
                     messageID,
                     priority,
                     affectedSopClassUid))
{
}


CEchoCommand::CEchoCommand(const CEchoCommand& source): DimseCommand(source)
{
}


CEchoCommand::~CEchoCommand()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CEchoResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
CEchoResponse::CEchoResponse(const std::shared_ptr<implementation::cEchoResponse>& pResponse):
    DimseResponse(pResponse)
{
}

CEchoResponse::CEchoResponse(
        const CEchoCommand& receivedCommand,
        dimseStatusCode_t responseCode):
    DimseResponse(
        std::make_shared<implementation::cEchoResponse>(
            std::static_pointer_cast<implementation::cEchoCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode
            ))
{
}


CEchoResponse::CEchoResponse(const CEchoResponse& source): DimseResponse(source)
{
}


CEchoResponse::~CEchoResponse()
{
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// CCancelCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CCancelCommand::CCancelCommand(const std::shared_ptr<implementation::cCancelCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
CCancelCommand::CCancelCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        dimseCommandPriority_t priority,
        std::uint16_t cancelMessageID):
    DimseCommand(std::make_shared<implementation::cCancelCommand>(
                     abstractSyntax,
                     messageID,
                     priority,
                     cancelMessageID))
{
}


CCancelCommand::CCancelCommand(const CCancelCommand& source): DimseCommand(source)
{
}


CCancelCommand::~CCancelCommand()
{
}


//////////////////////////////////////////////////////////////////
//
// Return the ID of the command to cancel
//
//////////////////////////////////////////////////////////////////
std::uint16_t CCancelCommand::getCancelMessageID() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::cCancelCommand>(getDimseCommandBaseImplementation(*this)))->getCancelMessageID();

    IMEBRA_FUNCTION_END_LOG();
}


DimseService::DimseService(AssociationBase& association):
    m_pDimseService(std::make_shared<implementation::dimseService>(getAssociationBaseImplementation(association)))
{
}


DimseService::DimseService(const DimseService& source): m_pDimseService(getDimseServiceImplementation(source))
{
}


DimseService::~DimseService()
{
}


const std::shared_ptr<implementation::dimseService>& getDimseServiceImplementation(const DimseService& service)
{
    return service.m_pDimseService;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NEventReportCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NEventReportCommand::NEventReportCommand(const std::shared_ptr<implementation::nEventReportCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NEventReportCommand::NEventReportCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& affectedSopClassUid,
        const std::string& affectedSopInstanceUid,
        std::uint16_t eventID
        ):
    DimseCommand(std::make_shared<implementation::nEventReportCommand>(
                     abstractSyntax,
                     messageID,
                     affectedSopClassUid,
                     affectedSopInstanceUid,
                     eventID))
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NEventReportCommand::NEventReportCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& affectedSopClassUid,
        const std::string& affectedSopInstanceUid,
        std::uint16_t eventID,
        const DataSet& eventInformation
        ):
    DimseCommand(std::make_shared<implementation::nEventReportCommand>(
                     abstractSyntax,
                     messageID,
                     affectedSopClassUid,
                     affectedSopInstanceUid,
                     eventID,
                     getDataSetImplementation(eventInformation)))
{
}


NEventReportCommand::NEventReportCommand(const NEventReportCommand& source): DimseCommand(source)
{
}


NEventReportCommand::~NEventReportCommand()
{
}


//////////////////////////////////////////////////////////////////
//
// Return the event ID
//
//////////////////////////////////////////////////////////////////
std::uint16_t NEventReportCommand::getEventID() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::nEventReportCommand>(getDimseCommandBaseImplementation(*this)))->getEventID();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NEventReportResponses
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
NEventReportResponse::NEventReportResponse(const std::shared_ptr<implementation::nEventReportResponse>& pResponse):
    DimseResponse(pResponse)
{
}


NEventReportResponse::NEventReportResponse(
        const NEventReportCommand& receivedCommand,
        const DataSet& eventReply
        ):
    DimseResponse(
        std::make_shared<implementation::nEventReportResponse>(
            std::static_pointer_cast<implementation::nEventReportCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            getDataSetImplementation(eventReply)) )
{
}


NEventReportResponse::NEventReportResponse(
        const NEventReportCommand& receivedCommand,
        dimseStatusCode_t responseCode
        ):
    DimseResponse(
        std::make_shared<implementation::nEventReportResponse>(
            std::static_pointer_cast<implementation::nEventReportCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode) )
{
}


NEventReportResponse::NEventReportResponse(const NEventReportResponse& source): DimseResponse(source)
{
}


NEventReportResponse::~NEventReportResponse()
{
}


//////////////////////////////////////////////////////////////////
//
// Return the event ID
//
//////////////////////////////////////////////////////////////////
std::uint16_t NEventReportResponse::getEventID() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::nEventReportResponse>(getDimseCommandBaseImplementation(*this)))->getEventID();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NGetCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NGetCommand::NGetCommand(const std::shared_ptr<implementation::nGetCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NGetCommand::NGetCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& requestedSopClassUid,
        const std::string& requestedSopInstanceUid,
        const attributeIdentifierList_t& attributeIdentifierList
        ):
    DimseCommand(std::make_shared<implementation::nGetCommand>(
                     abstractSyntax,
                     messageID,
                     requestedSopClassUid,
                     requestedSopInstanceUid,
                     attributeIdentifierList))
{
}


NGetCommand::NGetCommand(const NGetCommand& source): DimseCommand(source)
{
}


NGetCommand::~NGetCommand()
{
}


//////////////////////////////////////////////////////////////////
//
// Return the attribute list
//
//////////////////////////////////////////////////////////////////
attributeIdentifierList_t NGetCommand::getAttributeList() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::nGetCommand>(getDimseCommandBaseImplementation(*this)))->getAttributeList();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NGetResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
NGetResponse::NGetResponse(const std::shared_ptr<implementation::nGetResponse>& pResponse):
    DimseResponse(pResponse)
{
}


NGetResponse::NGetResponse(
        const NGetCommand& receivedCommand,
        dimseStatusCode_t responseCode,
        const DataSet& attributeList
        ):
    DimseResponse(
        std::make_shared<implementation::nGetResponse>(
            std::static_pointer_cast<implementation::nGetCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode,
            getDataSetImplementation(attributeList)) )
{
}


NGetResponse::NGetResponse(
        const NGetCommand& receivedCommand,
        dimseStatusCode_t responseCode
        ):
    DimseResponse(
        std::make_shared<implementation::nGetResponse>(
            std::static_pointer_cast<implementation::nGetCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode) )
{
}


NGetResponse::NGetResponse(const NGetResponse& source): DimseResponse(source)
{
}


NGetResponse::~NGetResponse()
{
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NSetCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NSetCommand::NSetCommand(const std::shared_ptr<implementation::nSetCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NSetCommand::NSetCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& requestedSopClassUid,
        const std::string& requestedSopInstanceUid,
        const DataSet& modificationList
        ):
    DimseCommand(std::make_shared<implementation::nSetCommand>(
                     abstractSyntax,
                     messageID,
                     requestedSopClassUid,
                     requestedSopInstanceUid,
                     getDataSetImplementation(modificationList)))
{
}


NSetCommand::NSetCommand(const NSetCommand& source): DimseCommand(source)
{
}


NSetCommand::~NSetCommand()
{
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NSetResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
NSetResponse::NSetResponse(const std::shared_ptr<implementation::nSetResponse>& pResponse):
    DimseResponse(pResponse)
{
}


NSetResponse::NSetResponse(
        const NSetCommand& receivedCommand,
        attributeIdentifierList_t modifiedAttributes
        ):
    DimseResponse(
        std::make_shared<implementation::nSetResponse>(
            std::static_pointer_cast<implementation::nSetCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            modifiedAttributes) )
{
}


NSetResponse::NSetResponse(
        const NSetCommand& receivedCommand,
        dimseStatusCode_t responseCode
        ):
    DimseResponse(
        std::make_shared<implementation::nSetResponse>(
            std::static_pointer_cast<implementation::nSetCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode) )
{
}


NSetResponse::NSetResponse(const NSetResponse& source): DimseResponse(source)
{
}


NSetResponse::~NSetResponse()
{
}


//////////////////////////////////////////////////////////////////
//
// Get the list of modified attributes
//
//////////////////////////////////////////////////////////////////
attributeIdentifierList_t NSetResponse::getModifiedAttributes() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::nSetResponse>(getDimseCommandBaseImplementation(*this)))->getModifiedAttributes();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NActionCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NActionCommand::NActionCommand(const std::shared_ptr<implementation::nActionCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NActionCommand::NActionCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& requestedSopClassUid,
        const std::string& requestedSopInstanceUid,
        std::uint16_t actionID,
        const DataSet& actionInformation
        ):
    DimseCommand(std::make_shared<implementation::nActionCommand>(
                     abstractSyntax,
                     messageID,
                     requestedSopClassUid,
                     requestedSopInstanceUid,
                     actionID,
                     getDataSetImplementation(actionInformation)))
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NActionCommand::NActionCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& requestedSopClassUid,
        const std::string& requestedSopInstanceUid,
        std::uint16_t actionID
        ):
    DimseCommand(std::make_shared<implementation::nActionCommand>(
                     abstractSyntax,
                     messageID,
                     requestedSopClassUid,
                     requestedSopInstanceUid,
                     actionID))
{
}


NActionCommand::NActionCommand(const NActionCommand& source): DimseCommand(source)
{
}


NActionCommand::~NActionCommand()
{
}



//////////////////////////////////////////////////////////////////
//
// Return the action ID
//
//////////////////////////////////////////////////////////////////
std::uint16_t NActionCommand::getActionID() const
{
    IMEBRA_FUNCTION_START();

    return (std::static_pointer_cast<implementation::nActionCommand>(getDimseCommandBaseImplementation(*this)))->getActionID();

    IMEBRA_FUNCTION_END_LOG();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NActionResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
NActionResponse::NActionResponse(const std::shared_ptr<implementation::nActionResponse>& pResponse):
    DimseResponse(pResponse)
{
}


NActionResponse::NActionResponse(
        const NActionCommand& receivedCommand,
        const DataSet& actionReply
        ):
    DimseResponse(
        std::make_shared<implementation::nActionResponse>(
            std::static_pointer_cast<implementation::nActionCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            getDataSetImplementation(actionReply)) )
{
}


NActionResponse::NActionResponse(
        const NActionCommand& receivedCommand,
        dimseStatusCode_t responseCode
        ):
    DimseResponse(
        std::make_shared<implementation::nActionResponse>(
            std::static_pointer_cast<implementation::nActionCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode) )
{
}


NActionResponse::NActionResponse(const NActionResponse& source): DimseResponse(source)
{
}


NActionResponse::~NActionResponse()
{
}


std::uint16_t NActionResponse::getActionID() const
{
    return (std::static_pointer_cast<implementation::nActionResponse>(getDimseCommandBaseImplementation(*this)))->getActionID();
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NCreateCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NCreateCommand::NCreateCommand(const std::shared_ptr<implementation::nCreateCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NCreateCommand::NCreateCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& affectedSopClassUid,
        const std::string& affectedSopInstanceUid,
        const DataSet& attributeList
        ):
    DimseCommand(std::make_shared<implementation::nCreateCommand>(
                     abstractSyntax,
                     messageID,
                     affectedSopClassUid,
                     affectedSopInstanceUid,
                     getDataSetImplementation(attributeList)))
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NCreateCommand::NCreateCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& affectedSopClassUid,
        const std::string& affectedSopInstanceUid
        ):
    DimseCommand(std::make_shared<implementation::nCreateCommand>(
                     abstractSyntax,
                     messageID,
                     affectedSopClassUid,
                     affectedSopInstanceUid))
{
}


NCreateCommand::NCreateCommand(const NCreateCommand& source): DimseCommand(source)
{
}


NCreateCommand::~NCreateCommand()
{
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NCreateResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
NCreateResponse::NCreateResponse(const std::shared_ptr<implementation::nCreateResponse>& pResponse):
    DimseResponse(pResponse)
{
}


NCreateResponse::NCreateResponse(
        const NCreateCommand& receivedCommand,
        const DataSet& attributeList
        ):
    DimseResponse(
        std::make_shared<implementation::nCreateResponse>(
            std::static_pointer_cast<implementation::nCreateCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            getDataSetImplementation(attributeList)) )
{
}


NCreateResponse::NCreateResponse(
        const NCreateCommand& receivedCommand,
        const std::string& affectedSopInstanceUid,
        const DataSet& attributeList
        ):
    DimseResponse(
        std::make_shared<implementation::nCreateResponse>(
            std::static_pointer_cast<implementation::nCreateCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            affectedSopInstanceUid,
            getDataSetImplementation(attributeList)) )
{
}


NCreateResponse::NCreateResponse(
        const NCreateCommand& receivedCommand,
        dimseStatusCode_t responseCode
        ):
    DimseResponse(
        std::make_shared<implementation::nCreateResponse>(
            std::static_pointer_cast<implementation::nCreateCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode) )
{
}


NCreateResponse::NCreateResponse(
        const NCreateCommand& receivedCommand,
        const std::string& affectedSopInstanceUid
        ):
    DimseResponse(
        std::make_shared<implementation::nCreateResponse>(
            std::static_pointer_cast<implementation::nCreateCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            affectedSopInstanceUid) )
{
}


NCreateResponse::NCreateResponse(const NCreateResponse& source): DimseResponse(source)
{
}


NCreateResponse::~NCreateResponse()
{
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NDeleteCommand
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NDeleteCommand::NDeleteCommand(const std::shared_ptr<implementation::nDeleteCommand>& pCommand):
    DimseCommand(pCommand)
{
}


//////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////
NDeleteCommand::NDeleteCommand(
        const std::string& abstractSyntax,
        std::uint16_t messageID,
        const std::string& requestedSopClassUid,
        const std::string& requestedSopInstanceUid
        ):
    DimseCommand(std::make_shared<implementation::nDeleteCommand>(
                     abstractSyntax,
                     messageID,
                     requestedSopClassUid,
                     requestedSopInstanceUid))
{
}


NDeleteCommand::NDeleteCommand(const NDeleteCommand& source): DimseCommand(source)
{
}


NDeleteCommand::~NDeleteCommand()
{
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// NDeleteResponse
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Constructors
//
//////////////////////////////////////////////////////////////////
NDeleteResponse::NDeleteResponse(const std::shared_ptr<implementation::nDeleteResponse>& pResponse):
    DimseResponse(pResponse)
{
}


NDeleteResponse::NDeleteResponse(
        NDeleteCommand& receivedCommand,
        dimseStatusCode_t responseCode
        ):
    DimseResponse(
        std::make_shared<implementation::nDeleteResponse>(
            std::static_pointer_cast<implementation::nDeleteCommand>(getDimseCommandBaseImplementation(receivedCommand)),
            responseCode) )
{
}


NDeleteResponse::NDeleteResponse(const NDeleteResponse& source): DimseResponse(source)
{
}


NDeleteResponse::~NDeleteResponse()
{
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// DimseService
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
// Get an ID for the next command
//
//////////////////////////////////////////////////////////////////
std::uint16_t DimseService::getNextCommandID()
{
    return m_pDimseService->getNextCommandID();
}


//////////////////////////////////////////////////////////////////
//
// Get the next command
//
//////////////////////////////////////////////////////////////////
const DimseCommand DimseService::getCommand()
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::dimseNCommand> pCommand(m_pDimseService->getCommand());

    switch(pCommand->getCommandType())
    {
    case dimseCommandType_t::cStore:
        return CStoreCommand(std::dynamic_pointer_cast<implementation::cStoreCommand>(pCommand));
    case dimseCommandType_t::cGet:
        return CGetCommand(std::dynamic_pointer_cast<implementation::cGetCommand>(pCommand));
    case dimseCommandType_t::cMove:
        return CMoveCommand(std::dynamic_pointer_cast<implementation::cMoveCommand>(pCommand));
    case dimseCommandType_t::cFind:
        return CFindCommand(std::dynamic_pointer_cast<implementation::cFindCommand>(pCommand));
    case dimseCommandType_t::cEcho:
        return CEchoCommand(std::dynamic_pointer_cast<implementation::cEchoCommand>(pCommand));
    case dimseCommandType_t::cCancel:
        return CCancelCommand(std::dynamic_pointer_cast<implementation::cCancelCommand>(pCommand));
    case dimseCommandType_t::nEventReport:
        return NEventReportCommand(std::dynamic_pointer_cast<implementation::nEventReportCommand>(pCommand));
    case dimseCommandType_t::nGet:
        return NGetCommand(std::dynamic_pointer_cast<implementation::nGetCommand>(pCommand));
    case dimseCommandType_t::nSet:
        return NSetCommand(std::dynamic_pointer_cast<implementation::nSetCommand>(pCommand));
    case dimseCommandType_t::nAction:
        return NActionCommand(std::dynamic_pointer_cast<implementation::nActionCommand>(pCommand));
    case dimseCommandType_t::nCreate:
        return NCreateCommand(std::dynamic_pointer_cast<implementation::nCreateCommand>(pCommand));
    case dimseCommandType_t::nDelete:
        return NDeleteCommand(std::dynamic_pointer_cast<implementation::nDeleteCommand>(pCommand));
    default:
        {}
    }

    IMEBRA_THROW(std::logic_error, "Should have received a valid command from the implementation layer");

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Returns the transfer syntax for a specific abstract syntax
//
//////////////////////////////////////////////////////////////////
std::string DimseService::getTransferSyntax(const std::string &abstractSyntax) const
{
    IMEBRA_FUNCTION_START();

    return m_pDimseService->getTransferSyntax(abstractSyntax);

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Send a command or response
//
//////////////////////////////////////////////////////////////////
void DimseService::sendCommandOrResponse(const DimseCommandBase &command)
{
    IMEBRA_FUNCTION_START();

    m_pDimseService->sendCommandOrResponse(getDimseCommandBaseImplementation(command));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a C-STORE response
//
//////////////////////////////////////////////////////////////////
const CStoreResponse DimseService::getCStoreResponse(const CStoreCommand& command)
{
    IMEBRA_FUNCTION_START();

    return CStoreResponse(m_pDimseService->getResponse<implementation::cStoreResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a C-GET response
//
//////////////////////////////////////////////////////////////////
const CGetResponse DimseService::getCGetResponse(const CGetCommand& command)
{
    IMEBRA_FUNCTION_START();

    return CGetResponse(m_pDimseService->getResponse<implementation::cGetResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a C-FIND response
//
//////////////////////////////////////////////////////////////////
const CFindResponse DimseService::getCFindResponse(const CFindCommand& command)
{
    IMEBRA_FUNCTION_START();

    return CFindResponse(m_pDimseService->getResponse<implementation::cFindResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a C-MOVE response
//
//////////////////////////////////////////////////////////////////
const CMoveResponse DimseService::getCMoveResponse(const CMoveCommand& command)
{
    IMEBRA_FUNCTION_START();

    return CMoveResponse(m_pDimseService->getResponse<implementation::cMoveResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a C-ECHO response
//
//////////////////////////////////////////////////////////////////
const CEchoResponse DimseService::getCEchoResponse(const CEchoCommand& command)
{
    IMEBRA_FUNCTION_START();

    return CEchoResponse(m_pDimseService->getResponse<implementation::cEchoResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a N-EVENT-REPORT response
//
//////////////////////////////////////////////////////////////////
const NEventReportResponse DimseService::getNEventReportResponse(const NEventReportCommand& command)
{
    IMEBRA_FUNCTION_START();

    return NEventReportResponse(m_pDimseService->getResponse<implementation::nEventReportResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a N-GET response
//
//////////////////////////////////////////////////////////////////
const NGetResponse DimseService::getNGetResponse(const NGetCommand& command)
{
    IMEBRA_FUNCTION_START();

    return NGetResponse(m_pDimseService->getResponse<implementation::nGetResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a N-SET response
//
//////////////////////////////////////////////////////////////////
const NSetResponse DimseService::getNSetResponse(const NSetCommand& command)
{
    IMEBRA_FUNCTION_START();

    return NSetResponse(m_pDimseService->getResponse<implementation::nSetResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a N-ACTION response
//
//////////////////////////////////////////////////////////////////
const NActionResponse DimseService::getNActionResponse(const NActionCommand& command)
{
    IMEBRA_FUNCTION_START();

    return NActionResponse(m_pDimseService->getResponse<implementation::nActionResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a N-CREATE response
//
//////////////////////////////////////////////////////////////////
const NCreateResponse DimseService::getNCreateResponse(const NCreateCommand& command)
{
    IMEBRA_FUNCTION_START();

    return NCreateResponse(m_pDimseService->getResponse<implementation::nCreateResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


//////////////////////////////////////////////////////////////////
//
// Wait for a N-DELETE response
//
//////////////////////////////////////////////////////////////////
const NDeleteResponse DimseService::getNDeleteResponse(const NDeleteCommand& command)
{
    IMEBRA_FUNCTION_START();

    return NDeleteResponse(m_pDimseService->getResponse<implementation::nDeleteResponse>(command.getID()));

    IMEBRA_FUNCTION_END_LOG();
}


} // namespace imebra
