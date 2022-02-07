import { startStoreScp, storeScpOptions } from '../index';
import p from 'path';
import fs from 'fs';

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
    writeTransfer: "", // write transfer syntax (storage only, recompress on the fly), keep empty (or leave out) to store ts as original
    permissive: false, // if true any AET can perform FIND,GET and STORE
    verbose: true,
    storeOnly: true, // do not provide FindSCP and MoveSCP (requires db) only StoreSCP
    writeFile: false, // do not write file to disk, send via buffer as base64 in response (only taken into account when storeOnly is true)
};

// starting the store scp for this example to actually receive anything from the move
startStoreScp(scpOptions, (result) => {
    const msg = JSON.parse(result);

    // retrieve and store the image 
    if (msg.message === 'BUFFER_STORAGE') {
        let buff = Buffer.from(msg.container.base64, 'base64');
        const directory = `${scpOptions.storagePath}/${msg.container.StudyInstanceUID}`;
        const filepath = `${directory}/${msg.container.SOPInstanceUID}.dcm`;
        if (!fs.existsSync(directory)) {
          fs.mkdirSync(directory);
        }
        fs.writeFile(filepath, buff, 'binary', (err) => {
            err ? console.log(err) : console.log(`The file was saved to ${filepath}`);
        });
    } else {
      console.log(msg);
    }
});