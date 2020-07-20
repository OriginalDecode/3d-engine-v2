import { spawn, ChildProcess } from 'child_process';
import split2 from 'split2';
import fs from 'fs';
import chalk from 'chalk';

interface BuildOptions {
  platform: string;
  solution: string | null; // rename target?
  verbosity: string;
  config: string;
  rebuild: boolean;
}

function isDebug(config: string): boolean {
  return config.toLowerCase() === 'debug';
}

function getConfig(options: BuildOptions): string {
  const is_debug: boolean = isDebug(options.config);
  if (options.platform === 'osx') return is_debug ? 'debug_osx' : 'release_osx';
  if (options.platform === 'win32') return is_debug ? 'debug_win32' : 'release_win32';
  if (options.platform === 'linux') return is_debug ? 'debug_linux' : 'release_linux';
  throw new Error('No valid platform');
}

function buildOSX(rootDir: string, options: BuildOptions): ChildProcess {
  process.env.CXXFLAGS = '-fdeclspec -std=c++2a';
  return spawn('make', ['-C', `${rootDir}/source/`, `config=${getConfig(options)}`]);
}

function buildLinux(rootDir: string, options: BuildOptions): ChildProcess {
  return spawn('make', [`${rootDir}/source`]);
}

function buildWindows(rootDir: string, options: BuildOptions): ChildProcess {
  const programFilesx86 = process.env['ProgramFiles(x86)'];
  const version = ['Community', 'Professional', 'Enterprise', 'BuildTools'];
  let path = `${programFilesx86!}\\Microsoft Visual Studio`;
  const visualStudio = fs.readdirSync(path);
  let compiler: string | undefined = undefined;
  if (visualStudio.indexOf('2019') !== -1) {
    path = `${path}\\2019`;
    version.forEach((version) => {
      const pathVersion = `${path}\\${version}`;
      if (fs.existsSync(pathVersion)) {
        if (fs.existsSync(`${pathVersion}\\MSBuild\\Current`)) {
          compiler = `${pathVersion}\\MSBuild\\Current\\Bin\\amd64\\MSBuild.exe`;
        } else {
          const dir = fs.readdirSync(`${pathVersion}`);
          dir.forEach((subDir) => {
            if (fs.existsSync(`${pathVersion}\\MSBuild\\${subDir}\\Bin\\`)) {
              compiler = `${pathVersion}\\MSBuild\\${subDir}\\Bin\\amd64\\MSBuild.exe`;
            }
          });
        }
      }
    });
  }
  if (!compiler) throw new Error('Failed to find a compiler!');
  console.log(`Compiler found!\ncompiler: ${chalk.green(compiler)}`);
  return spawn(compiler, [
    options.solution!,
    '/p:GenerateFullPaths=true',
    `/p:Configuration=${options.config}`,
    '/p:Platform=Windows',
    '/t:build',
    `-v:${options.verbosity}`,
  ]);
}

function doBuild(rootDir: string, options: BuildOptions): ChildProcess {
  console.log(options.platform);
  if (options.platform === 'win32') return buildWindows(rootDir, options);
  if (options.platform === 'osx') return buildOSX(rootDir, options);
  if (options.platform === 'linux') return buildLinux(rootDir, options);

  throw new Error('No valid build option found. Check platform');
}

function build(rootDir: string, options: BuildOptions) {
  const proc = doBuild(rootDir, options);
  if (!proc) throw new Error('No process spawned. Failed to build');

  let warnings = 0;
  let errors = 0;
  proc.stdout!.pipe(split2()).on('data', (data: any) => {
    const string = data.toString();
    if (string.indexOf('warning') !== -1) {
      console.log(chalk.yellow(string));
      warnings++;
    } else if (string.indexOf('error') !== -1) {
      console.log(chalk.red(string));
      errors++;
    } else if (string.indexOf('message') !== -1) {
      console.log(chalk.blueBright(string));
    } else {
      console.log(string);
    }
  });

  proc.stderr!.pipe(split2()).on('data', (data: any) => {
    const string = data.toString();
    if (string.indexOf('warning') !== -1) {
      console.log(chalk.yellow(string));
      warnings++;
    } else if (string.indexOf('error') !== -1) {
      console.log(chalk.red(string));
      errors++;
    } else if (string.indexOf('message') !== -1) {
      console.log(chalk.blueBright(string));
    } else {
      console.log(string);
    }
  });

  proc.on('exit', (data: any) => {
    console.log({ warnings, errors });
    if (data === 0) console.log(chalk.green('Compile finished successfully!'));
    else console.log(chalk.red.bold('Failed to compile!'));
  });
}

export { build, BuildOptions };
