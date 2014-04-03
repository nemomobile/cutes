#include <iostream>

#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>

#include <cor/util.hpp>
#include <cor/options.hpp>
#include "Env.hpp"
#include "QmlAdapter.hpp"

namespace cutes {

typedef cor::OptParse<std::string> argv_parser_type;
static argv_parser_type parser({{'h', "help"}, {'c', "cli"}},
                               {{"help", "help"}, {"cli", "cli"}});
struct CmdLine {
    CmdLine(int argc, char *argv[]) : argc_(argc), argv_(argv) {}
    mutable int argc_;
    mutable char **argv_;
    argv_parser_type::map_type opts;
    std::vector<char const*> args;
};

static int usage(int rc)
{
    parser.show_help(std::cerr, "cutes"
                     , " [options] <js_or_qml_script_name>\n"
                     "\twhere [options] are:\n");
    return rc;
}

int executeScript(CmdLine const &cmd_line)
{
    QCoreApplication app(cmd_line.argc_, cmd_line.argv_);

    QJSEngine engine;
    auto script_env = new Env(&engine, &app, &engine);
    int rc = EXIT_SUCCESS;

    if (cmd_line.args.size() < 2) {
        if (isTrace()) tracer() << "Run repl";
        QTextStream in(stdin);
        QTextStream out(stdout);
        QString line;
        do {
            out << "> " << flush;
            line = in.readLine();
            auto v = script_env->eval(line);
            if (!v.isUndefined()) {
                if (!v.isError())
                    out << v.toString() << endl;
                else
                    qDebug() << v.toString();
            }
        } while (!line.isNull());
        out << endl;
        return rc;
    }

    QString script_file(cmd_line.args[1]);
    if (isTrace()) tracer() << "Execute script " << script_file;
    try {
        auto res = script_env->load(script_file, false);
        if (res.isError())
            rc = EXIT_FAILURE;
    } catch (Error const &e) {
        qDebug() << "Failed to eval:" << script_file;
        qDebug() << e.msg;
        rc = EXIT_FAILURE;
    }
    return rc == EXIT_SUCCESS && script_env->shouldWait()
        ? app.exec() : rc;
}

int executeQmlCli(CmdLine const &cmd_line)
{
    QCoreApplication app(cmd_line.argc_, cmd_line.argv_);
    QString script_path(cmd_line.args.at(1));
    if (isTrace()) tracer() << "Execute qml cli " << script_path;
    QQmlApplicationEngine engine(script_path);
    return app.exec();
}

int executeDeclarative(CmdLine const &cmd_line)
{
    QApplication app(cmd_line.argc_, cmd_line.argv_);
    QString script_file(cmd_line.args.at(1));
    if (isTrace()) tracer() << "Execute qml " << script_file;

    QDeclarativeView view;
    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
    setupDeclarative(app, view, QFileInfo(script_file).absoluteFilePath());
    view.setSource(QUrl::fromLocalFile(script_file));

#if QT_VERSION < 0x050000
    view.setAttribute(Qt::WA_OpaquePaintEvent);
    view.setAttribute(Qt::WA_NoSystemBackground);
    view.viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    view.viewport()->setAttribute(Qt::WA_NoSystemBackground);
#endif

    view.showFullScreen();
    return app.exec();
}

}


int main(int argc, char *argv[])
{
    using namespace cutes;
    CmdLine cmd_line(argc, argv);
    parser.parse(argc, argv, cmd_line.opts, cmd_line.args);
    if (cmd_line.opts.count("help"))
        return usage(0);

    if (cmd_line.args.size() == 1)
        return executeScript(cmd_line);

    QString script_file(cmd_line.args.at(1));
    QFileInfo info(script_file);
    if (!info.isFile()) {
        qWarning() << "There is no " << script_file << ", exiting";
        return EXIT_FAILURE;
    }

    return (QFileInfo(script_file).suffix() == "qml")
        ? (cmd_line.opts.count("cli")
           ? executeQmlCli(cmd_line)
           : executeDeclarative(cmd_line))
        : executeScript(cmd_line);
}
