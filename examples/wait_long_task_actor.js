var os = require('os');
var time = require('time');

exports = function(data) {
    print("Got", data, ", waiting...");
    time.sleep(500);
    return "done";
};