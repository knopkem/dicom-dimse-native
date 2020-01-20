[![Build Status](https://travis-ci.com/knopkem/dicom-dimse-native.svg?branch=master)](https://travis-ci.com/knopkem/dicom-dimse-native)
[![GitHub issues](https://img.shields.io/github/issues/knopkem/dicom-dimse-native)](https://github.com/knopkem/dicom-dimse-native/issues) 
[![Greenkeeper badge](https://badges.greenkeeper.io/knopkem/dicom-dimse-native.svg)](https://greenkeeper.io/)

# dicom-dimse-native
node js native addon for dimse services using IMEBRA DICOM toolkit 
Attention: watch license restrictions coming from IMEBRA!

# supported DIMSE services
* C-Echo-scu 
* C-Store-scp
* C-Move-scu
* C-Find-scu

# in development
* C-Store-scu
* C-Get-scu

## How to install
```npm i -s dicom-native-addon```

## Examples
```
const addon = require('./module');

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
        "destination" : "IMEBRA",
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
    }
), (result) => {
    console.log("result: ", result);
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
    console.log("result: ", result);
});
```