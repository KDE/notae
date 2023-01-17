// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.notae 1.0

Kirigami.PageRow {
    id: pageStack

    globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar

    initialPage: Kirigami.ScrollablePage {
        title: i18nc("@title:window", "Settings")

        leftPadding: 0
        rightPadding: 0

        ColumnLayout {
            MobileForm.FormCard {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing

                contentItem: ColumnLayout {
                    spacing: 0

                    MobileForm.FormCardHeader {
                        title: i18n("General")
                    }

                    MobileForm.FormSwitchDelegate {
                        id: showStats
                        text: i18n("Remember last opened file")

                        checked: Config.rememberMostRecentFile
                        onToggled: {
                            Config.rememberMostRecentFile = checked
                            Config.save()
                        }
                    }

                    MobileForm.FormDelegateSeparator {}

                    MobileForm.FormSwitchDelegate {
                        text: i18n("Show Toolbar")

                        checked: Config.showToolbar
                        onToggled: {
                            Config.showToolbar = checked
                            Config.save()
                        }
                    }

                    MobileForm.FormSectionText {
                        text: i18n("Toggle it by pressing Ctrl+,")
                    }

                    MobileForm.FormDelegateSeparator {}
                }
            }

            MobileForm.FormCard {
                Layout.topMargin: Kirigami.Units.largeSpacing
                Layout.fillWidth: true
                contentItem: ColumnLayout {
                    spacing: 0
                    Component {
                        id: aboutPage
                        MobileForm.AboutPage {
                            aboutData: AboutType.aboutData
                        }
                    }
                    MobileForm.FormButtonDelegate {
                        text: i18n("About Notae")
                        icon.name: "help-about"
                        onClicked: applicationWindow().pageStack.layers.push(aboutPage)
                    }
                }
            }
        }
    }
}
