[![Build Status](https://travis-ci.com/knopkem/dicom-dimse-native.svg?branch=master)](https://travis-ci.com/knopkem/dicom-dimse-native)
[![GitHub issues](https://img.shields.io/github/issues/knopkem/dicom-dimse-native)](https://github.com/knopkem/dicom-dimse-native/issues) [![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native?ref=badge_shield)

[![Greenkeeper badge](https://badges.greenkeeper.io/knopkem/dicom-dimse-native.svg)](https://greenkeeper.io/)

# dicom-dimse-native
Nodejs native addon for DICOM DIMSE services using the IMEBRA DICOM c++ toolkit.

# supported DIMSE services
* C-Echo-scu 
* C-Find-scu
* C-Move-scu
* C-Get-scu
* C-Store-scp

# in development
* C-Store-scu

## How to install
This package uses prebuild to fetch precompiled binaries, so provided your platform is supported, all you need to do is:
```npm i -s dicom-native-addon```

Otherwise install will try to compile the sources for your platform, you will need:
* CMake installed and in path
* a working c++ compiler
* linux only: gobjc++ library installed (e.g for ubuntu: sudo apt-get install gobjc++)

## Examples
```
const dimse = require('dicom-dimse-native');

dimse.startScp(JSON.stringify(
    {
        "source": {
            "aet": "IMEBRA",
            "ip" : "127.0.0.1",
            "port": "9999"
        }
    }   
    ), (result) => {
        try {
            console.log(JSON.parse(result));
        }
        catch {
            console.log(result);
        }
});

dimse.moveScu(JSON.stringify(
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
    try {
        console.log(JSON.parse(result));
    }
    catch {
        console.log(result);
    }
});

dimse.findScu(JSON.stringify(
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
    try {
        console.log(JSON.parse(result));
    }
    catch {
        console.log(result);
    }
});
```

## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native?ref=badge_large)