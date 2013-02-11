(function () {
    var that = {}
    var names = ['1st', '2nd', '3rd', '4th', '5th']
    var util = qtscript.include('util.js')
    that.name = function(data, ctx) {
        print('name() is called with #', data)
        util.foreach(data, function(i) { ctx.reply(i.toString() + " is "
                                                   + names[i - 1]) } )
        return "done"
    }
    that.plus1 = function(data, ctx) {
        print('plus1() is called with #', data)
        util.foreach(data, function(i) { ctx.reply(i + 1) } )
        return "done"
    }
    return that
}).call(this)
