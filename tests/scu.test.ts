import { echoScu } from '../index';

const options = {
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
  verbose: true
};


test('echo should run', () => {
    echoScu(options, (result) => {
        const parsed = JSON.parse(result);
        expect(parsed.code).toBe(0);
    });
      
});