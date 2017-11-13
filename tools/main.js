import config from 'config';
import program from 'commander';

const main = (argv) => {

  console.log('let\'s go');
  program
    .command('installUsersAndTeams')
    .description('create not already defined users and set up teams')
    .action(() => {
      installUsersAndTeams().then(() => {process.exit(0);});
    });

  program.parse(argv);
};

module.exports = main;