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

#include "languageloader.h"

void LanguageManager::init() {
    initDB();
    updateDB();
}

QString LanguageManager::pathForId(QString id) {
    QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                   "WHERE id = '%1'").arg(id),
                    m_db);
    if(query.next())
        return query.value(0).toString();
    else
        return QString();
}

void LanguageManager::close() {
    m_db.close();
    QSqlDatabase::removeDatabase("languages");
}

void LanguageManager::initDB() {
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!dataDir.exists())
        dataDir.mkpath(".");

    m_db = QSqlDatabase::addDatabase("QSQLITE", "languages");
    m_db.setDatabaseName(dataDir.filePath("languages.db"));
    m_db.open();
    QSqlQuery("CREATE TABLE IF NOT EXISTS languages "
              "(id TEXT PRIMARY KEY, spec_path TEXT, mime_types TEXT, display TEXT)",
              m_db);
}

void LanguageManager::updateDB() {
    LanguageLoader *ll = new LanguageLoader(QSharedPointer<LanguageDefaultStyles>(new LanguageDefaultStyles));
    for (QDir dir : specsDirs) {
        for (QFileInfo file : dir.entryInfoList()) {
            if(file.isFile()) {
                LanguageMetadata langData = ll->loadMetadata(file.absoluteFilePath());
                QSqlQuery(QStringLiteral("UPDATE languages SET "
                                         "id='%1',spec_path='%2',mime_types='%3',display='%4' "
                                         "WHERE id='%1'").arg(
                              langData.id, file.absoluteFilePath(), langData.mimetypes, langData.name),
                                m_db);
                // If update failed, insert
                QSqlQuery(QStringLiteral("INSERT INTO languages (id, spec_path, mime_types, display) "
                                         "SELECT '%1','%2','%3','%4' "
                                         "WHERE (SELECT Changes() = 0)").arg(
                              langData.id, file.absoluteFilePath(), langData.mimetypes, langData.name),
                                m_db);
            }
        }
    }
}

QSqlDatabase LanguageManager::m_db;
QList<QDir> LanguageManager::specsDirs = QList<QDir>({
                                               QDir("/usr/share/gtksourceview-3.0/language-specs")
                                                     });
