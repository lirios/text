#include "languageloader.h"
#include <QFile>
#include <iostream>

LanguageLoader::LanguageLoader() { }

LanguageSpecification *LanguageLoader::loadFromFile(QString path) {
    LanguageSpecification *result = new LanguageSpecification();
    QFile *file = new QFile(path);
    file->open(QFile::ReadOnly);
    QXmlStreamReader *xml = new QXmlStreamReader(file);
    while (!xml->atEnd()) {
        xml->readNext();
        if(xml->name() == "metadata")
            parseMetadata(result, xml);
        if(xml->name() == "context")
            parseContext(result, xml);
    }
    delete xml;
    file->close();
    delete file;
    return result;
}

void LanguageLoader::parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml) {
    xml->skipCurrentElement();
}

void LanguageLoader::parseContext(LanguageSpecification *lang, QXmlStreamReader *xml) {
    std::cerr << "Context started\n";
    // TODO: parse attributes
    xml->readNext();
    while (xml->name() != "context" || xml->tokenType() != QXmlStreamReader::EndElement) {
        if(xml->name() == "include") {
            xml->readNext();
            while (xml->name() != "include" || xml->tokenType() != QXmlStreamReader::EndElement) {
                if(xml->name() == "context")
                    parseContext(lang, xml);
                xml->readNext();
            }
        }
        if(xml->name() == "keyword") {
            xml->readNext();
            lang->keywords.insert(QRegExp("\\b(" + xml->text().toString() + ")\\b"), "keyword");
            xml->readNext();
        }
        xml->readNext();
    }
    std::cerr << "Context ended\n";
}
