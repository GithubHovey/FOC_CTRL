import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: motorModeControlModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    // 后端管理器属性
    property alias motorModeManager: motorModeManager
    
    // 创建电机模式控制管理器
    MotorModeControlManager {
        id: motorModeManager
        
        onCurrentModeChanged: {
            // 模式改变时发送控制命令
            motorModeManager.sendControlCommand()
        }
        
        onParameterValueChanged: {
            // 参数值改变时发送控制命令
            motorModeManager.sendControlCommand()
        }
        
        onIsEnabledChanged: {
            // 使能状态改变时发送控制命令
            motorModeManager.sendControlCommand()
        }
        
        onLogMessage: {
            // 将日志消息转发到日志模块
            if (typeof logModule !== 'undefined') {
                logModule.addLogMessage(message)
            }
        }
    }
    
    // 前端属性别名
    property alias controlMode: controlModeComboBox.currentText
    property alias parameterValue: parameterTextField.text
    property bool isEnabled: motorModeManager.isEnabled
    
    signal enableToggled()
    signal parameterChanged(real value)

    ColumnLayout {
        anchors.fill: parent
        spacing: 5
        anchors.margins: 10

        Text {
            text: qsTr("电机模式控制")
            color: "#FFFFFF"
            font.pixelSize: 16
            font.bold: true
        }

        // 模式选择区域
        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            ComboBox {
                id: controlModeComboBox
                model: ["力矩模式", "速度模式", "位置模式"]
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                contentItem: Text {
                    text: controlModeComboBox.displayText
                    color: '#e7d8d8'
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                onActivated: {
                    // 将选择的模式设置到后端管理器
                    motorModeManager.setModeFromString(currentText)
                }
                Component.onCompleted: {
                    // 初始化时设置默认模式
                    motorModeManager.setModeFromString(currentText)
                }
            }

            Button {
                id: enableButton
                text: motorModeManager.isEnabled ? qsTr("禁能") : qsTr("使能")
                background: Rectangle {
                    color: motorModeManager.isEnabled ? "#E74C3C" : "#3C3C3C"
                }
                contentItem: Text {
                    text: motorModeManager.isEnabled ? qsTr("禁能") : qsTr("使能")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    // 切换使能状态
                    motorModeManager.toggleEnable()
                    enableToggled()
                }
            }
        }

        // 参数调节区域
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: qsTr("参数值：")
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignVCenter
                }

                TextField {
                    id: parameterTextField
                    width: 60
                    text: motorModeManager.parameterValue.toFixed(1)
                    horizontalAlignment: TextInput.AlignHCenter
                    background: Rectangle {
                        color: "#3C3C3C"
                        border.width: 1
                        border.color: "#464647"
                    }
                    color: "#FFFFFF"
                    onTextChanged: {
                        var value = parseFloat(text)
                        if (!isNaN(value)) {
                            motorModeManager.setParameterValue(value)
                            parameterChanged(value)
                        }
                    }
                }
            }

            Slider {
                id: parameterSlider
                Layout.fillWidth: true
                from: 0
                to: 100
                value: motorModeManager.parameterValue
                background: Rectangle {
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                }
                handle: Rectangle {
                    color: "#FFFFFF"
                    width: 16
                    height: 16
                    radius: 8
                }
                onValueChanged: {
                    motorModeManager.setParameterValue(value)
                    parameterTextField.text = value.toFixed(1)
                    parameterChanged(value)
                }
            }
        }
    }
}