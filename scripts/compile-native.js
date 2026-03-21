'use strict';

const { spawnSync } = require('node:child_process');

const { ensureLibIconvPrefix } = require('./libiconv');

const env = { ...process.env };

if (!env.LIB_ICONV && env.SKIP_LIB_ICONV_BOOTSTRAP !== '1') {
  env.LIB_ICONV = ensureLibIconvPrefix();
}

if (env.LIB_ICONV) {
  console.log(`Using LIB_ICONV=${env.LIB_ICONV}`);
} else if (env.SKIP_LIB_ICONV_BOOTSTRAP === '1') {
  console.log('SKIP_LIB_ICONV_BOOTSTRAP=1, proceeding without bootstrapping libiconv.');
}

const command = process.platform === 'win32' ? 'npx.cmd' : 'npx';
const sharedArgs = process.argv.slice(2);

for (const cmakeJsCommand of ['configure', 'compile']) {
  const result = spawnSync(command, ['cmake-js', cmakeJsCommand, ...sharedArgs], {
    env,
    stdio: 'inherit'
  });

  if (result.error) {
    throw result.error;
  }

  if (result.status !== 0) {
    process.exit(result.status === null ? 1 : result.status);
  }
}

process.exit(0);
