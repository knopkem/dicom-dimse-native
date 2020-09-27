const addon = require('../index');

addon.startScp(JSON.stringify(
    {
        "source": {
            "aet": "DIMSE",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "peers": [
                {
                "aet": "CONQUESTSRV1",
                "ip" : "RYZEN7",
                "port": "5678"
                }
            ],
        "storagePath": "./data",
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