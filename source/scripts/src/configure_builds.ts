import { spawn, exec } from 'child_process';
import split2 from 'split2';
import fs from 'fs';
import chalk from 'chalk';
import * as utils from './utils';

export namespace configure {
  export enum Platform {
    NONE,
    LINUX,
    WINDOWS,
    MACOS,
  };
  export interface Options {
    project: string;
    platform: string;
    generator: string;
    clean: boolean;
  }

  function isDir(path: string): boolean {
    return fs.existsSync(path) && fs.statSync(path).isDirectory();
  }

  // // Do i first need to count all the files, then parse the files?
  // // DO I first need to add
  // function clearDir(path: string) {
  //   const objects = fs.readdirSync(path);
  //   objects.forEach((obj) => {
  //     if (isDir(`${path}\\${obj}`)) {
  //       clearDir(`${path}\\${obj}`);
  //     } else {
  //       fs.unlink(`${path}\\${obj}`, (err) => {
  //         if (err) {
  //           throw 'Failed to remove file';
  //         }
  //         console.log(`Removed ${obj}`);
  //       });
  //     }
  //   });
  // }

  export function clean(rootDir: string) {
    console.log(chalk.bold.underline('Starting Cleanup'));

    // Get all the binaries from the bin folder & remove them
    const files = fs.readdirSync(`${rootDir}\\bin`).filter((file) => file.search(/Engine_*.*/) >= 0);
    files.forEach((file) => {
      fs.unlink(`${rootDir}\\bin\\${file}`, (err) => {
        if (err) {
          throw 'Failed to remove file';
        }
        console.log(`Removed ${file}`);
      });
    });

    // remove object files, pdb, ilk etc..
    if (isDir(`${rootDir}\\source\\obj`)) {
      exec(`rd -r ${rootDir}\\source\\obj /s /q`, (err) => {
        if (err) {
          console.error(err);
        }
      });
    }

    // remove static libraries
    if (isDir(`${rootDir}\\source\\lib`)) {
      exec(`rd -r ${rootDir}\\source\\lib /s /q`, (err) => {
        if (err) {
          console.error(err);
        }
      });
    }

    const ignore = ['external_libs', 'node_modules', 'unit_test', 'shaders', 'scripts', 'obj', 'premake5'];
    // remove all vcxproj related files
    let source = fs.readdirSync(`${rootDir}\\source`).filter((value) => value.search(/\..+/) < 0);
    source = source.filter((el) => {
      return ignore.indexOf(el) < 0;
    });

    source.forEach((dir) => {
      const files = fs
        .readdirSync(`${rootDir}\\source\\${dir}`)
        .filter((value) => value.search(/[A-Za-z].+\.vcxproj/) >= 0);

      files.forEach((file) => {
        fs.unlink(`${rootDir}\\source\\${dir}\\${file}`, (err) => {
          if (err) {
            throw 'Failed to remove file';
          }
          console.log(`Removed ${file}`);
        });
      });
    });

    console.log(source);
  }

  export async function configure(rootDir: string, options: Options) {
    console.log(rootDir);


    let premake = `${rootDir}\\premake5`;
    if (!fs.existsSync(premake)) {
      premake = `${rootDir}/source/premake5`;
      if(options.platform === 'windows') {
        premake += '.exe';
      }
      console.log(premake);
      if (!fs.existsSync(premake)) {
        throw 'Failed to find premake5';
      }
    }
    console.log(premake);

    if (options.clean) {
      await clean(rootDir);
    }

    console.log(options);

    const proc = spawn(premake, [
      `--file=${premake}.lua`,
      `--project=${options.project}`,
      `--platform=${options.platform}`,
      `${options.generator}`,
      options.clean ? 'clean' : '',
    ]);
    
    proc.stdout.pipe(split2()).on('data', (data: any) => {
      console.log(chalk.blueBright(data.toString()));
    });

    proc.stderr.pipe(split2()).on('data', (data: any) => {
      console.error(chalk.red.underline.bold(data.toString()));
    });
  }
}
