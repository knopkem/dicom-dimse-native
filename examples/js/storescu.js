const addon = require('../index');

addon.storeScu(JSON.stringify(
    {
        "source": {
            "aet": "DIMSE",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "target": {
            "aet": "CONQUESTSRV1",
            "ip" : "127.0.0.1",
            "port": "5678"
        },
        "netTransferPropose": "1.2.840.10008.1.2.4.80", // proposed network transfer syntax (for outgoing associations - additional to default ts)
        "sourcePath": "./examples/dicom",
        "verbose": true
    }
), (result) => {
        if (result && result.length > 0) {
            try
            {
                console.log(JSON.parse(result));
            }
            catch (e) {
                console.log(e, result);
            }
        }
});
