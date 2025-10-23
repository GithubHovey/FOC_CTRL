#include "foc_chart_manager.h"
#include <QDebug>
#include <QRandomGenerator>

FOCChartManager::FOCChartManager(QObject *parent)
    : QObject(parent)
    , m_isCollecting(false)  // 默认不开启采集
    , m_debugTimer(nullptr)  // 调试定时器
    , m_debugStartTime(0)    // 调试开始时间
    , m_debugSineWaveRunning(false) // 调试正弦波未运行
{
    // 初始化变量列表和颜色映射
    initializeAvailableVariables();
    initializeVariableColors();
    
    // 初始化变量数值存储
    m_variableValues.clear();
    
    // 创建调试定时器
    m_debugTimer = new QTimer(this);
    m_debugTimer->setInterval(50); // 50ms更新一次，20Hz频率
    connect(m_debugTimer, &QTimer::timeout, this, &FOCChartManager::updateDebugSineValue);
    
    log("FOCChartManager initialized - 采集状态: 未开启，变量数值接口已就绪");
}

QStringList FOCChartManager::availableVariables() const
{
    return m_availableVariables;
}

QStringList FOCChartManager::selectedVariables() const
{
    return m_selectedVariables;
}

QHash<QString, QColor> FOCChartManager::variableColors() const
{
    return m_variableColors;
}

QColor FOCChartManager::getVariableColor(const QString &variableName) const
{
    return m_variableColors.value(variableName, QColor("#FF6B6B"));
}

void FOCChartManager::addVariable(const QString &variableName)
{
    if (!m_availableVariables.contains(variableName)) {
        log(QString("Cannot add variable '%1': not in available list").arg(variableName));
        return;
    }
    
    if (m_selectedVariables.contains(variableName)) {
        log(QString("Variable '%1' is already selected").arg(variableName));
        return;
    }
    
    // 如果变量还没有颜色，分配一个随机颜色
    if (!m_variableColors.contains(variableName)) {
        m_variableColors[variableName] = generateRandomColor();
        emit variableColorsChanged();
    }
    
    // 为变量设置初始值0
    m_variableValues[variableName] = 0.0;
    
    m_selectedVariables.append(variableName);
    emit selectedVariablesChanged();
    
    log(QString("变量 '%1' 已添加到图表，初始值设置为: 0").arg(variableName));
    emit variableAdded(variableName, m_variableColors[variableName]);
}

void FOCChartManager::removeVariable(const QString &variableName)
{
    if (m_selectedVariables.removeOne(variableName)) {
        emit selectedVariablesChanged();
        log(QString("Variable '%1' removed from chart").arg(variableName));
        emit variableRemoved(variableName);
    }
}

QStringList FOCChartManager::getAllAvailableVariables() const
{
    return m_availableVariables;
}

double FOCChartManager::scrollPosition() const
{
    return m_viewState.scrollPosition;
}

void FOCChartManager::setScrollPosition(double position)
{
    if (position < 0.0) position = 0.0;
    if (position > 1.0) position = 1.0;
    
    if (qFuzzyCompare(m_viewState.scrollPosition, position))
        return;
    
    m_viewState.scrollPosition = position;
    emit scrollPositionChanged();
    
    log(QString("Scroll position updated to: %1").arg(position));
}

double FOCChartManager::zoomFactor() const
{
    return m_viewState.zoomFactor;
}

QRectF FOCChartManager::viewRange() const
{
    return m_viewState.viewRange;
}

void FOCChartManager::updateViewState(double xMin, double xMax, double yMin, double yMax, double zoom)
{
    bool changed = false;
    
    // 更新视图范围
    QRectF newRange(xMin, yMin, xMax - xMin, yMax - yMin);
    if (m_viewState.viewRange != newRange) {
        m_viewState.viewRange = newRange;
        changed = true;
    }
    
    // 更新缩放倍数
    if (!qFuzzyCompare(m_viewState.zoomFactor, zoom)) {
        m_viewState.zoomFactor = zoom;
        changed = true;
    }
    
    // 计算并更新滚动条位置（使用实际的数据长度）
    double totalRange = m_viewState.dataLengthMs;
    double visibleRange = xMax - xMin;
    
    // 避免除以零的情况
    double newPosition = 0.0;
    if (totalRange > visibleRange) {
        newPosition = xMin / (totalRange - visibleRange);
    } else {
        // 当可见范围大于等于总数据范围时，滚动条位置为0（显示全部数据）
        newPosition = 0.0;
    }
    
    if (newPosition < 0.0) newPosition = 0.0;
    if (newPosition > 1.0) newPosition = 1.0;
    
    if (!qFuzzyCompare(m_viewState.scrollPosition, newPosition)) {
        m_viewState.scrollPosition = newPosition;
        emit scrollPositionChanged();
    }
    
    if (changed) {
        emit viewRangeChanged();
        emit zoomFactorChanged();
        
        // 视图状态更新日志已移除，避免控制台输出过多信息
    }
}

double FOCChartManager::dataLengthMs() const
{
    return m_viewState.dataLengthMs;
}

void FOCChartManager::setDataLengthMs(double lengthMs)
{
    if (lengthMs < 0.0) lengthMs = 0.0;
    
    if (qFuzzyCompare(m_viewState.dataLengthMs, lengthMs))
        return;
    
    m_viewState.dataLengthMs = lengthMs;
    emit dataLengthMsChanged();
    
    log(QString("Data length updated to: %1 ms").arg(lengthMs));
}

void FOCChartManager::initializeAvailableVariables()
{
    // 初始化所有可用的FOC系统变量（基于协议定义）
    m_availableVariables = QStringList{
        "U相电流",
        "V相电流", 
        "W相电流",
        "转速",
        "Q轴电压",
        "D轴电压",
        "最大电流",
        "母线电压",
        "极对数",
        "电角度",
        "机械角",
        "控制模式",
        "接口模式",
        "工作状态",
        "力矩Kp",
        "力矩Ki",
        "力矩Kd",
        "速度Kp",
        "速度Ki",
        "速度Kd",
        "位置Kp",
        "位置Ki",
        "位置Kd",
        "电流环时",
        "CAN ID",
        "机械零位",
        "霍尔X偏",
        "霍尔Y偏",
        "霍尔状态",
        "速度环时",
        "位置环时",
        "调试正弦波"  // 调试曲线，用于测试和演示
    };
    
    log(QString("Available variables initialized: %1 variables").arg(m_availableVariables.size()));
}

void FOCChartManager::initializeVariableColors()
{
    // 定义循环颜色数组（避免白色和黑色）
    QList<QString> colorPalette = {
        "#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4", "#FECA57",
        "#FF9FF3", "#54A0FF", "#FF9F43", "#5F27CD", "#00D2D3",
        "#FFA500", "#6A0572", "#AB83A1", "#3D5A80", "#98C1D9",
        "#E0FBFC", "#EE6C4D", "#293241", "#F4A261", "#2A9D8F",
        "#E9C46A", "#F4A261", "#E76F51", "#2A9D8F", "#264653",
        "#E9C46A", "#F4A261", "#E76F51", "#2A9D8F", "#264653"
    };
    
    // 为每个变量分配颜色（循环使用颜色数组）
    for (int i = 0; i < m_availableVariables.size(); ++i) {
        QString variableName = m_availableVariables[i];
        QString colorCode = colorPalette[i % colorPalette.size()];
        m_variableColors[variableName] = QColor(colorCode);
    }
    
    log("Variable colors initialized");
}

QColor FOCChartManager::generateRandomColor() const
{
    // 生成随机但美观的颜色
    int hue = QRandomGenerator::global()->bounded(360);
    int saturation = QRandomGenerator::global()->bounded(30, 100);
    int lightness = QRandomGenerator::global()->bounded(40, 80);
    
    return QColor::fromHsl(hue, saturation, lightness);
}

void FOCChartManager::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logMsg = QString("[%1] FOCChartManager: %2").arg(timestamp, message);
    
    // 输出到控制台
    qDebug() << logMsg;
    
    // 发送日志信号
    emit logMessage(logMsg);
}

// 采集状态相关方法实现
bool FOCChartManager::isCollecting() const
{
    return m_isCollecting;
}

void FOCChartManager::setIsCollecting(bool collecting)
{
    if (m_isCollecting == collecting)
        return;
    
    m_isCollecting = collecting;
    emit isCollectingChanged();
    
    if (m_isCollecting) {
        log("开始采集数据");
        
        // 为所有选中的变量设置初始值0
        for (const QString &variableName : m_selectedVariables) {
            if (!m_variableValues.contains(variableName)) {
                m_variableValues[variableName] = 0.0;
                log(QString("变量 '%1' 初始值设置为: 0").arg(variableName));
            }
        }
        
        // 如果调试变量存在，自动启动调试正弦波信号发生器
        if (m_availableVariables.contains("调试正弦波")) {
            startDebugSineWave();
        }
    } else {
        log("停止采集数据");
        // 停止调试正弦波信号发生器
        stopDebugSineWave();
    }
}

void FOCChartManager::toggleCollection()
{
    setIsCollecting(!m_isCollecting);
}

// 变量数值更新方法实现
void FOCChartManager::updateVariableValue(const QString &variableName, double value)
{
    // 存储变量数值
    m_variableValues[variableName] = value;
    
    // 发出信号通知变量值已改变
    emit variableValueChanged(variableName, value);
    
    // 变量数值更新日志已移除，避免控制台输出过多信息
}

// 获取变量当前值方法实现
double FOCChartManager::getVariableValue(const QString &variableName) const
{
    // 如果变量存在，返回其当前值；否则返回0.0
    if (m_variableValues.contains(variableName)) {
        return m_variableValues[variableName];
    }
    
    // 变量不存在，返回默认值0.0
    return 0.0;
}

void FOCChartManager::updateDebugSineValue()
{
    // 检查运行条件：调试变量存在且采集状态打开
    if (!m_debugSineWaveRunning || !m_isCollecting) {
        return;
    }
    
    // 检查变量哈希表中是否存在"调试正弦波"变量
    if (!m_availableVariables.contains("调试正弦波")) {
        log("调试正弦波变量不存在，停止信号发生器");
        stopDebugSineWave();
        return;
    }
    
    // 计算经过的时间（秒）
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double elapsedTime = (currentTime - m_debugStartTime) / 1000.0;
    
    // 生成正弦波：周期2秒（频率0.5Hz），幅度1000，无偏置
    double debugValue = 1000.0 * std::sin(2.0 * M_PI * 0.5 * elapsedTime);
    
    // 使用统一的变量数值更新接口
    updateVariableValue("调试正弦波", debugValue);
}

void FOCChartManager::startDebugSineWave()
{
    if (m_debugSineWaveRunning) {
        log("调试正弦波已经在运行");
        return;
    }
    
    // 检查变量哈希表中是否存在"调试正弦波"变量
    if (!m_availableVariables.contains("调试正弦波")) {
        log("调试正弦波变量不存在，无法启动信号发生器");
        return;
    }
    
    // 检查采集状态是否打开
    if (!m_isCollecting) {
        log("采集状态未打开，无法启动调试正弦波信号发生器");
        return;
    }
    
    m_debugStartTime = QDateTime::currentMSecsSinceEpoch();
    m_debugSineWaveRunning = true;
    m_debugTimer->start();
    
    log("调试正弦波已启动 - 周期: 2秒, 幅度: 1000, 无偏置");
}

void FOCChartManager::stopDebugSineWave()
{
    if (!m_debugSineWaveRunning) {
        log("调试正弦波未在运行");
        return;
    }
    
    m_debugSineWaveRunning = false;
    m_debugTimer->stop();
    
    log("调试正弦波已停止");
}

void FOCChartManager::onReadDataReceived(uint8_t dataId, uint32_t dataValue)
{
    // 检查采集状态，只有在采集状态下才处理数据
    if (!m_isCollecting) {
        return;
    }
    
    // 根据数据ID映射到对应的变量名称
    QString variableName = getVariableNameFromDataId(dataId);
    
    // 如果变量名称无效，则忽略该数据
    if (variableName.isEmpty()) {
        log(QString("未知数据ID: %1, 值: %2").arg(dataId).arg(dataValue));
        return;
    }
    
    // 检查变量是否在可用变量列表中
    if (!m_availableVariables.contains(variableName)) {
        log(QString("数据ID %1 对应的变量 '%2' 不在可用变量列表中").arg(dataId).arg(variableName));
        return;
    }
    
    // 将32位无符号整数转换为double类型
    double convertedValue = static_cast<double>(dataValue);
    
    // 更新变量数值
    updateVariableValue(variableName, convertedValue);
    
    // 记录数据接收日志（避免过多输出，可以注释掉）
    // log(QString("接收到数据 - 变量: %1, ID: %2, 值: %3").arg(variableName).arg(dataId).arg(convertedValue));
}

QString FOCChartManager::getVariableNameFromDataId(uint8_t dataId)
{
    // 数据ID到变量名称的映射表（基于协议定义）
    switch (dataId) {
        // 相电流相关
        case 0x10: return "U相电流";  // DATA_ID_PHASE_CURRENT_U_TARGET
        case 0x11: return "U相电流";  // DATA_ID_PHASE_CURRENT_U_CURRENT
        case 0x12: return "V相电流";  // DATA_ID_PHASE_CURRENT_V_TARGET
        case 0x13: return "V相电流";  // DATA_ID_PHASE_CURRENT_V_CURRENT
        case 0x14: return "W相电流";  // DATA_ID_PHASE_CURRENT_W_TARGET
        case 0x15: return "W相电流";  // DATA_ID_PHASE_CURRENT_W_CURRENT
        
        // 转速相关
        case 0x16: return "转速";     // DATA_ID_SPEED_TARGET
        case 0x17: return "转速";     // DATA_ID_SPEED_CURRENT
        
        // Q轴电压相关
        case 0x18: return "Q轴电压";  // DATA_ID_Q_VOLTAGE_TARGET
        case 0x19: return "Q轴电压";  // DATA_ID_Q_VOLTAGE_CURRENT
        
        // D轴电压相关
        case 0x1A: return "D轴电压";  // DATA_ID_D_VOLTAGE_TARGET
        case 0x1B: return "D轴电压";  // DATA_ID_D_VOLTAGE_CURRENT
        
        // 系统参数
        case 0x1C: return "最大电流"; // DATA_ID_MAX_CURRENT_LIMIT
        case 0x1D: return "母线电压"; // DATA_ID_BUS_VOLTAGE
        case 0x1E: return "极对数";    // DATA_ID_POLE_PAIRS
        
        // 角度相关
        case 0x1F: return "电角度";   // DATA_ID_ELECTRICAL_ANGLE_TARGET
        case 0x20: return "电角度";   // DATA_ID_ELECTRICAL_ANGLE_CURRENT
        case 0x21: return "机械角";   // DATA_ID_MECHANICAL_ANGLE_TARGET
        case 0x22: return "机械角";   // DATA_ID_MECHANICAL_ANGLE_CURRENT
        
        // 模式状态
        case 0x23: return "控制模式"; // DATA_ID_CONTROL_MODE
        case 0x24: return "接口模式"; // DATA_ID_INTERFACE_MODE
        case 0x25: return "工作状态"; // DATA_ID_MOTOR_STATE
        
        // PID参数
        case 0x26: return "力矩Kp";   // DATA_ID_TORQUE_PID_KP
        case 0x27: return "力矩Ki";   // DATA_ID_TORQUE_PID_KI
        case 0x28: return "力矩Kd";   // DATA_ID_TORQUE_PID_KD
        case 0x29: return "速度Kp";   // DATA_ID_SPEED_PID_KP
        case 0x2A: return "速度Ki";   // DATA_ID_SPEED_PID_KI
        case 0x2B: return "速度Kd";   // DATA_ID_SPEED_PID_KD
        case 0x2C: return "位置Kp";   // DATA_ID_POSITION_PID_KP
        case 0x2D: return "位置Ki";   // DATA_ID_POSITION_PID_KI
        case 0x2E: return "位置Kd";   // DATA_ID_POSITION_PID_KD
        
        // 执行时间
        case 0x2F: return "电流环时"; // DATA_ID_TORQUE_LOOP_EXECUTION_TIME
        
        // 其他参数
        case 0x30: return "CAN ID";   // DATA_ID_CAN_ID
        case 0x31: return "机械零位"; // DATA_ID_MECHANICAL_ZERO_POSITION
        case 0x32: return "霍尔X偏";  // DATA_ID_HALL_X_DC_OFFSET
        case 0x33: return "霍尔Y偏";  // DATA_ID_HALL_Y_DC_OFFSET
        case 0x34: return "霍尔状态"; // DATA_ID_HALL_CALIBRATION_STATUS
        case 0x35: return "速度环时"; // DATA_ID_SPEED_LOOP_EXECUTION_TIME
        case 0x36: return "位置环时"; // DATA_ID_POSITION_LOOP_EXECUTION_TIME
        
        default: return QString(); // 未知数据ID返回空字符串
    }
}