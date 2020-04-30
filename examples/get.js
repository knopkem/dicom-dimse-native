const addon = require('../index');

addon.getScu(JSON.stringify(
    {
        "source": {
            "aet": "IMEBRA",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "target": {
            "aet": "CONQUESTSRV1",
            "ip" : "127.0.0.1",
            "port": "5678"
        },
        "tags" : [
            {
                "key": "0020000D", 
                "value": "1.3.46.670589.5.2.10.2156913941.892665384.993397",
            },
            {
                "key": "00080052", 
                "value": "STUDY",
            },
        ]
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
