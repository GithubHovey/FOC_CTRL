#ifndef SERIAL_COMMUNICATION_MANAGER_H
#define SERIAL_COMMUNICATION_MANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

class SerialCommunicationManager : public QObject
{
    Q_OBJECT
    
    // 暴露给QML的属性
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStateChanged)
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY availablePortsChanged)
    Q_PROPERTY(QStringList availablePortDetails READ availablePortDetails NOTIFY availablePortDetailsChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString displayData READ displayData NOTIFY displayDataChanged)
    Q_PROPERTY(bool showTx READ showTx WRITE setShowTx NOTIFY showTxChanged)
    Q_PROPERTY(bool showRx READ showRx WRITE setShowRx NOTIFY showRxChanged)
    Q_PROPERTY(bool hexDisplay READ hexDisplay WRITE setHexDisplay NOTIFY hexDisplayChanged)
    Q_PROPERTY(qint64 bytesReceived READ bytesReceived NOTIFY bytesReceivedChanged)
    Q_PROPERTY(qint64 bytesSent READ bytesSent NOTIFY bytesSentChanged)

public:
    explicit SerialCommunicationManager(QObject *parent = nullptr);
    ~SerialCommunicationManager();

    // Getter方法 - QML读取属性
    bool isConnected() const { return m_isConnected; }
    QStringList availablePorts() const { return m_availablePorts; }
    QStringList availablePortDetails() const { return m_availablePortDetails; }
    QString connectionStatus() const { return m_connectionStatus; }
    QString displayData() const { return m_displayData; }
    bool showTx() const { return m_showTx; }
    bool showRx() const { return m_showRx; }
    bool hexDisplay() const { return m_hexDisplay; }
    qint64 bytesReceived() const { return m_bytesReceived; }
    qint64 bytesSent() const { return m_bytesSent; }

public slots:
    // Setter方法 - QML设置属性
    void setShowTx(bool show) { 
        if (m_showTx != show) {
            m_showTx = show; 
            emit showTxChanged();
            updateDisplayData();
        }
    }
    
    void setShowRx(bool show) { 
        if (m_showRx != show) {
            m_showRx = show; 
            emit showRxChanged();
            updateDisplayData();
        }
    }
    
    void setHexDisplay(bool hex) { 
        if (m_hexDisplay != hex) {
            m_hexDisplay = hex; 
            emit hexDisplayChanged();
            updateDisplayData();
        }
    }
    
    // QML可调用的方法
    Q_INVOKABLE bool connectPort(const QString &portName, int baudRate);
    Q_INVOKABLE void disconnectPort();
    Q_INVOKABLE bool sendData(const QString &data);
    Q_INVOKABLE void clearData();
    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void resetByteCounters();

signals:
    // 属性变化通知
    void connectionStateChanged();
    void availablePortsChanged();
    void availablePortDetailsChanged();
    void connectionStatusChanged();
    void displayDataChanged();
    void showTxChanged();
    void showRxChanged();
    void hexDisplayChanged();
    void bytesReceivedChanged();
    void bytesSentChanged();
    
    // 数据更新信号
    void dataReceived(const QString &data, const QString &timestamp);
    void dataSent(const QString &data, const QString &timestamp);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void updateAvailablePorts();

private:
    QSerialPort *m_serialPort;
    bool m_isConnected;
    QString m_connectionStatus;
    QStringList m_availablePorts;
    QStringList m_availablePortDetails;
    QString m_displayData;
    
    // 显示设置
    bool m_showTx;
    bool m_showRx;
    bool m_hexDisplay;
    
    // 字节计数
    qint64 m_bytesReceived;
    qint64 m_bytesSent;
    
    // 数据存储
    struct DataItem {
        QString data;
        QString timestamp;
        bool isTx;
    };
    QList<DataItem> m_dataList;
    static const int MAX_LINES = 1000;
    
    // 内部方法
    void scanAvailablePorts();
    QString formatData(const QByteArray &data, bool isTx);
    QByteArray parseInputString(const QString &input);
    void appendToDataList(const QString &data, bool isTx);
    void updateDisplayData();
    QString byteArrayToHex(const QByteArray &data);
    
    // 定时器
    QTimer *m_updateTimer;
};

#endif // SERIAL_COMMUNICATION_MANAGER_H