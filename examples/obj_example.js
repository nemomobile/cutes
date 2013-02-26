var main = function() {
    var a = qtscript.actor()
    a.source = "actor_obj.js"
    a.error.connect(function(err) { print("ERR:", err) })
    // creating actors, executing in separate threads, replies are
    // processed by anon fn
    var names = ['name', 'plus1']
    var util = require('util.js')
    util.forEach(names, function(name) {
        a.request(name, [1, 2, 3, 4], function(d) {
            print(name, "reply", d)
            if (d === 'done')
                print(name, "actor function is returned")
        })
    })
}
main()
