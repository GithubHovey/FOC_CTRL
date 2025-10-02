#include "command_control_manager.h"
#include "serial_communication_manager.h"
#include <QDebug>

CommandControlManager* CommandControlManager::m_instance = nullptr;

CommandControlManager* CommandControlManager::getInstance()
{
    if (!m_instance) {
        m_instance = new CommandControlManager();
    }
    return m_instance;
}

CommandControlManager::CommandControlManager(QObject *parent)
    : QObject(parent)
{
}

CommandControlManager::~CommandControlManager()
{
}

bool CommandControlManager::performCalibration()
{
    auto* serialManager = SerialCommunicationManager::getInstance();
    
    if (!serialManager->isConnected()) {
        emit errorOccurred("串口未连接");
        return false;
    }

    QByteArray data = createCalibrationData();
    bool success = serialManager->pushCmd(data, CMD_MOTOR_CALIBRATE);
    
    if (success) {
        qDebug() << "校准命令发送成功";
        emit commandCompleted("calibration", true);
    } else {
        qWarning() << "校准命令发送失败";
        emit errorOccurred("校准命令发送失败");
        emit commandCompleted("calibration", false);
    }

    return success;
}

bool CommandControlManager::startMotor()
{
    qWarning() << "startMotor: 功能未实现";
    emit errorOccurred("startMotor: 功能未实现");
    emit commandCompleted("startMotor", false);
    return false;
}

bool CommandControlManager::stopMotor()
{
    qWarning() << "stopMotor: 功能未实现";
    emit errorOccurred("stopMotor: 功能未实现");
    emit commandCompleted("stopMotor", false);
    return false;
}

bool CommandControlManager::emergencyStop()
{
    qWarning() << "emergencyStop: 功能未实现";
    emit errorOccurred("emergencyStop: 功能未实现");
    emit commandCompleted("emergencyStop", false);
    return false;
}

bool CommandControlManager::clearErrors()
{
    qWarning() << "clearErrors: 功能未实现";
    emit errorOccurred("clearErrors: 功能未实现");
    emit commandCompleted("clearErrors", false);
    return false;
}

bool CommandControlManager::resetSystem()
{
    qWarning() << "resetSystem: 功能未实现";
    emit errorOccurred("resetSystem: 功能未实现");
    emit commandCompleted("resetSystem", false);
    return false;
}

bool CommandControlManager::sendCommand(motor_command_t cmd, const QByteArray &data)
{
    auto* serialManager = SerialCommunicationManager::getInstance();
    
    // 确保数据长度为10字节
    QByteArray sendData = data;
    if (sendData.size() < 10) {
        sendData.resize(10);
        sendData.fill(0);
    } else if (sendData.size() > 10) {
        sendData = sendData.left(10);
    }

    return serialManager->pushCmd(sendData, cmd);
}

QByteArray CommandControlManager::createCalibrationData()
{
    QByteArray data(10, 0);
    
    // 10字节数据全为0，使用memset确保清零（虽然QByteArray已自动初始化）
    memset(data.data(), 0, 10);
    
    return data;
}

QByteArray CommandControlManager::createStartData()
{
    QByteArray data(10, 0);
    
    // 启动命令数据格式
    // byte 0: 命令类型 - 启动
    data[0] = 0x01;  // START命令
    
    // byte 1-9: 启动参数（保留）
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x00;
    
    return data;
}

QByteArray CommandControlManager::createStopData()
{
    QByteArray data(10, 0);
    
    // 停止命令数据格式
    // byte 0: 命令类型 - 停止
    data[0] = 0x02;  // STOP命令
    
    // byte 1-9: 停止参数（保留）
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x00;
    
    return data;
}

QByteArray CommandControlManager::createEmergencyStopData()
{
    QByteArray data(10, 0);
    
    // 快速停止命令数据格式
    // byte 0: 命令类型 - 快速停止
    data[0] = 0x03;  // EMERGENCY_STOP命令
    
    // byte 1-9: 快速停止参数（保留）
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x00;
    
    return data;
}

QByteArray CommandControlManager::createClearErrorsData()
{
    QByteArray data(10, 0);
    
    // 清除错误命令数据格式
    // byte 0: 命令类型 - 清除错误
    data[0] = 0x04;  // CLEAR_ERRORS命令
    
    // byte 1-9: 清除错误参数（保留）
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x00;
    
    return data;
}

QByteArray CommandControlManager::createResetData()
{
    QByteArray data(10, 0);
    
    // 复位命令数据格式
    // byte 0: 命令类型 - 复位
    data[0] = 0x05;  // RESET命令
    
    // byte 1-9: 复位参数（保留）
    data[1] = 0x00;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x00;
    
    return data;
}