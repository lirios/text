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

#include "historymanager.h"

#include <QSqlDatabase>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QDebug>

const int MAX_HISTORY_SIZE = 24;

HistoryManager::HistoryManager(QObject *parent)
    : QAbstractListModel(parent)
    , m_connId(QStringLiteral("history"))
{

    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dataDir.exists())
        dataDir.mkpath(QStringLiteral("."));

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connId);
    db.setDatabaseName(dataDir.filePath(QStringLiteral("history.db")));
    db.open();
    QSqlQuery(QStringLiteral("CREATE TABLE IF NOT EXISTS history "
                             "(path TEXT PRIMARY KEY, display_name TEXT, last_view_time INTEGER, "
                             "preview TEXT, cursor_position INTEGER, scroll_position REAL)"),
              db);
}

HistoryManager::~HistoryManager()
{
    QSqlDatabase::removeDatabase(m_connId);
}

HistoryManager *HistoryManager::getInstance()
{
    if (!m_instance)
        m_instance = new HistoryManager();
    return m_instance;
}

int HistoryManager::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.exec(QStringLiteral("SELECT Count(*) FROM history"));
    if (query.first())
        return query.value(0).toInt();
    else
        return 0;
}

QVariant HistoryManager::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("SELECT %1 FROM history "
                                 "ORDER BY last_view_time DESC")
                      .arg(dbColumnFromRole(role)));
    query.exec();
    if (query.seek(index.row())) {
        switch (role) {
        case FileUrlRole:
            return QUrl::fromLocalFile(query.value(0).toString());
            break;
        case LastViewTimeRole:
            return QDateTime::fromSecsSinceEpoch(query.value(0).toInt());
            break;
        default:
            return query.value(0);
            break;
        }
    }

    return QVariant();
}

bool HistoryManager::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= rowCount())
        return false;

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("UPDATE history SET "
                                 "%1=? WHERE path=?")
                      .arg(dbIdForIndex(index.row())));
    query.addBindValue(dbColumnFromRole(role));
    query.addBindValue(value.toString());
    query.exec();

    emit dataChanged(index, index, { role });
    return true;
}

bool HistoryManager::removeRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (row < 0 || row >= rowCount())
        return false;
    beginRemoveRows(QModelIndex(), row, row);

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("DELETE FROM history "
                                 "WHERE path=?"));
    query.addBindValue(dbIdForIndex(row));
    query.exec();

    endRemoveRows();
    emit countChanged();
    return true;
}

bool HistoryManager::removeFile(const QUrl &fileUrl)
{
    int row = dbIndexForId(fileUrl.path());
    beginRemoveRows(QModelIndex(), row, row);

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("DELETE FROM history "
                                 "WHERE path=?"));
    query.addBindValue(fileUrl.path());
    query.exec();

    endRemoveRows();
    emit countChanged();
    return true;
}

Qt::ItemFlags HistoryManager::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return { Qt::ItemIsEnabled, Qt::ItemIsSelectable, Qt::ItemIsEditable };
}

QVariantMap HistoryManager::getFileEditingInfo(const QUrl &fileUrl) const
{
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("SELECT cursor_position, scroll_position FROM history "
                                 "WHERE path=?"));
    query.addBindValue(fileUrl.path());
    query.exec();
    QVariantMap result;
    if (query.first()) {
        result[QStringLiteral("cursorPosition")] = query.value(0);
        result[QStringLiteral("scrollPosition")] = query.value(1);
    }
    return result;
}

void HistoryManager::touchFile(const QString &name, const QUrl &fileUrl, int cursorPosition,
                               float scrollPosition, const QString &preview)
{
    qint64 currentTime = QDateTime::currentDateTime().toSecsSinceEpoch();

    int row = dbIndexForId(fileUrl.path());
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral(
        "UPDATE history SET "
        "path=?, display_name=?, last_view_time=?, preview=?, cursor_position=?, scroll_position=? "
        "WHERE path=?"));
    query.addBindValue(fileUrl.path());
    query.addBindValue(name);
    query.addBindValue(currentTime);
    query.addBindValue(preview);
    query.addBindValue(cursorPosition);
    query.addBindValue(scrollPosition);
    query.addBindValue(fileUrl.path());
    query.exec();

    query.exec(QStringLiteral("SELECT Changes() FROM history"));
    if (!query.first() || query.value(0) == 0) {
        // If update failed, insert
        beginInsertRows(QModelIndex(), 0, 0);
        query.prepare(QStringLiteral(
            "INSERT INTO history "
            "(path, display_name, last_view_time, preview, cursor_position, scroll_position) "
            "VALUES (?, ?, ?, ?, ?, ?)"));
        query.addBindValue(fileUrl.path());
        query.addBindValue(name);
        query.addBindValue(currentTime);
        query.addBindValue(preview);
        query.addBindValue(cursorPosition);
        query.addBindValue(scrollPosition);
        query.exec();

        endInsertRows();

        int entryCount = rowCount();
        if (entryCount > MAX_HISTORY_SIZE) {
            removeRow(entryCount - 1);
            /*
            query.exec("SELECT path FROM history "
                       "ORDER BY last_view_time ASC");
            if(query.first()) {
                QString id = query.value(0).toString();
                emit beginRemoveRows(QModelIndex(), entryCount - 1, entryCount - 1);
                query.prepare("DELETE FROM history "
                              "WHERE path=?");
                query.addBindValue(id);
                query.exec();
                emit endRemoveRows();
            }
            */
        } else
            emit countChanged();
    } else {
        emit dataChanged(
            index(row), index(row),
            { NameRole, LastViewTimeRole, PreviewRole, CursorPositionRole, ScrollPositionRole });
        if (row > 0) {
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), 0);
            endMoveRows();
        }
    }
}

QHash<int, QByteArray> HistoryManager::roleNames() const
{
    return QHash<int, QByteArray>({ { NameRole, "name" },
                                    { FileUrlRole, "fileUrl" },
                                    { FilePathRole, "filePath" },
                                    { LastViewTimeRole, "lastViewTime" },
                                    { PreviewRole, "previewText" },
                                    { CursorPositionRole, "cursorPosition" },
                                    { ScrollPositionRole, "scrollPosition" } });
}

QString HistoryManager::dbColumnFromRole(int role) const
{
    switch (role) {
    case NameRole:
        return QStringLiteral("display_name");
        break;
    case FileUrlRole:
    case FilePathRole:
        return QStringLiteral("path");
        break;
    case LastViewTimeRole:
        return QStringLiteral("last_view_time");
        break;
    case PreviewRole:
        return QStringLiteral("preview");
        break;
    case CursorPositionRole:
        return QStringLiteral("cursor_position");
        break;
    case ScrollPositionRole:
        return QStringLiteral("scroll_position");
        break;
    default:
        return QLatin1String("");
    }
}

QString HistoryManager::dbIdForIndex(int index) const
{
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.exec(QStringLiteral("SELECT path FROM history "
                              "ORDER BY last_view_time DESC"));
    if (query.seek(index))
        return query.value(0).toString();
    return QLatin1String("");
}

int HistoryManager::dbIndexForId(const QString &id) const
{
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("SELECT last_view_time FROM history "
                                 "WHERE path=?"));
    query.addBindValue(id);
    query.exec();
    if (query.first()) {
        int time = query.value(0).toInt();
        query.prepare(QStringLiteral("SELECT Count(*) FROM history "
                                     "WHERE last_view_time>?"));
        query.addBindValue(time);
        query.exec();
        if (query.first())
            return query.value(0).toInt();
    }

    return 0;
}

HistoryManager *HistoryManager::m_instance = nullptr;
