import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: focChartModule
    color: "#1E1E1E"
    border.width: 1
    border.color: "#464647"

    property alias chartTitle: chartTitleText.text
    
    signal addVariableRequested()
    signal exportDataRequested()
    signal clearChartRequested()
    signal pauseDisplayRequested()

    ColumnLayout {
        anchors.fill: parent
        spacing: 5
        anchors.margins: 10

        Text {
            id: chartTitleText
            text: qsTr("FOC数据曲线显示")
            color: "#FFFFFF"
            font.pixelSize: 16
            font.bold: true
        }

        // 曲线显示区域
        Rectangle {
            Layout.preferredHeight: parent.height * 0.8
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

        // 控制按钮区域
        RowLayout {
            Layout.preferredHeight: parent.height * 0.2
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: qsTr("添加变量")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("添加变量")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: addVariableRequested()
            }

            Button {
                text: qsTr("导出数据")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("导出数据")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: exportDataRequested()
            }

            Button {
                text: qsTr("清空曲线")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("清空曲线")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: clearChartRequested()
            }

            Button {
                text: qsTr("暂停显示")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("暂停显示")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: pauseDisplayRequested()
            }
        }
    }
}