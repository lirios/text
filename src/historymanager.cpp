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

#include "historymanager.h"

#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

const int MAX_HISTORY_SIZE = 24;
const QString NAME_KEY = "name";
const QString URL_KEY = "fileUrl";
const QString LAST_VIEW_KEY = "lastViewTime";
const QString PREVIEW_KEY = "previewStrings";

HistoryManager::HistoryManager() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/history.ini";
    history = new QSettings(path, QSettings::IniFormat);
}

HistoryManager::~HistoryManager() { }

int HistoryManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return history->childGroups().count();
}

QVariant HistoryManager::data(const QModelIndex &index, int role) const {
    if(index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QVariant val = QVariant();
    history->beginGroup(QString::number(index.row()));

    if(role == NameRole)
        val = history->value(NAME_KEY);
    if(role == FileUrlRole)
        val = history->value(URL_KEY);
    if(role == LastViewTimeRole)
        val = history->value(LAST_VIEW_KEY);
    if(role == PreviewRole)
        val = history->value(PREVIEW_KEY);

    history->endGroup();
    return val;
}

bool HistoryManager::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() < 0 || index.row() >= rowCount())
        return false;

    history->beginGroup(QString::number(index.row()));
    if(role == NameRole) {
        history->setValue(NAME_KEY, value);
        history->endGroup();
        emit dataChanged(index, index, {role});
        return true;
    }
    if(role == FileUrlRole) {
        history->setValue(URL_KEY, value);
        history->endGroup();
        emit dataChanged(index, index, {role});
        return true;
    }
    if(role == LastViewTimeRole) {
        history->setValue(LAST_VIEW_KEY, value);
        history->endGroup();
        emit dataChanged(index, index, {role});
        return true;
    }
    if(role == PreviewRole) {
        history->setValue(PREVIEW_KEY, value);
        history->endGroup();
        emit dataChanged(index, index, {role});
        return true;
    }

    history->endGroup();
    return false;
}

bool HistoryManager::removeFile(QUrl fileUrl) {
    int row;
    for(row = 0; row < rowCount(); row++) {
        if(data(index(row), FileUrlRole) == fileUrl)
            break;
        if(row == rowCount() - 1)
            return false;
    }
    emit beginRemoveRows(QModelIndex(), row, row);
    for(int i = row + 1; i < rowCount(); i++) {
        history->setValue(QString::number(i - 1) + "/" + NAME_KEY, history->value(QString::number(i) + "/" + NAME_KEY));
        history->setValue(QString::number(i - 1) + "/" + URL_KEY, history->value(QString::number(i) + "/" + URL_KEY));
        history->setValue(QString::number(i - 1) + "/" + LAST_VIEW_KEY, history->value(QString::number(i) + "/" + LAST_VIEW_KEY));
        history->setValue(QString::number(i - 1) + "/" + PREVIEW_KEY, history->value(QString::number(i) + "/" + PREVIEW_KEY));
    }
    for(int i = rowCount() - 1; i < rowCount(); i++) {
        history->beginGroup(QString::number(i));
        foreach (QString key, history->childKeys()) {
            history->remove(key);
        }
        history->endGroup();
    }
    emit endRemoveRows();
    qDebug() << "Entry removed, new count:" << rowCount();
    return true;
}

QString HistoryManager::prettifyPath(QUrl fileUrl, int length) {
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

QString HistoryManager::prettifyPath(QUrl fileUrl) {
    return fileUrl.path();
}

Qt::ItemFlags HistoryManager::flags(const QModelIndex &index) const {
    Q_UNUSED(index)
    return {Qt::ItemIsEnabled, Qt::ItemIsSelectable, Qt::ItemIsEditable};
}

void HistoryManager::touchFile(QString name, QUrl fileUrl, QStringList someStrings) {
    for(int i = 0; i < rowCount(); i++) {
        if(data(index(i), FileUrlRole).toUrl() == fileUrl) {
            setData(index(i), QDateTime::currentDateTime(), LastViewTimeRole);
            return;
        }
    }
    if(rowCount() >= MAX_HISTORY_SIZE) {
        int oldest = 0;
        QDateTime oldestDT = QDateTime::currentDateTime();
        for(int i = 1; i < rowCount(); i++) {
            QDateTime dt = data(index(i), LastViewTimeRole).toDateTime();
            if(dt < oldestDT) {
                oldestDT = dt;
                oldest = i;
            }
        }
        removeFile(data(index(oldest), FileUrlRole).toUrl());
    }

    emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
    history->beginGroup(QString::number(rowCount()));
    history->setValue(NAME_KEY, name);
    history->setValue(URL_KEY, fileUrl);
    history->setValue(LAST_VIEW_KEY, QDateTime::currentDateTime());
    history->setValue(PREVIEW_KEY, someStrings);
    history->endGroup();
    emit endInsertRows();
}

QHash<int, QByteArray> HistoryManager::roleNames() const {
    return QHash<int, QByteArray>({ {NameRole, "name"},
                                    {FileUrlRole, "fileUrl"},
                                    {LastViewTimeRole, "lastViewTime"},
                                    {PreviewRole, "previewText"}
                                  });
}
