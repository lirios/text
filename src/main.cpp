/*
 * Copyright © 2016-2017 Andrew Penkrat
 *
 * This file is part of Liri Text.
 *
 * Liri Text is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liri Text is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Liri Text.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QtQuickControls2/QQuickStyle>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSortFilterProxyModel>
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QDir>
#include <QDebug>

#include "documenthandler.h"
#include "historymanager.h"
#include "languagemanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QQuickStyle::setStyle("Material");
    QGuiApplication app(argc, argv);

    // Set app info
    app.setOrganizationName("Liri");
    app.setOrganizationDomain("liri.io");
    app.setApplicationName("Text");
    app.setDesktopFileName("io.liri.Text.desktop");
    app.setWindowIcon(QIcon(":/resources/icon.png"));

    // Load Translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator translator;
    if(translator.load(":/translations/" + QLocale::system().name()))
        app.installTranslator(&translator);

    // Parse command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Material Designed text editor");
    parser.addHelpOption();
    QCommandLineOption newFileOption("new-document", app.translate("main", "Start the editor with a new document."));
    parser.addOption(newFileOption);
    parser.addPositionalArgument("[file]", app.translate("main", "Path to a file to open for editing."));

    parser.process(app);
    QStringList args = parser.positionalArguments();
    bool nf = parser.isSet(newFileOption);

    // Register types and singletons
    qmlRegisterType<DocumentHandler>("io.liri.text", 1, 0, "DocumentHandler");

    QQmlApplicationEngine engine;

    HistoryManager *history = HistoryManager::getInstance();
    engine.rootContext()->setContextProperty("history", history);

    engine.rootContext()->setContextProperty("newDoc", nf);
    if(args.length() > 0)
        engine.rootContext()->setContextProperty("givenPath", QUrl::fromUserInput(args[0], QDir::currentPath()));
    else
        engine.rootContext()->setContextProperty("givenPath", nullptr);

    // Temporary solution untill we have font customization
    engine.rootContext()->setContextProperty("defaultFont", QFontDatabase::systemFont(QFontDatabase::FixedFont));

    // Init languages database
    LanguageManager *lManager = LanguageManager::getInstance();

    // Clean up on exiting application
    QObject::connect(&app, &QGuiApplication::lastWindowClosed, lManager, &LanguageManager::deleteLater);
    QObject::connect(&app, &QGuiApplication::lastWindowClosed, history, &HistoryManager::deleteLater);

    // Start with main.qml
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    return app.exec();
}
