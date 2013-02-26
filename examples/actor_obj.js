var that = {}
var names = ['1st', '2nd', '3rd', '4th', '5th']
var util = require('util.js')

that.name = function(data, ctx) {
    print('name() is called with #', data)
    util.forEach(data, function(i) {
        ctx.reply(i.toString() + " is " + names[i - 1])
    });
    return "done"
}
that.plus1 = function(data, ctx) {
    print('plus1() is called with #', data)
    util.forEach(data, function(i) { ctx.reply(i + 1) } )
    return "done"
}

that.cause_error = function(data, ctx) {
    print("I should raise error");
    throw {msg : "Expected error", src: "coming from actor"};
};

that.call_undefined = function(data, ctx) {
    print("I should try to call undefined");
    util.some_undefined_function();
};

exports = that
