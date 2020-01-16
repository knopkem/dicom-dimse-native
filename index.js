console.time('Program runtime');

const fs = require('fs');

const addon = require('./module');


const j = {
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

