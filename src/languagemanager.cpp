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

QString LanguageManager::pathForMimetype(QMimeType mimetype, QString filename) {
    // Original name first
    {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(mimetype.name()),
                        QSqlDatabase::database("languages"));
        if(query.next())
            return query.value(0).toString();
    }

    // Aliases second
    for (QString aType : mimetype.aliases()) {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(aType),
                        QSqlDatabase::database("languages"));
        if(query.next())
            return query.value(0).toString();
    }

    // Parents last
    for (QString pType : mimetype.allAncestors()) {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(pType),
                        QSqlDatabase::database("languages"));
        if(query.next())
            return query.value(0).toString();
    }

    // MIME type lookup failed
    // Search for glob fitting the filename
    QSqlQuery query(QStringLiteral("SELECT globs, spec_path FROM languages"),
                    QSqlDatabase::database("languages"));
    while (query.next()) {
        QString globs = query.value(0).toString();
        for (QString glob : globs.split(';')) {
            // Very simple glob-to-regex translation
            QRegularExpression regexp("^" + glob.replace('.', "\\.").replace('?', ".").replace('*', ".*") + "$");
            if(regexp.match(filename, 0).hasMatch())
                return query.value(1).toString();
        }
    }

    return QString();
}

void LanguageManager::close() {
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

// TODO: remove obsolete entries
void LanguageManager::updateDB() {
    LanguageLoader ll;
    for (QDir dir : specsDirs) {
        for (QFileInfo file : dir.entryInfoList()) {
            if(file.isFile()) {
                LanguageMetadata langData = ll.loadMetadata(file.absoluteFilePath());
                QSqlQuery(QStringLiteral("UPDATE languages SET "
                                         "id='%1',spec_path='%2',mime_types='%3',globs='%4',display='%5' "
                                         "WHERE id='%1'").arg(
                              langData.id, file.absoluteFilePath(), langData.mimetypes, langData.globs, langData.name),
                          QSqlDatabase::database("languages"));
                // If update failed, insert
                QSqlQuery(QStringLiteral("INSERT INTO languages (id, spec_path, mime_types, globs, display) "
                                         "SELECT '%1','%2','%3','%4','%5' "
                                         "WHERE (SELECT Changes() = 0)").arg(
                              langData.id, file.absoluteFilePath(), langData.mimetypes, langData.globs, langData.name),
                          QSqlDatabase::database("languages"));
            }
        }
    }
}

QList<QDir> LanguageManager::specsDirs = QList<QDir>({
                                               QDir("/usr/share/gtksourceview-3.0/language-specs")
                                                     });
