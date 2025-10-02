#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle> // 添加QQuickStyle头文件
#include "serial_communication_manager.h" // 添加串口通信管理器
#include "command_control_manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 设置应用样式为Basic，支持控件自定义
    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;
    
    // 注册类型到QML
    qmlRegisterSingletonType<SerialCommunicationManager>("FOC_CTRL", 1, 0, "SerialCommManager",
                                                       [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
                                                           Q_UNUSED(engine)
                                                           Q_UNUSED(scriptEngine)
                                                           return SerialCommunicationManager::getInstance();
                                                       });
    qmlRegisterSingletonType<CommandControlManager>("FOC_CTRL", 1, 0, "CommandControlManager",
                                                  [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
                                                      Q_UNUSED(engine)
                                                      Q_UNUSED(scriptEngine)
                                                      return CommandControlManager::getInstance();
                                                  });
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FOC_CTRL", "Main");

    return app.exec();
}
