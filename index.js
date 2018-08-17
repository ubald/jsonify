'use strict';

const {Jsonify} = require('jsonify');

const jsonify = new Jsonify((obj) => {
    console.log('Parser results:');
    console.log(obj);
});

jsonify.parse('{"key":"value"}');

console.log("Hey, I'm not blocked!");
