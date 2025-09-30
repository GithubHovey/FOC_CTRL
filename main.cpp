#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle> // 添加QQuickStyle头文件

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 设置应用样式为Basic，支持控件自定义
    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FOC_CTRL", "Main");

    return app.exec();
}
