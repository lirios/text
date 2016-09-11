#include "languageloader.h"
#include <QFile>
#include <iostream>

LanguageLoader::LanguageLoader() { }

void LanguageLoader::loadFromFile(QString path) {
    QFile *file = new QFile(path);
    file->open(QFile::ReadOnly);
    QXmlStreamReader *xml = new QXmlStreamReader(file);
    while(!xml->atEnd()) {
        xml->readNext();
        if(xml->name() == "metadata")
            parseMetadata(xml);
        if(xml->name() == "context")
            parseContext(xml);
    }
    delete xml;
    file->close();
    delete file;
}

void LanguageLoader::parseMetadata(QXmlStreamReader *xml) {
    xml->skipCurrentElement();
}

void LanguageLoader::parseContext(QXmlStreamReader *xml) {
    std::cerr << "Context started\n";
    // TODO: parse attributes
    xml->readNext();
    while (xml->name() != "context" || xml->tokenType() != QXmlStreamReader::EndElement) {
        if(xml->name() == "include") {
            xml->readNext();
            while (xml->name() != "include" || xml->tokenType() != QXmlStreamReader::EndElement) {
                if(xml->name() == "context")
                    parseContext(xml);
                xml->readNext();
            }
        }
        if(xml->name() == "keyword") {
            xml->readNext();
            std::cerr << "Keyword: " << xml->text().toString().toStdString() << "\n";
            xml->readNext();
        }
        xml->readNext();
    }
    std::cerr << "Context ended\n";
}
