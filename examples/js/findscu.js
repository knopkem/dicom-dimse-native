const addon = require('../index');

addon.findScu(JSON.stringify(
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
        "tags" : [
            {
                "key": "0020000D", 
                "value": "1.3.46.670589.5.2.10.2156913941.892665384.993397",
            },
            {
                "key": "00080052", 
                "value": "STUDY",
            },
            {
                "key": "00100010", 
                "value": "",
            },
        ],
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

addon.findScu(JSON.stringify(
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
        "tags" : [
            {
                "key": "0020000D", 
                "value": "1.3.46.670589.5.2.10.2156913941.892665384.993397",
            },
            {
                "key": "00080052", 
                "value": "SERIES",
            },
            {
                "key": "0020000E", 
                "value": "",
            },
            {
                "key": "00200010", 
                "value": "",
            },
            {
                "key": "00200011", 
                "value": "",
            },
        ],
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

addon.findScu(JSON.stringify(
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
        "tags" : [
            {
                "key": "0020000E", 
                "value": "1.3.46.670589.5.2.10.2156913941.892665339.860724",
            },
            {
                "key": "00080052", 
                "value": "IMAGE",
            },
            {
                "key": "00200012", 
                "value": "",
            },
            {
                "key": "00200013", 
                "value": "",
            },
            {
                "key": "00280103", 
                "value": "",
            },
        ],
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
