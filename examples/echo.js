const addon = require('../index');

addon.echoScu(JSON.stringify(
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
        }
    }
), (result) => {
    console.log(JSON.parse(result));
});
