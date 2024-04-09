# SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_executable(${CMAKE_PROJECT_NAME}
    main.cpp
    app.cpp
    documenthandler.cpp)

ecm_add_qml_module(${CMAKE_PROJECT_NAME}
        URI org.kde.notae
        VERSION 1.0
        DEPENDENCIES
        "QtQuick.Controls"
        GENERATE_PLUGIN_SOURCE)
qt_target_qml_sources(${CMAKE_PROJECT_NAME}
        QML_FILES
        ui/Main.qml
        ui/TextEditor.qml)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Svg
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::DBusAddons
    KF${QT_MAJOR_VERSION}::SyntaxHighlighting)

kconfig_add_kcfg_files(${CMAKE_PROJECT_NAME} GENERATE_MOC config.kcfgc)
install(TARGETS ${CMAKE_PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
