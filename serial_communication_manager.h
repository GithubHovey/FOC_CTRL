#ifndef SERIAL_COMMUNICATION_MANAGER_H
#define SERIAL_COMMUNICATION_MANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// 包含电机协议头文件和环形缓冲区
#include "ringbuf.h"
extern "C" {
#include "DOC/motor_protocol.h"
}

class SerialCommunicationManager : public QObject
{
    Q_OBJECT
    
public:
    // 获取单例实例
    static SerialCommunicationManager* getInstance() {
        static SerialCommunicationManager instance;
        return &instance;
    }
    
    // 删除拷贝构造函数和赋值运算符
    SerialCommunicationManager(const SerialCommunicationManager&) = delete;
    SerialCommunicationManager& operator=(const SerialCommunicationManager&) = delete;
    
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
private:
    // 构造函数私有化（单例模式）
    explicit SerialCommunicationManager(QObject *parent = nullptr);
    
public:
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
    Q_INVOKABLE bool pushCmd(const QByteArray &data, motor_command_t cmd); // 推送命令到队列，自动添加包头包尾和校验和

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
    
    // 协议命令分发信号
    void cmdReadDataReceived(uint8_t dataId, uint32_t dataValue);
    void cmdWriteDataReceived(uint8_t dataId, uint32_t dataValue);
    void cmdMotorStartReceived(uint8_t status, uint8_t state);
    void cmdMotorStopReceived(uint8_t status, uint8_t state);
    void cmdMotorCalibrateReceived(uint8_t status, uint8_t state);
    void cmdModeSetReceived(uint8_t mode);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void updateAvailablePorts();
    void processCmdQueue(); // 处理命令队列
    void parseProtocol(); // 协议解包函数

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
    
    // 命令队列 - 每条命令14字节
    QList<QByteArray> m_cmdList;
    QMutex m_cmdMutex; // 命令队列互斥锁
    QWaitCondition m_cmdCondition; // 命令队列条件变量
    bool m_stopCmdThread; // 停止命令线程标志
    
    // 协议接收缓冲区
    ringbuf_t *m_rxRingbuf; // 接收环形缓冲区
    uint8_t m_parseBuffer[PROTOCOL_LENGTH]; // 协议解析临时缓冲区
    
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