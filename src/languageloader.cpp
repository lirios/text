#include "languageloader.h"
#include <QFile>
#include <iostream>
#include <QDebug>
#include "languagecontextelementkeyword.h"

LanguageLoader::LanguageLoader() :
    knownContexts() { }

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
                LanguageContext *context = parseContext(xml, result->name);
                knownContexts[result->name + ":" + context->id] = context;
            }
        }
    }
    file.close();
    QString mainId = result->name + ":" + result->name;
    if(knownContexts.keys().contains(mainId))
        result->mainContext = knownContexts[mainId]->elements;
    return result;
}

void LanguageLoader::parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml) {
    xml->skipCurrentElement();
}

LanguageContext *LanguageLoader::parseContext(QXmlStreamReader &xml, QString langId) {
    LanguageContext *result = nullptr;
    QString id = xml.attributes().value("id").toString();
    if(xml.attributes().hasAttribute("ref")) {
        QStringRef refId = xml.attributes().value("ref");
        if(refId.contains(':') && !knownContexts.keys().contains(refId.toString())) {
            loadById(refId.left(refId.indexOf(':')).toString());
        }
        QString refIdCopy = refId.toString();
        if(!refIdCopy.contains(':'))
            refIdCopy = langId + ":" + refIdCopy;
        if(knownContexts.keys().contains(refIdCopy))
            result = knownContexts[refIdCopy];
    }
    if(!result) {
        result = new LanguageContext();
    }
    if(id != "")
        result->id = id;
    xml.readNext();
    while (xml.name() != "context" || xml.tokenType() != QXmlStreamReader::EndElement) {
        if(xml.name() == "include") {
            xml.readNext();
            while (xml.name() != "include" || xml.tokenType() != QXmlStreamReader::EndElement) {
                if(xml.name() == "context") {
                    LanguageContext *inc = parseContext(xml, langId);
                    result->include(inc);
                }
                xml.readNext();
            }
        }
        if(xml.name() == "keyword") {
            xml.readNext();
            LanguageContextElementKeyword *kw = new LanguageContextElementKeyword(xml.text().toString());
            result->elements.append(kw);
            xml.readNext();
        }
        xml.readNext();
    }
    return result;
}
