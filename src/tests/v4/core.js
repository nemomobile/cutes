var Q = require("qt-core");
var p = new Q.Process();

var dump = function(n, o) {
    print(n);
    for (var m in o)
        print("$", m, typeof o[m]);
};

print("!!!!", Q.Process.ExitStatus.CrashExit);
dump("P", p);
print(cutes.os);
cutes.setEnv("e", "RR");
cutes.print(Q);
fprint("stdout", "HI", "Another EOL");
fprint("stderr", "HI", "Std err EOL");
fprint("stderr", "New process");
p.start("/bin/echo", ["ECHO"]);
fprint("stderr", "Started");
p.waitForFinished(-1);
fprint("stderr", "Finished");
var o = p.readAllStandardOutput();
print("OUT:", o.toString());
o.append(new Q.ByteArray("Test"));
print("OUT2<", o.toString(), ">");
for (var m in o)
    print("$", m);

var fout = new Q.File("data.txt");
print("OPEN=", fout.open(Q.File.OpenMode.WriteOnly));
fout.write(new Q.ByteArray("SOME DATA"));
fout.close();

var f = new Q.File("data.txt");
print("OPEN=", f.open(Q.File.OpenMode.ReadOnly));
var data = f.readAll();
print(data.toString());
