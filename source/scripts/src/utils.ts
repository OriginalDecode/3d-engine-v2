import * as fs from 'fs';
import chalk from 'chalk';

export namespace utils {
  // returns the files in given directory - not sub folders?
  export async function getFiles(dirPath: string): Promise<Array<string>> {
    return new Promise((resolve, reject) => {
      return fs.readdir(dirPath, 'utf8', (err, files) => (err ? reject(err) : resolve(files)));
    });
  }

  export function getArgsFromString(str: string): string[] {
    return str
      .split('\n')[0]
      .replace('//', '')
      .split(/(?=\-)/);
  }
}

console.error = (buffer: any) => {
  // if(buffer.search(/error/gi)) process.stdout.write(chalk.red(buffer) + '\n');
  // else if(buffer.search(/warn/gi)) process.stdout.write(chalk.yellow(buffer) + '\n');
  // else 
  process.stdout.write(buffer + '\n');
};

console.warn = (buffer: any) => {
  // if(buffer.search(/warn/gi)) process.stdout.write(chalk.yellow(buffer) + '\n');
  // else process.stdout.write(buffer + '\n');
};
