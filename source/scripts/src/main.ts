import { compileShaders } from './compile_shaders';
import { build, BuildOptions } from './build';
import { configure } from './configure_builds';
import os from 'os';
import { utils } from './utils';

const args = process.argv.slice(2);

if (args.indexOf('-test') >= 0) {
  configure.clean(process.cwd());
}

if (args.indexOf('-compile_shaders') !== -1) {
  compileShaders(process.cwd());
}

function getPlatform(platform: string) : string {
  if(platform === 'win32') return 'windows';
  if(platform === 'darwin') return 'osx';
  return platform;
}

if (args.indexOf('-configure') !== -1) {
  const project = args[args.indexOf('-p') + 1];
  const generator = args[args.indexOf('-g') + 1];
  const clean = args.indexOf('-clean') != -1;
  const platform = os.platform();

  if (!project || !generator || !platform) {
    throw 'Invalid params';
  }

  configure.configure(process.cwd(), {
    project: project,
    platform: getPlatform(platform),
    clean: clean,
    generator: generator,
  });
}

function getArgValue(arg:string ) : string | null {
  if(args.indexOf(arg) !== -1) {
    return args[args.indexOf(arg) + 1];
  }
  return null;
}

if (getArgValue('-build')) {
  const options: BuildOptions = {
    verbosity: getArgValue('-v')!,
    config: getArgValue('-c')!,
    rebuild: getArgValue('-rebuild') !== null,
    solution: getArgValue('-solution'),
    platform: getPlatform(os.platform())
  };
  build(process.cwd(), options);
}
