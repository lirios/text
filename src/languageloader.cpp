/*
 * Copyright © 2016 Andrew Penkrat
 *
 * This file is part of Liri Text.
 *
 * Liri Text is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liri Text is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Liri Text.  If not, see <http://www.gnu.org/licenses/>.
 */

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
            if(xml.name() == "define-regex")
                parseDefineRegex(xml);
            if(xml.name() == "context")
                parseContext(xml, result->name);
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
            container->startPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "end") {
            if(!mainElement)
                mainElement = new LanguageContextElementContainer();
            LanguageContextElementContainer *container = static_cast<LanguageContextElementContainer *>(mainElement);
            xml.readNext();
            container->endPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "match") {
            if(!mainElement)
                mainElement = new LanguageContextElementSimple();
            LanguageContextElementSimple *simple = static_cast<LanguageContextElementSimple *>(mainElement);
            xml.readNext();
            simple->matchPattern = resolveRegex(xml.text().toString());
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
        result->elements.append(mainElement);
    }
    return result;
}

void LanguageLoader::parseDefineRegex(QXmlStreamReader &xml) {
    QString id = xml.attributes().value("id").toString();
    xml.readNext();
    knownRegexes[id] = resolveRegex(xml.text().toString());
    xml.readNext();
}

QString LanguageLoader::resolveRegex(QString pattern) {
    QString result = pattern;

    result = result.remove(QRegExp("\\s+#[^\n]*\n\\s*"));
    result = result.remove(QRegExp("\\s*\n\\s*"));
    result = result.remove(QRegExp("\\s"));

    result = result.replace("\\%[", "\\b");
    result = result.replace("\\%]", "\\b");
    for (QString id : knownRegexes.keys()) {
        result = result.replace("\\%{" + id + "}", knownRegexes[id]);
    }
    return result;
}
