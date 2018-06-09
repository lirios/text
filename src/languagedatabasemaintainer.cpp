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
#include <QDateTime>
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

    QSqlQuery query(db);
    query.exec(QStringLiteral("BEGIN TRANSACTION"));

    query.exec(QStringLiteral("PRAGMA user_version"));

    if(!query.first())
        qWarning() << "Database is missing user_version pragma!!! Something is wrong with SQLite?";

    int dbVersion = query.value(0).toInt();
    if(dbVersion != LANGUAGE_DB_VERSION) {
        // Database schema is outdated

        /* Since languages db is just a cache of languages' metadata,
         * we can won't lose anything if we drop it and create anew
         */
        query.exec(QStringLiteral("DROP TABLE IF EXISTS languages"));
        query.exec(QStringLiteral("PRAGMA user_version = %1").arg(LANGUAGE_DB_VERSION));
    }
    query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS languages "
                             "(spec_path TEXT PRIMARY KEY, id TEXT, priority INTEGER, mime_types TEXT, globs TEXT, display TEXT, modification_time INTEGER)"));

    query.exec(QStringLiteral("COMMIT TRANSACTION"));
}

void LanguageDatabaseMaintainer::updateDB() {
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.exec(QStringLiteral("BEGIN TRANSACTION"));

    // Search for obsolete entries
    query.exec(QStringLiteral("SELECT spec_path FROM languages"));
    while (query.next()) {
        QFileInfo file(query.value(0).toString());
        if(!file.exists())
            query.exec(QStringLiteral("DELETE FROM languages "
                                      "WHERE spec_path='%1'").arg(query.value(0).toString()));
    }

    int priority = 0;
    LanguageLoader ll;
    for (const QDir &dir : qAsConst(specsDirs)) {
        const QFileInfoList &filesList = dir.entryInfoList();
        for (const QFileInfo &file : filesList) {
            if(file.isFile()) {
                QString filePath = file.absoluteFilePath();
                qint64 fileTime = file.fileTime(QFile::FileModificationTime).toMSecsSinceEpoch();
                query.exec(QStringLiteral("SELECT modification_time FROM languages "
                                          "WHERE spec_path='%1'").arg(filePath));
                if(query.first()) {
                    /* Compare timestamp with file modification time
                     */
                    qint64 dbTime = query.value(0).toLongLong();
                    if(dbTime < fileTime) {
                        /* Entry is outdated, update it
                         */
                        LanguageMetadata langData = ll.loadMetadata(filePath);
                        query.exec(QStringLiteral("UPDATE languages SET "
                                                  "spec_path='%1',id='%2',priority='%3',mime_types='%4',globs='%5',display='%6',modification_time='%7' "
                                                  "WHERE spec_path='%1'").arg(
                                   filePath, langData.id, QString::number(priority), langData.mimeTypes, langData.globs, langData.name, QString::number(fileTime)));
                    }
                } else {
                    /* There's no existing entry for this file,
                     * parse metadata and insert
                     */
                    LanguageMetadata langData = ll.loadMetadata(filePath);
                    query.exec(QStringLiteral("INSERT INTO languages (spec_path, id, priority, mime_types, globs, display, modification_time) "
                                              "SELECT '%1','%2','%3','%4','%5','%6','%7'").arg(
                               filePath, langData.id, QString::number(priority), langData.mimeTypes, langData.globs, langData.name, QString::number(fileTime)));
                }
            }
        }
        priority++;
    }

    query.exec(QStringLiteral("COMMIT TRANSACTION"));
    emit dbUpdated();
}
