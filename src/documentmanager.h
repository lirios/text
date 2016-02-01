#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>
#include <QQuickTextDocument>
#include <QTextCodec>
#include <QFile>

class DocumentManager : public QObject
{
	Q_OBJECT
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle NOTIFY documentTitleChanged)

public:
    DocumentManager();
    ~DocumentManager();

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

public slots:
    void saveAs(QUrl filename);

private:
    QQuickItem *m_target;
    QTextDocument *m_document;

    QUrl m_fileUrl;
    QString m_text;
    QString m_documentTitle;
};

#endif // DOCUMENTMANAGER_H
