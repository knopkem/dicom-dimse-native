'use strict';

const { ensureLibIconvPrefix } = require('./libiconv');

const prefix = ensureLibIconvPrefix({ argv: process.argv.slice(2) });
console.log(prefix);
