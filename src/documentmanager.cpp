#include "documentmanager.h"

#include <QTextDocument>
#include <QFileInfo>
#include <QDebug>

DocumentManager::DocumentManager() :
    m_target(0),
    m_document(0) { }

DocumentManager::~DocumentManager() { }

void DocumentManager::setTarget(QQuickItem *target) {
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

void DocumentManager::setFileUrl(QUrl fileUrl) {
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
                    m_document->setModified();
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

void DocumentManager::setDocumentTitle(QString title) {
    if(title != m_documentTitle) {
        m_documentTitle = title;
        emit documentTitleChanged();
    }
}

void DocumentManager::setText(QString text) {
    if(text != m_text) {
        m_text = text;
        emit textChanged();
    }
}

void DocumentManager::saveAs(QUrl filename) {
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
