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
#include <QDebug>

DocumentHandler::DocumentHandler() :
    m_target(0),
    m_document(0) { }

DocumentHandler::~DocumentHandler() { }

void DocumentHandler::setTarget(QQuickItem *target) {
    m_document = nullptr;
    m_target = target;
    if(!m_target)
        return;

    QVariant doc = m_target->property("textDocument");
    if(doc.canConvert<QQuickTextDocument*>()) {
        QQuickTextDocument *qqdoc = doc.value<QQuickTextDocument*>();
        if(qqdoc)
            m_document = qqdoc->textDocument();
    }
    emit targetChanged();
}

void DocumentHandler::setFileUrl(QUrl fileUrl) {
    if(fileUrl != m_fileUrl) {
        m_fileUrl = fileUrl;
        QString filename = m_fileUrl.toLocalFile();
        qDebug() << m_fileUrl << filename;
        if(QFile::exists(filename)) {
            QFile file(filename);
            if(file.open(QFile::ReadOnly)) {
                QByteArray data = file.readAll();
                QTextCodec *codec = QTextCodec::codecForUtfText(data, QTextCodec::codecForLocale());
                setText(codec->toUnicode(data));
                if(m_document)
                    m_document->setModified(false);
                if(m_fileUrl.isEmpty())
                    m_documentTitle = "New Document.txt";
                else
                    m_documentTitle = QFileInfo(filename).fileName();

                //emit textChanged();
                emit documentTitleChanged();
            }
        }
        emit fileUrlChanged();
    }
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

void DocumentHandler::saveAs(QUrl filename) {
    QString localPath = filename.toLocalFile();
    QFile f(localPath);
    if(!f.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        //emit error(tr("Cannot save: ") + f.errorString());
        return;
    }
    f.write(m_document->toPlainText().toLocal8Bit());
    f.close();
    qDebug() << "saved to" << localPath;
    setFileUrl(QUrl::fromLocalFile(localPath));
}
