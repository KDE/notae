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
    KF6::I18n
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::DBusAddons
    KF6::SyntaxHighlighting)

kconfig_add_kcfg_files(${CMAKE_PROJECT_NAME} GENERATE_MOC config.kcfgc)
install(TARGETS ${CMAKE_PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
