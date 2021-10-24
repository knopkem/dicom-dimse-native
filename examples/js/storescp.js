const addon = require('../index');

addon.startScp(JSON.stringify(
    {
        "source": {
            "aet": "DIMSE",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "peers": [ // peers needed for C-MOVE destination
                {
                "aet": "CONQUESTSRV1",
                "ip" : "127.0.0.1",
                "port": "5678"
                }
            ],
        "storagePath": "./data",
        "netTransferPrefer": "1.2.840.10008.1.2.4.80", // preferred network transfer syntax (accepted additional to default ts uncompressed)
        "netTransferPropose": "1.2.840.10008.1.2.4.80", // proposed network transfer syntax (for outgoing associations)
        "writeTransfer": "", // write transfer syntax (storage only), keep empty to store original
        "permissive": true, // any AET can perform FIND,GET and STORE
        "verbose": true // verbose logging
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