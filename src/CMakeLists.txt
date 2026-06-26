set_source_files_properties(
    shell/qml/main.qml
    PROPERTIES QT_QML_SOURCE_TYPENAME Main
)

set_source_files_properties(
    shell/qml/Theme.qml
    shell/qml/Constants.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(fluent
    main.cpp
)

qt_add_qml_module(fluent
    URI Fluent
    VERSION 1.0
    QML_FILES
        shell/qml/main.qml
        shell/qml/Theme.qml
        shell/qml/Constants.qml
        shell/topbar/TopBar.qml
        shell/topbar/ApplicationTitle.qml
        shell/topbar/Clock.qml
        shell/topbar/NotificationCenter.qml
        shell/topbar/StatusArea.qml
        shell/topbar/BatteryIndicator.qml
        shell/topbar/MediaBar.qml
        shell/qml/IconButton.qml
        shell/qml/Icon.qml

    SOURCES
        backend/systemstate.h
        backend/systemstate.cpp
        backend/dbus/dbuswrapper.h
        backend/dbus/dbuswrapper.cpp
    RESOURCES
        assets/icons/pause.svg 
        assets/icons/play.svg 
        assets/icons/skipnext.svg 
        assets/icons/skipprevious.svg    
)

target_link_libraries(fluent PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::DBus
)

target_include_directories(fluent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
)

install(TARGETS fluent DESTINATION bin)
