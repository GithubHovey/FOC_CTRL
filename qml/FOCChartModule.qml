import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts
import FOC_CTRL 1.0 as FOC

Rectangle {
    id: focChartModule
    color: "#1E1E1E"
    border.width: 1
    border.color: "#464647"

    property alias chartTitle: chartTitleText.text
    property bool isPaused: false
    
    // 从后端管理器获取变量列表
    property var variableList: []
    
    signal addVariableRequested()
    signal exportDataRequested()
    signal clearChartRequested()
    signal pauseDisplayRequested()
    
    // 添加变量函数 - 现在调用后端管理器
    function addVariable(varName) {
        // 调用后端管理器添加变量
        FOC.FOCChartManager.addVariable(varName);
        
        // 获取变量的颜色
        var varColor = FOC.FOCChartManager.getVariableColor(varName);
        
        // 检查是否已经存在同名的变量
        for (var i = 0; i < variableList.length; i++) {
            if (variableList[i].name === varName) {
                // 变量已存在，不重复添加
                return;
            }
        }
        
        // 创建新的曲线系列
        var newSeries = chartView.createSeries(ChartView.SeriesTypeLine, varName, axisX, axisY);
        newSeries.color = varColor;
        newSeries.width = 2;
        
        // 添加到本地变量列表
        variableList.push({
            name: varName,
            color: varColor,
            enabled: true,
            series: newSeries
        });
        
        // 更新变量列表显示
        updateVariableList();
    }
    
    // 更新变量列表显示
    function updateVariableList() {
        // 清空现有变量标签
        for (var i = variableListContainer.children.length - 1; i >= 0; i--) {
            if (variableListContainer.children[i].objectName === "variableTag") {
                variableListContainer.children[i].destroy();
            }
        }
        
        // 重新创建变量标签
        for (var i = 0; i < variableList.length; i++) {
            var tag = Qt.createQmlObject(`
                import QtQuick 2.15
                import QtQuick.Controls 2.15
                
                Rectangle {
                    property string varName: "${variableList[i].name}"
                    property string varColor: "${variableList[i].color}"
                    property bool enabled: ${variableList[i].enabled}
                    property int index: ${i}
                    objectName: "variableTag"
                    
                    width: 80
                    height: 30
                    radius: 4
                    color: varColor
                    
                    Text {
                        anchors.centerIn: parent
                        text: varName
                        color: "#FFFFFF"
                        font.pixelSize: 12
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            enabled = !enabled;
                            variableList[index].enabled = enabled;
                            variableList[index].series.visible = enabled;
                        }
                    }
                }
            `, variableListContainer, "variableTag");
        }
    }
    
    // 添加变量对话框显示状态
    property bool showAddVariableDialog: false
    
    // 清除变量对话框显示状态
    property bool showRemoveVariableDialog: false
    
    // 删除变量函数
    function removeVariable(varName) {
        // 调用后端管理器删除变量
        FOC.FOCChartManager.removeVariable(varName);
        
        // 从本地变量列表中移除
        for (var i = 0; i < variableList.length; i++) {
            if (variableList[i].name === varName) {
                // 从图表中移除对应的曲线系列
                chartView.removeSeries(variableList[i].series);
                variableList.splice(i, 1);
                break;
            }
        }
        
        // 更新变量列表显示
        updateVariableList();
    }
    
    // 初始化变量列表
    function initializeVariables() {
        // 添加默认的转速变量
        addVariable("转速");
    }
    
    Component.onCompleted: {
        // 初始化变量列表
        initializeVariables();
        // 初始化变量列表显示
        updateVariableList();
    }
    
    // 实际数据接收器 - 从C++后端获取转速数据
    property real timeCounter: 0
    
    // 连接到串口通信管理器的数据接收信号
    Connections {
        target: SerialCommManager
        
        function onCmdReadDataReceived(dataId, dataValue) {
            // 检查是否为当前转速数据 (DATA_ID_SPEED_CURRENT = 0x18)
            if (dataId === 0x18) {
                // 时间计数器递增
                timeCounter += 0.1
                
                // 将数据值转换为实际转速值 (单位: RPM)
                var speedValue = dataValue
                
                // 添加数据点到曲线
                series1.append(timeCounter, speedValue)
                
                // 限制数据点数量，避免内存溢出
                if (series1.count > 1000) {
                    series1.remove(0)
                }
                
                // 自动调整X轴范围
                if (timeCounter > axisX.max) {
                    axisX.max = timeCounter
                    axisX.min = Math.max(0, timeCounter - 100)
                }
            }
        }
    }

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

        // 变量列表区域
        Rectangle {
            Layout.preferredHeight: 40
            Layout.fillWidth: true
            color: "#2D2D30"
            border.width: 1
            border.color: "#464647"
            
            // 变量列表
            Flow {
                id: variableListContainer
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10
                
                // 动态生成的变量标签将在这里显示
            }
        }

        // 曲线显示区域
        Rectangle {
            Layout.preferredHeight: parent.height * 0.7
            Layout.fillWidth: true
            color: "#000000"
            border.width: 1
            border.color: "#464647"
            
            // 图表视图
            ChartView {
                id: chartView
                anchors.fill: parent
                antialiasing: true
                
                // X轴
                ValueAxis {
                    id: axisX
                    min: 0
                    max: 100
                    titleText: qsTr("时间 (秒)")
                }
                
                // Y轴
                ValueAxis {
                    id: axisY
                    min: -10
                    max: 10
                    titleText: qsTr("数值")
                }
                

            }
            
            // X轴滑块控制
            Slider {
                id: xAxisSlider
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    margins: 5
                }
                height: 20
                from: 0
                to: 100
                value: 0
                onValueChanged: {
                    // 控制X轴视窗移动
                    var range = axisX.max - axisX.min
                    var newMin = value / 100 * range
                    var newMax = newMin + range * 0.2 // 显示20%的数据范围
                    axisX.min = newMin
                    axisX.max = newMax
                }
                background: Rectangle {
                    color: "#3C3C3C"
                    radius: 2
                }
                handle: Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }
            
            // Y轴滑块控制
            Slider {
                id: yAxisSlider
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                    margins: 5
                }
                width: 20
                orientation: Qt.Vertical
                from: 0
                to: 100
                value: 50
                onValueChanged: {
                    // 控制Y轴视窗移动
                    var range = axisY.max - axisY.min
                    var center = axisY.min + range * (value / 100)
                    var halfRange = range * 0.5
                    axisY.min = center - halfRange
                    axisY.max = center + halfRange
                }
                background: Rectangle {
                    color: "#3C3C3C"
                    radius: 2
                }
                handle: Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }
            
            // 缩放控制按钮
            Row {
                anchors {
                    top: parent.top
                    right: yAxisSlider.left
                    margins: 5
                }
                spacing: 5
                
                Button {
                    text: "+"
                    width: 30
                    height: 30
                    onClicked: {
                        // X轴放大
                        var xRange = axisX.max - axisX.min
                        axisX.min += xRange * 0.1
                        axisX.max -= xRange * 0.1
                    }
                    background: Rectangle {
                        color: "#3C3C3C"
                        radius: 3
                    }
                }
                
                Button {
                    text: "-"
                    width: 30
                    height: 30
                    onClicked: {
                        // X轴缩小
                        var xRange = axisX.max - axisX.min
                        axisX.min -= xRange * 0.1
                        axisX.max += xRange * 0.1
                    }
                    background: Rectangle {
                        color: "#3C3C3C"
                        radius: 3
                    }
                }
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
                onClicked: showAddVariableDialog = true
            }

            Button {
                text: qsTr("清除变量")
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#3C3C3C"
                }
                contentItem: Text {
                    text: qsTr("清除变量")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: showRemoveVariableDialog = true
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
                onClicked: {
                    series1.clear()
                    dataTimer.timeCounter = 0
                    axisX.min = 0
                    axisX.max = 100
                    axisY.min = -10
                    axisY.max = 10
                    clearChartRequested()
                }
            }

            Button {
                id: pauseButton
                text: isPaused ? qsTr("继续显示") : qsTr("暂停显示")
                Layout.fillWidth: true
                background: Rectangle {
                    color: isPaused ? "#FF6B6B" : "#3C3C3C"
                }
                contentItem: Text {
                    text: isPaused ? qsTr("继续显示") : qsTr("暂停显示")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    isPaused = !isPaused
                    pauseDisplayRequested()
                }
            }
        }
        
        // 添加变量对话框
        Dialog {
            id: addVariableDialog
            title: qsTr("选择要添加的变量")
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            visible: showAddVariableDialog
            width: 400
            height: 300
            
            onAccepted: {
                // 获取选中的变量
                var selectedIndex = variableListView.currentIndex;
                if (selectedIndex >= 0) {
                    var selectedVariable = FOC.FOCChartManager.getAllAvailableVariables()[selectedIndex];
                    addVariable(selectedVariable);
                }
                showAddVariableDialog = false;
            }
            
            onRejected: {
                showAddVariableDialog = false;
            }
            
            contentItem: ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                Text {
                    text: qsTr("请选择要添加到图表的变量：")
                    color: "#FFFFFF"
                    font.pixelSize: 14
                }
                
                ListView {
                    id: variableListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: FOC.FOCChartManager.getAllAvailableVariables()
                    clip: true
                    
                    delegate: Rectangle {
                        width: parent.width
                        height: 40
                        color: ListView.isCurrentItem ? "#3C3C3C" : "transparent"
                        
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            text: modelData
                            color: "#FFFFFF"
                            font.pixelSize: 14
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                variableListView.currentIndex = index;
                            }
                        }
                    }
                    
                    highlight: Rectangle {
                        color: "#4ECDC4"
                        radius: 2
                    }
                }
            }
            
            background: Rectangle {
                color: "#2D2D30"
                border.width: 1
                border.color: "#464647"
            }
        }
        
        // 清除变量对话框
        Dialog {
            id: removeVariableDialog
            title: qsTr("选择要清除的变量")
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            visible: showRemoveVariableDialog
            width: 400
            height: 300
            
            onOpened: {
                console.log("清除变量对话框打开，当前选中的变量列表:", FOC.FOCChartManager.selectedVariables);
                console.log("变量数量:", FOC.FOCChartManager.selectedVariables.length);
            }
            
            onAccepted: {
                // 获取选中的变量
                var selectedIndex = removeVariableListView.currentIndex;
                if (selectedIndex >= 0) {
                    var selectedVariable = FOC.FOCChartManager.selectedVariables[selectedIndex];
                    console.log("准备删除变量:", selectedVariable);
                    removeVariable(selectedVariable);
                }
                showRemoveVariableDialog = false;
            }
            
            onRejected: {
                showRemoveVariableDialog = false;
            }
            
            contentItem: ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                Text {
                    text: qsTr("请选择要从图表中清除的变量：")
                    color: "#FFFFFF"
                    font.pixelSize: 14
                }
                
                ListView {
                    id: removeVariableListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: FOC.FOCChartManager.selectedVariables
                    clip: true
                    
                    delegate: Rectangle {
                        width: parent.width
                        height: 40
                        color: ListView.isCurrentItem ? "#3C3C3C" : "transparent"
                        
                        Row {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10
                            
                            Rectangle {
                                width: 20
                                height: 20
                                radius: 10
                                color: FOC.FOCChartManager.getVariableColor(modelData)
                            }
                            
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData
                                color: "#FFFFFF"
                                font.pixelSize: 14
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                removeVariableListView.currentIndex = index;
                            }
                        }
                    }
                    
                    highlight: Rectangle {
                        color: "#FF6B6B"
                        radius: 2
                    }
                }
            }
            
            background: Rectangle {
                color: "#2D2D30"
                border.width: 1
                border.color: "#464647"
            }
        }
    }
}