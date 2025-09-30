import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: motorDataReadWriteModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    property alias dataId: dataIdComboBox.currentText
    property alias dataType: dataTypeComboBox.currentText
    property alias dataValue: dataValueTextField.text
    property alias resultText: resultText.text
    
    signal readDataRequested()
    signal writeDataRequested()

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
                onClicked: readDataRequested()
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
                onClicked: writeDataRequested()
            }
        }

        // 结果显示区域
        Text {
            id: resultText
            text: qsTr("操作结果将显示在这里")
            color: "#CCCCCC"
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
    }
}