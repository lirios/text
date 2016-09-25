#include "languageloader.h"
#include <QFile>
#include <iostream>
#include <QDebug>

LanguageLoader::LanguageLoader() { }

LanguageSpecification *LanguageLoader::loadById(QString name) {
    std::cerr << "Loading " << name.toStdString() << "\n";
    // TODO: Build languages database, querry it here for path to spec
    return loadFromFile(QString("/usr/share/gtksourceview-3.0/language-specs/%1.lang").arg(name));
}

LanguageSpecification *LanguageLoader::loadFromFile(QString path) {
    LanguageSpecification *result = new LanguageSpecification();
    QFile file(path);
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.name() == "language" && xml.tokenType() == QXmlStreamReader::StartElement)
                result->name = xml.attributes().value("id").toString();
            if(xml.name() == "metadata")
                parseMetadata(result, &xml);
            if(xml.name() == "context") {
                parseContext(result, &xml);
            }
        }
    }
    file.close();
    return result;
}

void LanguageLoader::parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml) {
    xml->skipCurrentElement();
}

QString LanguageLoader::parseContext(LanguageSpecification *lang, QXmlStreamReader *xml) {
    QString id = xml->attributes().value("id").toString();
    if(xml->attributes().hasAttribute("ref")) {
        QStringRef refId = xml->attributes().value("ref");
        if(refId.contains(':') && !lang->contexts.keys().contains(refId.toString())) {
            lang->mergeLanguage(loadById(refId.left(refId.indexOf(':')).toString()));
        }
        lang->contexts[id] = lang->contexts[refId.toString()];
    } else {
        lang->contexts[id] = new LanguageContext();
    }
    xml->readNext();
    while (xml->name() != "context" || xml->tokenType() != QXmlStreamReader::EndElement) {
        if(xml->name() == "include") {
            xml->readNext();
            while (xml->name() != "include" || xml->tokenType() != QXmlStreamReader::EndElement) {
                if(xml->name() == "context") {
                    QString includeId = parseContext(lang, xml);
                    lang->contexts[id]->includes.append(lang->contexts[includeId]);
                }
                xml->readNext();
            }
        }
        if(xml->name() == "keyword") {
            xml->readNext();
            lang->contexts[id]->keywords.insert(xml->text().toString(), "keyword");
            xml->readNext();
        }
        xml->readNext();
    }
    return id;
}
