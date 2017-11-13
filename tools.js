'use strict';

require ('babel-register' );
require('babel-polyfill');

let main = require ('./tools/main.js');

main(process.argv);