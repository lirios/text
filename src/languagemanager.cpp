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

#include "languagemanager.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QVariant>
#include <QRegularExpression>

#include "languageloader.h"

void LanguageManager::init() {
    initDB();
    updateDB();
    watcher = new QFileSystemWatcher(specsDirs);
    connect(watcher, &QFileSystemWatcher::directoryChanged, updateDB);
}

QString LanguageManager::pathForId(QString id) {
    QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                   "WHERE id = '%1'").arg(id),
                    QSqlDatabase::database("languages"));
    if(query.next())
        return query.value(0).toString();
    else
        return QString();
}

QString LanguageManager::pathForMimeType(QMimeType mimeType, QString filename) {
    // Original name first
    {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(mimeType.name()),
                        QSqlDatabase::database("languages"));
        if(query.next())
            return query.value(0).toString();
    }

    // Aliases second
    for (QString aType : mimeType.aliases()) {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(aType),
                        QSqlDatabase::database("languages"));
        if(query.next())
            return query.value(0).toString();
    }

    // Parents last
    for (QString pType : mimeType.allAncestors()) {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(pType),
                        QSqlDatabase::database("languages"));
        if(query.next())
            return query.value(0).toString();
    }

    // MIME type lookup failed
    // Search for glob fitting the filename
    {
        QSqlQuery query(QStringLiteral("SELECT globs, spec_path FROM languages"),
                        QSqlDatabase::database("languages"));
        while (query.next()) {
            QString globs = query.value(0).toString();
            for (QString glob : globs.split(';')) {
                // Very simple glob-to-regexp translation

                glob.replace('.', "\\.");
                glob.replace('?', ".");
                // In glob starting with *. * shouldn't match empty string
                if(glob.startsWith("*\\."))
                    glob.replace(0, 1, ".+");
                // Elsewhere it can
                glob.replace('*', ".*");

                QRegularExpression regexp("^" + glob + "$");

                if(regexp.match(filename, 0).hasMatch())
                    return query.value(1).toString();
            }
        }
    }

    return QString();
}

void LanguageManager::close() {
    delete watcher;
    QSqlDatabase::removeDatabase("languages");
}

void LanguageManager::initDB() {
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!dataDir.exists())
        dataDir.mkpath(".");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "languages");
    db.setDatabaseName(dataDir.filePath("languages.db"));
    db.open();
    QSqlQuery("CREATE TABLE IF NOT EXISTS languages "
              "(id TEXT PRIMARY KEY, spec_path TEXT, mime_types TEXT, globs TEXT, display TEXT)",
              db);
}

void LanguageManager::updateDB() {
    LanguageLoader ll;
    for (QDir dir : specsDirs) {
        for (QFileInfo file : dir.entryInfoList()) {
            if(file.isFile()) {
                LanguageMetadata langData = ll.loadMetadata(file.absoluteFilePath());
                QSqlQuery(QStringLiteral("UPDATE languages SET "
                                         "id='%1',spec_path='%2',mime_types='%3',globs='%4',display='%5' "
                                         "WHERE id='%1'").arg(
                              langData.id, file.absoluteFilePath(), langData.mimeTypes, langData.globs, langData.name),
                          QSqlDatabase::database("languages"));
                // If update failed, insert
                QSqlQuery(QStringLiteral("INSERT INTO languages (id, spec_path, mime_types, globs, display) "
                                         "SELECT '%1','%2','%3','%4','%5' "
                                         "WHERE (SELECT Changes() = 0)").arg(
                              langData.id, file.absoluteFilePath(), langData.mimeTypes, langData.globs, langData.name),
                          QSqlDatabase::database("languages"));
            }
        }
    }

    // Search for obsolete entries
    QSqlQuery query(QStringLiteral("SELECT id, spec_path FROM languages"),
                    QSqlDatabase::database("languages"));
    while (query.next()) {
        QFileInfo file(query.value(1).toString());
        if(!file.exists())
            QSqlQuery(QStringLiteral("DELETE FROM languages "
                                     "WHERE id='%1'").arg(query.value(0).toString()),
                      QSqlDatabase::database("languages"));
    }
}

// List of language specification directories, ascending by priority
QStringList LanguageManager::specsDirs = QStringList({
            #ifdef GTKSOURCEVIEW_LANGUAGE_SPECS
                                         QString(GTKSOURCEVIEW_LANGUAGE_SPECS),
            #endif
                                         QString(LIRI_LANGUAGE_SPECS)
                                                     });

QFileSystemWatcher *LanguageManager::watcher = nullptr;
