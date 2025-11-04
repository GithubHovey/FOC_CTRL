#ifndef MOTOR_MODE_CONTROL_MANAGER_H
#define MOTOR_MODE_CONTROL_MANAGER_H

#include <QObject>
#include <QString>

class MotorModeControlManager : public QObject
{
    Q_OBJECT
    
public:
    // 电机控制模式枚举
    enum ControlMode {
        TORQUE_MODE = 0,    // 力矩模式
        SPEED_MODE = 1,      // 速度模式
        POSITION_MODE = 2   // 位置模式
    };
    Q_ENUM(ControlMode)
    
    // 属性声明
    Q_PROPERTY(ControlMode currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(double targetTorque READ targetTorque WRITE setTargetTorque NOTIFY targetTorqueChanged)
    Q_PROPERTY(double targetSpeed READ targetSpeed WRITE setTargetSpeed NOTIFY targetSpeedChanged)
    Q_PROPERTY(double targetPosition READ targetPosition WRITE setTargetPosition NOTIFY targetPositionChanged)
    Q_PROPERTY(bool isEnabled READ isEnabled WRITE setIsEnabled NOTIFY isEnabledChanged)
    Q_PROPERTY(double parameterValue READ parameterValue WRITE setParameterValue NOTIFY parameterValueChanged)

public:
    explicit MotorModeControlManager(QObject *parent = nullptr);
    
    // 属性读取方法
    ControlMode currentMode() const;
    double targetTorque() const;
    double targetSpeed() const;
    double targetPosition() const;
    bool isEnabled() const;
    double parameterValue() const;
    
    // 属性设置方法
    void setCurrentMode(ControlMode mode);
    void setTargetTorque(double torque);
    void setTargetSpeed(double speed);
    void setTargetPosition(double position);
    void setIsEnabled(bool enabled);
    void setParameterValue(double value);
    
    // QML可调用方法
    Q_INVOKABLE void toggleEnable();
    Q_INVOKABLE void setModeFromString(const QString &modeString);
    Q_INVOKABLE QString getModeString() const;
    Q_INVOKABLE void sendControlCommand();

signals:
    void currentModeChanged();
    void targetTorqueChanged();
    void targetSpeedChanged();
    void targetPositionChanged();
    void isEnabledChanged();
    void parameterValueChanged();
    void controlCommandSent(ControlMode mode, double value);
    void logMessage(const QString &message);

private:
    // 根据当前模式更新参数值到对应的目标值
    void updateTargetValueFromParameter();
    
    // 记录日志
    void log(const QString &message);
    
    ControlMode m_currentMode;      // 当前控制模式
    double m_targetTorque;          // 目标力矩 (Nm)
    double m_targetSpeed;           // 目标速度 (RPM)
    double m_targetPosition;        // 目标位置 (度)
    bool m_isEnabled;               // 使能状态
    double m_parameterValue;        // 参数值 (0-100)
};

#endif // MOTOR_MODE_CONTROL_MANAGER_H