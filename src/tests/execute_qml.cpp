#include <QString>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        qDebug() << "Usage:" << (argc ? argv[0] : "app") << "<filename.qml>";
        ::exit(1);
    }
    QString script_file(argv[1]);
    QCoreApplication app(argc, argv);
    QQmlApplicationEngine engine(script_file);
    return app.exec();
}

