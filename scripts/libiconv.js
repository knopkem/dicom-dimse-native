'use strict';

const { execFileSync } = require('node:child_process');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');

const repoRoot = path.resolve(__dirname, '..');
const libIconvRoot = path.join(repoRoot, '.build-deps', 'libiconv', `${process.platform}-${process.arch}`);
const prefixDir = path.join(libIconvRoot, 'prefix');

const unixVersion = '1.18';
const unixArchiveName = `libiconv-${unixVersion}.tar.gz`;
const unixArchiveUrl = `https://ftp.gnu.org/pub/gnu/libiconv/${unixArchiveName}`;

const windowsRepoUrl = 'https://github.com/pffang/libiconv-for-Windows.git';
const windowsTag = '1.18';

function ensureDir(dirPath) {
  fs.mkdirSync(dirPath, { recursive: true });
}

function run(command, args, options = {}) {
  console.log(`> ${command} ${args.join(' ')}`);
  execFileSync(command, args, {
    stdio: 'inherit',
    ...options
  });
}

function expectedFiles() {
  if (process.platform === 'win32') {
    return [
      path.join(prefixDir, 'include', 'iconv.h'),
      path.join(prefixDir, 'lib', 'libiconv_d.lib'),
      path.join(prefixDir, 'lib', 'libiconv_o.lib')
    ];
  }

  return [
    path.join(prefixDir, 'include', 'iconv.h'),
    path.join(prefixDir, 'lib', 'libiconv.a'),
    path.join(prefixDir, 'lib', 'libcharset.a')
  ];
}

function hasBuiltLibIconv() {
  return expectedFiles().every((filePath) => fs.existsSync(filePath));
}

function ensureCleanDirectory(dirPath) {
  if (fs.existsSync(dirPath)) {
    fs.rmSync(dirPath, { recursive: true, force: true });
  }

  ensureDir(dirPath);
}

function buildUnixLibIconv() {
  ensureDir(libIconvRoot);

  const archivePath = path.join(libIconvRoot, unixArchiveName);
  const sourceDir = path.join(libIconvRoot, `libiconv-${unixVersion}`);

  if (!fs.existsSync(archivePath)) {
    run('curl', ['-L', unixArchiveUrl, '-o', archivePath]);
  }

  if (!fs.existsSync(sourceDir)) {
    run('tar', ['-xzf', archivePath, '-C', libIconvRoot]);
  }

  const makeJobs = String(Math.max(os.cpus().length, 1));
  const command = [
    `cd "${sourceDir}"`,
    `./configure --prefix="${prefixDir}" --enable-static --disable-shared CFLAGS="-fPIC" CXXFLAGS="-fPIC"`,
    `make -j${makeJobs}`,
    'make install'
  ].join(' && ');

  run('sh', ['-lc', command]);
}

function getWindowsPlatform() {
  switch (process.arch) {
    case 'x64':
      return 'x64';
    case 'arm64':
      return 'ARM64';
    case 'ia32':
      return 'Win32';
    default:
      throw new Error(`Unsupported Windows architecture for libiconv bootstrap: ${process.arch}`);
  }
}

function tryReadCommandOutput(command, args) {
  try {
    return execFileSync(command, args, {
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'ignore']
    }).trim();
  } catch (error) {
    return '';
  }
}

function resolveWindowsMsbuild() {
  const msbuildFromPath = tryReadCommandOutput('where.exe', ['msbuild.exe']);
  if (msbuildFromPath) {
    return msbuildFromPath.split(/\r?\n/)[0].trim();
  }

  const vswhereCandidates = [
    process.env['ProgramFiles(x86)'],
    process.env.ProgramFiles
  ]
    .filter(Boolean)
    .map((rootDir) => path.join(rootDir, 'Microsoft Visual Studio', 'Installer', 'vswhere.exe'));

  for (const vswherePath of vswhereCandidates) {
    if (!fs.existsSync(vswherePath)) {
      continue;
    }

    const locatedMsbuild = tryReadCommandOutput(vswherePath, [
      '-latest',
      '-products',
      '*',
      '-requires',
      'Microsoft.Component.MSBuild',
      '-find',
      'MSBuild\\**\\Bin\\MSBuild.exe'
    ]);

    if (locatedMsbuild) {
      return locatedMsbuild.split(/\r?\n/)[0].trim();
    }
  }

  throw new Error(
    'Could not locate MSBuild. Install Visual Studio 2022 with Desktop development with C++ or add msbuild.exe to PATH.'
  );
}

function buildWindowsLibIconv() {
  ensureDir(libIconvRoot);

  const sourceDir = path.join(libIconvRoot, 'libiconv-for-windows');
  if (!fs.existsSync(path.join(sourceDir, '.git'))) {
    ensureCleanDirectory(sourceDir);
    run('git', ['clone', '--depth', '1', '--branch', windowsTag, windowsRepoUrl, sourceDir]);
  }

  const platform = getWindowsPlatform();
  const msbuildPath = resolveWindowsMsbuild();

  run(msbuildPath, ['LibIconv.sln', '/m', `/p:Configuration=ReleaseStatic`, `/p:Platform=${platform}`], {
    cwd: sourceDir
  });
  run(msbuildPath, ['LibIconv.sln', '/m', `/p:Configuration=DebugStatic`, `/p:Platform=${platform}`], {
    cwd: sourceDir
  });

  ensureDir(path.join(prefixDir, 'include'));
  ensureDir(path.join(prefixDir, 'lib'));

  fs.copyFileSync(
    path.join(sourceDir, 'include', 'iconv.h'),
    path.join(prefixDir, 'include', 'iconv.h')
  );
  fs.copyFileSync(
    path.join(sourceDir, 'output', platform, 'ReleaseStatic', 'libiconvStatic.lib'),
    path.join(prefixDir, 'lib', 'libiconv_o.lib')
  );
  fs.copyFileSync(
    path.join(sourceDir, 'output', platform, 'DebugStatic', 'libiconvStaticD.lib'),
    path.join(prefixDir, 'lib', 'libiconv_d.lib')
  );
}

function ensureLibIconvPrefix() {
  if (hasBuiltLibIconv()) {
    return prefixDir;
  }

  console.log(`Bootstrapping libiconv into ${prefixDir}`);

  if (process.platform === 'win32') {
    buildWindowsLibIconv();
  } else {
    buildUnixLibIconv();
  }

  if (!hasBuiltLibIconv()) {
    throw new Error(`libiconv bootstrap completed without producing the expected files in ${prefixDir}`);
  }

  return prefixDir;
}

module.exports = {
  ensureLibIconvPrefix,
  getLibIconvPrefix: () => prefixDir,
  hasBuiltLibIconv
};
