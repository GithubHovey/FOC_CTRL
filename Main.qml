import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

// 导入自定义模块
import "qml"

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

                SerialCommunicationModule {
                    id: serialModule
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    onConnectClicked: {
                        console.log("串口连接状态:", serialModule.isConnected)
                        console.log("串口:", serialModule.comPort, "波特率:", serialModule.baudRate)
                    }
                    onSendDataRequested: function(data) {
                        console.log("发送数据:", data)
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

                    // FOC曲线显示模块 - 包含图表和控制按钮
                    FOCChartModule {
                        id: chartModule
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        
                        onAddVariableRequested: {
                            console.log("添加变量")
                        }
                        onExportDataRequested: {
                            console.log("导出数据")
                        }
                        onClearChartRequested: {
                            console.log("清空曲线")
                        }
                        onPauseDisplayRequested: {
                            console.log("暂停显示")
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
                    MotorModeControlModule {
                        id: motorModeControl
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.height * 0.33  // 占三分之一
                        
                        onEnableToggled: {
                            console.log("电机使能状态:", motorModeControl.isEnabled)
                        }
                        onParameterValueChanged: function(value) {
                            console.log("参数值改变:", value)
                        }
                    }

                    // 电机命令读写模块
                    MotorDataReadWriteModule {
                        id: motorDataRW
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.height * 0.33  // 占三分之一
                        
                        onReadDataRequested: {
                            console.log("读取数据 - ID:", motorDataRW.dataId, "类型:", motorDataRW.dataType)
                            motorDataRW.resultText = "读取成功: " + Math.random().toFixed(2)
                        }
                        onWriteDataRequested: {
                            console.log("写入数据 - ID:", motorDataRW.dataId, "类型:", motorDataRW.dataType, "值:", motorDataRW.dataValue)
                            motorDataRW.resultText = "写入成功"
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

            LogModule {
                id: logModule
                anchors.fill: parent
                anchors.margins: 10
                
                onLogLevelChanged: {
                    console.log("日志级别改变 - Debug:", logModule.debugChecked, "Info:", logModule.infoChecked, 
                               "Warning:", logModule.warningChecked, "Error:", logModule.errorChecked)
                }
                onClearLogRequested: {
                    logModule.logText = ""
                    console.log("清空日志")
                }
            }
        }

        standardButtons: Dialog.Close
    }
}
