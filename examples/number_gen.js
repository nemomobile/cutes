
var main = function(i) {
    var a = qtscript.actor()
    a.source = "number_gen_actor.js"
    a.error.connect(function(err) { print("ERR:", err) })
    // creating actors, executing in separate threads, replies are
    // processed by anon fn
    a.send(i, function(d) {
        print("reply", d)
        if (d === 'done')
            print("actor returned")
    })
}
var count = 100
// 'count' actors are performing work
for (var i = 0; i < count; ++i)
    main(i)
