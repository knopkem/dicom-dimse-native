import { findScu, findScuOptions } from '../index';

const options: findScuOptions = {
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
      key: "00100010",
      value: "*",
    },
    {
      key: "00080052",
      value: "STUDY",
    },
    {
      key: "00100010",
      value: "",
    }
  ],
  verbose: true
};

findScu(options, (result) => {
  console.log(JSON.parse(result));
});
