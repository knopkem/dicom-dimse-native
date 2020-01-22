const addon = require('../index');

addon.startScp(JSON.stringify(
    {
        "source": {
            "aet": "IMEBRA",
            "ip" : "127.0.0.1",
            "port": "9999"
        }
    }   
    ), (result) => {
        console.log(result);
});

setTimeout(function() {
    addon.moveScu(JSON.stringify(
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
                    "value": "1.3.46.670589.11.0.1.1996082307380006",
                },
                {
                    "key": "00080052", 
                    "value": "STUDY",
                },
            ],
            "destination" : "IMEBRA"
        }
    ), (result) => {
        console.log("move result: ", result);
    });
}, 3000);

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
    console.log("echo result: ", result);
});

addon.findScu(JSON.stringify(
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
        ]
    }
), (result) => {
    console.log("find result: ", result);
});

