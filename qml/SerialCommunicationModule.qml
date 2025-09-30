import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: serialCommunicationModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    property alias comPort: comPortComboBox.currentText
    property alias baudRate: baudRateComboBox.currentText
    property bool isConnected: connectButton.text === "断开"
    
    signal connectClicked()
    signal sendDataRequested(string data)

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
            spacing: 5

            ComboBox {
                id: comPortComboBox
                model: ["COM1", "COM2", "COM3", "COM4"]
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                contentItem: Text {
                    text: comPortComboBox.displayText
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            ComboBox {
                id: baudRateComboBox
                model: ["9600", "19200", "38400", "57600", "115200"]
                currentIndex: 4 // 默认115200
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
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            Button {
                id: connectButton
                text: qsTr("连接")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("连接")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (text === "连接") {
                        text = "断开"
                        serialCommunicationModule.isConnected = true
                    } else {
                        text = "连接"
                        serialCommunicationModule.isConnected = false
                    }
                    connectClicked()
                }
            }
        }

        // 数据显示区域
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#1E1E1E"
            border.width: 1
            border.color: "#464647"
            // 数据显示内容
            Text {
                anchors.centerIn: parent
                text: qsTr("串口数据显示区域")
                color: "#CCCCCC"
            }
        }

        // 发送区域
        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            TextField {
                id: sendTextField
                Layout.fillWidth: true
                placeholderText: qsTr("输入发送数据...")
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                color: "#FFFFFF"
            }

            Button {
                text: qsTr("发送")
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("发送")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (sendTextField.text !== "") {
                        sendDataRequested(sendTextField.text)
                        sendTextField.text = ""
                    }
                }
            }
        }
    }
}