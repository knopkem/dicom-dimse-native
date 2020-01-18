const addon = require('./module');

const j = {
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
            "key": "00100010", 
            "value": "",
        },
        {
            "key": "0020000D", 
            "value": "1.3.46.670589.11.0.1.1996082307380006",
        },
        {
            "key": "00080052", 
            "value": "STUDY",
        },
    ],
    "destination": "IMEBRA"
};

addon.findScu(JSON.stringify(j), (result) => {
    console.log("result: ", result);
});

