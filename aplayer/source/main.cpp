#include <QQmlContext>
#include <QQuickStyle>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "backend.hpp"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/aplayer/layout/Main.qml"_qs);
    engine.rootContext()->setContextProperty("backend",new BackEnd(&app));
    qmlRegisterType<enums>("Enums",1,0,"Enums");

    QString styleName = "Material";
    QQuickStyle::setStyle(styleName);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
