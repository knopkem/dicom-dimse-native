'use strict';

const { ensureLibIconvPrefix } = require('./libiconv');

const prefix = ensureLibIconvPrefix();
console.log(prefix);
