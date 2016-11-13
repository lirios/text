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
#include <QRegularExpression>
#include "languagecontextkeyword.h"
#include "languagecontextsimple.h"
#include "languagecontextcontainer.h"
#include "languagecontextsubpattern.h"

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
        result->mainContext = static_cast<LanguageContextSimple *>(knownContexts[mainId]);
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
    if(result && id != "") {
        knownContexts[langId + ":" + id] = result;
    }

    if(xml.attributes().hasAttribute("sub-pattern")) {
        result = new LanguageContextSubPattern();
        if(id != "")
            knownContexts[langId + ":" + id] = result;
        LanguageContextSubPattern *subpattern = static_cast<LanguageContextSubPattern *>(result);

        subpattern->group = xml.attributes().value("sub-pattern").toInt();
        if(xml.attributes().value("where") == "start")
            subpattern->where = LanguageContextSubPattern::Start;
        if(xml.attributes().value("where") == "end")
            subpattern->where = LanguageContextSubPattern::End;
    }

    xml.readNext();
    while (xml.name() != "context" || xml.tokenType() != QXmlStreamReader::EndElement) {
        if(xml.name() == "start") {
            if(!result) {
                result = new LanguageContextContainer();
                if(id != "") {
                    knownContexts[langId + ":" + id] = result;
                }
            }
            LanguageContextContainer *container = static_cast<LanguageContextContainer *>(result);
            xml.readNext();
            container->startPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "end") {
            if(!result) {
                result = new LanguageContextContainer();
                if(id != "") {
                    knownContexts[langId + ":" + id] = result;
                }
            }
            LanguageContextContainer *container = static_cast<LanguageContextContainer *>(result);
            xml.readNext();
            container->endPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "match") {
            if(!result) {
                result = new LanguageContextSimple();
                if(id != "") {
                    knownContexts[langId + ":" + id] = result;
                }
            }
            LanguageContextSimple *simple = static_cast<LanguageContextSimple *>(result);
            xml.readNext();
            simple->matchPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "keyword") {
            if(!result) {
                result = new LanguageContextKeyword();
                if(id != "") {
                    knownContexts[langId + ":" + id] = result;
                }
            }
            LanguageContextKeyword *kw = static_cast<LanguageContextKeyword *>(result);
            xml.readNext();
            kw->keywords.append(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "include") {
            xml.readNext();
            while (xml.name() != "include" || xml.tokenType() != QXmlStreamReader::EndElement) {
                if(xml.name() == "context") {
                    LanguageContext *inc = parseContext(xml, langId);
                    if(!result) {
                        result = new LanguageContextSimple();
                        if(id != "") {
                            knownContexts[langId + ":" + id] = result;
                        }
                    }

                    if(result->type == LanguageContext::Simple) {
                        LanguageContextSimple *simple = static_cast<LanguageContextSimple *>(result);
                        if(inc)
                            simple->includes.append(inc);
                    } else if(result->type == LanguageContext::Container) {
                        LanguageContextContainer *container = static_cast<LanguageContextContainer *>(result);
                        if(inc)
                            container->includes.append(inc);
                    } else {
                        Q_ASSERT(false);
                    }
                }
                xml.readNext();
            }
        }
        xml.readNext();
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

    QRegularExpression whiteSpace("(\\s+#[^\n]*\n\\s*|\\s*\n\\s*|(?<!\\\\)\\s)");
    result.replace(whiteSpace, "");

    result = result.replace("\\%[", "\\b");
    result = result.replace("\\%]", "\\b");
    for (QString id : knownRegexes.keys()) {
        result = result.replace("\\%{" + id + "}", knownRegexes[id]);
    }
    return result;
}
