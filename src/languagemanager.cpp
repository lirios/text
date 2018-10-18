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

#include "languagemanager.h"

#include <QSqlQuery>
#include <QVariant>
#include <QRegularExpression>
#include <QThread>
#include <QDir>
#include <QStandardPaths>

LanguageManager::LanguageManager(QObject *parent) :
    QObject(parent),
    m_connId(QStringLiteral("languages")) {

    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!dataDir.exists())
        dataDir.mkpath(QStringLiteral("."));
    QString dbPath = dataDir.filePath(QStringLiteral("languages.db"));

    m_thread = new QThread;
    LanguageDatabaseMaintainer *dbMaintainer = new LanguageDatabaseMaintainer(dbPath);
    dbMaintainer->moveToThread(m_thread);
    connect(m_thread, &QThread::started, dbMaintainer, &LanguageDatabaseMaintainer::init);
    connect(m_thread, &QThread::finished, dbMaintainer, &LanguageDatabaseMaintainer::deleteLater);
    m_thread->start();

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connId);
    db.setDatabaseName(dbPath);
    db.open();
}

LanguageManager *LanguageManager::getInstance() {
    if(!m_instance)
        m_instance = new LanguageManager;
    return m_instance;
}

QString LanguageManager::pathForId(const QString &id) {
    QSqlQuery query(QStringLiteral("SELECT spec_path, MAX(priority) FROM languages "
                                   "WHERE id = '%1' GROUP BY id").arg(id),
                    QSqlDatabase::database(m_connId));
    if(query.next())
        return query.value(0).toString();
    else
        return QString();
}

QString LanguageManager::pathForMimeType(const QMimeType &mimeType, const QString &filename) {
    // Original name first
    {
        QSqlQuery query(QStringLiteral("SELECT spec_path, MAX(priority) FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0 GROUP BY id").arg(mimeType.name()),
                        QSqlDatabase::database(m_connId));
        if(query.next())
            return query.value(0).toString();
    }

    // Aliases and parents second
    // TODO: Check if we actually need to check all ancestors
    const QStringList &alternatives = mimeType.aliases() + mimeType.allAncestors();
    for (const QString &aType : alternatives) {
        QSqlQuery query(QStringLiteral("SELECT spec_path, MAX(priority) FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0 GROUP BY id").arg(aType),
                        QSqlDatabase::database(m_connId));
        if(query.next())
            return query.value(0).toString();
    }

    // MIME type lookup failed
    // Search for glob fitting the filename
    {
        QSqlQuery query(QStringLiteral("SELECT globs, spec_path, MAX(priority) FROM languages GROUP BY id"),
                        QSqlDatabase::database(m_connId));
        while (query.next()) {
            const QStringList &globs = query.value(0).toString().split(';');
            for (QString glob : globs) {
                // Very simple glob-to-regexp translation

                glob.replace('.', QLatin1String("\\."));
                glob.replace('?', QLatin1String("."));
                // In glob starting with *. * shouldn't match empty string
                if(glob.startsWith(QLatin1String("*\\.")))
                    glob.replace(0, 1, QStringLiteral(".+"));
                // Elsewhere it can
                glob.replace('*', QLatin1String(".*"));

                QRegularExpression regexp("^" + glob + "$");

                if(regexp.match(filename, 0).hasMatch())
                    return query.value(1).toString();
            }
        }
    }

    return QString();
}

LanguageManager::~LanguageManager() {
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

LanguageManager *LanguageManager::m_instance = nullptr;
