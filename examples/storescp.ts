import { startStoreScp, storeScpOptions } from '../index';
import p from 'path';

const scpOptions: storeScpOptions = {
    source: {
        aet: "DIMSE",
        ip: "127.0.0.1",
        port: "9999",
    },
    peers: [
        {
            aet: "CONQUESTSRV1",
            ip: "127.0.0.1",
            port: "5678"
        }
    ],
    storagePath: p.join(__dirname, "data"),
    netTransferPrefer: "1.2.840.10008.1.2.4.80", // preferred network transfer syntax (accepted ts - additional to default ts)
    netTransferPropose: "1.2.840.10008.1.2.4.80", // proposed network transfer syntax (for outgoing associations - additional to default ts)
    writeTransfer: "", // write transfer syntax (storage only, recompress on the fly), keep empty (or leave out) to store ts as original
    permissive: false, // if true any AET can perform FIND,GET and STORE
    verbose: true
};

// starting the store scp for this example to actually receive anything from the move
startStoreScp(scpOptions, (result) => {
    //TODO: fixme - currently we never get here as the thread does not return
    console.log(JSON.parse(result));
});