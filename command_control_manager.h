#ifndef COMMAND_CONTROL_MANAGER_H
#define COMMAND_CONTROL_MANAGER_H

#include <QObject>
#include <QByteArray>
#include "DOC/motor_protocol.h"

class SerialCommunicationManager;

/**
 * @brief 命令控制管理器 - 处理各种电机控制命令
 * 对应QML中的CommandControlModule，提供一键校准等功能的实现
 */
class CommandControlManager : public QObject
{
    Q_OBJECT

public:
    ~CommandControlManager();

    /**
     * @brief 获取单例实例
     */
    static CommandControlManager* getInstance();

    /**
     * @brief 执行一键校准
     * @return 是否成功发送校准命令
     */
    Q_INVOKABLE bool performCalibration();

    /**
     * @brief 执行电机启动
     * @return 是否成功发送启动命令
     */
    Q_INVOKABLE bool startMotor();

    /**
     * @brief 执行电机停止
     * @return 是否成功发送停止命令
     */
    Q_INVOKABLE bool stopMotor();

    /**
     * @brief 执行快速停止
     * @return 是否成功发送快速停止命令
     */
    Q_INVOKABLE bool emergencyStop();

    /**
     * @brief 清除错误状态
     * @return 是否成功发送清除错误命令
     */
    Q_INVOKABLE bool clearErrors();

    /**
     * @brief 复位系统
     * @return 是否成功发送复位命令
     */
    Q_INVOKABLE bool resetSystem();

signals:
    /**
     * @brief 命令执行完成信号
     */
    void commandCompleted(const QString &commandName, bool success);

    /**
     * @brief 错误发生信号
     */
    void errorOccurred(const QString &error);

private:
    // 构造函数私有化（单例模式）
    explicit CommandControlManager(QObject *parent = nullptr);
    
    // 静态单例实例指针
    static CommandControlManager* m_instance;
    
    // 删除拷贝构造函数和赋值运算符
    CommandControlManager(const CommandControlManager&) = delete;
    CommandControlManager& operator=(const CommandControlManager&) = delete;

    /**
     * @brief 发送命令到串口
     * @param cmd 命令类型
     * @param data 数据数组（10字节）
     * @return 是否成功发送
     */
    bool sendCommand(motor_command_t cmd, const QByteArray &data = QByteArray(10, 0));

    /**
     * @brief 创建校准数据包
     * @return 10字节校准数据
     */
    QByteArray createCalibrationData();

    /**
     * @brief 创建启动数据包
     * @return 10字节启动数据
     */
    QByteArray createStartData();

    /**
     * @brief 创建停止数据包
     * @return 10字节停止数据
     */
    QByteArray createStopData();

    /**
     * @brief 创建快速停止数据包
     * @return 10字节快速停止数据
     */
    QByteArray createEmergencyStopData();

    /**
     * @brief 创建清除错误数据包
     * @return 10字节清除错误数据
     */
    QByteArray createClearErrorsData();

    /**
     * @brief 创建复位数据包
     * @return 10字节复位数据
     */
    QByteArray createResetData();
};

#endif // COMMAND_CONTROL_MANAGER_H