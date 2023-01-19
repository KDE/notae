// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.notae 1.0

RowLayout {
    id: pageHeader

    Layout.fillWidth: true
    spacing: 0

    QQC2.ToolButton {
        focusPolicy: Qt.NoFocus
        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon

        action: Kirigami.Action {
            text: i18n("New")
            icon.name: "document-new"
            shortcut: StandardKey.New
            onTriggered: FileController.newFile()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Create new note (Ctrl+N)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        focusPolicy: Qt.NoFocus
        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon

        action: Kirigami.Action {
            text: i18n("Open")
            icon.name: "document-open"
            shortcut: StandardKey.Open
            onTriggered: fileDialog.open()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Open an existing note (Ctrl+O)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        visible: FileController.isEmptyFile
        focusPolicy: Qt.NoFocus
        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon

        action: Kirigami.Action {
            text: i18n("Save")
            icon.name: "document-save"
            shortcut: StandardKey.Save
            onTriggered: saveFileDialog.open()
            enabled: FileController.isEmptyFile
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Save note (Ctrl+S)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolSeparator {
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
    }

    QQC2.ToolButton {
        focusPolicy: Qt.NoFocus

        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon
        action: Kirigami.Action {
            text: i18n("Cut")
            icon.name: "edit-cut"
            shortcut: StandardKey.Cut
            enabled: !textarea.selectedText.length <= 0
            onTriggered: textarea.cut()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Cut selection to clipboard (Ctrl+X)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        focusPolicy: Qt.NoFocus

        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon
        action: Kirigami.Action {
            text: i18n("Copy")
            icon.name: "edit-copy"
            shortcut: StandardKey.Copy
            enabled: textarea.selectedText.length > 0
            onTriggered: textarea.copy()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Copy selection to clipboard (Ctrl+C)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        focusPolicy: Qt.NoFocus

        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon
        action: Kirigami.Action {
            text: i18n("Paste")
            icon.name: "edit-paste"
            shortcut: StandardKey.Paste
            enabled: textarea.canPaste
            onTriggered: textarea.paste()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Paste clipboard content (Ctrl+V)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon
        action: Kirigami.Action {
            text: i18n("Undo")
            icon.name: "edit-undo"
            shortcut: StandardKey.Undo
            enabled: textarea.canUndo
            onTriggered: textarea.undo()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Undo last action (Ctrl+Z)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        focusPolicy: Qt.NoFocus

        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon
        action: Kirigami.Action {
            text: i18n("Redo")
            icon.name: "edit-redo"
            shortcut: StandardKey.Redo
            enabled: textarea.canRedo
            onTriggered: textarea.redo()
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Redo last undone action (Ctrl+Shift+Z)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolSeparator {
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
    }

    Item {
        Layout.fillWidth: true
    }

    QQC2.ToolButton {
        display: Kirigami.Settings.isMobile ? QQC2.AbstractButton.IconOnly : QQC2.AbstractButton.TextBesideIcon
        action: Kirigami.Action {
            text: i18n("Settings")
            icon.name: "settings-configure"
            shortcut: StandardKey.Preferences
            onTriggered: {
                if (!Kirigami.Settings.isMobile) {
                    pageStack.pushDialogLayer(Qt.resolvedUrl("SettingsPage.qml"), {}, {
                        title: i18n("Configure"),
                        width: root.width - Kirigami.Units.gridUnit * 12,
                        height: root.height - Kirigami.Units.gridUnit * 6
                    })
                } else {
                    pageStack.layers.push(Qt.resolvedUrl("SettingsPage.qml"))
                }
            }
            enabled: pageStack.layers.depth <= 1
        }

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: i18n("Configure Notae (Ctrl+Shift+,)")
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
