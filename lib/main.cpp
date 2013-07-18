#include <cutes/sys.hpp>

static v8::Handle<v8::Value> log(const v8::Arguments& args)
{
    auto v8e = V8ENGINE();

    if (args.IsConstructCall())
        V8THROW_ERROR("Cannot call function as ctor");

    QList<QVariant> vals;
    for (auto i = 0; i < args.Length(); ++i) {
        vals.push_back(v8e->toVariant(args[i], QVariant::String));
    }
    qDebug() << "Log:" << vals;
    return v8::Undefined();
}

QString exec_(QJSEngine *jseng, int nr)
{
    using namespace v8;
    //QJSEngine *jseng = new QJSEngine();
    v8::HandleScope hscope;
    auto v8e = jseng->handle();
    v8::Context::Scope cscope(v8e->context());

    v8e->global()->Set(v8::String::New("log"), V8FUNCTION(log, v8e));

    using namespace cutes::js;
    v8EngineAdd<File>(v8e, "File");
    v8EngineAdd<FileInfo>(v8e, "FileInfo");
    v8EngineAdd<IODevice>(v8e, "IODevice");


    QString code(
                 // "var module = {name : 'afile%1', exports : {}}\n"
                 // "var exports = module.exports;\n"
                 // "exports.q = 12;\n"
                 // "log(module.exports.q, 12, true, '123');\n"
                 // "log('ee');\r\n"
                 "var fname = 'afilex%1';"
                 "var f = new File(fname);\n"
                 "IODevice.WriteOnly\n"
                 "log('opened', f.open(IODevice.WriteOnly));\n"
                 "f.write('%1=');\n"
                 "f.write(13);\n"
                 "f.close()\n"
                 "var fi = new FileInfo(fname);"
                 "delete f;\n"
                 "fi.exists();"
                 );
    auto v = jseng->evaluate(code.arg(nr));
    return v.toString();
}

QString exec(int nr)
{
    QJSEngine *jseng = new QJSEngine();
    auto res = exec_(jseng, nr);
    //v8::V8::AdjustAmountOfExternalAllocatedMemory((intptr_t)1024 * 1024 * 1024 * 2);
    // while(!v8::V8::IdleNotification()) {};
    jseng->collectGarbage();
    return res;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::vector<std::future<QString> > f;
    for (int i = 0; i < 1; ++i) {
        f.push_back(std::async(exec, i));
    }
    for (auto &v : f) {
        qDebug() << v.get();
    }
	return 0;
}
