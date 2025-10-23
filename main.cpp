#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle> // 添加QQuickStyle头文件
#include "serial_communication_manager.h" // 添加串口通信管理器
#include "command_control_manager.h"
#include "foc_chart_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
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
    qmlRegisterSingletonType<FOCChartManager>("FOC_CTRL", 1, 0, "FOCChartManager",
                                             [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
                                                 Q_UNUSED(engine)
                                                 Q_UNUSED(scriptEngine)
                                                 return new FOCChartManager();
                                             });
    // 连接串口通信管理器到图表管理器，用于接收实时数据
    // 注意：FOCChartManager不是单例模式，需要通过QML上下文获取实例
    // 信号连接将在QML中完成
    
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FOC_CTRL", "Main");

    return app.exec();
}
