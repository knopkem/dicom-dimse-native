import { echoScu, echoScuOptions } from '../index';

const options : echoScuOptions = { 
  source: {
    aet: "DIMSE",
    ip: "127.0.0.1",
    port: "9999"
  },
  target: {
    aet: "CONQUESTSRV1",
    ip: "127.0.0.1",
    port: "5678"
  },
  verbose: true
};

echoScu(options, (result) => {
  console.log(JSON.parse(result));
});
