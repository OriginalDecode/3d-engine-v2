import { spawn } from 'child_process';
import split2 from 'split2';
import fs from 'fs';
import chalk from 'chalk';

interface BuildOptions {
  platform: string;
  solution?: string; // only valid on windows
  verbosity: string;
  config: string;
  rebuild: boolean;
}

function buildLinux(rootDir: string) {
  //
}

function buildWindows(rootDir: string, options: BuildOptions): any {
  const programFilesx86 = process.env['ProgramFiles(x86)'];
  const version = ['Community', 'Professional', 'Enterprise', 'BuildTools'];
  let path = `${programFilesx86!}\\Microsoft Visual Studio`;
  const visualStudio = fs.readdirSync(path);
  let compiler: string | undefined = undefined;
  if (visualStudio.indexOf('2019') !== -1) {
    path = `${path}\\2019`;
    version.forEach((version) => {
      if (!compiler) {
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
      }
    });
  }

  if (compiler) {
    console.log(`Compiler found!\ncompiler: ${chalk.green(compiler)}`);
    const proc = spawn(compiler, [
      options.solution!,
      '/p:GenerateFullPaths=true',
      `/p:Configuration=${options.config}`,
      '/p:Platform=Windows',
      '/t:build',
      `-v:${options.verbosity}`,
    ]);

    return proc;
  }

  throw new Error('No compiler found');
}

function build(rootDir: string, options: BuildOptions) {
  let proc;
  if (options.platform === 'win32') {
    proc = buildWindows(rootDir, options);
  } else if (options.platform === 'linux') {
    proc = buildLinux(rootDir);
  }
  let warnings = 0;
  let errors = 0;

  proc.stdout.pipe(split2()).on('data', (data: any) => {
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

  proc.stderr.pipe(split2()).on('data', (data: any) => {
    console.error(chalk.red.underline.bold(data.toString()));
  });

  proc.on('exit', (data: any) => {
    console.log({ warnings, errors });
    if (data === 0) {
      console.log(chalk.green('Compile finished successfully!'));
    }
  });
}

export { build, BuildOptions };
