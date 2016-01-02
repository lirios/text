#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QFile>

class FileProcessor : public QObject
{
	Q_OBJECT
public:
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString fileContent READ fileContent WRITE setFileContent NOTIFY fileContentChanged)
    explicit FileProcessor(QObject *parent = 0);
    ~FileProcessor();
    inline QString fileName() { return filename; }
    void setFileName(QString newFileName);
    inline QString fileContent() { return content; }
    void setFileContent(QString newFileContent);
    Q_INVOKABLE bool open(QString filename);
    Q_INVOKABLE QString readAll();
    Q_INVOKABLE bool writeAll(QString data);

signals:
    void fileNameChanged();
    void fileContentChanged();
public slots:

private:
    QString filename;
    QString content;
    QFile *file;
};

#endif // FILEPROCESSOR_H
