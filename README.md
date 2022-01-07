[![Build Status](https://travis-ci.com/knopkem/dicom-dimse-native.svg?branch=master)](https://travis-ci.com/knopkem/dicom-dimse-native)
[![GitHub issues](https://img.shields.io/github/issues/knopkem/dicom-dimse-native)](https://github.com/knopkem/dicom-dimse-native/issues) [![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fknopkem%2Fdicom-dimse-native?ref=badge_shield)

[![Greenkeeper badge](https://badges.greenkeeper.io/knopkem/dicom-dimse-native.svg)](https://greenkeeper.io/)

# dicom-dimse-native
Nodejs native addon for DICOM DIMSE services using the DCMTK DICOM c++ toolkit.

# Supported DIMSE services
* C-Echo as SCU and SCP
* C-Find as SCU and SCP
* C-Move as SCU and SCP
* C-Get  as SCU and SCP
* C-Store as SCU and SCP

# Features
* typescript support
* build on solid c++ DICOM framework (DCMTK)
* prebuilds for: mac, windows, linux
* extended characterSet support
* simple to use

# Roadmap:
* JPEG 2000 support (c-store scu/scp)
* Worklist SCP and SCU

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

## Features:

* ECHO, FIND, MOVE, GET and STORE-SCP
* sqlite db backend
* multithreaded association handling


```
import { startStoreScp, storeScpOptions } from 'dicom-dimse-native';

const scpOptions: storeScpOptions = {
    source: {
        aet: "MY_AET",
        ip: "127.0.0.1",
        port: 9999,
    },
    peers: [
        {
            aet: "TARGET_AET",
            ip: "127.0.0.1",
            port: 5678
        }
    ],
    storagePath: "path_to_storage_dir",
};

startStoreScp(scpOptions, (result) => {
    console.log(JSON.parse(result));
});
```

# Move-SCU
```
 import { moveScu, moveScuOptions } from 'dicom-dimse-native';

 const moveOptions: moveScuOptions =
    {
        source: {
            aet: "MY_AET",
            ip: "127.0.0.1",
            port: 9999
        },
        target: {
            aet: "TARGET_AET",
            ip: "127.0.0.1",
            port: 5678
        },
        tags: [
            {
                key: "0020000D",
                value: "1.3.46.670589.5.2.10.2156913941.892665384.993397",
            },
            {
                key: "00080052",
                value: "STUDY",
            },
        ],
        destination: "MY_AET", // e.g. sending to ourself
        verbose: true
    };
    moveScu(moveOptions, (result) => {
        console.log(JSON.parse(result));
    });
```

# Find-SCU

```
import { findScu, findScuOptions } from 'dicom-dimse-native';

const options: findScuOptions = {
  source: {
    aet: "MY_AET",
    ip: "127.0.0.1",
    port: 9999
  },
  target: {
    aet: "TARGET_AET",
    ip: "127.0.0.1",
    port: 5678
  },
  tags: [
    {
      key: "0020000D",
      value: "1.3.46.670589.5.2.10.2156913941.892665384.993397",
    },
    {
      key: "00080052",
      value: "STUDY",
    },
    {
      key: "00100010",
      value: "",
    }
  ],
  verbose: true
};

findScu(options, (result) => {
  console.log(JSON.parse(result));
});
```

For more information see examples.

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
