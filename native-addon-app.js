console.time('Program runtime');

const fs = require('fs');

const addon = require('./index');

/*
const buf = fs.readFileSync('test-data');

console.time('Time spent in addon on main event loop thread');
console.time('Data manipulation');

addon.processData(buf, () => {
    console.timeEnd('Data manipulation');

    fs.writeFileSync('test-data-modified', buf);

    console.timeEnd('Program runtime');
});
*/
const j = {
    "tags" : [
        {
            "key": "00100010", 
            "value": "hello world",
        },
        {
            "key": "0020000D", 
            "value": "1.3.46.670589.11.0.1.1996082307380006",
        },
    ]
};

addon.doFind(JSON.stringify(j), (result) => {
    console.log(result);
});

// console.timeEnd('Time spent in addon on main event loop thread');
