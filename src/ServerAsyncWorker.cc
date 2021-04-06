#include "ServerAsyncWorker.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>

#include "json.h"
#include "Utils.h"

using json = nlohmann::json;

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmqrdb/dcmqrsrv.h"
#include "dcmtk/dcmqrdb/dcmqrcbf.h" 
#include "dcmtk/dcmqrdb/dcmqrcbm.h" 
#include "dcmtk/dcmqrdb/dcmqrcbg.h"
#include "dcmtk/dcmqrdb/dcmqrcbs.h"
#include "dcmtk/dcmqrdb/dcmqropt.h"

#include "dcmsqlhdl.h"


ServerAsyncWorker::ServerAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
    ns::registerCodecs();
}

void ServerAsyncWorker::Execute(const ExecutionProgress &progress)
{
  ns::sInput in = ns::parseInputJson(_input);

  EnableVerboseLogging(in.verbose);

  if (!in.source.valid())
  {
    SetErrorJson("Source not set");
    return;
  }

  if (in.storagePath.empty())
  {
    in.storagePath = "./data";
    SendInfo("storage path not set, defaulting to " + in.storagePath, progress);
  }

  int opt_port = std::stoi(in.source.port);
  OFString callingAETitle = OFString(in.source.aet.c_str());
  const char *opt_respondingAETitle = in.source.aet.c_str();
  static OFString opt_outputDirectory =  OFString(in.storagePath.c_str());

  T_ASC_Network *net;
  DcmAssociationConfiguration asccfg;

  OFStandard::initializeNetwork();

  /* make sure data dictionary is loaded */
  if (!dcmDataDict.isDictionaryLoaded())
  {
    SetErrorJson(std::string("No data dictionary loaded"));
  }

  /* if the output directory does not equal "." (default directory) */
  if (opt_outputDirectory != ".")
  {
    /* if there is a path separator at the end of the path, get rid of it */
    OFStandard::normalizeDirName(opt_outputDirectory, opt_outputDirectory);

    /* check if the specified directory exists and if it is a directory.
     * If the output directory is invalid, dump an error message and terminate execution.
     */
    if (!OFStandard::dirExists(opt_outputDirectory))
    {
      SetErrorJson(std::string("Specified output directory does not exist: ") + std::string(opt_outputDirectory.c_str()));
      return;
    }
  }

  /* check if the output directory is writeable */
  if (!OFStandard::isWriteable(opt_outputDirectory))
  {
    SetErrorJson(std::string("Specified output directory is not writeable: ") + std::string(opt_outputDirectory.c_str()));
    return;
  }

  /* initialize network, i.e. create an instance of T_ASC_Network*. */
  OFCondition cond = ASC_initializeNetwork(NET_ACCEPTOR, opt_port, 30, &net);
  if (cond.bad())
  {
    SetErrorJson(std::string("Cannot create network: ") + std::string(cond.text()));
    return;
  }

  /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
  if (OFStandard::dropPrivileges().bad())
  {
    SetErrorJson(std::string("setuid() failed, maximum number of threads for uid already running"));
    return;
  }

  T_ASC_Network* network = NULL;
  cond = ASC_initializeNetwork(NET_REQUESTOR, 0, 10000, &network);

  if (cond.bad()) {
      OFString temp_str;
      DCMNET_ERROR("Failed to create requestor network: " << DimseCondition::dump(temp_str, cond));
      return;
  }

  DcmQueryRetriveConfigExt cfg;
  for (auto peer: in.peers) {
    cfg.addPeer(peer.aet.c_str(), peer.ip.c_str(), std::stoi(peer.port));
  }
  cfg.setStorageArea(in.storagePath.c_str());
  cfg.setPermissiveMode(in.permissive);
 
  DcmQueryRetrieveOptions options;
  options.net_ = network;
  options.disableGetSupport_ = true;
  DcmXfer netTransPrefer = in.netTransferPrefer.empty() ? DcmXfer(EXS_Unknown) : DcmXfer(in.netTransferPrefer.c_str());
  DcmXfer netTransPropose = in.netTransferPropose.empty() ? DcmXfer(EXS_Unknown) : DcmXfer(in.netTransferPropose.c_str());
  DcmXfer writeTrans = in.writeTransfer.empty() ? DcmXfer(EXS_Unknown) : DcmXfer(in.writeTransfer.c_str());
  std::cout << "preferred (accepted) network transfer syntax for incoming associations: " << netTransPrefer.getXferName() << std::endl;
  std::cout << "proposed network transfer syntax for outgoing associations: " << netTransPropose.getXferName() << std::endl;
  std::cout << "write transfer syntax (recompress if different to accepted ts): " << writeTrans.getXferName() << std::endl;
  options.networkTransferSyntax_ = netTransPrefer.getXfer();
  options.networkTransferSyntaxOut_ = netTransPropose.getXfer();
  options.writeTransferSyntax_ = writeTrans.getXfer();

  DcmQueryRetrieveSQLiteDatabaseHandleFactory factory(&cfg);
  DcmAssociationConfiguration associationConfiguration;

  DcmQueryRetrieveSCP scp(cfg, options, factory, associationConfiguration);
  while (cond.good()) {
      cond = scp.waitForAssociation(net);
  }
  
  /* drop the network, i.e. free memory of T_ASC_Network* structure. This call */
  /* is the counterpart of ASC_initializeNetwork(...) which was called above. */
  cond = ASC_dropNetwork(&net);
  if (cond.bad())
  {
    SetErrorJson(std::string(cond.text()));
    return;
  }

  OFStandard::shutdownNetwork();
}
