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

#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H

#include <QObject>
#include <QQuickTextDocument>
#include <QTextCodec>
#include <QFile>
#ifndef QT_NO_FILESYSTEMWATCHER
#  include <QFileSystemWatcher>
#endif

#include "lirisyntaxhighlighter.h"

class DocumentHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle NOTIFY documentTitleChanged)
    Q_PROPERTY(bool modified READ modified NOTIFY modifiedChanged)

public:
    DocumentHandler(QObject *parent = nullptr);
    ~DocumentHandler();

    inline QQuickItem *target() { return m_target; }
    void setTarget(QQuickItem *target);

    inline QUrl fileUrl() { return m_fileUrl; }
    Q_INVOKABLE bool setFileUrl(const QUrl &fileUrl);

    inline QString text() { return m_text; }
    void setText(const QString &text);

    inline QString documentTitle() { return m_documentTitle; }
    void setDocumentTitle(const QString &title);

    inline bool modified() { return m_document->isModified(); }

    Q_INVOKABLE QString textFragment(int position, int blockCount);

signals:
    void targetChanged();
    void fileUrlChanged();
    void textChanged();
    void documentTitleChanged();
    void fileChangedOnDisk();
    void modifiedChanged();
    void error(const QString &description);

public slots:
    bool saveAs(const QUrl &filename);
    bool reloadText();

private slots:
    void fileChanged(const QString &file);

private:
    QQuickItem *m_target;
    QTextDocument *m_document;
#ifndef QT_NO_FILESYSTEMWATCHER
    // QFileSystemWatcher is not supported on all platforms like WinRT:
    // https://codereview.qt-project.org/#/c/64825/
    QFileSystemWatcher *m_watcher;
#endif
    LiriSyntaxHighlighter *m_highlighter;
    QSharedPointer<LanguageDefaultStyles> m_defStyles;

    QUrl m_fileUrl;
    QString m_text;
    QString m_documentTitle;
};

#endif // DOCUMENTHANDLER_H
