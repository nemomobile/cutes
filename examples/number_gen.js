
var main = function(i) {
    var a = cutes.actor()
    a.source = "number_gen_actor.js"
    a.error.connect(function(err) { print("ERR:", err) })
    // creating actors, executing in separate threads, replies are
    // processed by anon fn
    a.send(i, {
        on_reply : function(d) {
            if (d === 'done')
                print("actor returned what was expected");
            else
                print("actor returned unexpected " + d);
        }
        , on_error : function(e) { print("Some error " + e); }
        , on_progress : function(d) { print("reply" + d); }
    });
}
var count = 10
// 'count' actors are performing work
for (var i = 0; i < count; ++i)
    main(i)
