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

bool SerialCommunicationManager::sendData(const QString &data)
{
    if (!m_isConnected) {
        emit errorOccurred("串口未连接");
        return false;
    }
    
    QByteArray sendData = parseInputString(data);
    qint64 bytesWritten = m_serialPort->write(sendData);
    
    if (bytesWritten > 0) {
        QString formattedData = formatData(sendData, true);
        
        // *** 优化：根据显示设置决定是否将数据添加到缓冲区 ***
        // 只有当用户选择显示发送数据时，才将数据添加到内部缓冲区
        // 这样可以避免不必要的数据搬运，提高性能
        if (m_showTx) {
            appendToDataList(formattedData, true);
        }
        
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        emit dataSent(formattedData, timestamp);
        return true;
    } else {
        QString error = "发送数据失败: " + m_serialPort->errorString();
        emit errorOccurred(error);
        return false;
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
    // 从串口读取所有可用数据
    // 注意：readAll()会一次性读取缓冲区中的所有数据，可能导致多个数据包被合并
    QByteArray data = m_serialPort->readAll();
    
    // 防御性编程：确保确实读取到了数据
    if (!data.isEmpty()) {
        // 将原始二进制数据格式化为可显示的字符串
        // formatData()会处理非打印字符、根据m_hexDisplay设置进行HEX转换等
        QString formattedData = formatData(data, false);
        
        // *** 优化：根据显示设置决定是否将数据添加到缓冲区 ***
        // 只有当用户选择显示接收数据时，才将数据添加到内部缓冲区
        // 这样可以避免不必要的数据搬运，提高性能
        if (m_showRx) {
            // false表示这是接收的数据（而非发送的数据）
            appendToDataList(formattedData, false);
        }
        
        // 生成精确到毫秒的时间戳，用于日志记录
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        
        // 发射信号通知其他模块有新数据到达
        // 注意：目前这个信号没有连接到任何消费者，是预留接口
        // 其他模块（如FOC曲线打印、数据解析等）可以通过连接此信号来获取数据
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
    
    // 限制数据量，防止缓冲区无限增长
    if (m_dataList.size() > MAX_LINES) {
        m_dataList.removeFirst();  // 移除最老的数据
    }
    
    updateDisplayData();
}

void SerialCommunicationManager::updateDisplayData()
{
    QString display;
    for (const DataItem &item : m_dataList) {
        // *** 修改：移除过滤逻辑，因为数据添加阶段已经过滤 ***
        // 数据已经在添加阶段根据显示设置进行了过滤，这里直接显示即可
        
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