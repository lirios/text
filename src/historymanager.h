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
#include <QUrl>
#include <QDateTime>

class HistoryManager : public QAbstractListModel
{
    Q_OBJECT
public:
    enum HistoryRoles {
        NameRole = Qt::UserRole + 1,
        FileUrlRole,
        FilePathRole,
        LastViewTimeRole,
        PreviewRole,
        CursorPositionRole,
        ScrollPositionRole
    };

    static HistoryManager *getInstance();

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    inline int count() const { return rowCount(); }
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    Q_INVOKABLE bool removeFile(QUrl fileUrl);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap getFileEditingInfo(QUrl fileUrl) const;

signals:
    void countChanged();

public slots:
    void touchFile(QString name, QUrl fileUrl, int cursorPosition, float scrollPosition, QString preview);

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    HistoryManager();
    ~HistoryManager();
    static HistoryManager *m_instance;

    QString m_connId;

    QString dbColumnFromRole(int role) const;
    QString dbIdForIndex(int index) const;
    int dbIndexForId(QString id) const;
};

#endif // HISTORYMANAGER_H
