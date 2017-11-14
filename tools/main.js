import config from 'config';
import program from 'commander';
import fs from 'fs';
import dvbtee from 'dvbtee';
const main = (argv) => {

  console.log('let\'s go');
  program
    .command('parse')
    .description('')
    .action(() => {
      var parser = new dvbtee.Parser({ 'passThru': true });
      parser.on('psip', data => {
        console.log(data.tableId);
      });
      fs.createReadStream('mux1-cp.ts').pipe(parser);
    });

  program.parse(argv);
};

module.exports = main;