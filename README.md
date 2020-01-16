[![GitHub issues](https://img.shields.io/github/issues/knopkem/dicom-dimse-native)](https://github.com/knopkem/dicom-dimse-native/issues)

# dicom-dimse-native
node js native addon for dimse services using IMEBRA (attention: see license!)

## How to install
```npm i -s dicom-native-addon```

## How to use
```const addon = require('./module');```

### C-Echo
```
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
    }
};

addon.echoScu(JSON.stringify(j), (result) => {
    console.log(result);
});
```

### C-Find
```
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
    ]
};

addon.findScu(JSON.stringify(j), (result) => {
    console.log(result);
});
```

### Store-Scp
```
const j = {
    "source": {
        "aet": "IMEBRA",
        "ip" : "127.0.0.1",
        "port": "9999"
    }
}
addon.startServer(JSON.stringify(j), (result) => {
    console.log(result);
}
```

### C-Move
```
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
    "destination": "IMEBRA",
    "tags" : [
        {
            "key": "0020000D", 
            "value": "1.3.46.670589.11.0.1.1996082307380006",
        },
        {
            "key": "00080052", 
            "value": "STUDY",
        },
    ]
};

addon.moveScu(JSON.stringify(j), (result) => {
    console.log(result);
});
```

### C-Get
```
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
            "key": "0020000D", 
            "value": "1.3.46.670589.11.0.1.1996082307380006",
        },
        {
            "key": "00080052", 
            "value": "STUDY",
        },
    ]
};

addon.getScu(JSON.stringify(j), (result) => {
    console.log(result);
});
```
