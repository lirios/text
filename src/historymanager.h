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

#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QHash>
#include <QSettings>
#include <QUrl>
#include <QDateTime>

class HistoryManager : public QAbstractListModel
{
    Q_OBJECT
public:
    enum HistoryRoles {
        NameRole = Qt::UserRole + 1,
        FileUrlRole,
        LastViewTimeRole,
        PreviewRole,
        CursorPositionRole
    };

    HistoryManager();
    ~HistoryManager();

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    inline int count() { return rowCount(); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    Q_INVOKABLE bool removeFile(QUrl fileUrl);
    Q_INVOKABLE QString prettifyPath(QUrl fileUrl, int length) const;
    Q_INVOKABLE QString prettifyPath(QUrl fileUrl) const;
    Q_INVOKABLE QVariantMap getFileInfo(QUrl fileUrl) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

signals:
    void countChanged();

public slots:
    void touchFile(QString name, QUrl fileUrl, int cursorPosition, QString preview);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QSettings *historyStorage;

    struct FileData {
        QString name;
        QUrl url;
        QDateTime viewTime;
        QString preview;
        int cursorPosition;
    };
    QList<FileData> history;

    void loadHistory();
    void saveHistory();
};

#endif // HISTORYMANAGER_H
