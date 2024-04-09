// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.notae

QQC2.Page {
    id: control

    focus: true

    property alias text: body.text
    property alias body: body
    property alias document: document
    property alias scrollView: _scrollView

    property alias canUndo: body.canUndo
    property alias canRedo: body.canRedo

    DocumentHandler {
        id: document

        document: body.textDocument
        cursorPosition: body.cursorPosition
        selectionStart: body.selectionStart
        selectionEnd: body.selectionEnd
        backgroundColor: control.Kirigami.Theme.backgroundColor

        onSearchFound: {
            body.select(start, end)
        }

        onFileUrlChanged: {
            root.title = fileName
        }
    }

    contentItem: Item {
        clip: false

        QQC2.ScrollView {
            id: _scrollView

            anchors.fill: parent

            clip: false

            Flickable {
                id: _flickable

                anchors.fill: parent

                clip: false

                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds

                QQC2.TextArea.flickable: QQC2.TextArea {
                    id: body

                    text: document.text
                    textFormat: TextEdit.PlainText

                    wrapMode: Text.Wrap

                    padding: Kirigami.Units.largeSpacing

                    background: Rectangle {
                        Kirigami.Theme.colorSet: Kirigami.Theme.View
                        Kirigami.Theme.inherit: false
                        color: Kirigami.Theme.backgroundColor

                        Rectangle {
                            Kirigami.Theme.colorSet: Kirigami.Theme.View
                            Kirigami.Theme.inherit: false
                            color: Qt.darker(Kirigami.Theme.backgroundColor, 1.03)

                            visible: control.focus
                            width: body.width
                            height: body.cursorRectangle.height
                            y: body.cursorRectangle.y
                        }
                    }

                    Component.onCompleted: body.forceActiveFocus()
                }
            }
        }
    }
}
