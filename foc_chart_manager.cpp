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