const R = require('ramda');

import program from 'commander';
import fs from 'fs';
import path from 'path';
import dvbtee from 'dvbtee';

const main = (argv) => {
  console.log(__dirname);
  console.log('let\'s go');
  program
    .command('parse <stream>')
    .option('-t, --table [tableName]', 'table name')
    .action((stream, { table }) => {
      console.log('>>>>', stream);
      console.log('>>>>', table);
      const filepath = path.join(__dirname, '..', 'streams', stream);
      const parser = new dvbtee.Parser;
      parser.on('data', (data) => {
        if (!R.isNil(table) && (table !== data.tableName)) return;
        console.log('table id: ' + data.tableId,
          '\ntable name: ' + data.tableName,
          '\ntable data:\n', JSON.stringify(data, null, 2));
      });
      fs.createReadStream(filepath).pipe(parser);
    });

  program.parse(argv);
};

module.exports = main;