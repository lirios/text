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
#include <QSettings>
#include <QDebug>

#include "documenthandler.h"
#include "historymanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("liri-project");
    QCoreApplication::setOrganizationDomain("liriproject.me");
    QCoreApplication::setApplicationName("liri-text");

    QGuiApplication app(argc, argv);
    qmlRegisterType<DocumentHandler>("me.liriproject.text", 1, 0, "DocumentHandler");

	QQmlApplicationEngine engine;
    HistoryManager *history = new HistoryManager();
    engine.rootContext()->setContextProperty("history", history);
	engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

	return app.exec();
}
