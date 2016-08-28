#include "languageloader.h"
#include <QFile>
#include <QXmlStreamReader>
#include <iostream>

LanguageLoader::LanguageLoader() { }

void LanguageLoader::loadFromFile(QString path) {
    QFile *file = new QFile(path);
    file->open(QFile::ReadOnly);
    QXmlStreamReader *xml = new QXmlStreamReader(file);
    while(!xml->atEnd()) {
        xml->readNext();
        std::cerr << xml->name().toString().toStdString() << "\n";
        if(xml->name() == "start")
            std::cerr << xml->readElementText().toStdString() << "\n";
    }
    delete xml;
    file->close();
    delete file;
}
