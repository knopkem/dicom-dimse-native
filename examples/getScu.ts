import { getScu, getScuOptions } from '../index';
import p from 'path';

const options: getScuOptions = {
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
    netTransferPrefer: "1.2.840.10008.1.2.4.80", // preferred network transfer syntax (accepted ts - additional to default ts)
    storagePath: p.join(__dirname, "data"),
    verbose: true
};

getScu(options, (result) => {
    console.log(JSON.parse(result));
});
