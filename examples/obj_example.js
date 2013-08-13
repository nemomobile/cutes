var main = function() {
    var a = cutes.actor()
    a.source = "actor_obj.js"
    a.error.connect(function(err) {
        print("Caller got error:", err)
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
                , { on_reply : function(name) {
                    if (name === 'done')
                        print(name, "actor function is returned");
                    else
                        print(name, "actor returned smth. wrong");
                }, on_progress : function(d) {
                    print(name, "reply", d)
                }})
        });

    a.request('throw_Error', 1, function(d) {
        print("Why you are here? There must be a error");
    });

    a.request('throw_exception', 1, function(d) {
        print("Why you are here? There must be a error");
    });

    a.request('call_undefined', 2, function(d) {
        print("Why you are here? There must be a error");
    });

    a.request('throw_exception', 1, { on_reply : function(d) {
        print("Why you are here? There must be a error");
    }, on_error : function(err) {
        print("Error processed in function:", err)
        for (var v in err)
            print("\t", v, "=", err[v])
    }});
}
main()
