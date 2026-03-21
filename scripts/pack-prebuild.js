'use strict';

const { execFileSync } = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');

const repoRoot = path.resolve(__dirname, '..');
const packageJsonPath = path.join(repoRoot, 'package.json');
const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
const binaryPath = path.join(repoRoot, 'build', 'Release', 'dcmtk.node');

if (!fs.existsSync(binaryPath)) {
  throw new Error(`Expected native addon at ${binaryPath}. Run the native build before packaging prebuilds.`);
}

const napiVersion = packageJson.binary && packageJson.binary.napi_versions && packageJson.binary.napi_versions[0];
if (!napiVersion) {
  throw new Error('Expected package.json to define binary.napi_versions[0].');
}

const prebuildsDir = path.join(repoRoot, 'prebuilds');
fs.mkdirSync(prebuildsDir, { recursive: true });

const archiveName =
  `${packageJson.name}-v${packageJson.version}-napi-v${napiVersion}-${process.platform}-${process.arch}.tar.gz`;
const archivePath = path.join(prebuildsDir, archiveName);

if (fs.existsSync(archivePath)) {
  fs.rmSync(archivePath);
}

execFileSync('tar', ['-czf', archivePath, '-C', repoRoot, 'build/Release/dcmtk.node'], {
  stdio: 'inherit'
});

console.log(`Created ${archivePath}`);
