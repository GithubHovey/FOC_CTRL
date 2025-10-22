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
        
        log(QString("View state updated - X: [%1, %2], Y: [%3, %4], Zoom: %5")
            .arg(xMin).arg(xMax).arg(yMin).arg(yMax).arg(zoom));
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
    // 初始化所有可用的FOC系统变量
    m_availableVariables = QStringList{
        "转速",
        "Q轴电流",
        "D轴电流",
        "位置",
        "速度",
        "转矩",
        "电压",
        "温度",
        "功率",
        "效率",
        "调试正弦波"  // 新增调试变量
    };
    
    log(QString("Available variables initialized: %1 variables").arg(m_availableVariables.size()));
}

void FOCChartManager::initializeVariableColors()
{
    // 为常用变量设置预定义颜色
    m_variableColors["转速"] = QColor("#FF6B6B");     // 红色
    m_variableColors["Q轴电流"] = QColor("#4ECDC4");  // 青色
    m_variableColors["D轴电流"] = QColor("#45B7D1");  // 蓝色
    m_variableColors["位置"] = QColor("#96CEB4");     // 绿色
    m_variableColors["速度"] = QColor("#FECA57");     // 黄色
    m_variableColors["转矩"] = QColor("#FF9FF3");     // 粉色
    m_variableColors["电压"] = QColor("#54A0FF");     // 蓝色
    m_variableColors["温度"] = QColor("#FF9F43");     // 橙色
    m_variableColors["功率"] = QColor("#5F27CD");     // 紫色
    m_variableColors["效率"] = QColor("#00D2D3");     // 青色
    m_variableColors["调试正弦波"] = QColor("#FFA500"); // 橙色，用于调试变量
    
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
    
    // 记录调试信息
    log(QString("变量 '%1' 数值已更新: %2").arg(variableName).arg(value));
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