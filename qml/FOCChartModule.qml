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
    property bool isCollecting: false
    
    // 从后端管理器获取变量列表
    property var variableList: []
    
    signal addVariableRequested()
    signal exportDataRequested()
    signal clearChartRequested()
    signal toggleCollectionRequested()
    
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
        console.log("创建曲线系列:", varName, "成功:", newSeries !== null);
        newSeries.color = varColor;
        newSeries.width = 2;
        
        // 添加到本地变量列表
        variableList.push({
            name: varName,
            color: varColor,
            enabled: true,
            series: newSeries
        });
        
        console.log("添加变量后变量列表长度:", variableList.length);
        
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
                            // 移除可见性切换功能，保持变量标签为纯显示用途
                            console.log("变量标签点击: ", varName, " (可见性功能已禁用)");
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
        console.log("开始初始化变量列表...");
        // 添加默认的转速变量
        addVariable("转速");
        // 添加调试正弦波变量
        addVariable("调试正弦波");
        console.log("变量列表初始化完成，当前变量数量:", variableList.length);
    }
    
    Component.onCompleted: {
        // 初始化变量列表
        initializeVariables();
        // 初始化变量列表显示
        updateVariableList();
        
        // 连接后端采集状态变化信号
        FOC.FOCChartManager.isCollectingChanged.connect(function() {
            // 同步前端采集状态
            isCollecting = FOC.FOCChartManager.isCollecting
        })
    }
    
    // 实际数据接收器 - 从C++后端获取转速数据
    property real timeCounter: 0
    
    // 主动读取定时器 - 10ms读取一次变量值
    Timer {
        id: variableReadTimer
        interval: 10  // 10ms读取一次
        running: isCollecting  // 只在采集状态下运行
        repeat: true
        onTriggered: {
            // 为每个变量读取当前值并更新曲线
            for (var i = 0; i < variableList.length; i++) {
                var variableName = variableList[i].name
                var currentValue = FOC.FOCChartManager.getVariableValue(variableName)
                
                // 时间计数器递增（以毫秒为单位，与X轴一致）
                timeCounter += 10  // 10ms，对应100Hz的更新频率
                
                // 处理数据，添加到对应的曲线系列
                variableList[i].series.append(timeCounter, currentValue)
                
                // 限制数据点数量，避免内存溢出（保留20000ms的数据）
                if (variableList[i].series.count > 2000) {  // 20000ms / 10ms = 2000个点
                    variableList[i].series.remove(0)
                }
                
                // 自动调整X轴范围（以毫秒为单位）
                if (timeCounter > axisX.max) {
                    axisX.max = timeCounter
                    axisX.min = Math.max(0, timeCounter - 20000)  // 显示最近20000ms的数据
                }
                
                // 调试信息已移除，避免控制台输出过多信息
            }
        }
    }
    
    // 连接到串口通信管理器的数据接收信号
    Connections {
        target: SerialCommManager
        
        function onCmdReadDataReceived(dataId, dataValue) {
            // 检查是否为当前转速数据 (DATA_ID_SPEED_CURRENT = 0x18)
            if (dataId === 0x18) {
                console.log("接收到转速数据: ", dataValue)
                // 使用统一的变量数值更新接口
                FOC.FOCChartManager.updateVariableValue("转速", dataValue)
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
                    max: 20000  // 改为毫秒为单位，显示20000ms范围
                    titleText: qsTr("时间 (ms)")
                    labelFormat: "%.0f"
                }
                
                // Y轴
                ValueAxis {
                    id: axisY
                    min: -1200  // 调整为适合调试正弦波的幅值范围
                    max: 1200
                    titleText: qsTr("值")
                    labelFormat: "%.0f"
                }
                

            }
            
            // X轴滚动条
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
                to: 1
                value: FOC.FOCChartManager.scrollPosition
                
                // 动态计算滚动条长度（可见范围占总数据范围的比例）
                property real visibleRange: axisX.max - axisX.min
                property real totalRange: FOC.FOCChartManager.dataLengthMs
                property real handleSizeRatio: Math.max(visibleRange / totalRange, 0.05) // 最小为5%，避免滚动条太小
                
                onValueChanged: {
                    // 更新C++后端的滚动条位置，添加安全检查
                    if (FOC.FOCChartManager && typeof FOC.FOCChartManager.setScrollPosition === "function") {
                        FOC.FOCChartManager.setScrollPosition(value)
                    }
                }
                
                background: Rectangle {
                    color: "#3C3C3C"
                    radius: 2
                }
                handle: Rectangle {
                    width: parent.width * parent.handleSizeRatio
                    height: 12
                    radius: 6
                    color: "#FFFFFF"
                    border.color: "#CCCCCC"
                }
            }
            
            // 鼠标交互功能
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.MiddleButton | Qt.LeftButton | Qt.RightButton
                hoverEnabled: true
                
                property real zoomFactor: 1.1
                property real lastX: 0
                property real lastY: 0
                property bool dragging: false
                
                onPressed: {
                    if (mouse.button === Qt.MiddleButton) {
                        dragging = true
                        lastX = mouse.x
                        lastY = mouse.y
                        cursorShape = Qt.ClosedHandCursor
                    }
                }
                
                onPositionChanged: {
                    if (dragging && mouse.buttons === Qt.MiddleButton) {
                        var deltaX = mouse.x - lastX
                        var deltaY = mouse.y - lastY
                        
                        // 计算X轴和Y轴的范围
                        var xRange = axisX.max - axisX.min
                        var yRange = axisY.max - axisY.min
                        
                        // 根据鼠标移动距离计算坐标变化量
                        var xDelta = -deltaX / chartView.width * xRange
                        var yDelta = deltaY / chartView.height * yRange
                        
                        // 更新X轴和Y轴范围
                        axisX.min += xDelta
                        axisX.max += xDelta
                        axisY.min += yDelta
                        axisY.max += yDelta
                        
                        // 更新滚动条位置
                        var totalDataLength = FOC.FOCChartManager.dataLengthMs
                        var visibleRange = axisX.max - axisX.min
                        var scrollPosition = axisX.min / (totalDataLength - visibleRange)
                        
                        if (scrollPosition < 0.0) scrollPosition = 0.0
                        if (scrollPosition > 1.0) scrollPosition = 1.0
                        
                        // 更新C++后端的视图状态
                        FOC.FOCChartManager.updateViewState(axisX.min, axisX.max, axisY.min, axisY.max, 1.0)
                        
                        lastX = mouse.x
                        lastY = mouse.y
                    }
                }
                
                onReleased: {
                    if (mouse.button === Qt.MiddleButton) {
                        dragging = false
                        cursorShape = Qt.ArrowCursor
                    }
                }
                
                onWheel: {
                    var delta = wheel.angleDelta.y / 120
                    // 修正缩放方向：向上滚动放大，向下滚动缩小
                    var zoom = Math.pow(zoomFactor, -delta)
                    
                    // 获取当前X轴和Y轴范围
                    var xRange = axisX.max - axisX.min
                    var yRange = axisY.max - axisY.min
                    
                    // 计算鼠标位置对应的坐标值
                    var mouseX = wheel.x / chartView.width * xRange + axisX.min
                    var mouseY = (chartView.height - wheel.y) / chartView.height * yRange + axisY.min
                    
                    // 计算新的X轴和Y轴范围
                    var newXRange = xRange * zoom
                    var newYRange = yRange * zoom
                    
                    // 以鼠标位置为中心进行缩放
                    axisX.min = mouseX - (mouseX - axisX.min) * zoom
                    axisX.max = mouseX + (axisX.max - mouseX) * zoom
                    axisY.min = mouseY - (mouseY - axisY.min) * zoom
                    axisY.max = mouseY + (axisY.max - mouseY) * zoom
                    
                    // 更新滚动条位置
                    var totalDataLength = FOC.FOCChartManager.dataLengthMs
                    var visibleRange = axisX.max - axisX.min
                    var scrollPosition = axisX.min / (totalDataLength - visibleRange)
                    
                    if (scrollPosition < 0.0) scrollPosition = 0.0
                    if (scrollPosition > 1.0) scrollPosition = 1.0
                    
                    // 更新C++后端的视图状态
                    FOC.FOCChartManager.updateViewState(axisX.min, axisX.max, axisY.min, axisY.max, zoom)
                }
            }
        }

        // 控制按钮区域
        RowLayout {
            Layout.preferredHeight: parent.height * 0.2
            Layout.fillWidth: true
            spacing: 10

            Button {
            id: collectionButton
            Layout.fillWidth: true
            background: Rectangle {
                color: isCollecting ? "#f44336" : "#4CAF50"
                radius: 5
            }
            contentItem: Row {
                anchors.centerIn: parent
                spacing: 5
                
                Text {
                    text: isCollecting ? "⏹" : "▶"
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                }
                
                Text {
                    text: isCollecting ? qsTr("停止采集") : qsTr("开始采集")
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
            onClicked: {
                // 调用后端管理器的采集状态切换方法
                FOC.FOCChartManager.toggleCollection()
                // 同步前端状态
                isCollecting = FOC.FOCChartManager.isCollecting
                
                toggleCollectionRequested()
            }
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
                    // 清空所有动态添加的曲线
                    for (var i = 0; i < chartView.count; i++) {
                        var series = chartView.series(i)
                        if (series && series.objectName !== "axisX" && series.objectName !== "axisY") {
                            series.clear()
                        }
                    }
                    dataTimer.timeCounter = 0
                    axisX.min = 0
                    axisX.max = 20000
                    axisY.min = -1200
                    axisY.max = 1200
                    clearChartRequested()
                }
            }

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
        }
        
        // 添加变量对话框
        Dialog {
            id: addVariableDialog
            title: qsTr("选择要添加的变量")
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            visible: showAddVariableDialog
            width: 400
            height: 350  // 对话框总高度
            
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
                    Layout.alignment: Qt.AlignTop
                }
                
                // 列表容器，设置固定高度，为底部按钮预留空间
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 250  // 列表区域固定高度
                    color: "transparent"
                    
                    ListView {
                        id: variableListView
                        anchors.fill: parent
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
                
                // 占位空间，确保底部按钮不会遮挡内容
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 20  // 为底部按钮预留额外空间
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