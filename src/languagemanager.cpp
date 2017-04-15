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

LanguageManager::LanguageManager(QObject *parent) :
    QObject(parent),
    m_connId("languages") {

    m_thread = new QThread;
    LanguageDatabaseMaintainer *dbMaintainer = new LanguageDatabaseMaintainer(m_connId);
    dbMaintainer->moveToThread(m_thread);
    connect(m_thread, &QThread::started, dbMaintainer, &LanguageDatabaseMaintainer::init);
    connect(m_thread, &QThread::finished, dbMaintainer, &LanguageDatabaseMaintainer::deleteLater);
    m_thread->start();
}

LanguageManager *LanguageManager::getInstance() {
    if(!m_instance)
        m_instance = new LanguageManager;
    return m_instance;
}

QString LanguageManager::pathForId(QString id) {
    QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                   "WHERE id = '%1'").arg(id),
                    QSqlDatabase::database(m_connId));
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
                        QSqlDatabase::database(m_connId));
        if(query.next())
            return query.value(0).toString();
    }

    // Aliases second
    for (QString aType : mimeType.aliases()) {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(aType),
                        QSqlDatabase::database(m_connId));
        if(query.next())
            return query.value(0).toString();
    }

    // Parents last
    for (QString pType : mimeType.allAncestors()) {
        QSqlQuery query(QStringLiteral("SELECT spec_path FROM languages "
                                       "WHERE instr(mime_types, '%1') > 0").arg(pType),
                        QSqlDatabase::database(m_connId));
        if(query.next())
            return query.value(0).toString();
    }

    // MIME type lookup failed
    // Search for glob fitting the filename
    {
        QSqlQuery query(QStringLiteral("SELECT globs, spec_path FROM languages"),
                        QSqlDatabase::database(m_connId));
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

LanguageManager::~LanguageManager() {
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

LanguageManager *LanguageManager::m_instance = nullptr;
