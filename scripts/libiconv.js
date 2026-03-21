'use strict';

const { execFileSync } = require('node:child_process');
const fs = require('node:fs');
const path = require('node:path');

const repoRoot = path.resolve(__dirname, '..');
const libIconvCmakeRepository =
  process.env.LIBICONV_CMAKE_REPOSITORY || 'https://github.com/knopkem/libiconv-cmake.git';
const libIconvCmakeRef =
  process.env.LIBICONV_CMAKE_REF || 'ddbb89e5625ee4c05bb817ab47a0e566fa06eb9c';
const libIconvCmakeSourceDir = process.env.LIBICONV_CMAKE_SOURCE_DIR || '';

function sanitizePathSegment(value) {
  return (value || 'default').replace(/[^A-Za-z0-9._-]/g, '-');
}

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

function normalizeBuildConfiguration(buildConfiguration) {
  switch ((buildConfiguration || '').toLowerCase()) {
    case 'debug':
      return 'Debug';
    case 'relwithdebinfo':
      return 'RelWithDebInfo';
    case 'minsizerel':
      return 'MinSizeRel';
    case 'release':
      return 'Release';
    default:
      return buildConfiguration || 'Release';
  }
}

function hasCliFlag(argv, longName, shortName) {
  return argv.some((arg) => arg === longName || arg === shortName);
}

function readCliOption(argv, longName, shortName) {
  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];

    if (arg === longName || arg === shortName) {
      if (index + 1 >= argv.length) {
        throw new Error(`Missing value for ${arg}`);
      }

      return argv[index + 1];
    }

    if (arg.startsWith(`${longName}=`)) {
      return arg.slice(longName.length + 1);
    }

    if (shortName && arg.startsWith(`${shortName}=`)) {
      return arg.slice(shortName.length + 1);
    }
  }

  return '';
}

function resolveBuildConfiguration(argv = []) {
  const configuredBuild = readCliOption(argv, '--config', '-B');
  if (configuredBuild) {
    return normalizeBuildConfiguration(configuredBuild);
  }

  if (hasCliFlag(argv, '--debug', '-D')) {
    return 'Debug';
  }

  return 'Release';
}

function resolveTargetArchitecture(argv = []) {
  return readCliOption(argv, '--arch', '-a') || process.arch;
}

function resolveCmakePath(argv = []) {
  return readCliOption(argv, '--cmake-path', '-c') || 'cmake';
}

function getWindowsPlatform(targetArchitecture) {
  switch (targetArchitecture) {
    case 'x64':
      return 'x64';
    case 'arm64':
      return 'ARM64';
    case 'ia32':
    case 'x86':
      return 'Win32';
    default:
      throw new Error(`Unsupported Windows architecture for libiconv bootstrap: ${targetArchitecture}`);
  }
}

function getWindowsConfiguration(buildConfiguration) {
  return buildConfiguration === 'Debug' ? 'DebugStatic' : 'ReleaseStatic';
}

function getBootstrapProfile(buildConfiguration) {
  if (process.platform === 'win32') {
    return getWindowsConfiguration(buildConfiguration).toLowerCase();
  }

  return 'static';
}

function getBootstrapLayout(options = {}) {
  const argv = options.argv || [];
  const buildConfiguration = normalizeBuildConfiguration(
    options.buildConfiguration || resolveBuildConfiguration(argv)
  );
  const targetArchitecture = options.targetArchitecture || resolveTargetArchitecture(argv);
  const bootstrapProfile = getBootstrapProfile(buildConfiguration);
  const bootstrapRoot = path.join(
    repoRoot,
    '.build-deps',
    'libiconv',
    `${process.platform}-${targetArchitecture}`,
    `libiconv-cmake-${sanitizePathSegment(libIconvCmakeRef).slice(0, 24)}`,
    bootstrapProfile
  );

  return {
    buildConfiguration,
    bootstrapProfile,
    buildDir: path.join(bootstrapRoot, 'build'),
    prefixDir: path.join(bootstrapRoot, 'prefix'),
    sourceDir: libIconvCmakeSourceDir
      ? path.resolve(libIconvCmakeSourceDir)
      : path.join(bootstrapRoot, 'src', 'libiconv-cmake'),
    targetArchitecture
  };
}

function expectedFiles(options = {}) {
  const { prefixDir } = getBootstrapLayout(options);
  if (process.platform === 'win32') {
    return [path.join(prefixDir, 'include', 'iconv.h'), path.join(prefixDir, 'lib', 'libiconv.lib')];
  }

  return [
    path.join(prefixDir, 'include', 'iconv.h'),
    path.join(prefixDir, 'lib', 'libiconv.a'),
    path.join(prefixDir, 'lib', 'libcharset.a')
  ];
}

function hasBuiltLibIconv(options = {}) {
  return expectedFiles(options).every((filePath) => fs.existsSync(filePath));
}

function ensureCleanDirectory(dirPath) {
  if (fs.existsSync(dirPath)) {
    fs.rmSync(dirPath, { recursive: true, force: true });
  }

  ensureDir(dirPath);
}

function tryReadCommandOutput(command, args, options = {}) {
  try {
    return execFileSync(command, args, {
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'ignore'],
      ...options
    }).trim();
  } catch (error) {
    return '';
  }
}

function ensureLibIconvSource(layout) {
  if (libIconvCmakeSourceDir) {
    if (!fs.existsSync(path.join(layout.sourceDir, 'CMakeLists.txt'))) {
      throw new Error(
        `LIBICONV_CMAKE_SOURCE_DIR does not point to a libiconv-cmake checkout: ${layout.sourceDir}`
      );
    }

    return layout.sourceDir;
  }

  ensureDir(path.dirname(layout.sourceDir));

  if (fs.existsSync(path.join(layout.sourceDir, '.git'))) {
    const currentRef = tryReadCommandOutput('git', ['rev-parse', 'HEAD'], { cwd: layout.sourceDir });
    if (currentRef === libIconvCmakeRef) {
      return layout.sourceDir;
    }
  }

  ensureCleanDirectory(layout.sourceDir);
  run('git', ['clone', libIconvCmakeRepository, layout.sourceDir]);
  run('git', ['checkout', libIconvCmakeRef], { cwd: layout.sourceDir });

  return layout.sourceDir;
}

function shouldPassWindowsPlatform(generator, configuredPlatform, targetArchitecture) {
  if (configuredPlatform) {
    return true;
  }

  if (generator) {
    return /visual studio/i.test(generator);
  }

  return targetArchitecture !== process.arch;
}

function buildLibIconv(options = {}) {
  const argv = options.argv || [];
  const layout = getBootstrapLayout(options);
  const cmakePath = resolveCmakePath(argv);
  const sourceDir = ensureLibIconvSource(layout);
  const generator = readCliOption(argv, '--generator', '-G');
  const toolset = readCliOption(argv, '--toolset', '-t');
  const configuredPlatform = readCliOption(argv, '--platform', '-A');

  ensureCleanDirectory(layout.buildDir);
  ensureCleanDirectory(layout.prefixDir);

  const configureArgs = [
    '-S',
    sourceDir,
    '-B',
    layout.buildDir,
    `-DCMAKE_INSTALL_PREFIX=${layout.prefixDir}`,
    '-DBUILD_SHARED_LIBS=OFF',
    '-DBUILD_TESTING=OFF'
  ];

  if (generator) {
    configureArgs.push('-G', generator);
  }

  if (toolset) {
    configureArgs.push('-T', toolset);
  }

  if (process.platform === 'win32') {
    if (shouldPassWindowsPlatform(generator, configuredPlatform, layout.targetArchitecture)) {
      configureArgs.push('-A', configuredPlatform || getWindowsPlatform(layout.targetArchitecture));
    }

    configureArgs.push(`-DLIBICONV_WINDOWS_CONFIGURATION=${getWindowsConfiguration(layout.buildConfiguration)}`);
  }

  run(cmakePath, configureArgs);

  const buildArgs = ['--build', layout.buildDir, '--parallel'];
  const installArgs = ['--install', layout.buildDir];

  if (process.platform === 'win32') {
    buildArgs.push('--config', layout.buildConfiguration);
    installArgs.push('--config', layout.buildConfiguration);
  }

  run(cmakePath, buildArgs);
  run(cmakePath, installArgs);
}

function ensureLibIconvPrefix(options = {}) {
  const layout = getBootstrapLayout(options);
  if (hasBuiltLibIconv(options)) {
    return layout.prefixDir;
  }

  if (libIconvCmakeSourceDir) {
    console.log(`Bootstrapping libiconv from ${layout.sourceDir} into ${layout.prefixDir}`);
  } else {
    console.log(
      `Bootstrapping libiconv via ${libIconvCmakeRepository}#${libIconvCmakeRef} into ${layout.prefixDir}`
    );
  }

  buildLibIconv(options);

  if (!hasBuiltLibIconv(options)) {
    throw new Error(
      `libiconv bootstrap completed without producing the expected files in ${layout.prefixDir}`
    );
  }

  return layout.prefixDir;
}

module.exports = {
  ensureLibIconvPrefix,
  getLibIconvPrefix: (options = {}) => getBootstrapLayout(options).prefixDir,
  resolveBuildConfiguration,
  hasBuiltLibIconv
};
