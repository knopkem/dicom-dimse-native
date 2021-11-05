import { storeScu, storeScuOptions } from '../index';
import p from 'path';

const options: storeScuOptions = {
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
  netTransferPropose: "1.2.840.10008.1.2.4.80", // proposed network transfer syntax (for outgoing associations - additional to default ts)
  sourcePath: p.join(__dirname, "dicom"),
  verbose: true
};

storeScu(options, (result) => {
  console.log(JSON.parse(result));
});
