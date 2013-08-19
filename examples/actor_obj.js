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

that.throw_Error = function(data, ctx) {
    print("I should raise error, should be passed to the caller");
    throw Error("Expected error, coming from actor");
    print("This line should not be printed");
    return "This value should never be returned";
};

that.throw_exception = function(data, ctx) {
    print("I should raise string exception, should be passed to the caller");
    throw "Expected string exception, coming from actor";
    print("This line should not be printed");
    return "This value should never be returned";
};

that.call_undefined = function(data, ctx) {
    print("Trying to call undefined from actor, should pass error to the caller");
    util.some_undefined_function();
};

exports = that
