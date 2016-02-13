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

#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H

#include <QObject>
#include <QQuickTextDocument>
#include <QTextCodec>
#include <QFile>
#include <KDirWatch>

class DocumentHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle NOTIFY documentTitleChanged)

public:
    DocumentHandler();
    ~DocumentHandler();

    inline QQuickItem *target() { return m_target; }
    void setTarget(QQuickItem *target);

    inline QUrl fileUrl() { return m_fileUrl; }
    void setFileUrl(QUrl fileUrl);

    inline QString text() { return m_text; }
    void setText(QString text);

    inline QString documentTitle() { return m_documentTitle; }
    void setDocumentTitle(QString title);

signals:
    void targetChanged();
    void fileUrlChanged();
    void textChanged();
    void documentTitleChanged();
    void fileChangedOnDisk();

public slots:
    void saveAs(QUrl filename);

private:
    QQuickItem *m_target;
    QTextDocument *m_document;
    KDirWatch *m_watcher;

    QUrl m_fileUrl;
    QString m_text;
    QString m_documentTitle;
};

#endif // DOCUMENTHANDLER_H
