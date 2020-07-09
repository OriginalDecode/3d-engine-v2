import { utils } from './utils';
import { spawn } from 'child_process';
import fs from 'fs';
import split2 from 'split2';
import chalk from 'chalk';

const types = ['vs', 'ps', 'gs', 'ds', 'hs', 'ms', 'cs', 'as'];
const versions = ['6_0', '6_1', '6_2', '6_3', '6_4', '6_5'];

const validate = (arg: string, array: string[]) => {
  let result = false;
  array.forEach((type) => {
    if (arg.indexOf(type) >= 0) {
      result = true;
    }
  });
  return result;
};

function getArgs(inputFile: string): { failed: boolean; data: string[] } {
  const args = fs
    .readFileSync(inputFile, 'utf8')
    .split('\n')[0]
    .replace('//', '')
    .split(/(?=\-)/)
    .slice(1);

  for (let i = 0; i < args.length; ++i) {
    const arg = args[i];
    if (arg.indexOf('-T') >= 0) {
      // validate types
      if (!validate(arg, types)) {
        return {
          failed: true,
          data: [
            `Failed to validate ${chalk.underline('shader type')}. Invalid type ${chalk.underline(
              `${arg}`
            )}. Valid types ${types}`,
          ],
        };
      }

      // validate versions
      if (!validate(arg, versions)) {
        return {
          failed: true,
          data: [
            `Failed to validate ${chalk.underline('shader version')}. Invalid version ${chalk.underline(
              `${arg}`
            )}. Valid versions ${versions}`,
          ],
        };
      }
    }

    args[i] = arg.replace(/\s/g, '');
  }

  return { failed: false, data: args };
}

interface CompiledShaderObject {
  timestamp: Date;
  shader: String;
}

// this function is to just collect the objects, we will then compare the timestamps and see which shaders has been updated.
// might want to write a cache for the shaders, only compile what has been changed. This is not relevant yet tho.
function getCompiledShaders(): Array<CompiledShaderObject> {
  return new Array<CompiledShaderObject>();
}

async function compileShaders(rootDir: string) {
  const config = {
    compiler: `${rootDir}\\source\\external_libs\\dxc\\bin\\Debug\\bin\\dxc.exe`,
    shaderFolder: `${rootDir}\\source\\shaders\\`,
    outputFolder: `${rootDir}\\bin\\Data\\Shaders\\`,
    options: ['-spirv', '-Zpr', '-fspv-target-env=vulkan1.1'],
  };

  let shaders = new Array<string>();
  await utils
    .getFiles(config.shaderFolder)
    .then((resolve) => {
      shaders = [...resolve];
      console.log(shaders);
    })
    .catch((reject) => {
      console.error('Failed:', { reject });
    });

  if (shaders.length <= 0) {
    console.error('No shaders');
    return;
  }

  shaders.forEach((shader: string) => {
    const inputFile = `${config.shaderFolder}${shader}`;
    const args = getArgs(inputFile);
    if (args.failed) {
      console.error(args.data);
    } else {
      const proc = spawn(config.compiler, [
        ...args.data,
        ...config.options,
        `-Fo${config.outputFolder}${shader}`,
        `${inputFile}`,
      ]);

      proc.stdout.pipe(split2()).on('data', (data: any) => {
        console.log(data.toString());
      });

      proc.stderr.pipe(split2()).on('data', (data: any) => {
        console.error(data.toString());
      });
    }
  });
}

export { compileShaders };
