import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FOC_CTRL 1.0 // 导入FOC_CTRL模块

Rectangle {
    id: serialCommunicationModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    // 格式化字节数的函数
    function formatBytes(bytes) {
        if (bytes === 0) return "0 B"
        if (bytes < 1024) return bytes + " B"
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB"
        return (bytes / (1024 * 1024)).toFixed(1) + " MB"
    }

    // 使用FOC_CTRL模块中的SerialCommManager单例
    property var serialManager: SerialCommManager

    // 连接信号处理
    Connections {
        target: SerialCommManager
        
        // 处理连接状态变化
        function onConnectionStateChanged() {
            connectButton.text = SerialCommManager.isConnected ? "断开" : "连接"
        }
        
        // 处理错误
        function onErrorOccurred(error) {
            console.error("串口错误:", error)
        }
        
        // 添加availablePortsChanged信号处理
        function onAvailablePortsChanged() {
            console.log("Available ports updated, new count:", SerialCommManager.availablePorts.length)
            console.log("Available ports list:", SerialCommManager.availablePorts)
            // 如果当前没有选中的端口，但有可用端口，则选中第一个
            if (comPortComboBox.currentIndex === -1 && SerialCommManager.availablePorts.length > 0) {
                comPortComboBox.currentIndex = 0
                console.log("Auto-selected first port:", comPortComboBox.currentText)
            }
        }
        
        // 组件完成时刷新端口列表
        Component.onCompleted: {
            console.log("SerialManager initialized, refreshing ports...")
            SerialCommManager.refreshPorts()
        }
    }


    property alias comPort: comPortComboBox.currentText
    property alias baudRate: baudRateComboBox.currentText
    property bool isConnected: SerialCommManager ? SerialCommManager.isConnected : false

    ColumnLayout {
        anchors.fill: parent
        spacing: 5
        anchors.margins: 10

        Text {
            text: qsTr("串口通信")
            color: "#FFFFFF"
            font.pixelSize: 16
            font.bold: true
        }

        // 串口配置区域
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width  // 确保不超过父容器宽度
            spacing: 5

            ComboBox {
                id: comPortComboBox
                model: SerialCommManager ? SerialCommManager.availablePortDetails : []
                currentIndex: model.length > 0 ? 0 : -1
                Layout.fillWidth: true
                Layout.maximumWidth: 350
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                contentItem: Text {
                    text: comPortComboBox.displayText
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onCurrentIndexChanged: {
                    // 提取端口号（详细信息的前缀部分）
                    var portName = currentText.split(" - ")[0];
                    console.log("Selected COM port:", portName)
                }
                Component.onCompleted: {
                    console.log("Initial port count:", model.length)
                    if (model.length === 0) {
                        console.log("No serial ports found initially")
                    }
                }
            }
            // 手动刷新按钮
            Button {
                text: qsTr("刷新端口")
                onClicked: {
                    if(SerialCommManager) SerialCommManager.refreshPorts();
                }
                Layout.alignment: Qt.AlignRight
                Layout.maximumWidth: 100
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("刷新端口")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
        
        // 波特率和连接按钮区域
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width  // 确保不超过父容器宽度
            spacing: 5

            // 波特率选择
            ComboBox {
                id: baudRateComboBox
                model: [9600, 19200, 38400, 57600, 115200, 230400]
                currentIndex: 4  // 默认115200
                width: 120
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                contentItem: Text {
                    text: baudRateComboBox.displayText
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
            
            // 连接按钮
            Button {
                id: connectButton
                text: SerialCommManager ? (SerialCommManager.isConnected ? "断开" : "连接") : "连接"
                Layout.maximumWidth: 120
                enabled: comPortComboBox.currentIndex >= 0
                background: Rectangle {
                    color: SerialCommManager ? (SerialCommManager.isConnected ? "#FF4444" : "#3C3C3C") : "#3C3C3C"
                }
                contentItem: Text {
                    text: SerialCommManager ? (SerialCommManager.isConnected ? "断开" : "连接") : "连接"
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (SerialCommManager) {
                        if (!SerialCommManager.isConnected) {
                            // 提取端口号（详细信息的前缀部分）
                        var portName = comPortComboBox.currentText.split(" - ")[0];
                        console.log("Connecting to port:", portName, "with baud rate:", baudRateComboBox.currentText)
                            var success = SerialCommManager.connectPort(portName, parseInt(baudRateComboBox.currentText))
                            console.log("Connection attempt result:", success)
                        } else {
                            console.log("Disconnecting from port")
                            SerialCommManager.disconnectPort()
                        }
                    }
                }
            }
            
            Item { Layout.fillWidth: true } // 弹簧
        }

        // 显示连接状态
        Text {
            text: SerialCommManager ? SerialCommManager.connectionStatus : "未连接"
            color: SerialCommManager ? (SerialCommManager.isConnected ? "#00FF00" : "#FF4444") : "#FF4444"
            font.pixelSize: 12
        }

        // 数据显示区域
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width  // 确保不超过父容器宽度
            color: "#1E1E1E"
            border.width: 1
            border.color: "#464647"
            
            TextArea {
                id: dataDisplayArea
                anchors.fill: parent
                anchors.margins: 5
                text: SerialCommManager ? SerialCommManager.displayData : ""
                readOnly: true
                font.family: "Consolas, Monaco, monospace"
                font.pixelSize: 12
                color: "#FFFFFF"
                background: Rectangle {
                    color: "transparent"
                }
                wrapMode: TextEdit.WrapAnywhere  // 允许在任意字符处换行
                // 自动滚动到底部
                onTextChanged: {
                    // 使用更可靠的滚动方式
                    dataDisplayArea.selectAll()
                    dataDisplayArea.cursorPosition = dataDisplayArea.length
                    dataDisplayArea.deselect()
                }
            }
        }

        // 字节计数显示（发送区域上方，右对齐）
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            spacing: 10
            
            Text {
                text: "接收: " + (serialManager ? formatBytes(serialManager.bytesReceived) : "0 B")
                color: "#00FF00"
                font.pixelSize: 12
            }
            
            Text {
                text: "发送: " + (serialManager ? formatBytes(serialManager.bytesSent) : "0 B")
                color: "#00BFFF"
                font.pixelSize: 12
            }
        }

        // 发送区域
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width  // 确保不超过父容器宽度
            spacing: 5

            // 显示连接状态提示
            Text {
                visible: serialManager ? !serialManager.isConnected : false
                text: "请先连接串口以启用发送功能"
                color: "#FF6B6B"
                font.pixelSize: 12
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
            }

            TextField {
                id: sendTextField
                visible: serialManager ? serialManager.isConnected : false
                Layout.fillWidth: true
                placeholderText: serialManager ? (serialManager.hexDisplay ? "输入十六进制数据，如: 48 65 6C 6C 6F" : "输入发送数据...") : "输入发送数据..."
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                color: "#FFFFFF"
                enabled: serialManager ? serialManager.isConnected : false
            }

            Button {
                visible: serialManager ? serialManager.isConnected : false
                text: "发送"
                width: 60
                enabled: serialManager ? (serialManager.isConnected && sendTextField.text !== "") : false
                background: Rectangle {
                    color: serialManager ? (serialManager.isConnected ? "#0E639C" : "#3C3C3C") : "#3C3C3C"
                }
                contentItem: Text {
                    text: "发送"
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (serialManager && sendTextField.text !== "") {
                        serialManager.sendData(sendTextField.text)
                    }
                }
            }
        }

        // 显示控制区域
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width  // 确保不超过父容器宽度
            spacing: 15
            
            CheckBox {
                text: "显示发送"
                checked: serialManager ? serialManager.showTx : true
                onCheckedChanged: if(serialManager) serialManager.showTx = checked
                Layout.preferredWidth: 100
                Layout.preferredHeight: 24

                // 修复文本与勾选框重叠问题
                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 20 // 为勾选框留出空间
                }

                indicator: Rectangle {
                    width: 16
                    height: 16
                    x: 0
                    y: (parent.height - height) / 2
                    border.width: 1
                    // parent.checked 中的 parent 指的是包含此indicator的CheckBox组件
                    // parent.checked 表示获取CheckBox的选中状态
                    border.color: (serialManager ? parent.checked : true) ? "#00BFFF" : "#CCCCCC"
                    color: (serialManager ? parent.checked : true) ? "#00BFFF" : "transparent"
                    radius: 2
                    
                    Text {
                        // 这里的parent指的是Rectangle(indicator)，它没有checked属性
                        // 我们需要通过parent.parent.checked来获取CheckBox的checked属性
                        visible: serialManager ? parent.parent.checked : true
                        text: "✓"
                        color: "#FFFFFF"
                        font.pixelSize: 10
                        anchors.centerIn: parent
                    }
                }
            }
            
            CheckBox {
                text: "显示接收"
                checked: serialManager ? serialManager.showRx : true
                onCheckedChanged: if(serialManager) serialManager.showRx = checked
                Layout.preferredWidth: 100
                Layout.preferredHeight: 24

                // 修复文本与勾选框重叠问题
                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 20 // 为勾选框留出空间
                }

                indicator: Rectangle {
                    width: 16
                    height: 16
                    x: 0
                    y: (parent.height - height) / 2
                    border.width: 1
                    border.color: (serialManager ? parent.checked : true) ? "#00BFFF" : "#CCCCCC"
                    color: (serialManager ? parent.checked : true) ? "#00BFFF" : "transparent"
                    radius: 2
                    
                    Text {
                        // 这里的parent指的是Rectangle(indicator)，它没有checked属性
                        // 我们需要通过parent.parent.checked来获取CheckBox的checked属性
                        visible: serialManager ? parent.parent.checked : true
                        text: "✓"
                        color: "#FFFFFF"
                        font.pixelSize: 10
                        anchors.centerIn: parent
                    }
                }
            }
            
            Item { Layout.fillWidth: true } // 弹簧
            
            Button {
                id: clearButton
                text: "清空"
                width: 50
                height: 24
                background: Rectangle {
                    color: clearButton.pressed ? "#1066CC" : (clearButton.hovered ? "#1177DD" : "#0E639C")
                    radius: 3
                    border.width: 1
                    border.color: "#464647"
                }
                contentItem: Text {
                    text: clearButton.text
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if(serialManager) {
                        serialManager.clearData()
                        serialManager.resetByteCounters()
                    }
                }
            }
    
        }
    }
}