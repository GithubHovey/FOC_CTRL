import QtQuick
import QtQuick.Controls // 添加Controls模块导入
import QtQuick.Layouts
import QtCharts

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1280
    height: 720
    title: qsTr("FOC控制器")
    color: "#1E1E1E"

    // 主布局容器 - 使用Column布局
    Column {
        anchors.fill: parent
        spacing: 0

        // 顶栏 - 固定高度
        Rectangle {
            id: topBar
            width: parent.width
            height: 40
            color: "#2D2D30"
            border.width: 1
            border.color: "#464647"

            RowLayout {
                anchors.fill: parent
                spacing: 10
                anchors.margins: 10

                Text {
                    text: qsTr("FOC控制上位机")
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                    Layout.alignment: Qt.AlignVCenter
                }

                // 顶栏中间留白
                Item {
                    Layout.fillWidth: true
                }

                // 日志系统图标
                Button {
                    id: logButton
                    width: 30
                    height: 30
                    background: Rectangle {
                        color: "#3C3C3C"
                        radius: 15
                    }
                    contentItem: Text {
                        text: "📋"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        logDialog.visible = !logDialog.visible;
                    }
                }
            }
        }

        // 主内容区域 - 使用Row布局，支持响应式缩放
        Row {
            width: parent.width
            height: parent.height - topBar.height
            spacing: 0

            // 左侧栏 - 串口通信模块，30%宽度
            Rectangle {
                id: leftBar
                width: parent.width * 0.30  // 30%宽度，响应式
                height: parent.height
                color: "#2D2D30"
                border.width: 1
                border.color: "#464647"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 5
                    anchors.margins: 10

                    // 串口通信模块
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#2D2D30"
                        border.width: 1
                        border.color: "#464647"
                        radius: 5

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
                                }
                            }
                        }
                    }
                }
            }

            // 中央栏 - 图表显示区域，55%宽度
            Rectangle {
                id: centerBar
                width: parent.width * 0.55  // 55%宽度，响应式
                height: parent.height
                color: "#2D2D30"
                border.width: 1
                border.color: "#464647"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0  // 移除间距以实现精确比例分割
                    anchors.margins: 10

                    // 上部分 - 曲线图显示区域 (80%)
                    Rectangle {
                        id: chartArea
                        Layout.preferredHeight: parent.height * 0.8
                        Layout.fillWidth: true
                        color: "#1E1E1E"
                        border.width: 1
                        border.color: "#464647"
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 5
                            anchors.margins: 10

                            Text {
                                text: qsTr("FOC数据曲线显示")
                                color: "#FFFFFF"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            // 曲线显示区域
                            Rectangle {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                color: "#000000"
                                border.width: 1
                                border.color: "#464647"
                                // 曲线显示区域
                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("曲线图将在这里显示")
                                    color: "#CCCCCC"
                                }
                            }
                        }
                    }

                    // 中间分隔条
                    Item {
                        Layout.preferredHeight: 5
                        Layout.fillWidth: true
                    }

                    // 下部分 - 变量添加和控制区域 (20%)
                    Rectangle {
                        id: variableControlArea
                        Layout.preferredHeight: parent.height * 0.2
                        Layout.fillWidth: true
                        color: "#2D2D30"
                        border.width: 1
                        border.color: "#464647"
                        radius: 3

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 5
                            anchors.margins: 10

                            Text {
                                text: qsTr("变量添加与控制")
                                color: "#FFFFFF"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            // 变量添加和控制按钮区域
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Button {
                                    text: qsTr("添加变量")
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("添加变量")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: qsTr("导出数据")
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("导出数据")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: qsTr("清空曲线")
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("清空曲线")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: qsTr("暂停显示")
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("暂停显示")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 右侧栏 - 15%宽度
            Rectangle {
                id: rightBar
                width: parent.width * 0.15  // 15%宽度，响应式
                height: parent.height
                color: "#2D2D30"
                border.width: 1
                border.color: "#464647"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 5
                    anchors.margins: 10

                    // 电机模式控制模块
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.height * 0.33  // 占三分之一
                        color: "#2D2D30"
                        border.width: 1
                        border.color: "#464647"
                        radius: 5

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
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                }

                                Button {
                                    id: enableButton
                                    text: qsTr("使能")
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("使能")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
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
                                        text: "50"
                                        horizontalAlignment: TextInput.AlignHCenter
                                        background: Rectangle {
                                            color: "#3C3C3C"
                                            border.width: 1
                                            border.color: "#464647"
                                        }
                                        color: "#FFFFFF"
                                    }
                                }

                                Slider {
                                    id: parameterSlider
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 100
                                    value: 50
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
                                }
                            }
                        }
                    }

                    // 电机命令读写模块
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.height * 0.33  // 占三分之一
                        color: "#2D2D30"
                        border.width: 1
                        border.color: "#464647"
                        radius: 5

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 5
                            anchors.margins: 10

                            Text {
                                text: qsTr("电机变量读写")
                                color: "#FFFFFF"
                                font.pixelSize: 16
                                font.bold: true
                            }

                            // 数据ID选择区域
                            ComboBox {
                                id: dataIdComboBox
                                Layout.fillWidth: true
                                model: ["U相电流目标值", "U相电流当前值", "V相电流目标值", "V相电流当前值", "W相电流目标值", "W相电流当前值", "转速目标值", "转速当前值"]
                                background: Rectangle {
                                    color: "#3C3C3C"
                                    border.width: 1
                                    border.color: "#464647"
                                }
                                contentItem: Text {
                                    text: dataIdComboBox.displayText
                                    color: "#FFFFFF"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }

                            // 数据类型选择区域
                            ComboBox {
                                id: dataTypeComboBox
                                Layout.fillWidth: true
                                model: ["float", "int", "short"]
                                currentIndex: 0
                                background: Rectangle {
                                    color: "#3C3C3C"
                                    border.width: 1
                                    border.color: "#464647"
                                }
                                contentItem: Text {
                                    text: dataTypeComboBox.displayText
                                    color: "#FFFFFF"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }

                            // 数值输入区域
                            TextField {
                                id: dataValueTextField
                                Layout.fillWidth: true
                                placeholderText: qsTr("输入数值...")
                                background: Rectangle {
                                    color: "#3C3C3C"
                                    border.width: 1
                                    border.color: "#464647"
                                }
                                color: "#FFFFFF"
                            }

                            // 操作按钮区域
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Button {
                                    text: qsTr("读取")
                                    Layout.fillWidth: true
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("读取")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: qsTr("写入")
                                    Layout.fillWidth: true
                                    background: Rectangle {
                                        color: "#3C3C3C"
                                    }
                                    contentItem: Text {
                                        text: qsTr("写入")
                                        color: "#FFFFFF"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }

                            // 结果显示区域
                            Text {
                                id: resultText
                                text: qsTr("操作结果将显示在这里")
                                color: "#CCCCCC"
                                Layout.fillWidth: true
                            }
                        }
                    }

                    // 预留区域 - 占三分之一
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true  // 填充剩余空间
                        color: "#2D2D30"
                        border.width: 1
                        border.color: "#464647"
                        radius: 5

                        // 空白区域，不添加任何内容
                    }
                }
            }
        }
    }

    // 日志对话框
    Dialog {
        id: logDialog
        visible: false
        title: qsTr("日志系统")
        width: parent.width * 0.6
        height: parent.height * 0.6
        modal: true
        closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside

        contentItem: Rectangle {
            color: "#2D2D30"

            ColumnLayout {
                anchors.fill: parent
                spacing: 5
                anchors.margins: 10

                // 日志级别选择区域
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    CheckBox {
                        text: qsTr("Debug")
                        checked: true
                        background: Rectangle {
                            color: "transparent"  // 修复：添加引号
                        }
                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            border.width: 2
                            border.color: "#464647"
                            color: parent.checked ? "#3C3C3C" : "transparent"  // 修复：使用parent.checked
                        }
                        contentItem: Text {
                            text: qsTr("Debug")
                            color: "#FFFFFF"
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Info")
                        checked: true
                        background: Rectangle {
                            color: "transparent"  // 修复：添加引号
                        }
                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            border.width: 2
                            border.color: "#464647"
                            color: parent.checked ? "#3C3C3C" : "transparent"  // 修复：使用parent.checked
                        }
                        contentItem: Text {
                            text: qsTr("Info")
                            color: "#FFFFFF"
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Warning")
                        checked: true
                        background: Rectangle {
                            color: "transparent"  // 修复：添加引号
                        }
                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            border.width: 2
                            border.color: "#464647"
                            color: parent.checked ? "#3C3C3C" : "transparent"  // 修复：使用parent.checked
                        }
                        contentItem: Text {
                            text: qsTr("Warning")
                            color: "#FFFFFF"
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    CheckBox {
                        text: qsTr("Error")
                        checked: true
                        background: Rectangle {
                            color: "transparent"  // 修复：添加引号
                        }
                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            border.width: 2
                            border.color: "#464647"
                            color: parent.checked ? "#3C3C3C" : "transparent"  // 修复：使用parent.checked
                        }
                        contentItem: Text {
                            text: qsTr("Error")
                            color: "#FFFFFF"
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                // 日志显示区域
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: "#1E1E1E"
                    border.width: 1
                    border.color: "#464647"
                    // 日志内容
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("日志显示区域")
                        color: "#CCCCCC"
                    }
                }
            }
        }

        standardButtons: Dialog.Close
    }
}
