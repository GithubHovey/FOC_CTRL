#include "serial_communication_manager.h"

SerialCommunicationManager::SerialCommunicationManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_isConnected(false)
    , m_connectionStatus("未连接")
    , m_showTx(true)
    , m_showRx(true)
    , m_hexDisplay(true)
{
    // 连接串口信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunicationManager::onReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &SerialCommunicationManager::onErrorOccurred);
    
    // 初始化可用端口列表
    scanAvailablePorts();
}

SerialCommunicationManager::~SerialCommunicationManager()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool SerialCommunicationManager::connectPort(const QString &portName, int baudRate)
{
    if (m_isConnected) {
        disconnectPort();
    }
    
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_isConnected = true;
        m_connectionStatus = QString("已连接到 %1 (%2)").arg(portName).arg(baudRate);
        emit connectionStateChanged();
        emit connectionStatusChanged();
        
        // 不再显示连接成功消息到数据区域
        // appendToDataList(QString("[系统] 串口连接成功: %1 @ %2").arg(portName).arg(baudRate), false);
        return true;
    } else {
        QString error = m_serialPort->errorString();
        m_connectionStatus = QString("连接失败: %1").arg(error);
        emit connectionStatusChanged();
        emit errorOccurred(error);
        return false;
    }
}

void SerialCommunicationManager::disconnectPort()
{
    if (m_serialPort->isOpen()) {
        QString portName = m_serialPort->portName();
        m_serialPort->close();
        
        m_isConnected = false;
        m_connectionStatus = "未连接";
        emit connectionStateChanged();
        emit connectionStatusChanged();
        
        // 不再显示断开连接消息到数据区域
        // appendToDataList(QString("[系统] 串口已断开: %1").arg(portName), false);
    }
}

void SerialCommunicationManager::sendData(const QString &data)
{
    if (!m_isConnected) {
        emit errorOccurred("串口未连接");
        return;
    }
    
    QByteArray sendData = parseInputString(data);
    qint64 bytesWritten = m_serialPort->write(sendData);
    
    if (bytesWritten > 0) {
        QString formattedData = formatData(sendData, true);
        appendToDataList(formattedData, true);
        
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        emit dataSent(formattedData, timestamp);
    } else {
        QString error = "发送数据失败: " + m_serialPort->errorString();
        emit errorOccurred(error);
    }
}

void SerialCommunicationManager::clearData()
{
    m_dataList.clear();
    m_displayData.clear();
    emit displayDataChanged();
}

void SerialCommunicationManager::refreshPorts()
{
    scanAvailablePorts();
}

void SerialCommunicationManager::onReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        QString formattedData = formatData(data, false);
        appendToDataList(formattedData, false);
        
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        emit dataReceived(formattedData, timestamp);
    }
}

void SerialCommunicationManager::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        QString errorString = m_serialPort->errorString();
        m_connectionStatus = QString("错误: %1").arg(errorString);
        emit connectionStatusChanged();
        emit errorOccurred(errorString);
    }
}

void SerialCommunicationManager::scanAvailablePorts()
{
    m_availablePorts.clear();
    m_availablePortDetails.clear();
    
    // 添加调试信息，查看QSerialPortInfo::availablePorts()是否返回结果
    const auto ports = QSerialPortInfo::availablePorts();
    qDebug() << "Total ports found by QSerialPortInfo::availablePorts():" << ports.count();
    
    if (ports.isEmpty()) {
        qDebug() << "No serial ports available. Possible reasons:"
                 << "1. No physical serial ports connected"
                 << "2. No virtual COM ports installed"
                 << "3. Permission issues (try running as administrator)"
                 << "4. Qt SerialPort module may not be properly configured";
    }
    
    for (const QSerialPortInfo &info : ports) {
        qDebug() << "Found port: " << info.portName()
                 << "Description: " << info.description()
                 << "Manufacturer: " << info.manufacturer()
                 << "Vendor ID: " << info.vendorIdentifier()
                 << "Product ID: " << info.productIdentifier();
        
        // 基础端口名称
        m_availablePorts.append(info.portName());
        
        // 构建详细信息，包含驱动名称
        QString detail = info.portName();
        if (!info.description().isEmpty()) {
            detail += " - " + info.description();
        }
        if (!info.manufacturer().isEmpty()) {
            detail += " (" + info.manufacturer() + ")";
        }
        m_availablePortDetails.append(detail);
    }
    
    emit availablePortsChanged();
    emit availablePortDetailsChanged();
}

QString SerialCommunicationManager::formatData(const QByteArray &data, bool isTx)
{
    QString result;
    if (m_hexDisplay) {
        result = byteArrayToHex(data);
    } else {
        result = QString::fromLatin1(data);
        // 处理非打印字符
        result.replace('\r', "\\r");
        result.replace('\n', "\\n");
        result.replace('\t', "\\t");
    }
    return result;
}

QByteArray SerialCommunicationManager::parseInputString(const QString &input)
{
    if (m_hexDisplay) {
        // HEX模式：解析十六进制字符串
        QString hexString = input.simplified().replace(" ", "");
        return QByteArray::fromHex(hexString.toLatin1());
    } else {
        // 普通模式：直接发送
        return input.toLatin1();
    }
}

QString SerialCommunicationManager::byteArrayToHex(const QByteArray &data)
{
    QString hex;
    for (int i = 0; i < data.size(); ++i) {
        hex.append(QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0')).toUpper());
    }
    return hex.trimmed();
}

void SerialCommunicationManager::appendToDataList(const QString &data, bool isTx)
{
    DataItem item;
    item.data = data;
    item.timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    item.isTx = isTx;
    
    m_dataList.append(item);
    
    // 限制数据量
    if (m_dataList.size() > MAX_LINES) {
        m_dataList.removeFirst();
    }
    
    updateDisplayData();
}

void SerialCommunicationManager::updateDisplayData()
{
    QString display;
    for (const DataItem &item : m_dataList) {
        // 根据显示设置过滤数据
        if ((item.isTx && !m_showTx) || (!item.isTx && !m_showRx)) {
            continue;
        }
        
        QString prefix = item.isTx ? "[发送] " : "[接收] ";
        QString line = QString("%1 %2: %3\n")
                          .arg(item.timestamp)
                          .arg(prefix)
                          .arg(item.data);
        display.append(line);
    }
    
    if (m_displayData != display) {
        m_displayData = display;
        emit displayDataChanged();
    }
}

void SerialCommunicationManager::updateAvailablePorts()
{
    // 调用现有的scanAvailablePorts()方法来实现相同功能
    scanAvailablePorts();
}