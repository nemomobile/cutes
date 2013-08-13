cutes.extend("qt.core")

exports = function(data, ctx) {
    print("Actor got:", data.from_qml)
    var e = 1
    // delays using mutex tryLock timeout
    var lock = new QMutex()
    lock.lock()
    try {
        for (var j = 0; j < 100; ++j) {
            lock.tryLock(50)
            ctx.reply({ intermediate : j })
        }
    } finally {
        lock.unlock()
    }
    print("done!")
    res = { res1 : 1, res2 : 2, res3 : 3 }
    for (var n in data)
        res[n] = data[n]
    return res
}
