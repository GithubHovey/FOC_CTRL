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
                calibrationRequested()
            }
        }
        
        // 预留空间，用于添加更多命令按钮
        Item {
            Layout.fillHeight: true
        }
    }
}