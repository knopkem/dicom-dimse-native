/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file acse.cpp
    \brief Implementation of the the ACSE classes.
*/

#include "../include/imebra/acse.h"
#include "../include/imebra/streamReader.h"
#include "../include/imebra/streamWriter.h"
#include "../include/imebra/dataSet.h"
#include "../implementation/acseImpl.h"

namespace imebra
{

//
// PresentationContext methods
//
///////////////////////////////////////////////////////////////////////////////

PresentationContext::PresentationContext(const std::string& abstractSyntax):
    m_pPresentationContext(std::make_shared<implementation::presentationContext>(abstractSyntax))
{
}

PresentationContext::PresentationContext(const std::string& abstractSyntax, bool bSCURole, bool bSCPRole):
    m_pPresentationContext(std::make_shared<implementation::presentationContext>(abstractSyntax, bSCURole, bSCPRole))
{
}

PresentationContext::PresentationContext(const PresentationContext& source):
    m_pPresentationContext(getPresentationContextImplementation(source))
{
}

PresentationContext::~PresentationContext()
{
}

void PresentationContext::addTransferSyntax(const std::string& transferSyntax)
{
    m_pPresentationContext->addTransferSyntax(transferSyntax);
}

const std::shared_ptr<implementation::presentationContext>& getPresentationContextImplementation(const PresentationContext& message)
{
    return message.m_pPresentationContext;
}


//
// PresentationContexts methods
//
///////////////////////////////////////////////////////////////////////////////

PresentationContexts::PresentationContexts():
    m_pPresentationContexts(std::make_shared<implementation::presentationContexts>())
{
}

PresentationContexts::PresentationContexts(const PresentationContexts& source):
    m_pPresentationContexts(getPresentationContextsImplementation(source))
{
}

PresentationContexts::~PresentationContexts()
{
}

void PresentationContexts::addPresentationContext(const PresentationContext& presentationContext)
{
    m_pPresentationContexts->m_presentationContexts.push_back(getPresentationContextImplementation(presentationContext));
}

const std::shared_ptr<implementation::presentationContexts>& getPresentationContextsImplementation(const PresentationContexts& message)
{
    return message.m_pPresentationContexts;
}


//
// AssociationMessage methods
//
///////////////////////////////////////////////////////////////////////////////

AssociationMessage::AssociationMessage(const std::shared_ptr<implementation::associationMessage>& pMessage):
    m_pMessage(pMessage)
{
}

AssociationMessage::AssociationMessage(const AssociationMessage& source):
    m_pMessage(getAssociationMessageImplementation(source))
{
}

AssociationMessage::~AssociationMessage()
{
}

std::string AssociationMessage::getAbstractSyntax() const
{
    IMEBRA_FUNCTION_START();

    return m_pMessage->getAbstractSyntax();

    IMEBRA_FUNCTION_END_LOG();
}

const DataSet AssociationMessage::getCommand() const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pMessage->getCommandDataSet());

    IMEBRA_FUNCTION_END_LOG();
}

const DataSet AssociationMessage::getPayload() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::dataSet> pDataSet(m_pMessage->getPayloadDataSetNoThrow());
    if(pDataSet == nullptr)
    {
        IMEBRA_THROW(AcseNoPayloadError, "Payload not available");
    }
    return DataSet(pDataSet);

    IMEBRA_FUNCTION_END_LOG();
}

bool AssociationMessage::hasPayload() const
{
    return m_pMessage->getPayloadDataSetNoThrow() != nullptr;
}

const std::shared_ptr<implementation::associationMessage>& getAssociationMessageImplementation(const AssociationMessage& message)
{
    return message.m_pMessage;
}


//
// MutableAssociationMessage methods
//
///////////////////////////////////////////////////////////////////////////////

MutableAssociationMessage::MutableAssociationMessage(const std::string& abstractSyntax):
    AssociationMessage(std::make_shared<implementation::associationMessage>(abstractSyntax))
{
}

MutableAssociationMessage::MutableAssociationMessage(const MutableAssociationMessage& source):
    AssociationMessage(getAssociationMessageImplementation(source))
{
}

MutableAssociationMessage::~MutableAssociationMessage()
{
}

void MutableAssociationMessage::addDataSet(const DataSet& dataSet)
{
    IMEBRA_FUNCTION_START();

    getAssociationMessageImplementation(*this)->addDataset(getDataSetImplementation(dataSet));

    IMEBRA_FUNCTION_END_LOG();
}


//
// AssociationBase methods
//
///////////////////////////////////////////////////////////////////////////////

AssociationBase::AssociationBase(const std::shared_ptr<implementation::associationBase>& pAssociationBase):
    m_pAssociation(pAssociationBase)
{
}

AssociationBase::~AssociationBase()
{
}

const AssociationMessage AssociationBase::getCommand()
{
    IMEBRA_FUNCTION_START();

    return AssociationMessage(m_pAssociation->getCommand());

    IMEBRA_FUNCTION_END_LOG();
}

const AssociationMessage AssociationBase::getResponse(std::uint16_t messageId)
{
    IMEBRA_FUNCTION_START();

    return AssociationMessage(m_pAssociation->getResponse(messageId));

    IMEBRA_FUNCTION_END_LOG();
}

void AssociationBase::sendMessage(const AssociationMessage& messageDataSet)
{
    IMEBRA_FUNCTION_START();

    m_pAssociation->sendMessage(messageDataSet.m_pMessage);

    IMEBRA_FUNCTION_END_LOG();
}

void AssociationBase::release()
{
    IMEBRA_FUNCTION_START();

    m_pAssociation->release();

    IMEBRA_FUNCTION_END_LOG();
}

void AssociationBase::abort()
{
    IMEBRA_FUNCTION_START();

    m_pAssociation->abort(implementation::acsePDUAAbort::reason_t::serviceUser);

    IMEBRA_FUNCTION_END_LOG();
}

std::string AssociationBase::getThisAET() const
{
    IMEBRA_FUNCTION_START();

    return m_pAssociation->getThisAET();

    IMEBRA_FUNCTION_END_LOG();
}

std::string AssociationBase::getOtherAET() const
{
    IMEBRA_FUNCTION_START();

    return m_pAssociation->getOtherAET();

    IMEBRA_FUNCTION_END_LOG();
}

std::string AssociationBase::getTransferSyntax(const std::string &abstractSyntax) const
{
    IMEBRA_FUNCTION_START();

    return m_pAssociation->getPresentationContextTransferSyntax(abstractSyntax);

    IMEBRA_FUNCTION_END_LOG();
}

std::vector<std::string> AssociationBase::getTransferSyntaxes(const std::string& abstractSyntax) const
{
    return m_pAssociation->getPresentationContextTransferSyntaxes(abstractSyntax);
}


const std::shared_ptr<implementation::associationBase>& getAssociationBaseImplementation(const AssociationBase& associationBase)
{
    return associationBase.m_pAssociation;
}


//
// AssociationSCU methods
//
///////////////////////////////////////////////////////////////////////////////

AssociationSCU::AssociationSCU(
        const std::string& thisAET,
        const std::string& otherAET,
        std::uint32_t invokedOperations,
        std::uint32_t performedOperations,
        const PresentationContexts& presentationContexts,
        StreamReader& pInput,
        StreamWriter& pOutput,
        std::uint32_t dimseTimeoutSeconds):
    AssociationBase(std::make_shared<implementation::associationSCU>(
                        getPresentationContextsImplementation(presentationContexts),
                thisAET,
                otherAET,
                invokedOperations,
                performedOperations,
                pInput.m_pReader,
                pOutput.m_pWriter,
                        dimseTimeoutSeconds))
{
}

AssociationSCU::AssociationSCU(const AssociationSCU& source):
    AssociationBase(getAssociationBaseImplementation(source))
{
}

AssociationSCU::~AssociationSCU()
{
}

//
// AssociationSCP methods
//
///////////////////////////////////////////////////////////////////////////////

AssociationSCP::AssociationSCP(
        const std::string& thisAET,
        std::uint32_t invokedOperations,
        std::uint32_t performedOperations,
        const PresentationContexts& presentationContexts,
        StreamReader& pInput,
        StreamWriter& pOutput,
        std::uint32_t dimseTimeoutSeconds,
        std::uint32_t artimTimeoutSeconds):
    AssociationBase(std::make_shared<implementation::associationSCP>(
                        getPresentationContextsImplementation(presentationContexts),
                thisAET,
                invokedOperations,
                performedOperations,
                pInput.m_pReader,
                pOutput.m_pWriter,
                dimseTimeoutSeconds,
                        artimTimeoutSeconds))
{
}

AssociationSCP::AssociationSCP(const AssociationSCP& source):
    AssociationBase(getAssociationBaseImplementation(source))
{
}

AssociationSCP::~AssociationSCP()
{
}

}
