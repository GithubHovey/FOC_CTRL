cmake_minimum_required(VERSION 3.16)

project(FOC_CTRL VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Charts SerialPort Concurrent)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appFOC_CTRL
    main.cpp
)

qt_add_qml_module(appFOC_CTRL
    URI FOC_CTRL
    VERSION 1.0
    QML_FILES
        Main.qml
        qml/SerialCommunicationModule.qml
        qml/FOCChartModule.qml
        qml/MotorModeControlModule.qml
        qml/MotorDataReadWriteModule.qml
        qml/LogModule.qml
        qml/CommandControlModule.qml
    SOURCES 
        serial_communication_manager.h
        serial_communication_manager.cpp
        command_control_manager.h
        command_control_manager.cpp
        ringbuf.h
        ringbuf.c
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appFOC_CTRL PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appFOC_CTRL
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appFOC_CTRL
    PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Charts Qt6::SerialPort Qt6::Concurrent
)

include(GNUInstallDirs)
install(TARGETS appFOC_CTRL
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
