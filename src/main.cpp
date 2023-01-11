// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QtQml>
#include <QQuickWindow>

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KDBusService>

constexpr auto APPLICATION_ID = "org.kde.notae";

#include "about.h"
#include "version-notae.h"
#include "config.h"
#include "app.h"
#include "filecontroller.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));

    KAboutData aboutData(
                         // The program name used internally.
                         QStringLiteral("notae"),
                         // A displayable program name string.
                         i18nc("@title", "Notae"),
                         // The program version string.
                         QStringLiteral(NOTAE_VERSION_STRING),
                         // Short description of what the app does.
                         i18n("Note Taking Application"),
                         // The license this code is released under.
                         KAboutLicense::GPL,
                         // Copyright Statement.
                         i18n("© Felipe Kinoshita 2023"));
    aboutData.addAuthor(i18nc("@info:credit", "Felipe Kinoshita"), i18nc("@info:credit", "Author"), QStringLiteral("kinofhek@gmail.com"), QStringLiteral("https://fhek.gitlab.io"));
    aboutData.setBugAddress("https://invent.kde.org/fhek/notae/-/issues/new");
    KAboutData::setApplicationData(aboutData);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.notae")));

    QQmlApplicationEngine engine;

    auto config = Config::self();
    AboutType about;
    App application;
    FileController fileController;

    qmlRegisterSingletonInstance(APPLICATION_ID, 1, 0, "Config", config);
    qmlRegisterSingletonInstance(APPLICATION_ID, 1, 0, "AboutType", &about);
    qmlRegisterSingletonInstance(APPLICATION_ID, 1, 0, "App", &application);
    qmlRegisterSingletonInstance(APPLICATION_ID, 1, 0, "FileController", &fileController);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("notae");
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    KDBusService service(KDBusService::Unique);

    // Restore window size and position
    const auto rootObjects = engine.rootObjects();
    for (auto obj : rootObjects) {
        auto view = qobject_cast<QQuickWindow *>(obj);
        if (view) {
            if (view->isVisible()) {
                application.restoreWindowGeometry(view);
            }
            break;
        }
    }

    return app.exec();
}
