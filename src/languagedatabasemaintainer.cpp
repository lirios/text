/*
 * Copyright © 2017 Andrew Penkrat
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

#include "languagedatabasemaintainer.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>
#include "languageloader.h"

LanguageDatabaseMaintainer::LanguageDatabaseMaintainer(const QString &path, QObject *parent) :
    QObject(parent),
    m_connId(QStringLiteral("lang_db_maintainer")),
    m_dbPath(path) {

    // List of language specification directories, ascending by priority
#ifdef GTKSOURCEVIEW_LANGUAGE_PATH
    specsDirs.append(QStringLiteral(GTKSOURCEVIEW_LANGUAGE_PATH));
#endif
#ifdef ABSOLUTE_LANGUAGE_PATH
    specsDirs.append(QStringLiteral(ABSOLUTE_LANGUAGE_PATH));
#endif
#ifdef RELATIVE_LANGUAGE_PATH
    specsDirs.append(QCoreApplication::applicationDirPath() + QStringLiteral(RELATIVE_LANGUAGE_PATH));
#endif
    specsDirs.append(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral(USER_LANGUAGE_PATH));
}

LanguageDatabaseMaintainer::~LanguageDatabaseMaintainer() {
    delete watcher;
    QSqlDatabase::removeDatabase(m_connId);
}

void LanguageDatabaseMaintainer::init() {
    initDB(m_dbPath);
    updateDB();
    watcher = new QFileSystemWatcher(specsDirs);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &LanguageDatabaseMaintainer::updateDB);
}

void LanguageDatabaseMaintainer::initDB(const QString &path) {
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connId);
    db.setDatabaseName(path);
    db.open();

    QSqlQuery(QStringLiteral("CREATE TABLE IF NOT EXISTS languages "
                             "(id TEXT PRIMARY KEY, spec_path TEXT, mime_types TEXT, globs TEXT, display TEXT)"),
              db);
}

void LanguageDatabaseMaintainer::updateDB() {
    LanguageLoader ll;
    for (const QDir &dir : qAsConst(specsDirs)) {
        const QFileInfoList &filesList = dir.entryInfoList();
        for (const QFileInfo &file : filesList) {
            if(file.isFile()) {
                LanguageMetadata langData = ll.loadMetadata(file.absoluteFilePath());
                QSqlQuery(QStringLiteral("UPDATE languages SET "
                                         "id='%1',spec_path='%2',mime_types='%3',globs='%4',display='%5' "
                                         "WHERE id='%1'").arg(
                              langData.id, file.absoluteFilePath(), langData.mimeTypes, langData.globs, langData.name),
                          QSqlDatabase::database(m_connId));
                // If update failed, insert
                QSqlQuery(QStringLiteral("INSERT INTO languages (id, spec_path, mime_types, globs, display) "
                                         "SELECT '%1','%2','%3','%4','%5' "
                                         "WHERE (SELECT Changes() = 0)").arg(
                              langData.id, file.absoluteFilePath(), langData.mimeTypes, langData.globs, langData.name),
                          QSqlDatabase::database(m_connId));
            }
        }
    }

    // Search for obsolete entries
    QSqlQuery query(QStringLiteral("SELECT id, spec_path FROM languages"),
                    QSqlDatabase::database(m_connId));
    while (query.next()) {
        QFileInfo file(query.value(1).toString());
        if(!file.exists())
            QSqlQuery(QStringLiteral("DELETE FROM languages "
                                     "WHERE id='%1'").arg(query.value(0).toString()),
                      QSqlDatabase::database(m_connId));
    }

    emit dbUpdated();
}
