(function () {
    return function(data, ctx) {
        print('actor called with #', data)
        for (var i = 0; i < 10; ++i)
            ctx.reply(i + data)
        return "actor is done"
    }
}).call(this)
