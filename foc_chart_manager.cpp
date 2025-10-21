#include "foc_chart_manager.h"
#include <QDebug>
#include <QRandomGenerator>

FOCChartManager::FOCChartManager(QObject *parent)
    : QObject(parent)
{
    // 初始化变量列表和颜色映射
    initializeAvailableVariables();
    initializeVariableColors();
    
    log("FOCChartManager initialized");
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
    
    m_selectedVariables.append(variableName);
    emit selectedVariablesChanged();
    
    log(QString("Variable '%1' added to chart").arg(variableName));
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
        "效率"
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