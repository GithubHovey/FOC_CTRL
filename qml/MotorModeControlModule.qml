import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: motorModeControlModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    // 使用FOC_CTRL模块中的MotorModeControlManager单例
    property var motorModeManager: MotorModeControlManager
    
    // 连接信号处理
    Connections {
        target: motorModeManager
        
        function onCurrentModeChanged() {
            // 模式改变时发送控制命令
            motorModeManager.sendControlCommand()
        }
        
        function onParameterValueChanged() {
            // 参数值改变时发送控制命令
            motorModeManager.sendControlCommand()
        }
        
        function onIsEnabledChanged() {
            // 使能状态改变时发送控制命令
            motorModeManager.sendControlCommand()
        }
        
        function onLogMessage(message) {
            // 日志消息处理（暂时不转发到日志模块）
            console.log("[电机模式控制] " + message)
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
                implicitWidth: 32
                implicitHeight: 32
                background: Rectangle {
                    color: motorModeManager.isEnabled ? "#3498DB" : "#3498DB"
                    radius: 5
                }
                contentItem: Item {
                    anchors.fill: parent
                    
                    // 播放图标（三角形）
                    Rectangle {
                        id: playIcon
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        visible: !motorModeManager.isEnabled
                        color: "transparent"
                        
                        Canvas {
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.fillStyle = "#FFFFFF"
                                ctx.beginPath()
                                ctx.moveTo(4, 4)
                                ctx.lineTo(12, 8)
                                ctx.lineTo(4, 12)
                                ctx.closePath()
                                ctx.fill()
                            }
                        }
                    }
                    
                    // 暂停图标（两条竖线）
                    Rectangle {
                        id: pauseIcon
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        visible: motorModeManager.isEnabled
                        color: "transparent"
                        
                        Canvas {
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d")
                                ctx.clearRect(0, 0, width, height)
                                ctx.fillStyle = "#FFFFFF"
                                // 第一条竖线
                                ctx.fillRect(4, 4, 2, 8)
                                // 第二条竖线
                                ctx.fillRect(10, 4, 2, 8)
                            }
                        }
                    }
                }
                onClicked: {
                    // 切换使能状态
                    motorModeManager.toggleEnable()
                    enableToggled()
                    // 强制重绘Canvas
                    playIcon.children[0].requestPaint()
                    pauseIcon.children[0].requestPaint()
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
                    text: getCurrentParameterValue().toFixed(1)
                    horizontalAlignment: TextInput.AlignHCenter
                    background: Rectangle {
                        color: "#3C3C3C"
                        border.width: 1
                        border.color: "#464647"
                    }
                    color: "#FFFFFF"
                    
                    // 根据当前模式获取对应的参数值
                    function getCurrentParameterValue() {
                        switch (motorModeManager.currentMode) {
                        case 0: // TORQUE_MODE
                            return motorModeManager.torqueParameter
                        case 1: // SPEED_MODE
                            return motorModeManager.speedParameter
                        case 2: // POSITION_MODE
                            return motorModeManager.positionParameter
                        default:
                            return motorModeManager.parameterValue
                        }
                    }
                    
                    onTextChanged: {
                        var value = Number(text)
                        if (!isNaN(value)) {
                            switch (motorModeManager.currentMode) {
                            case 0: // TORQUE_MODE
                                motorModeManager.setTorqueParameter(value)
                                break
                            case 1: // SPEED_MODE
                                motorModeManager.setSpeedParameter(value)
                                break
                            case 2: // POSITION_MODE
                                motorModeManager.setPositionParameter(value)
                                break
                            default:
                                motorModeManager.setParameterValue(value)
                            }
                            parameterChanged(value)
                        }
                    }
                    
                    // 当参数值改变时更新文本框显示
                    Connections {
                        target: motorModeManager
                        function onParameterValueChanged() {
                            parameterTextField.text = getCurrentParameterValue().toFixed(1)
                        }
                        
                        function onTorqueParameterChanged() {
                            if (motorModeManager.currentMode === 0) {
                                parameterTextField.text = motorModeManager.torqueParameter.toFixed(1)
                            }
                        }
                        
                        function onSpeedParameterChanged() {
                            if (motorModeManager.currentMode === 1) {
                                parameterTextField.text = motorModeManager.speedParameter.toFixed(1)
                            }
                        }
                        
                        function onPositionParameterChanged() {
                            if (motorModeManager.currentMode === 2) {
                                parameterTextField.text = motorModeManager.positionParameter.toFixed(1)
                            }
                        }
                        
                        function onCurrentModeChanged() {
                            parameterTextField.text = getCurrentParameterValue().toFixed(1)
                        }
                    }
                }
            }

            Slider {
                id: parameterSlider
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                from: 0
                to: 100
                value: getCurrentParameterValue()
                live: true
                
                // 根据当前模式获取对应的参数值
                function getCurrentParameterValue() {
                    switch (motorModeManager.currentMode) {
                    case 0: // TORQUE_MODE
                        return motorModeManager.torqueParameter
                    case 1: // SPEED_MODE
                        return motorModeManager.speedParameter
                    case 2: // POSITION_MODE
                        return motorModeManager.positionParameter
                    default:
                        return motorModeManager.parameterValue
                    }
                }
                
                background: Rectangle {
                    implicitHeight: 6
                    color: "#3C3C3C"
                    border.width: 1
                    border.color: "#464647"
                    radius: 3
                    
                    Rectangle {
                        width: parameterSlider.visualPosition * parent.width
                        height: parent.height
                        color: "#3498DB"
                        radius: 3
                    }
                }
                
                handle: Rectangle {
                    x: parameterSlider.visualPosition * (parameterSlider.width - width)
                    y: (parameterSlider.height - height) / 2
                    implicitWidth: 20
                    implicitHeight: 20
                    radius: 10
                    color: parameterSlider.pressed ? "#F1C40F" : "#FFFFFF"
                    border.color: "#464647"
                    border.width: 2
                }
                
                onValueChanged: {
                    // 实时更新文本框显示
                    parameterTextField.text = value.toFixed(1)
                }
                
                onPressedChanged: {
                    if (!pressed) {
                        // 只在滑块释放时更新对应模式的参数值
                        var paramValue = Number(value)
                        switch (motorModeManager.currentMode) {
                        case 0: // TORQUE_MODE
                            motorModeManager.setTorqueParameter(paramValue)
                            break
                        case 1: // SPEED_MODE
                            motorModeManager.setSpeedParameter(paramValue)
                            break
                        case 2: // POSITION_MODE
                            motorModeManager.setPositionParameter(paramValue)
                            break
                        default:
                            motorModeManager.setParameterValue(paramValue)
                        }
                        parameterChanged(paramValue)
                    }
                }
                
                // 当后端参数值改变时，同步更新滑块位置
                Connections {
                    target: motorModeManager
                    function onParameterValueChanged() {
                        parameterSlider.value = getCurrentParameterValue()
                    }
                    
                    function onTorqueParameterChanged() {
                        if (motorModeManager.currentMode === 0) {
                            parameterSlider.value = motorModeManager.torqueParameter
                        }
                    }
                    
                    function onSpeedParameterChanged() {
                        if (motorModeManager.currentMode === 1) {
                            parameterSlider.value = motorModeManager.speedParameter
                        }
                    }
                    
                    function onPositionParameterChanged() {
                        if (motorModeManager.currentMode === 2) {
                            parameterSlider.value = motorModeManager.positionParameter
                        }
                    }
                }
            }
        }
    }
}