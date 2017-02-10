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

#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

const int MAX_HISTORY_SIZE = 24;
const QString NAME_KEY = "name";
const QString URL_KEY = "fileUrl";
const QString LAST_VIEW_KEY = "lastViewTime";
const QString PREVIEW_KEY = "previewStrings";
const QString CURSOR_POSITION_KEY = "cursorPosition";

HistoryManager::HistoryManager() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/history.ini";
    historyStorage = new QSettings(path, QSettings::IniFormat);
    loadHistory();
}

HistoryManager::~HistoryManager() {
    historyStorage->deleteLater();
}

int HistoryManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return history.length();
}

QVariant HistoryManager::data(const QModelIndex &index, int role) const {
    if(index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    if(role == NameRole)
        return history[index.row()].name;
    if(role == FileUrlRole)
        return history[index.row()].url;
    if(role == LastViewTimeRole)
        return history[index.row()].viewTime;
    if(role == PreviewRole)
        return history[index.row()].preview;
    if(role == CursorPositionRole)
        return history[index.row()].cursorPosition;

    return QVariant();
}

bool HistoryManager::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() < 0 || index.row() >= rowCount())
        return false;

    if(role == NameRole) {
        history[index.row()].name = value.toString();
    } else if(role == FileUrlRole) {
        history[index.row()].url = value.toUrl();
    } else if(role == LastViewTimeRole) {
        history[index.row()].viewTime = value.toDateTime();
    } else if(role == PreviewRole) {
        history[index.row()].preview = value.toString();
    } else if(role == CursorPositionRole) {
        history[index.row()].cursorPosition = value.toInt();
    } else {
        return false;
    }

    saveHistory();
    emit dataChanged(index, index, {role});
    return true;
}

bool HistoryManager::removeRow(int row, const QModelIndex &parent) {
    Q_UNUSED(parent)
    if(row < 0 || row >= rowCount())
        return false;
    emit beginRemoveRows(QModelIndex(), row, row);
    history.removeAt(row);
    saveHistory();
    emit endRemoveRows();
    return true;
}

bool HistoryManager::removeFile(QUrl fileUrl) {
    int row;
    for(row = 0; row < rowCount(); row++) {
        if(data(index(row), FileUrlRole) == fileUrl)
            break;
        if(row == rowCount() - 1)
            return false;
    }
    removeRow(row);
    qDebug() << "Entry removed, new count:" << rowCount();
    return true;
}

QString HistoryManager::prettifyPath(QUrl fileUrl, int length) const {
    if(length < 6)
        return "";

    QString path = fileUrl.path();
    if(path.length() <= length)
        return path;
    int startLen = path.indexOf('/');
    if(startLen > 0)
        startLen++;
    if(startLen > length - 6)
        startLen = length - 6;
    int endLen = length - startLen - 3;
    return path.left(startLen) + "..." + path.right(endLen);
}

QString HistoryManager::prettifyPath(QUrl fileUrl) const {
    return fileUrl.path();
}

QVariantMap HistoryManager::getFileInfo(QUrl fileUrl) const {
    for(int row = 0; row < rowCount(); row++) {
        if(data(index(row), FileUrlRole).toUrl() == fileUrl) {
            QHash<int,QByteArray> names = roleNames();
            QHashIterator<int, QByteArray> i(names);
            QVariantMap res;
            while (i.hasNext()) {
                i.next();
                QModelIndex idx = index(row, 0);
                QVariant data = idx.data(i.key());
                res[i.value()] = data;
            }
            return res;
        }
    }
    return QVariantMap();
}

Qt::ItemFlags HistoryManager::flags(const QModelIndex &index) const {
    Q_UNUSED(index)
    return {Qt::ItemIsEnabled, Qt::ItemIsSelectable, Qt::ItemIsEditable};
}

void HistoryManager::touchFile(QString name, QUrl fileUrl, int cursorPosition, QString preview) {
    int fileIndex;
    for(fileIndex = 0; fileIndex < rowCount(); fileIndex++) {
        if(data(index(fileIndex), FileUrlRole).toUrl() == fileUrl) {
            break;
        }
    }
    if(fileIndex >= MAX_HISTORY_SIZE) {
        int oldest = 0;
        QDateTime oldestDT = QDateTime::currentDateTime();
        for(int i = 1; i < rowCount(); i++) {
            QDateTime dt = data(index(i), LastViewTimeRole).toDateTime();
            if(dt < oldestDT) {
                oldestDT = dt;
                oldest = i;
            }
        }
        removeRow(oldest);
    }

    FileData file;
    file.name = name;
    file.url = fileUrl;
    file.viewTime = QDateTime::currentDateTime();
    file.preview = preview;
    file.cursorPosition = cursorPosition;
    if(fileIndex >= rowCount()) {
        emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
        history.append(file);
        saveHistory();
        emit endInsertRows();
    } else {
        history[fileIndex] = file;
        saveHistory();
        emit dataChanged(index(fileIndex), index(fileIndex), {NameRole, FileUrlRole, LastViewTimeRole, PreviewRole, CursorPositionRole});
    }
}

QHash<int, QByteArray> HistoryManager::roleNames() const {
    return QHash<int, QByteArray>({ {NameRole, "name"},
                                    {FileUrlRole, "fileUrl"},
                                    {LastViewTimeRole, "lastViewTime"},
                                    {PreviewRole, "previewText"},
                                    {CursorPositionRole, "cursorPosition"}
                                  });
}

void HistoryManager::loadHistory() {
    history = QList<FileData>();
    int size = historyStorage->beginReadArray("recentFiles");
    for(int i = 0; i < size; i++) {
        historyStorage->setArrayIndex(i);
        FileData file;
        file.name = historyStorage->value(NAME_KEY).toString();
        file.url = historyStorage->value(URL_KEY).toUrl();
        file.viewTime = historyStorage->value(LAST_VIEW_KEY).toDateTime();
        file.preview = historyStorage->value(PREVIEW_KEY).toString();
        file.cursorPosition = historyStorage->value(CURSOR_POSITION_KEY).toInt();
        history.append(file);
    }
    historyStorage->endArray();
}

void HistoryManager::saveHistory() {
    historyStorage->beginWriteArray("recentFiles", history.length());
    for(int i = 0; i < history.length(); i++) {
        historyStorage->setArrayIndex(i);
        historyStorage->setValue(NAME_KEY, history[i].name);
        historyStorage->setValue(URL_KEY, history[i].url);
        historyStorage->setValue(LAST_VIEW_KEY, history[i].viewTime);
        historyStorage->setValue(PREVIEW_KEY, history[i].preview);
        historyStorage->setValue(CURSOR_POSITION_KEY, history[i].cursorPosition);
    }
    historyStorage->endArray();
}
