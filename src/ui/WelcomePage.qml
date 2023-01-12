// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.notae 1.0

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
