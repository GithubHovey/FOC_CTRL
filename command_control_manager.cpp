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
    : QObject(parent),
      m_calibrationTimeoutTimer(nullptr),
      m_calibrationInProgress(false)
{
    // 连接串口管理器的校准应答信号到本对象的槽函数
    auto* serialManager = SerialCommunicationManager::getInstance();
    connect(serialManager, &SerialCommunicationManager::cmdMotorCalibrateReceived,
            this, &CommandControlManager::onCmdMotorCalibrateReceived);
    
    // 初始化超时定时器
    m_calibrationTimeoutTimer = new QTimer(this);
    m_calibrationTimeoutTimer->setSingleShot(true);  // 单次触发
    m_calibrationTimeoutTimer->setInterval(2000);    // 2秒超时
    connect(m_calibrationTimeoutTimer, &QTimer::timeout,
            this, &CommandControlManager::onCalibrationTimeout);
}

CommandControlManager::~CommandControlManager()
{
}

bool CommandControlManager::performCalibration()
{
    auto* serialManager = SerialCommunicationManager::getInstance();
    
    if (!serialManager->isConnected()) {
        emit errorOccurred("串口未连接");
        emit calibrationStatusChanged("串口未连接", "#FF0000");
        return false;
    }

    // 如果校准已在进行中，直接返回
    if (m_calibrationInProgress) {
        qWarning() << "校准已在进行中，忽略重复请求";
        return false;
    }

    // 设置校准状态为进行中
    m_calibrationInProgress = true;

    // 发送状态更新到QML
    emit calibrationStatusChanged("正在发送校准命令...", "#FFA500");

    QByteArray data = createCalibrationData();
    bool success = serialManager->pushCmd(data, CMD_MOTOR_CALIBRATE);
    
    if (success) {
        qDebug() << "校准命令发送成功";
        emit calibrationStatusChanged("校准命令已发送，等待应答...", "#FFA500");
        // 启动2秒超时定时器
        m_calibrationTimeoutTimer->start();
        // 注意：此时不立即发送commandCompleted，等待应答或超时后再发送
    } else {
        qWarning() << "校准命令发送失败";
        emit errorOccurred("校准命令发送失败");
        emit calibrationStatusChanged("校准命令发送失败", "#FF0000");
        emit commandCompleted("calibration", false);
        // 重置校准状态
        m_calibrationInProgress = false;
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

void CommandControlManager::onCalibrationTimeout()
{
    qWarning() << "校准超时：2秒内未收到应答";
    
    // 只有在校准进行中才处理超时
    if (!m_calibrationInProgress) {
        return;
    }
    
    // 重置校准状态
    m_calibrationInProgress = false;
    
    // 发送超时状态更新到QML
    emit calibrationStatusChanged("校准超时：2秒内未收到应答", "#FF0000");
    emit errorOccurred("校准超时：2秒内未收到应答");
    emit commandCompleted("calibration", false);
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

void CommandControlManager::onCmdMotorCalibrateReceived(uint8_t status, uint8_t state)
{
    qDebug() << "收到校准应答 - 状态:" << status << "电机状态:" << state;
    
    // 只有在校准进行中才处理应答
    if (!m_calibrationInProgress) {
        qWarning() << "收到未预期的校准应答，忽略";
        return;
    }
    
    // 停止超时定时器
    if (m_calibrationTimeoutTimer->isActive()) {
        m_calibrationTimeoutTimer->stop();
    }
    
    // 重置校准状态
    m_calibrationInProgress = false;
    
    // 根据应答状态处理
    switch (status) {
        case 0x00: // RESPONSE_OK
            qDebug() << "校准成功";
            emit calibrationStatusChanged("校准成功！电机状态: " + QString::number(state), "#00FF00");
            emit commandCompleted("calibration", true);
            break;
            
        case 0x01: // RESPONSE_ERROR
            qWarning() << "校准失败：通用错误";
            emit calibrationStatusChanged("校准失败：通用错误", "#FF0000");
            emit errorOccurred("校准失败：通用错误");
            emit commandCompleted("calibration", false);
            break;
            
        case 0x06: // RESPONSE_CALIBRATE_FAILED
            qWarning() << "校准失败：校准过程错误";
            emit calibrationStatusChanged("校准失败：校准过程错误", "#FF0000");
            emit errorOccurred("校准失败：校准过程错误");
            emit commandCompleted("calibration", false);
            break;
            
        default:
            qWarning() << "校准失败：未知错误码" << status;
            emit calibrationStatusChanged(QString("校准失败：未知错误码 %1").arg(status), "#FF0000");
            emit errorOccurred(QString("校准失败：未知错误码 %1").arg(status));
            emit commandCompleted("calibration", false);
            break;
    }
}