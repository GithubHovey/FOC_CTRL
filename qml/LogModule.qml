import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: logModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"

    property alias debugChecked: debugCheckBox.checked
    property alias infoChecked: infoCheckBox.checked
    property alias warningChecked: warningCheckBox.checked
    property alias errorChecked: errorCheckBox.checked
    property alias logText: logContentText.text
    
    signal logLevelChanged()
    signal clearLogRequested()

    ColumnLayout {
        anchors.fill: parent
        spacing: 5
        anchors.margins: 10

        // 日志级别选择区域
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            CheckBox {
                id: debugCheckBox
                text: qsTr("Debug")
                checked: true
                background: Rectangle {
                    color: "transparent"
                }
                indicator: Rectangle {
                    implicitWidth: 20
                    implicitHeight: 20
                    border.width: 2
                    border.color: "#464647"
                    color: parent.checked ? "#3C3C3C" : "transparent"
                }
                contentItem: Text {
                    text: qsTr("Debug")
                    color: "#FFFFFF"
                    verticalAlignment: Text.AlignVCenter
                }
                onCheckedChanged: logLevelChanged()
            }

            CheckBox {
                id: infoCheckBox
                text: qsTr("Info")
                checked: true
                background: Rectangle {
                    color: "transparent"
                }
                indicator: Rectangle {
                    implicitWidth: 20
                    implicitHeight: 20
                    border.width: 2
                    border.color: "#464647"
                    color: parent.checked ? "#3C3C3C" : "transparent"
                }
                contentItem: Text {
                    text: qsTr("Info")
                    color: "#FFFFFF"
                    verticalAlignment: Text.AlignVCenter
                }
                onCheckedChanged: logLevelChanged()
            }

            CheckBox {
                id: warningCheckBox
                text: qsTr("Warning")
                checked: true
                background: Rectangle {
                    color: "transparent"
                }
                indicator: Rectangle {
                    implicitWidth: 20
                    implicitHeight: 20
                    border.width: 2
                    border.color: "#464647"
                    color: parent.checked ? "#3C3C3C" : "transparent"
                }
                contentItem: Text {
                    text: qsTr("Warning")
                    color: "#FFFFFF"
                    verticalAlignment: Text.AlignVCenter
                }
                onCheckedChanged: logLevelChanged()
            }

            CheckBox {
                id: errorCheckBox
                text: qsTr("Error")
                checked: true
                background: Rectangle {
                    color: "transparent"
                }
                indicator: Rectangle {
                    implicitWidth: 20
                    implicitHeight: 20
                    border.width: 2
                    border.color: "#464647"
                    color: parent.checked ? "#3C3C3C" : "transparent"
                }
                contentItem: Text {
                    text: qsTr("Error")
                    color: "#FFFFFF"
                    verticalAlignment: Text.AlignVCenter
                }
                onCheckedChanged: logLevelChanged()
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("清空日志")
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("清空日志")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: clearLogRequested()
            }
        }

        // 日志显示区域
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#1E1E1E"
            border.width: 1
            border.color: "#464647"
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: 5
                
                TextEdit {
                    id: logContentText
                    text: qsTr("日志显示区域")
                    color: "#CCCCCC"
                    readOnly: true
                    wrapMode: Text.WordWrap
                    selectByMouse: true
                }
            }
        }
    }
}