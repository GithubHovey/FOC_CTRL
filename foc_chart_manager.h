#ifndef FOC_CHART_MANAGER_H
#define FOC_CHART_MANAGER_H

#include <QObject>
#include <QVector>
#include <QColor>
#include <QDateTime>
#include <QMap>
#include <QHash>
#include <QStringList>

class FOCChartManager : public QObject
{
    Q_OBJECT
    
    // 可用的变量列表属性
    Q_PROPERTY(QStringList availableVariables READ availableVariables NOTIFY availableVariablesChanged)
    
    // 当前选中的变量列表属性
    Q_PROPERTY(QStringList selectedVariables READ selectedVariables NOTIFY selectedVariablesChanged)
    
    // 变量颜色映射属性
    Q_PROPERTY(QHash<QString, QColor> variableColors READ variableColors NOTIFY variableColorsChanged)

public:
    explicit FOCChartManager(QObject *parent = nullptr);
    
    // 获取可用变量列表
    QStringList availableVariables() const;
    
    // 获取选中的变量列表
    QStringList selectedVariables() const;
    
    // 获取变量颜色映射
    QHash<QString, QColor> variableColors() const;
    
    // 获取变量的颜色
    Q_INVOKABLE QColor getVariableColor(const QString &variableName) const;
    
    // 添加变量到选中列表
    Q_INVOKABLE void addVariable(const QString &variableName);
    
    // 从选中列表中移除变量
    Q_INVOKABLE void removeVariable(const QString &variableName);
    
    // 获取所有可用的变量名称
    Q_INVOKABLE QStringList getAllAvailableVariables() const;

signals:
    void availableVariablesChanged();
    void selectedVariablesChanged();
    void variableColorsChanged();
    void variableAdded(const QString &variableName, const QColor &color);
    void variableRemoved(const QString &variableName);
    void logMessage(const QString &message);

private:
    // 初始化可用变量列表
    void initializeAvailableVariables();
    
    // 初始化变量颜色映射
    void initializeVariableColors();
    
    // 生成随机颜色
    QColor generateRandomColor() const;
    
    // 记录日志
    void log(const QString &message);
    
    QStringList m_availableVariables;      // 所有可用的变量
    QStringList m_selectedVariables;       // 当前选中的变量
    QHash<QString, QColor> m_variableColors; // 变量颜色映射
};

#endif // FOC_CHART_MANAGER_H