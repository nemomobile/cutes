#include <iostream>

#include <QApplication>
#include <QDebug>
#include <QCommandLineParser>

#include <cor/util.hpp>
#include "Env.hpp"
#include "QmlAdapter.hpp"

namespace cutes {

static int usage(int, char *argv[])
{
    qDebug() << argv[0] << " <script_name>";
    return 0;
}

std::unique_ptr<QCommandLineParser> parseCmdLine(QCoreApplication &app)
{
    auto res = cor::make_unique<QCommandLineParser>();
    res->addHelpOption();
    res->addPositionalArgument("source", "Source file (js or qml)");
    res->process(app);
    return res;
}

int executeScript(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    auto parser = parseCmdLine(app);

    QJSEngine engine;
    auto script_env = new Env(&engine, &app, &engine);
    int rc = EXIT_SUCCESS;

    auto args = parser->positionalArguments();
    if (!args.size()) {
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

    QString script_file(args[0]);
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

int executeDeclarative(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (app.arguments().size() < 2)
        return usage(argc, argv);
    QString script_file(app.arguments().at(1));

    QDeclarativeView view;
    QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QCoreApplication::quit);
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
    QString script_file;
    if (argc == 1)
        return executeScript(argc, argv);

    if (argc >= 2) {
        script_file = argv[1];
        return (QFileInfo(script_file).suffix() == "qml")
            ? executeDeclarative(argc, argv)
            : executeScript(argc, argv);
    }
}
