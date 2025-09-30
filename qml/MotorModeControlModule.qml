import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: motorModeControlModule
    color: "#2D2D30"
    border.width: 1
    border.color: "#464647"
    radius: 5

    property alias controlMode: controlModeComboBox.currentText
    property alias parameterValue: parameterTextField.text
    property alias parameterSliderValue: parameterSlider.value
    property bool isEnabled: enableButton.text === "禁能"
    
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
                onClicked: {
                    if (text === "使能") {
                        text = "禁能"
                        motorModeControlModule.isEnabled = true
                    } else {
                        text = "使能"
                        motorModeControlModule.isEnabled = false
                    }
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
                    text: "50"
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
                            parameterSlider.value = value
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
                onValueChanged: {
                    parameterTextField.text = value.toString()
                    parameterChanged(value)
                }
            }
        }
    }
}