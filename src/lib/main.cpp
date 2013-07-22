#include <cutes/core.hpp>
#include <QCoreApplication>

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

    registerLibrary(v8e);
    // using namespace cutes::js;
    // v8EngineAdd<File>(v8e, "File");
    // v8EngineAdd<FileInfo>(v8e, "FileInfo");
    // v8EngineAdd<IODevice>(v8e, "IODevice");
    // v8EngineAdd<ByteArray>(v8e, "ByteArray");
    // v8EngineAdd<Dir>(v8e, "Dir");
    // v8EngineAdd<Process>(v8e, "Process");


    QString code(
                 // "var module = {name : 'afile%1', exports : {}}\n"
                 // "var exports = module.exports;\n"
                 // "exports.q = 12;\n"
                 // "log(module.exports.q, 12, true, '123');\n"
                 // "log('ee');\r\n"
                 "var fname = 'afilex%1';"
                 "var f = new QFile(fname);\n"
                 "QIODevice.WriteOnly\n"
                 "log('opened', f.open(QIODevice.WriteOnly));\n"
                 "log('isOpened', f.isOpen());\n"
                 "f.write('%1=');\n"
                 //"f.write(6);\n"
                 "f.close()\n"
                 "var fi = new QFileInfo(fname);"
                 "delete f;\n"
                 "var f2 = new QFile(fname);\n"
                 "log('opened read', f2.open(QIODevice.ReadOnly));\n"
                 "var d = new QDir('.');\n"
                 "log('created sub', d.mkdir('qqq'));\n"
                 "fi.fileName() + fi.exists() + fi.group() + f2.readAll()"
                 "+ ' ' + QDir.homePath()"
                 );
    auto v = jseng->evaluate(code.arg(nr));
    return v.toString();
}

QString exec(int nr)
{
    QJSEngine *jseng = new QJSEngine();
    auto res = exec_(jseng, nr);
    jseng->collectGarbage();
    return res;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qDebug() << exec(13);
	return 0;
}
