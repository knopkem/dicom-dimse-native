import { startStoreScp, storeScpOptions, shutdownScu, shutdownScuOptions } from '../index';
import p from 'path';

const scpOptions: storeScpOptions = {
    source: {
        aet: "DIMSE",
        ip: "127.0.0.1",
        port: 9999,
    },
    peers: [
        {
            aet: "SHUTDOWN_SCU",
            ip: "127.0.0.1",
            port: 10000
        }
    ],
    storagePath: p.join(__dirname, "data"),
    permissive: false, // permissive true would allow any scu to shut us down, we don't want this
    verbose: true
};

// starting the store scp for this example to be able to shut it down later
startStoreScp(scpOptions, (result) => {
    //TODO: fixme - currently we never get here as the thread does not return
    console.log(JSON.parse(result));
});

const options: shutdownScuOptions = {
    source: {
        aet: "SHUTDOWN_SCU",
        ip: "127.0.0.1",
        port: 10000
    },
    target: {
        aet: "DIMSE",
        ip: "127.0.0.1",
        port: 9999
    },
    verbose: true
};

setTimeout(() => {
    shutdownScu(options, (result) => {
        console.log(JSON.parse(result));
    });
}, 3000);
