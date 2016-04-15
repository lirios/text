/*
 * Copyright © 2016 Andrew Penkrat
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
#include <QSortFilterProxyModel>
#include <QCommandLineParser>
#include <QFontDatabase>
#include <QDebug>

#include "documenthandler.h"
#include "historymanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("liri-project");
    QCoreApplication::setOrganizationDomain("liriproject.me");
    QCoreApplication::setApplicationName("liri-text");

    QCommandLineParser parser;
    parser.setApplicationDescription("Material Designed text editor");
    parser.addHelpOption();
    QCommandLineOption newFileOption("new-document", app.translate("main", "Start the editor with a new document."));
    parser.addOption(newFileOption);
    parser.addPositionalArgument("[file]", app.translate("main", "Path to a file to open for editing."));

    parser.process(app);
    QStringList args = parser.positionalArguments();
    bool nf = parser.isSet(newFileOption);

    qmlRegisterType<DocumentHandler>("me.liriproject.text", 1, 0, "DocumentHandler");

	QQmlApplicationEngine engine;
    HistoryManager *history = new HistoryManager();
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(history);
    proxyModel->setSortRole(HistoryManager::LastViewTimeRole);
    proxyModel->sort(0, Qt::DescendingOrder);
    engine.rootContext()->setContextProperty("history", history);
    engine.rootContext()->setContextProperty("sortedHistory", proxyModel);

    engine.rootContext()->setContextProperty("newDoc", nf);
    if(args.length() > 0)
        engine.rootContext()->setContextProperty("givenPath", QUrl::fromLocalFile(args[0]));
    else
        engine.rootContext()->setContextProperty("givenPath", nullptr);

    // Temporary solution untill we have font configuration
    engine.rootContext()->setContextProperty("defaultFont", QFontDatabase::systemFont(QFontDatabase::FixedFont));
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

	return app.exec();
}
