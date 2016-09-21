#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <qqml.h>

#include <QQuickMapboxGL>
#include <QQuickMapboxGLStyle>
#include <QQuickMapboxGLMapParameter>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#if QT_VERSION >= 0x050300
    app.setWindowIcon(QIcon(":icon.png"));
#endif

    qmlRegisterType<QQuickMapboxGL>("QQuickMapboxGL", 1, 0, "MapboxMap");
    qmlRegisterType<QQuickMapboxGLStyle>("QQuickMapboxGL", 1, 0, "MapboxStyle");
    qmlRegisterType<QQuickMapboxGLMapParameter>("QQuickMapboxGL", 1, 0, "MapParameter");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
