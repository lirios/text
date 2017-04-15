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

HistoryManager::HistoryManager() :
    m_connId("history") {

    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!dataDir.exists())
        dataDir.mkpath(".");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connId);
    db.setDatabaseName(dataDir.filePath("history.db"));
    db.open();
    QSqlQuery("CREATE TABLE IF NOT EXISTS history "
              "(path TEXT PRIMARY KEY, display_name TEXT, last_view_time INTEGER, preview TEXT, cursor_position INTEGER, scroll_position REAL)",
              db);
}

HistoryManager::~HistoryManager() {
    QSqlDatabase::removeDatabase(m_connId);
}

HistoryManager *HistoryManager::getInstance() {
    if(!m_instance)
        m_instance = new HistoryManager();
    return m_instance;
}

int HistoryManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.exec("SELECT Count(*) FROM history");
    if(query.first())
        return query.value(0).toInt();
    else
        return 0;
}

QVariant HistoryManager::data(const QModelIndex &index, int role) const {
    if(index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("SELECT %1 FROM history "
                                 "ORDER BY last_view_time DESC").arg(
                      dbColumnFromRole(role)));
    query.exec();
    if(query.seek(index.row())) {
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

bool HistoryManager::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() < 0 || index.row() >= rowCount())
        return false;

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare(QStringLiteral("UPDATE history SET "
                                 "%1=? WHERE path=?").arg(
                      dbIdForIndex(index.row())));
    query.addBindValue(dbColumnFromRole(role));
    query.addBindValue(value.toString());
    query.exec();

    emit dataChanged(index, index, {role});
    return true;
}

bool HistoryManager::removeRow(int row, const QModelIndex &parent) {
    Q_UNUSED(parent)
    if(row < 0 || row >= rowCount())
        return false;
    emit beginRemoveRows(QModelIndex(), row, row);

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare("DELETE FROM history "
                  "WHERE path=?");
    query.addBindValue(dbIdForIndex(row));
    query.exec();

    emit endRemoveRows();
    emit countChanged();
    return true;
}

bool HistoryManager::removeFile(QUrl fileUrl) {
    int row = dbIndexForId(fileUrl.path());
    emit beginRemoveRows(QModelIndex(), row, row);

    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare("DELETE FROM history "
                  "WHERE path=?");
    query.addBindValue(fileUrl.path());
    query.exec();

    emit endRemoveRows();
    emit countChanged();
    return true;
}

Qt::ItemFlags HistoryManager::flags(const QModelIndex &index) const {
    Q_UNUSED(index)
    return {Qt::ItemIsEnabled, Qt::ItemIsSelectable, Qt::ItemIsEditable};
}

QVariantMap HistoryManager::getFileEditingInfo(QUrl fileUrl) const {
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare("SELECT cursor_position, scroll_position FROM history "
                  "WHERE path=?");
    query.addBindValue(fileUrl.path());
    query.exec();
    QVariantMap result;
    if(query.first()) {
        result["cursorPosition"] = query.value(0);
        result["scrollPosition"] = query.value(1);
    }
    return result;
}

void HistoryManager::touchFile(QString name, QUrl fileUrl, int cursorPosition, float scrollPosition, QString preview) {
    int currentTime = QDateTime::currentDateTime().toSecsSinceEpoch();

    int row = dbIndexForId(fileUrl.path());
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare("UPDATE history SET "
                  "path=?, display_name=?, last_view_time=?, preview=?, cursor_position=?, scroll_position=? "
                  "WHERE path=?");
    query.addBindValue(fileUrl.path());
    query.addBindValue(name);
    query.addBindValue(currentTime);
    query.addBindValue(preview);
    query.addBindValue(cursorPosition);
    query.addBindValue(scrollPosition);
    query.addBindValue(fileUrl.path());
    query.exec();

    query.exec("SELECT Changes() FROM history");
    if(!query.first() || query.value(0) == 0) {
        // If update failed, insert
        emit beginInsertRows(QModelIndex(), 0, 0);
        query.prepare("INSERT INTO history "
                      "(path, display_name, last_view_time, preview, cursor_position, scroll_position) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(fileUrl.path());
        query.addBindValue(name);
        query.addBindValue(currentTime);
        query.addBindValue(preview);
        query.addBindValue(cursorPosition);
        query.addBindValue(scrollPosition);
        query.exec();

        emit endInsertRows();

        int entryCount = rowCount();
        if(entryCount > MAX_HISTORY_SIZE) {
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
        emit dataChanged(index(row), index(row), {NameRole, LastViewTimeRole, PreviewRole, CursorPositionRole, ScrollPositionRole});
        if(row > 0) {
            emit beginMoveRows(QModelIndex(), row, row, QModelIndex(), 0);
            emit endMoveRows();
        }
    }
}

QHash<int, QByteArray> HistoryManager::roleNames() const {
    return QHash<int, QByteArray>({ {NameRole, "name"},
                                    {FileUrlRole, "fileUrl"},
                                    {FilePathRole, "filePath"},
                                    {LastViewTimeRole, "lastViewTime"},
                                    {PreviewRole, "previewText"},
                                    {CursorPositionRole, "cursorPosition"},
                                    {ScrollPositionRole, "scrollPosition"}
                                  });
}

QString HistoryManager::dbColumnFromRole(int role) const {
    switch (role) {
    case NameRole:
        return "display_name";
        break;
    case FileUrlRole:
    case FilePathRole:
        return "path";
        break;
    case LastViewTimeRole:
        return "last_view_time";
        break;
    case PreviewRole:
        return "preview";
        break;
    case CursorPositionRole:
        return "cursor_position";
        break;
    case ScrollPositionRole:
        return "scroll_position";
        break;
    default:
        return "";
    }
}

QString HistoryManager::dbIdForIndex(int index) const {
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.exec("SELECT path FROM history "
               "ORDER BY last_view_time DESC");
    if(query.seek(index))
        return query.value(0).toString();
    return "";
}

int HistoryManager::dbIndexForId(QString id) const {
    QSqlQuery query(QSqlDatabase::database(m_connId));
    query.prepare("SELECT last_view_time FROM history "
                  "WHERE path=?");
    query.addBindValue(id);
    query.exec();
    if(query.first()) {
        int time = query.value(0).toInt();
        query.prepare("SELECT Count(*) FROM history "
                      "WHERE last_view_time>?");
        query.addBindValue(time);
        query.exec();
        if(query.first())
            return query.value(0).toInt();
    }

    return 0;
}

HistoryManager *HistoryManager::m_instance = nullptr;
