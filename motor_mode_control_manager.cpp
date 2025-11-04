#include "motor_mode_control_manager.h"
#include <QDebug>

MotorModeControlManager::MotorModeControlManager(QObject *parent)
    : QObject(parent)
    , m_currentMode(TORQUE_MODE)
    , m_targetTorque(0.0)
    , m_targetSpeed(0.0)
    , m_targetPosition(0.0)
    , m_isEnabled(false)
    , m_parameterValue(50.0)
{
    log("电机模式控制管理器初始化完成");
}

// 属性读取方法
MotorModeControlManager::ControlMode MotorModeControlManager::currentMode() const
{
    return m_currentMode;
}

double MotorModeControlManager::targetTorque() const
{
    return m_targetTorque;
}

double MotorModeControlManager::targetSpeed() const
{
    return m_targetSpeed;
}

double MotorModeControlManager::targetPosition() const
{
    return m_targetPosition;
}

bool MotorModeControlManager::isEnabled() const
{
    return m_isEnabled;
}

double MotorModeControlManager::parameterValue() const
{
    return m_parameterValue;
}

// 属性设置方法
void MotorModeControlManager::setCurrentMode(ControlMode mode)
{
    if (m_currentMode != mode) {
        m_currentMode = mode;
        log(QString("控制模式切换为: %1").arg(getModeString()));
        
        // 切换模式时，根据当前参数值更新对应的目标值
        updateTargetValueFromParameter();
        
        emit currentModeChanged();
    }
}

void MotorModeControlManager::setTargetTorque(double torque)
{
    if (qFuzzyCompare(m_targetTorque, torque)) {
        return;
    }
    
    // 力矩范围限制 (-100 Nm 到 100 Nm)
    if (torque < -100.0) torque = -100.0;
    if (torque > 100.0) torque = 100.0;
    
    m_targetTorque = torque;
    log(QString("目标力矩设置为: %1 Nm").arg(torque));
    emit targetTorqueChanged();
}

void MotorModeControlManager::setTargetSpeed(double speed)
{
    if (qFuzzyCompare(m_targetSpeed, speed)) {
        return;
    }
    
    // 速度范围限制 (-3000 RPM 到 3000 RPM)
    if (speed < -3000.0) speed = -3000.0;
    if (speed > 3000.0) speed = 3000.0;
    
    m_targetSpeed = speed;
    log(QString("目标速度设置为: %1 RPM").arg(speed));
    emit targetSpeedChanged();
}

void MotorModeControlManager::setTargetPosition(double position)
{
    if (qFuzzyCompare(m_targetPosition, position)) {
        return;
    }
    
    // 位置范围限制 (-360 度 到 360 度)
    if (position < -360.0) position = -360.0;
    if (position > 360.0) position = 360.0;
    
    m_targetPosition = position;
    log(QString("目标位置设置为: %1 度").arg(position));
    emit targetPositionChanged();
}

void MotorModeControlManager::setIsEnabled(bool enabled)
{
    if (m_isEnabled != enabled) {
        m_isEnabled = enabled;
        log(QString("电机%1").arg(enabled ? "使能" : "禁能"));
        emit isEnabledChanged();
    }
}

void MotorModeControlManager::setParameterValue(double value)
{
    if (qFuzzyCompare(m_parameterValue, value)) {
        return;
    }
    
    // 参数值范围限制 (0 到 100)
    if (value < 0.0) value = 0.0;
    if (value > 100.0) value = 100.0;
    
    m_parameterValue = value;
    
    // 根据当前模式更新对应的目标值
    updateTargetValueFromParameter();
    
    log(QString("参数值设置为: %1").arg(value));
    emit parameterValueChanged();
}

// QML可调用方法
void MotorModeControlManager::toggleEnable()
{
    setIsEnabled(!m_isEnabled);
}

void MotorModeControlManager::setModeFromString(const QString &modeString)
{
    if (modeString == "力矩模式") {
        setCurrentMode(TORQUE_MODE);
    } else if (modeString == "速度模式") {
        setCurrentMode(SPEED_MODE);
    } else if (modeString == "位置模式") {
        setCurrentMode(POSITION_MODE);
    } else {
        log(QString("未知的控制模式: %1").arg(modeString));
    }
}

QString MotorModeControlManager::getModeString() const
{
    switch (m_currentMode) {
    case TORQUE_MODE: return "力矩模式";
    case SPEED_MODE: return "速度模式";
    case POSITION_MODE: return "位置模式";
    default: return "未知模式";
    }
}

void MotorModeControlManager::sendControlCommand()
{
    if (!m_isEnabled) {
        log("电机未使能，无法发送控制命令");
        return;
    }
    
    double targetValue = 0.0;
    QString valueUnit;
    
    switch (m_currentMode) {
    case TORQUE_MODE:
        targetValue = m_targetTorque;
        valueUnit = "Nm";
        break;
    case SPEED_MODE:
        targetValue = m_targetSpeed;
        valueUnit = "RPM";
        break;
    case POSITION_MODE:
        targetValue = m_targetPosition;
        valueUnit = "度";
        break;
    }
    
    log(QString("发送控制命令 - 模式: %1, 目标值: %2 %3")
        .arg(getModeString())
        .arg(targetValue)
        .arg(valueUnit));
    
    emit controlCommandSent(m_currentMode, targetValue);
}

// 私有方法
void MotorModeControlManager::updateTargetValueFromParameter()
{
    // 将参数值 (0-100) 转换为对应模式的目标值范围
    switch (m_currentMode) {
    case TORQUE_MODE:
        // 参数值 0-100 映射到力矩 -100 Nm 到 100 Nm
        setTargetTorque((m_parameterValue - 50.0) * 2.0);
        break;
    case SPEED_MODE:
        // 参数值 0-100 映射到速度 -3000 RPM 到 3000 RPM
        setTargetSpeed((m_parameterValue - 50.0) * 60.0);
        break;
    case POSITION_MODE:
        // 参数值 0-100 映射到位置 -360 度 到 360 度
        setTargetPosition((m_parameterValue - 50.0) * 7.2);
        break;
    }
}

void MotorModeControlManager::log(const QString &message)
{
    QString formattedMessage = QString("[电机模式控制] %1").arg(message);
    qDebug() << formattedMessage;
    emit logMessage(formattedMessage);
}