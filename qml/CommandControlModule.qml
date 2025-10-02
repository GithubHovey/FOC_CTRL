import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    property alias calibrationEnabled: calibrationButton.enabled
    
    // 信号定义
    signal calibrationRequested()
    

    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // 标题
        Text {
            text: qsTr("命令控制")
            color: "#FFFFFF"
            font.pixelSize: 14
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }
        
        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#464647"
        }
        
        // 一键校准按钮
        Button {
            id: calibrationButton
            text: qsTr("一键校准")
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            
            background: Rectangle {
                color: calibrationButton.pressed ? "#1066CC" : (calibrationButton.hovered ? "#1177DD" : "#0E639C")
                radius: 3
                border.width: 1
                border.color: "#464647"
            }
            
            contentItem: Text {
                text: calibrationButton.text
                color: "#FFFFFF"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            onClicked: {
                console.log("一键校准按钮被点击")
                var result = CommandControlManager.performCalibration()
                console.log("校准结果:", result)
                if (result) {
                    console.log("校准命令发送成功")
                } else {
                    console.log("校准命令发送失败")
                }
                calibrationRequested()
            }
        }
        
        // 电机控制按钮组
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            Button {
                id: startButton
                text: qsTr("启动")
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                
                background: Rectangle {
                    color: startButton.pressed ? "#00AA00" : (startButton.hovered ? "#00BB00" : "#00AA00")
                    radius: 3
                    border.width: 1
                    border.color: "#464647"
                }
                
                contentItem: Text {
                    text: startButton.text
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    console.log("启动按钮被点击")
                    var result = CommandControlManager.startMotor()
                    console.log("启动结果:", result)
                }
            }
            
            Button {
                id: stopButton
                text: qsTr("停止")
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                
                background: Rectangle {
                    color: stopButton.pressed ? "#CC0000" : (stopButton.hovered ? "#DD0000" : "#CC0000")
                    radius: 3
                    border.width: 1
                    border.color: "#464647"
                }
                
                contentItem: Text {
                    text: stopButton.text
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    console.log("停止按钮被点击")
                    var result = CommandControlManager.stopMotor()
                    console.log("停止结果:", result)
                }
            }
        }
        
        // 系统控制按钮组
        RowLayout {
            Layout.fillWidth: true
            spacing: 5
            
            Button {
                id: clearErrorsButton
                text: qsTr("清错")
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                
                background: Rectangle {
                    color: clearErrorsButton.pressed ? "#FF8800" : (clearErrorsButton.hovered ? "#FF9900" : "#FF8800")
                    radius: 3
                    border.width: 1
                    border.color: "#464647"
                }
                
                contentItem: Text {
                    text: clearErrorsButton.text
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    console.log("清错按钮被点击")
                    var result = CommandControlManager.clearErrors()
                    console.log("清错结果:", result)
                }
            }
            
            Button {
                id: resetButton
                text: qsTr("复位")
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                
                background: Rectangle {
                    color: resetButton.pressed ? "#8800FF" : (resetButton.hovered ? "#9900FF" : "#8800FF")
                    radius: 3
                    border.width: 1
                    border.color: "#464647"
                }
                
                contentItem: Text {
                    text: resetButton.text
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    console.log("复位按钮被点击")
                    var result = CommandControlManager.resetSystem()
                    console.log("复位结果:", result)
                }
            }
        }
        
        // 预留空间，用于添加更多命令按钮
        Item {
            Layout.fillHeight: true
        }
    }
}