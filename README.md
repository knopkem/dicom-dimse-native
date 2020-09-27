[![Build Status](https://travis-ci.com/knopkem/dicom-dimse-native.svg?branch=master)](https://travis-ci.com/knopkem/dicom-dimse-native)
[![GitHub issues](https://img.shields.io/github/issues/knopkem/dicom-dimse-native)](https://github.com/knopkem/dicom-dimse-native/issues) [![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native?ref=badge_shield)

[![Greenkeeper badge](https://badges.greenkeeper.io/knopkem/dicom-dimse-native.svg)](https://greenkeeper.io/)

# dicom-dimse-native
Nodejs native addon for DICOM DIMSE services using the DCMTK DICOM c++ toolkit.

# supported DIMSE services
* C-Echo as SCU and SCP
* C-Find as SCU and SCP
* C-Move as SCU and SCP
* C-Get  as SCU and SCP
* C-Store as SCU and SCP

# Roadmap:
* multithreaded association handling (scp)
* extended characterSet support
* JPEG 2000 support (c-store scu)

## How to install
This package uses prebuild to fetch precompiled binaries, so provided your platform is supported, all you need to do is:

```npm i -s dicom-native-addon```

Otherwise install will try to compile the sources for your platform, you will need:
* CMake installed and in path
* a working c++ compiler (vs 2015+ or g++5.3+)

## Examples

run the examples:
```npm run example:[echoscu|findscu|getscu|movescu|storescu|storescp]```

# PACS-server 

## Supports: ECHO, FIND, MOVE, GET and STORE-SCP using sqlite db backend
```
const dimse = require('dicom-dimse-native');

dimse.startScp(JSON.stringify(
    {
        "source": {
            "aet": "DCMTK",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "peers": [
                {
                "aet": "CONQUESTSRV1",
                "ip" : "127.0.0.1",
                "port": "5678"
                },
                {
                "aet": "DCM4CHEE",
                "ip" : "127.0.0.1",
                "port": "11112"
                }
            ],
        "storagePath": "./data" // Directory where incoming DICOM files will be stored
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

# Move-SCU
```
dimse.moveScu(JSON.stringify(
    {
        "source": {
            "aet": "DCMTK",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "target": {
            "aet": "CONQUESTSRV1",
            "ip" : "127.0.0.1",
            "port": "5678"
        },
        "destination" : "DCMTK",
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
```

# Get-SCU
```
dimse.getScu(JSON.stringify(
    {
        "source": {
            "aet": "DCMTK",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "target": {
            "aet": "CONQUESTSRV1",
            "ip" : "127.0.0.1",
            "port": "5678"
        },
        "storagePath": "./data" // Directory where incoming DICOM files will be stored
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
```

# Store-SCU
```
dimse.storeScu(JSON.stringify(
    {
        "source": {
            "aet": "DCMTK",
            "ip" : "127.0.0.1",
            "port": "9999"
        },
        "target": {
            "aet": "CONQUESTSRV1",
            "ip" : "127.0.0.1",
            "port": "5678"
        },
        "sourcePath": "./input" // Directory with DICOM files to be send
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

# Find-SCU
```
dimse.findScu(JSON.stringify(
    {
        "source": {
            "aet": "DCMTK",
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

# Result Format:
```
{
  code: 0 (success) / 1 (pending) / 2 (failure),
  container: null / 'DICOMJSON (only when using c-find)',
  messsage: 'request succeeded' / 'descriptive problem',
  status: 'success' / 'pending' / 'failure'
}
```
C-FIND results are returned in DICOMJSON format see https://www.dicomstandard.org/dicomweb/dicom-json-format/


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native?ref=badge_large)
