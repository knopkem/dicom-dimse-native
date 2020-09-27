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
                "ip" : "RYZEN7",
                "port": "5678"
                }
            ],
        "storagePath": "./data",
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