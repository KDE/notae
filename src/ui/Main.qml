// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami

import org.kde.notae

Kirigami.ApplicationWindow {
    id: root

    visible: true
    title: i18n("Notae")

    width: Kirigami.Units.gridUnit * 25
    height: Kirigami.Units.gridUnit * 33
    minimumWidth: Kirigami.Units.gridUnit * 25
    minimumHeight: Kirigami.Units.gridUnit * 15

    Timer {
        id: saveWindowGeometryTimer
        interval: 1000
        onTriggered: App.saveWindowGeometry(root)
    }

    Connections {
        id: saveWindowGeometryConnections
        enabled: root.visible
        target: root

        function onClosing() { App.saveWindowGeometry(root) }
        function onWidthChanged() { saveWindowGeometryTimer.restart() }
        function onHeightChanged() { saveWindowGeometryTimer.restart() }
        function onXChanged() { saveWindowGeometryTimer.restart() }
        function onYChanged() { saveWindowGeometryTimer.restart() }
    }

    component ToolBar : Item {
        default property alias children: headerLayout.children

        width: root.width
        height: headerLayout.implicitHeight + (headerLayout.anchors.margins * 2)

        Kirigami.AbstractApplicationHeader {
            id: headerToolbar

            anchors.top: parent.top
            anchors.bottom: separator.top
            width: parent.width
            visible: true

            contentItem: RowLayout {
                id: headerLayout

                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing

                spacing: Kirigami.Units.smallSpacing
            }
        }
        Kirigami.Separator {
            id: separator

            visible: !headerToolbar.visible
            anchors.bottom: parent.bottom
            width: parent.width
        }
    }

    FileDialog {
        id: openDialog

        nameFilters: ["Markdown files (*.md)", "Text files (*.txt)"]

        onAccepted: textEditor.document.fileUrl = currentFile
    }

    FileDialog {
        id: saveDialog

        fileMode: FileDialog.SaveFile
        onAccepted: textEditor.document.saveAs(currentFile)
    }

    pageStack.initialPage: TextEditor {
        id: textEditor

        // Use our own header on the main page
        globalToolBarStyle: pageStack.layers.depth > 1 ? Kirigami.ApplicationHeaderStyle.ToolBar : Kirigami.ApplicationHeaderStyle.None

        header: Loader {
            active: true
            sourceComponent: ToolBar {
                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus
                    display: QQC2.AbstractButton.IconOnly

                    action: Kirigami.Action {
                        text: i18n("New")
                        icon.name: "document-new"
                        shortcut: StandardKey.New
                        onTriggered: {
                            textEditor.document.saveAs(textEditor.document.fileUrl)
                            textEditor.document.fileUrl = ""
                            textEditor.body.text = ""
                        }
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Create new document (Ctrl+N)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus
                    display: QQC2.AbstractButton.IconOnly

                    action: Kirigami.Action {
                        text: i18n("Open…")
                        icon.name: "document-open"
                        shortcut: StandardKey.Open
                        onTriggered: openDialog.open()
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Open an existing note (Ctrl+O)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus
                    display: QQC2.AbstractButton.IconOnly

                    action: Kirigami.Action {
                        text: textEditor.document.fileUrl == "" ? i18n("Save As…") : i18n("Save")
                        icon.name: textEditor.document.fileUrl == "" ? "document-save-as" : "document-save"
                        shortcut: StandardKey.Save
                        onTriggered: {
                            if (textEditor.document.fileUrl == "") {
                                saveDialog.open()
                                return
                            }

                            textEditor.document.saveAs(textEditor.document.fileUrl)
                        }
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: textEditor.document.fileUrl == "" ? i18n("Save new document (Ctrl+Shift+S)") : i18n("Save document (Ctrl+S)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolSeparator {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus

                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Cut")
                        icon.name: "edit-cut"
                        shortcut: StandardKey.Cut
                        enabled: !textEditor.body.selectedText.length <= 0
                        onTriggered: textEditor.body.cut()
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Cut selection to clipboard (Ctrl+X)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus

                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Copy")
                        icon.name: "edit-copy"
                        shortcut: StandardKey.Copy
                        enabled: textEditor.body.selectedText.length > 0
                        onTriggered: textEditor.body.copy()
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Copy selection to clipboard (Ctrl+C)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus

                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Paste")
                        icon.name: "edit-paste"
                        shortcut: StandardKey.Paste
                        enabled: textEditor.canPaste
                        onTriggered: textEditor.body.paste()
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Paste clipboard content (Ctrl+V)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolSeparator {
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                QQC2.ToolButton {
                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Undo")
                        icon.name: "edit-undo"
                        shortcut: StandardKey.Undo
                        enabled: textEditor.canUndo
                        onTriggered: textEditor.body.undo()
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Undo last action (Ctrl+Z)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.ToolButton {
                    focusPolicy: Qt.NoFocus

                    display: QQC2.AbstractButton.IconOnly
                    action: Kirigami.Action {
                        text: i18n("Redo")
                        icon.name: "edit-redo"
                        shortcut: StandardKey.Redo
                        enabled: textEditor.canRedo
                        onTriggered: textEditor.body.redo()
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Redo last undone action (Ctrl+Shift+Z)")
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                Item {
                    Layout.fillWidth: true
                }

                QQC2.ToolButton {
                    id: menuButton

                    focusPolicy: Qt.NoFocus

                    display: QQC2.AbstractButton.IconOnly
                    down: applicationMenu.opened
                    action: Kirigami.Action {
                        text: i18n("Menu")
                        icon.name: "application-menu-symbolic"
                        onTriggered: applicationMenu.popup(menuButton, 0.0, menuButton.height)
                    }

                    QQC2.Menu {
                        id: applicationMenu
                        QQC2.MenuItem {
                            text: i18nc("@action:inmenu", "About Notae")
                            icon.name: "org.kde.notae"
                            onClicked: pageStack.layers.push(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"))
                        }
                        QQC2.MenuItem {
                            text: i18nc("@action:inmenu", "About KDE")
                            icon.name: "kdeapp"
                            onClicked: pageStack.layers.push(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutKDE"))
                        }
                    }

                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }

        body.font.family: "Monospace"
        document.tabSpace: Kirigami.Units.gridUnit * 2
        document.enableSyntaxHighlighting: true
        document.autoSave: false//Config.autoSave
    }
}
