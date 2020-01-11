console.time('Program runtime');

const fs = require('fs');

const addon = require('./index');

const buf = fs.readFileSync('test-data');

console.time('Time spent in addon on main event loop thread');
console.time('Data manipulation');

addon.processData(buf, () => {
    console.timeEnd('Data manipulation');

    fs.writeFileSync('test-data-modified', buf);

    console.timeEnd('Program runtime');
});

console.timeEnd('Time spent in addon on main event loop thread');
