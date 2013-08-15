#include <cutes/core.hpp>
#include <cor/util.hpp>
#include <QCoreApplication>
#include <tut/tut.hpp>
#include <memory>

namespace tut
{

struct cutes_lib_test
{
    virtual ~cutes_lib_test()
    {
    }
};

typedef test_group<cutes_lib_test> tf;
typedef tf::object object;
tf cor_cutes_lib_test("cutes_lib");

enum test_ids {
    tid_qfile = 1
};

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

struct ExecEnv
{
    ExecEnv()
        : jseng(new QJSEngine())
    {
        using namespace v8;
        v8::HandleScope hscope;
        auto v8e = v8engine();
        v8::Context::Scope cscope(v8e->context());
        v8e->global()->Set(v8::String::New("log"), V8FUNCTION(log, v8e));
        jseng->globalObject().setProperty("Q", cutesRegister(jseng.get()));
    }

    QV8Engine *v8engine()
    {
        return jseng->handle();
    }

    QJSValue exec(QString const& code)
    {
        using namespace v8;
        v8::HandleScope hscope;
        auto v8e = v8engine();
        v8::Context::Scope cscope(v8e->context());
        return jseng->evaluate(code);
    }

    std::unique_ptr<QJSEngine> jseng;
};


template<> template<>
void object::test<tid_qfile>()
{
    std::unique_ptr<ExecEnv> env(new ExecEnv());
    auto t = ::time(nullptr);
    QDir wdir("/tmp");
    QString fname("cutes-test-file-%1");
    fname = fname.arg(t);
    QString fpath(wdir.filePath(fname));
    QString code(
                 "var f = new Q.File('%1');"
                 "var is_opened = f.open(Q.IODevice.WriteOnly);"
                 "f.write('ee');"
                 " f.close();"
                 "is_opened"
                 );
    auto res = env->exec(code.arg(fpath));
    auto rmfile = cor::on_scope_exit([&]() { wdir.remove(fname); });
    ensure("Code should not return error", !res.isError());
    ensure("Code should return bool", res.isBool());
    ensure("Code should return true", res.toBool());
    ensure("File should be created", QFileInfo(fpath).exists());
}

}
