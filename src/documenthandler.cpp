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

#include "documenthandler.h"

#include <QTextDocument>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>
#include "languageloader.h"
#include "languagemanager.h"

DocumentHandler::DocumentHandler() :
    m_target(0),
    m_document(0),
    m_highlighter(0) {

    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

    m_defStyles = QSharedPointer<LanguageDefaultStyles>::create();
}

DocumentHandler::~DocumentHandler() {
    delete m_watcher;
    delete m_highlighter;
}

void DocumentHandler::setTarget(QQuickItem *target) {
    m_document = nullptr;
    m_target = target;
    if(!m_target)
        return;

    QVariant doc = m_target->property("textDocument");
    if(doc.canConvert<QQuickTextDocument*>()) {
        QQuickTextDocument *qqdoc = doc.value<QQuickTextDocument*>();
        if(qqdoc) {
            m_document = qqdoc->textDocument();
            connect(m_document, SIGNAL(modificationChanged(bool)), this, SIGNAL(modifiedChanged()));
            if(m_highlighter != nullptr)
                delete m_highlighter;
            m_highlighter = new LiriSyntaxHighlighter(m_document);
            m_highlighter->setDefaultStyle(m_defStyles);
        }
    }
    emit targetChanged();
}

bool DocumentHandler::setFileUrl(QUrl fileUrl) {
    if(fileUrl != m_fileUrl) {
        if(m_watcher->files().contains(m_fileUrl.toLocalFile()))
            m_watcher->removePath(m_fileUrl.toLocalFile());
        m_watcher->addPath(fileUrl.toLocalFile());
        m_fileUrl = fileUrl;
        QString filename = m_fileUrl.toLocalFile();
        qDebug() << m_fileUrl << filename;
        QFile file(filename);
        if(!file.open(QFile::ReadOnly)) {
            emit error(file.errorString());
            return false;
        }
        QByteArray data = file.readAll();
        if(file.error() != QFileDevice::NoError) {
            emit error(file.errorString());
            file.close();
            return false;
        }
        QTextCodec *codec = QTextCodec::codecForUtfText(data, QTextCodec::codecForLocale());
        setText(codec->toUnicode(data));
        if(m_document) {
            m_document->setModified(false);

            // Enable syntax highlighting
            QMimeDatabase db;
            QMimeType mimeType = db.mimeTypeForFileNameAndData(m_fileUrl.toString(), data);
            LanguageLoader ll(m_defStyles);
            m_highlighter->setLanguage(ll.loadMainContextByMimeType(mimeType, m_fileUrl.fileName()));
        }
        if(m_fileUrl.isEmpty())
            m_documentTitle = "New Document";
        else
            m_documentTitle = QFileInfo(filename).fileName();

        emit documentTitleChanged();
        emit fileUrlChanged();
    }
    return true;
}

void DocumentHandler::setDocumentTitle(QString title) {
    if(title != m_documentTitle) {
        m_documentTitle = title;
        emit documentTitleChanged();
    }
}

void DocumentHandler::setText(QString text) {
    if(text != m_text) {
        m_text = text;
        emit textChanged();
    }
}

bool DocumentHandler::saveAs(QUrl filename) {
    // Stop monitoring file while saving
    if(m_watcher->files().contains(m_fileUrl.toLocalFile()))
        m_watcher->removePath(m_fileUrl.toLocalFile());

    bool success = true;
    QString localPath = filename.toLocalFile();
    QFile file(localPath);
    if(!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        emit error(file.errorString());
        success = false;
    } else {
        if(file.write(m_document->toPlainText().toLocal8Bit()) == -1) {
            emit error(file.errorString());
            success = false;
        }
        file.close();
        qDebug() << "saved to" << localPath;
        setFileUrl(QUrl::fromLocalFile(localPath));

        m_document->setModified(false);
    }

    // Restart file watcher back after saving completes
    if(!m_watcher->files().contains(m_fileUrl.toLocalFile()))
        m_watcher->addPath(m_fileUrl.toLocalFile());

    return success;
}

bool DocumentHandler::reloadText() {
    QString filename = m_fileUrl.toLocalFile();
    QFile file(filename);
    if(!file.open(QFile::ReadOnly)) {
        emit error(file.errorString());
        return false;
    }
    QByteArray data = file.readAll();
    QTextCodec *codec = QTextCodec::codecForUtfText(data, QTextCodec::codecForLocale());
    setText(codec->toUnicode(data));
    if(file.error() == QFileDevice::NoError) {
        file.close();
        return true;
    } else {
        emit error(file.errorString());
        file.close();
        return false;
    }
}

void DocumentHandler::fileChanged(QString file) {
    emit fileChangedOnDisk();
    if(!m_watcher->files().contains(file))
        m_watcher->addPath(file);
}
