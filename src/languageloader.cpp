#include "languageloader.h"
#include <QFile>
#include <iostream>
#include <QDebug>
#include "languagecontextelementkeyword.h"
#include "languagecontextelementsimple.h"
#include "languagecontextelementcontainer.h"
#include "languagecontextelementsubpattern.h"

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
    if(id != "") {
        result->id = id;
        knownContexts[langId + ":" + id] = result;
    }
    xml.readNext();
    LanguageContextElement *mainElement = nullptr;
    while (xml.name() != "context" || xml.tokenType() != QXmlStreamReader::EndElement) {
        if(xml.name() == "start") {
            if(!mainElement)
                mainElement = new LanguageContextElementContainer();
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(mainElement);
            xml.readNext();
            container->start = QRegExp(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "end") {
            if(!mainElement)
                mainElement = new LanguageContextElementContainer();
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(mainElement);
            xml.readNext();
            container->end = QRegExp(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "match") {
            if(!mainElement)
                mainElement = new LanguageContextElementSimple();
            LanguageContextElementSimple *simple = static_cast<LanguageContextElementSimple *>(mainElement);
            xml.readNext();
            simple->match = QRegExp(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "keyword") {
            xml.readNext();
            LanguageContextElementKeyword *kw = new LanguageContextElementKeyword(xml.text().toString());
            result->elements.append(kw);
            xml.readNext();
        }
        if(xml.name() == "include") {
            xml.readNext();
            while (xml.name() != "include" || xml.tokenType() != QXmlStreamReader::EndElement) {
                if(xml.name() == "context") {
                    LanguageContext *inc = parseContext(xml, langId);
                    if(mainElement) {
                        if(mainElement->type == LanguageContextElement::Simple) {
                            LanguageContextElementSimple *simple = static_cast<LanguageContextElementSimple *>(mainElement);
                            simple->includes.append(inc->elements);
                        } else if(mainElement->type == LanguageContextElement::Container) {
                            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(mainElement);
                            container->includes.append(inc->elements);
                        } else {
                            Q_ASSERT(false);
                        }
                    } else {
                        result->include(inc);
                    }
                }
                xml.readNext();
            }
        }
        xml.readNext();
    }
    if(mainElement) {
        if(mainElement->type == LanguageContextElement::Container) {
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(mainElement);
            if(container->end.pattern() == "")
                container->end = QRegExp("\n");
        }
        result->elements.append(mainElement);
    }
    return result;
}
