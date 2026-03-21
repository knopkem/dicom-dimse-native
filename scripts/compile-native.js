'use strict';

const path = require('node:path');
const { spawnSync } = require('node:child_process');

const { ensureLibIconvPrefix } = require('./libiconv');

const repoRoot = path.resolve(__dirname, '..');
const cmakeJsCli = require.resolve('cmake-js/bin/cmake-js');
const env = { ...process.env };
const sharedArgs = process.argv.slice(2);

if (!env.LIB_ICONV && env.SKIP_LIB_ICONV_BOOTSTRAP !== '1') {
  env.LIB_ICONV = ensureLibIconvPrefix({ argv: sharedArgs });
}

if (env.LIB_ICONV) {
  console.log(`Using LIB_ICONV=${env.LIB_ICONV}`);
} else if (env.SKIP_LIB_ICONV_BOOTSTRAP === '1') {
  console.log('SKIP_LIB_ICONV_BOOTSTRAP=1, proceeding without bootstrapping libiconv.');
}

for (const cmakeJsCommand of ['configure', 'compile']) {
  const result = spawnSync(process.execPath, [cmakeJsCli, cmakeJsCommand, ...sharedArgs], {
    cwd: repoRoot,
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
