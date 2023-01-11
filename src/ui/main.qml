// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.20 as Kirigami
import org.kde.syntaxhighlighting 1.0

import org.kde.notae 1.0

Kirigami.ApplicationWindow {
    id: root

    property string documentText: ""

    title: i18n("Notae")

    width: Kirigami.Units.gridUnit * 50
    height: Kirigami.Units.gridUnit * 40
    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 20

     FileDialog {
        id: fileDialog
        title: i18n("Open File")
        nameFilters: ["Markdown files (*.md)", "Text files (*.txt)"]
        onAccepted: {
            FileController.open(fileDialog.file)
            pageStack.replace(textPage)
        }
    }

    FileDialog {
        id: saveFileDialog
        title: i18n("Save File As")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "md"
        onAccepted: {
            FileController.saveAs(saveFileDialog.file, documentText)
            pageStack.replace(textPage)
        }
    }

    Timer {
        id: saveWindowGeometryTimer
        interval: 1000
        onTriggered: App.saveWindowGeometry(root)
    }

    Connections {
        target: FileController

        function onFileChanged(text, title) {
            documentText = text
            root.title = title
        }
    }

    Connections {
        id: saveWindowGeometryConnections
        enabled: root.visible
        target: root

        function onClosing() { App.saveWindowGeometry(root); }
        function onWidthChanged() { saveWindowGeometryTimer.restart(); }
        function onHeightChanged() { saveWindowGeometryTimer.restart(); }
        function onXChanged() { saveWindowGeometryTimer.restart(); }
        function onYChanged() { saveWindowGeometryTimer.restart(); }
    }

    Loader {
        active: !Kirigami.Settings.isMobile
        sourceComponent: GlobalMenu {}
    }

    pageStack.initialPage: welcomePage

    Component {
        id: welcomePage

        Kirigami.Page {
            padding: 0
            titleDelegate: RowLayout {}

            ColumnLayout {
                anchors.fill: parent

                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter

                    QQC2.Button {
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 8

                        action: Kirigami.Action {
                            text: i18n("New File")
                            icon.name: "document-new"
                            shortcut: StandardKey.New
                            onTriggered: {
                                saveFileDialog.open()
                            }
                        }
                    }
                    QQC2.Button {
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 8

                        action: Kirigami.Action {
                            text: i18n("Open File…")
                            icon.name: "document-open"
                            shortcut: StandardKey.Open
                            onTriggered: fileDialog.open()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: textPage

        Kirigami.Page {
            padding: 0
            titleDelegate: PageHeader {}

            QQC2.ScrollView {
                anchors.fill: parent

                QQC2.TextArea {
                    id: textarea

                    padding: Kirigami.Units.gridUnit
                    leftPadding: Kirigami.Units.gridUnit + (root.width / 8)
                    rightPadding: Kirigami.Units.gridUnit + (root.width / 8)

                    background: Rectangle {
                        Kirigami.Theme.colorSet: Kirigami.Theme.View
                        Kirigami.Theme.inherit: false
                        color: Kirigami.Theme.backgroundColor
                    }

                    text: documentText
                    wrapMode: Text.Wrap
                    font.pointSize: 11

                    onTextChanged: {
                        FileController.save(textarea.text)
                    }

                    SyntaxHighlighter {
                        textEdit: textarea
                        definition: "Markdown"
                    }

                    Component.onCompleted: forceActiveFocus()
                }
            }
        }
    }
}
