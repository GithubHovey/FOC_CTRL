#include "serial_communication_manager.h"
#include <QtConcurrent>

SerialCommunicationManager::SerialCommunicationManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_isConnected(false)
    , m_connectionStatus("未连接")
    , m_showTx(true)
    , m_showRx(true)
    , m_hexDisplay(true)
    , m_bytesReceived(0)
    , m_bytesSent(0)
    , m_updateTimer(new QTimer(this))
    , m_stopCmdThread(false)
{
    // 连接串口信号
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunicationManager::onReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &SerialCommunicationManager::onErrorOccurred);
    
    // 设置定时器，每100ms更新一次字节计数显示
    connect(m_updateTimer, &QTimer::timeout, [this]() {
        emit bytesReceivedChanged();
        emit bytesSentChanged();
    });
    m_updateTimer->start(100); // 100ms更新一次
    
    // 启动命令处理线程
    QThreadPool::globalInstance()->start([this]() {
        this->processCmdQueue();
    });
    
    // 初始化协议接收环形缓冲区
    m_rxRingbuf = ringbuf_alloc(1024); // 创建1KB的接收缓冲区
    
    // 初始化可用端口列表
    scanAvailablePorts();
}

SerialCommunicationManager::~SerialCommunicationManager()
{
    // 停止命令处理线程
    m_stopCmdThread = true;
    m_cmdCondition.wakeAll(); // 唤醒等待的线程
    
    // 等待一段时间让线程安全退出
    QThread::msleep(100);
    
    // 释放协议接收环形缓冲区
    if (m_rxRingbuf) {
        ringbuf_free(m_rxRingbuf);
        m_rxRingbuf = nullptr;
    }
    
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
        
        // 唤醒命令处理线程（串口已连接）
        m_cmdCondition.wakeAll();
        
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
        
        // 唤醒命令处理线程（让线程检查连接状态并等待）
        m_cmdCondition.wakeAll();
        
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
        // 更新发送字节计数
        m_bytesSent += bytesWritten;
        
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
    m_bytesReceived = 0;
    m_bytesSent = 0;
    emit displayDataChanged();
    emit bytesReceivedChanged();
    emit bytesSentChanged();
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
        // 更新接收字节计数
        m_bytesReceived += data.size();
        
        // 将接收到的数据写入环形缓冲区
        if (m_rxRingbuf) {
            ringbuf_push(m_rxRingbuf, reinterpret_cast<const uint8_t*>(data.constData()), data.size());
            // 调用协议解析函数
            parseProtocol();
        }
        
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

void SerialCommunicationManager::resetByteCounters()
{
    m_bytesReceived = 0;
    m_bytesSent = 0;
    emit bytesReceivedChanged();
    emit bytesSentChanged();
}

void SerialCommunicationManager::processCmdQueue()
{
    qDebug() << "命令处理线程已启动";
    
    while (!m_stopCmdThread) {
        QByteArray cmd;
        
        // 从队列中获取命令
        {
            QMutexLocker locker(&m_cmdMutex);
            
            // 等待条件：有命令可用且串口已连接，或者需要停止线程
            while (!m_stopCmdThread && (m_cmdList.isEmpty() || !m_isConnected)) {
                qDebug() << "命令处理线程等待中... 队列大小:" << m_cmdList.size() 
                         << "连接状态:" << m_isConnected;
                m_cmdCondition.wait(&m_cmdMutex);
            }
            
            // 检查是否需要停止线程
            if (m_stopCmdThread) {
                break;
            }
            
            // 再次检查是否有命令可用（可能被其他线程消费）
            if (m_cmdList.isEmpty()) {
                continue;
            }
            
            cmd = m_cmdList.takeFirst();
        }
        
        // 只有在串口连接时才发送命令
        if (m_isConnected && !cmd.isEmpty()) {
            qint64 bytesWritten = m_serialPort->write(cmd);
            if (bytesWritten > 0) {
                // 更新发送字节计数
                m_bytesSent += bytesWritten;
                emit bytesSentChanged();
                
                QString formattedData = formatData(cmd, true);
                QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
                
                // 根据显示设置决定是否显示
                if (m_showTx) {
                    appendToDataList(formattedData, true);
                }
                
                emit dataSent(formattedData, timestamp);
                qDebug() << "命令已发送，长度：" << bytesWritten << "字节";
            } else {
                QString error = "发送命令失败: " + m_serialPort->errorString();
                emit errorOccurred(error);
                qDebug() << error;
            }
        } else {
            qDebug() << "串口未连接，命令发送失败";
        }
        
        // 发送间隔，避免过快发送
        QThread::msleep(10);
    }
    
    qDebug() << "命令处理线程已停止";
}

bool SerialCommunicationManager::pushCmd(const QByteArray &data, motor_command_t cmd)
{
    // 验证数据长度是否为10字节
    if (data.size() != 10) {
        qDebug() << "数据长度错误：期望10字节，实际" << data.size() << "字节";
        return false;
    }
    
    // 构建完整的14字节命令包
    QByteArray fullCmd;
    fullCmd.resize(PROTOCOL_LENGTH);
    
    // 添加包头
    fullCmd[0] = PROTOCOL_HEADER;
    
    // 添加命令字
    fullCmd[1] = static_cast<uint8_t>(cmd);
    
    // 添加10字节数据
    for (int i = 0; i < 10; i++) {
        fullCmd[2 + i] = static_cast<uint8_t>(data[i]);
    }
    
    // 计算校验和（从包头到数据区结束，不包括包尾和校验字节本身）
    uint16_t checksum = 0;
    for (int i = 0; i < 12; i++) { // 0-11字节（包头+命令字+10字节数据）
        checksum += static_cast<uint8_t>(fullCmd[i]);
    }
    fullCmd[12] = static_cast<uint8_t>(checksum & 0xFF); // 取低8位
    
    // 添加包尾
    fullCmd[13] = PROTOCOL_FOOTER;
    
    // 将命令添加到队列
    {
        QMutexLocker locker(&m_cmdMutex);
        m_cmdList.append(fullCmd);
        qDebug() << "命令已添加到队列，当前队列长度：" << m_cmdList.size() 
                 << "命令字: 0x" << QString::number(cmd, 16).toUpper();
    }
    
    // 唤醒命令处理线程
    m_cmdCondition.wakeOne();
    
    return true;
}

void SerialCommunicationManager::parseProtocol()
{
    // 协议解析方法
    // 创建一个临时ringbuf指针，指向接收缓冲区
    ringbuf_t *cmd_rb = m_rxRingbuf;
    
    uint8_t cmd_len = 0;
    
    // 查找协议包头
    while(1) {
        uint16_t rb_len = ringbuf_len(cmd_rb);
        
        if(rb_len < PROTOCOL_LENGTH) {
            return; // 数据长度不足1帧
        }
        
        if(ringbuf_get_at(cmd_rb, 0) == PROTOCOL_HEADER) {
            cmd_len = PROTOCOL_LENGTH; // 使用固定包长
            
            if(rb_len < cmd_len) {
                return; // 可能发生拆包，数据长度不足一帧
            }
            
            if (ringbuf_get_at(cmd_rb, cmd_len - 1) == PROTOCOL_FOOTER) {
                // 计算校验和（从包头到数据区结束，不包括包尾和校验字节）
                // ringbuf_checksum返回16位累加和，取低8位作为校验和
                uint16_t calc_checksum_full = ringbuf_checksum(cmd_rb, 0, cmd_len - 3);
                uint8_t calc_checksum = (uint8_t)(calc_checksum_full & 0xFF);
                uint8_t recv_checksum = ringbuf_get_at(cmd_rb, cmd_len - 2);
                
                if (calc_checksum == recv_checksum) {
                    break; // 校验通过，跳出循环
                } else {
                    ringbuf_remove(cmd_rb, 1); // 校验失败，丢弃一个字节
                    continue;
                }
            } else {
                ringbuf_remove(cmd_rb, 1); // 包尾不匹配，丢弃一个字节
                continue;
            }
        } else {
            ringbuf_remove(cmd_rb, 1); // 丢弃一个字节，重新开始解析
            continue;
        }
    }
    
    // 提取完整的数据包到临时缓冲区
    for (int i = 0; i < PROTOCOL_LENGTH; i++) {
        m_parseBuffer[i] = ringbuf_get_at(cmd_rb, i);
    }
    
    // 解析命令字并分发到不同模块
    uint8_t cmd = m_parseBuffer[1]; // 命令字在第2个字节
    
    switch (cmd) {
        case CMD_READ_DATA:
            {
                uint8_t dataId = m_parseBuffer[2]; // 数据ID
                uint32_t dataValue = 0;
                // 从第3个字节开始提取4字节数据值（小端模式）
                dataValue |= m_parseBuffer[3];
                dataValue |= (m_parseBuffer[4] << 8);
                dataValue |= (m_parseBuffer[5] << 16);
                dataValue |= (m_parseBuffer[6] << 24);
                emit cmdReadDataReceived(dataId, dataValue);
            }
            break;
            
        case CMD_WRITE_DATA:
            {
                uint8_t dataId = m_parseBuffer[2]; // 数据ID
                uint32_t dataValue = 0;
                // 从第3个字节开始提取4字节数据值（小端模式）
                dataValue |= m_parseBuffer[3];
                dataValue |= (m_parseBuffer[4] << 8);
                dataValue |= (m_parseBuffer[5] << 16);
                dataValue |= (m_parseBuffer[6] << 24);
                emit cmdWriteDataReceived(dataId, dataValue);
            }
            break;
            
        case CMD_MOTOR_START:
            {
                uint8_t status = m_parseBuffer[2]; // 状态
                uint8_t state = m_parseBuffer[3];  // 电机状态
                emit cmdMotorStartReceived(status, state);
            }
            break;
            
        case CMD_MOTOR_STOP:
            {
                uint8_t status = m_parseBuffer[2]; // 状态
                uint8_t state = m_parseBuffer[3];  // 电机状态
                emit cmdMotorStopReceived(status, state);
            }
            break;
            
        case CMD_MOTOR_CALIBRATE:
            {
                uint8_t status = m_parseBuffer[2]; // 状态
                uint8_t state = m_parseBuffer[3];  // 电机状态
                emit cmdMotorCalibrateReceived(status, state);
            }
            break;
            
        case CMD_MODE_SET:
            {
                uint8_t mode = m_parseBuffer[2]; // 模式
                emit cmdModeSetReceived(mode);
            }
            break;
            
        default:
            qDebug() << "收到未知命令字:" << QString("0x%1").arg(cmd, 2, 16, QChar('0'));
            break;
    }
    
    // 移除已处理的数据包
    ringbuf_remove(cmd_rb, PROTOCOL_LENGTH);
}