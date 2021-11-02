import { startStoreScp, storeScpOptions, moveScu, moveScuOptions } from '../index';
import p from "path";

const scpOptions: storeScpOptions = {
    source: {
        aet: "DIMSE",
        ip: "127.0.0.1",
        port: 9999,
    },
    peers: [
        {
            aet: "CONQUESTSRV1",
            ip: "127.0.0.1",
            port: 5678
        }
    ],
    storagePath: p.join(__dirname, "data"),
    netTransferPrefer: "1.2.840.10008.1.2.4.80", // preferred network transfer syntax (accepted ts - additional to default ts)
    netTransferPropose: "1.2.840.10008.1.2.4.80", // proposed network transfer syntax (for outgoing associations - additional to default ts)
    writeTransfer: "", // write transfer syntax (storage only, recompress on the fly), keep empty to store ts as original
    permissive: false,
    verbose: true
};

// starting the store scp for this example to actually receive anything from the move
startStoreScp(scpOptions, (result) => {
    //TODO: fixme - currently we never get here as the thread does not return
    console.log(JSON.parse(result));
});

// we need to wait a bit for the store scp to initialize 
setTimeout(() => {
    const moveOptions: moveScuOptions =
    {
        source: {
            aet: "DIMSE",
            ip: "127.0.0.1",
            port: 9999
        },
        target: {
            aet: "CONQUESTSRV1",
            ip: "127.0.0.1",
            port: 5678
        },
        tags: [
            {
                key: "0020000D",
                value: "1.3.46.670589.5.2.10.2156913941.892665384.993397",
            },
            {
                key: "00080052",
                value: "STUDY",
            },
        ],
        destination: "DIMSE",
        verbose: true
    };
    moveScu(moveOptions, (result) => {
        console.log(JSON.parse(result));
    });
}, 3000);

