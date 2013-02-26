var forEach = function(arr, fn) {
    var i;
    for (i = 0; i < arr.length; ++i)
        fn(arr[i]);
}

var map = function(arr, fn) {
    var i;
    var res = [];
    for (i = 0; i < arr.length; ++i)
        res.push(fn(arr[i]));
    return res;
}

exports = {
    forEach : forEach,
    map : map
}
