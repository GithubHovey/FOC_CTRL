#ifndef FOC_CHART_MANAGER_H
#define FOC_CHART_MANAGER_H

#include <QObject>
#include <QVector>
#include <QColor>
#include <QDateTime>
#include <QMap>
#include <QHash>
#include <QStringList>
#include <QRectF>
#include <QTimer>
#include <QThread>
#include <cmath>

class FOCChartManager : public QObject
{
    Q_OBJECT
    
    // 视图状态结构体
    struct ViewState {
            QRectF viewRange;      // 当前视窗范围 (x: min, y: min, width: range, height: range)
            double zoomFactor;      // 当前缩放倍数
            double scrollPosition; // X轴滚动条位置 (0.0 - 1.0)
            double dataLengthMs;   // 当前数据所占的X轴长度（毫秒）
            
            ViewState() : viewRange(0, -50000, 20000, 100000), zoomFactor(1.0), scrollPosition(0.0), dataLengthMs(20000.0) {}
        };
    
    // 可用的变量列表属性
    Q_PROPERTY(QStringList availableVariables READ availableVariables NOTIFY availableVariablesChanged)
    
    // 当前选中的变量列表属性
    Q_PROPERTY(QStringList selectedVariables READ selectedVariables NOTIFY selectedVariablesChanged)
    
    // 变量颜色映射属性
    Q_PROPERTY(QHash<QString, QColor> variableColors READ variableColors NOTIFY variableColorsChanged)
    
    // 视图状态属性
    Q_PROPERTY(double scrollPosition READ scrollPosition WRITE setScrollPosition NOTIFY scrollPositionChanged)
    Q_PROPERTY(double zoomFactor READ zoomFactor NOTIFY zoomFactorChanged)
    Q_PROPERTY(QRectF viewRange READ viewRange NOTIFY viewRangeChanged)
    Q_PROPERTY(double dataLengthMs READ dataLengthMs WRITE setDataLengthMs NOTIFY dataLengthMsChanged)
    
    // 采集状态属性
    Q_PROPERTY(bool isCollecting READ isCollecting WRITE setIsCollecting NOTIFY isCollectingChanged)
    


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
    
    // 视图状态相关方法
    double scrollPosition() const;
    void setScrollPosition(double position);
    double zoomFactor() const;
    QRectF viewRange() const;
    
    // 更新视图状态
    Q_INVOKABLE void updateViewState(double xMin, double xMax, double yMin, double yMax, double zoom);
    
    // 数据长度相关方法
    double dataLengthMs() const;
    void setDataLengthMs(double lengthMs);
    
    // 采集状态相关方法
    bool isCollecting() const;
    void setIsCollecting(bool collecting);
    Q_INVOKABLE void toggleCollection();
    
    // 变量数值更新方法
    Q_INVOKABLE void updateVariableValue(const QString &variableName, double value);
    
    // 获取变量当前值
    Q_INVOKABLE double getVariableValue(const QString &variableName) const;
    
    // 调试变量信号发生器控制方法
    Q_INVOKABLE void startDebugSineWave();
    Q_INVOKABLE void stopDebugSineWave();
    
    // 串口数据接收处理槽函数
    Q_INVOKABLE void onReadDataReceived(uint8_t dataId, uint32_t dataValue);

signals:
    void availableVariablesChanged();
    void selectedVariablesChanged();
    void variableColorsChanged();
    void variableAdded(const QString &variableName, const QColor &color);
    void variableRemoved(const QString &variableName);
    void logMessage(const QString &message);
    void scrollPositionChanged();
    void zoomFactorChanged();
    void viewRangeChanged();
    void dataLengthMsChanged();
    void isCollectingChanged();
    void variableValueChanged(const QString &variableName, double value);

private:
    // 初始化可用变量列表
    void initializeAvailableVariables();
    
    // 初始化变量颜色映射
    void initializeVariableColors();
    
    // 生成随机颜色
    QColor generateRandomColor() const;
    
    // 记录日志
    void log(const QString &message);
    
    // 调试变量相关方法
    void updateDebugSineValue();
    
    // 数据ID到变量名称映射方法
    QString getVariableNameFromDataId(uint8_t dataId);
    
    QStringList m_availableVariables;      // 所有可用的变量
    QStringList m_selectedVariables;       // 当前选中的变量
    QHash<QString, QColor> m_variableColors; // 变量颜色映射
    ViewState m_viewState;                  // 视图状态
    bool m_isCollecting;                    // 采集状态
    
    // 变量数值存储
    QHash<QString, double> m_variableValues;
    
    // 调试变量相关成员
    QTimer* m_debugTimer;                  // 调试定时器
    qint64 m_debugStartTime;               // 调试开始时间
    bool m_debugSineWaveRunning;           // 调试正弦波是否运行
};

#endif // FOC_CHART_MANAGER_H