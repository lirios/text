#include "fileprocessor.h"
#include <QDebug>

FileProcessor::FileProcessor(QObject *parent) : QObject(parent) {
    file = new QFile();
    qDebug() << "init";
}

FileProcessor::~FileProcessor() {
    file->close();
    delete file;
}

void FileProcessor::setFileName(QString newFileName) {
    if(newFileName != filename) {
        file->close();
        qDebug() << "setting file name" << newFileName;
        file->setFileName(newFileName);
        qDebug() << file->open(QIODevice::ReadWrite | QIODevice::Text);
        filename = newFileName;
        emit fileNameChanged();
        content = readAll();
        emit fileContentChanged();
    }
}

void FileProcessor::setFileContent(QString newFileContent) {
    if(newFileContent != content) {
        if(writeAll(newFileContent)) {
            content = readAll();
            emit fileContentChanged();
        }
    }
}

bool FileProcessor::open(QString filename) {
    file = new QFile(filename);
    qDebug() << filename << file->exists();
    return file->open(QIODevice::ReadWrite | QIODevice::Text);
}

QString FileProcessor::readAll() {
    if(!file->isReadable())
        return QString();
    file->close();
    file->open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream rStr(file);
    QString fileContent = rStr.readAll();
    rStr.reset();
    qDebug() << "Content:" << fileContent;
    return fileContent;
}

bool FileProcessor::writeAll(QString data) {
    if(!file->isWritable())
        return false;
    file->resize(0);
    QTextStream wStr(file);
    wStr << data;
    wStr.reset();
    file->reset();
    return true;
}
