// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQml>
#include <QUrl>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedContext>
#include <KLocalizedString>

constexpr auto APPLICATION_ID = "org.kde.notae";

#include "version-notae.h"
#include "config.h"
#include "app.h"
#include "documenthandler.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));

    KLocalizedString::setApplicationDomain("notae");

    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setApplicationName(QStringLiteral("notae"));

    KAboutData aboutData(
                         // The program name used internally.
                         QStringLiteral("notae"),
                         // A displayable program name string.
                         i18nc("@title", "Notae"),
                         // The program version string.
                         QStringLiteral(NOTAE_VERSION_STRING),
                         // Short description of what the app does.
                         i18n("Take notes easily"),
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
    App application;

    qmlRegisterSingletonInstance(APPLICATION_ID, 1, 0, "Config", config);

    qmlRegisterType<DocumentHandler>(APPLICATION_ID, 1, 0, "DocumentHandler");

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
