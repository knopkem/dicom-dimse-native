#pragma once

#include <napi.h>

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/oftypes.h"
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/dcasccfg.h"


class RetrieveScp 
{
public:
    RetrieveScp(const OFString& outputDirectory, const OFString& aet, bool writeFile) : m_outputDirectory(outputDirectory), m_aet(aet), m_writeFile(writeFile) {}

    OFCondition waitForAssociation(T_ASC_Network* theNet, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress);

protected:

    OFCondition acceptAssociation(T_ASC_Network* net, DcmAssociationConfiguration& asccfg, OFBool secureConnection, const OFString& outputDirectory, const OFString& aet, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress);
    
    OFCondition processCommands(T_ASC_Association* assoc, const OFString& outputDirectory, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress);

    OFCondition storeSCP(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID presID, const OFString& outputDirectory, const Napi::AsyncProgressQueueWorker<char>::ExecutionProgress& progress);

    OFCondition echoSCP(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID presID);

private:
    OFString m_outputDirectory;
    OFString m_aet;
    DcmAssociationConfiguration asccfg;
    bool m_writeFile;
};
