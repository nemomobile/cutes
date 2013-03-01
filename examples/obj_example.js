var main = function() {
    var a = qtscript.actor()
    a.source = "actor_obj.js"
    a.error.connect(function(err) {
        print("ERR:", err)
        for (var v in err)
            print("\t", v, "=", err[v])
    })
    // creating actors, executing in separate threads, replies are
    // processed by anon fn
    var names = ['name', 'plus1']
    var util = require('util.js')
    util.forEach(
        names
      , function(name) {
            a.request(
                name, [1, 2, 3, 4]
              , function(name) {
                    if (name === 'done')
                        print(name, "actor function is returned");
                    else
                        print(name, "actor returned smth. wrong");
                }
              , null
              , function(d) {
                    print(name, "reply", d)
                })
        });

    a.request('cause_error', 1, function(d) {
        print("Why you are here? There must be a error");
    });

    a.request('call_undefined', 2, function(d) {
        print("Why you are here? There must be a error");
    });

    a.request('cause_error', 1, function(d) {
        print("Why you are here? There must be a error");
    }, function(err) {
        print("Error processed in function:", err)
        for (var v in err)
            print("\t", v, "=", err[v])
    });
}
main()
