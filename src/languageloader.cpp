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
#include <QDebug>
#include <QRegularExpression>
#include "languagecontextkeyword.h"
#include "languagecontextsimple.h"
#include "languagecontextcontainer.h"
#include "languagecontextsubpattern.h"
#include "languagemanager.h"

LanguageLoader::LanguageLoader() { }

LanguageLoader::LanguageLoader(QSharedPointer<LanguageDefaultStyles> defaultStyles) {
    for (auto styleId : defaultStyles->styles.keys()) {
        knownStyles[styleId] = QSharedPointer<LanguageStyle>();
    }
}

LanguageLoader::~LanguageLoader() { }

QSharedPointer<LanguageContextSimple> LanguageLoader::loadMainContextById(QString id) {
    qDebug() << "Loading" << id;
    QString path = LanguageManager::pathForId(id);
    return loadMainContext(path);
}

QSharedPointer<LanguageContextSimple> LanguageLoader::loadMainContextByMimeType(QMimeType mimeType, QString filename) {
    QString path = LanguageManager::pathForMimetype(mimeType, filename);
    return loadMainContext(path);
}

// TODO: fix loading of json, perl and ruby
QSharedPointer<LanguageContextSimple> LanguageLoader::loadMainContext(QString path) {
    QFile file(path);
    QString langId;
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language")
                    langId = xml.attributes().value("id").toString();
                if(xml.name() == "define-regex")
                    parseDefineRegex(xml);
                if(xml.name() == "context")
                    parseContext(xml, langId);
                if(xml.name() == "style")
                    parseStyle(xml, langId);
            }
        }
    }
    file.close();
    QString contextId = langId + ":" + langId;
    if(knownContexts.keys().contains(contextId))
        return knownContexts[contextId]->staticCast<LanguageContextSimple>();
    else
        return QSharedPointer<LanguageContextSimple>();
}

LanguageMetadata LanguageLoader::loadMetadata(QString path) {
    LanguageMetadata result;
    QFile file(path);
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language") {
                    result.id = xml.attributes().value("id").toString();
                    if(xml.attributes().hasAttribute("_name")) // Translatable
                        result.name = xml.attributes().value("_name").toString();
                    else
                        result.name = xml.attributes().value("name").toString();
                }
                if(xml.name() == "metadata") {
                    parseMetadata(xml, result);
                    break;
                }
            }
        }
    }
    file.close();
    return result;
}

void LanguageLoader::parseMetadata(QXmlStreamReader &xml, LanguageMetadata &metadata) {
    while (!(xml.name() == "metadata" && xml.isEndElement())) {
        xml.readNext();
        if(xml.name() == "property") {
            QString pName = xml.attributes().value("name").toString();
            xml.readNext();
            if(pName == "mimetypes")
                metadata.mimeTypes = xml.text().toString();
            if(pName == "globs")
                metadata.globs = xml.text().toString();
            // Note: metadata can also have line-comment and block-comment properties
            xml.readNext();
        }
    }
}

ContextDPtr LanguageLoader::parseContext(QXmlStreamReader &xml, QString langId) {
    ContextDPtr result;
    QString id = xml.attributes().value("id").toString();
    if(!result && id != "" && knownContexts.keys().contains(langId + ":" + id))
        result = knownContexts[langId + ":" + id];
    if(xml.attributes().hasAttribute("ref")) {
        QStringRef refId = xml.attributes().value("ref");
        if(refId.contains(':') && !knownContexts.keys().contains(refId.toString())) {
            loadMainContextById(refId.left(refId.indexOf(':')).toString());
        }
        QString refIdCopy = refId.toString();
        if(!refIdCopy.contains(':'))
            refIdCopy = langId + ":" + refIdCopy;
        if(knownContexts.keys().contains(refIdCopy))
            result = knownContexts[refIdCopy];
        else
            result = knownContexts[refIdCopy] = ContextDPtr(new QSharedPointer<LanguageContext>());
    }
    if(!result)
        result = ContextDPtr(new QSharedPointer<LanguageContext>());

    if(id != "")
        knownContexts[langId + ":" + id] = result;

    QString styleId = "";
    if(xml.attributes().hasAttribute("style-ref")) {
        QStringRef styleIdRef = xml.attributes().value("style-ref");
        if(styleIdRef.contains(':') && !knownStyles.keys().contains(styleIdRef.toString()))
            loadMainContextById(styleIdRef.left(styleIdRef.indexOf(':')).toString());
        styleId = styleIdRef.toString();
        if(!styleId.contains(':'))
            styleId = langId + ":" + styleId;
    }

    if(xml.attributes().hasAttribute("sub-pattern")) {
        if(!*result.data())
            *result.data() = QSharedPointer<LanguageContext>(new LanguageContextSubPattern());
        QSharedPointer<LanguageContextSubPattern> subpattern = result->staticCast<LanguageContextSubPattern>();

        bool isId;
        subpattern->groupId = xml.attributes().value("sub-pattern").toInt(&isId);
        if(!isId)
            subpattern->groupName = xml.attributes().value("sub-pattern").toString();

        if(xml.attributes().value("where") == "start")
            subpattern->where = LanguageContextSubPattern::Start;
        if(xml.attributes().value("where") == "end")
            subpattern->where = LanguageContextSubPattern::End;
    }

    xml.readNext();
    while (!(xml.name() == "context" && xml.isEndElement())) {
        if(xml.name() == "start") {
            if(!*result.data())
                *result.data() = QSharedPointer<LanguageContext>(new LanguageContextContainer());

            QSharedPointer<LanguageContextContainer> container = result->staticCast<LanguageContextContainer>();
            xml.readNext();
            container->startPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "end") {
            if(!*result.data())
                *result = QSharedPointer<LanguageContext>(new LanguageContextContainer());

            QSharedPointer<LanguageContextContainer> container = result->staticCast<LanguageContextContainer>();
            xml.readNext();
            container->endPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "match") {
            if(!*result.data())
                *result.data() = QSharedPointer<LanguageContext>(new LanguageContextSimple());

            xml.readNext();
            QSharedPointer<LanguageContextSimple> simple = result->staticCast<LanguageContextSimple>();
            simple->matchPattern = resolveRegex(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "keyword") {
            if(!*result.data())
                *result.data() = QSharedPointer<LanguageContext>(new LanguageContextKeyword());

            QSharedPointer<LanguageContextKeyword> kw = result->staticCast<LanguageContextKeyword>();
            xml.readNext();
            kw->keywords.append(xml.text().toString());
            xml.readNext();
        }
        if(xml.name() == "include") {
            xml.readNext();
            while (!(xml.name() == "include" && xml.isEndElement())) {
                if(xml.name() == "context") {
                    ContextDPtr inc = parseContext(xml, langId);
                    if(!*result.data())
                        *result.data() = QSharedPointer<LanguageContext>(new LanguageContextSimple());

                    if(result->data()->type == LanguageContext::Simple) {
                        QSharedPointer<LanguageContextSimple> simple = result->staticCast<LanguageContextSimple>();
                        if(inc)
                            simple->includes.append(inc);
                    } else if(result->data()->type == LanguageContext::Container) {
                        QSharedPointer<LanguageContextContainer> container = result->staticCast<LanguageContextContainer>();
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

    if(knownStyles.keys().contains(styleId)) {
        if(knownStyles[styleId])
            result->data()->style = knownStyles[styleId];
        else {
            result->data()->style = QSharedPointer<LanguageStyle>(new LanguageStyle());
            result->data()->style->defaultId = styleId;
        }
    }

    return result;
}

QSharedPointer<LanguageStyle> LanguageLoader::parseStyle(QXmlStreamReader &xml, QString langId) {
    QSharedPointer<LanguageStyle> result = QSharedPointer<LanguageStyle>();
    QString id = xml.attributes().value("id").toString();
    if(xml.attributes().hasAttribute("map-to")) {
        QStringRef refId = xml.attributes().value("map-to");
        if(refId.contains(':') && !knownStyles.keys().contains(refId.toString())) {
            loadMainContextById(refId.left(refId.indexOf(':')).toString());
        }
        QString refIdCopy = refId.toString();
        if(!refIdCopy.contains(':'))
            refIdCopy = langId + ":" + refIdCopy;
        if(knownStyles.keys().contains(refIdCopy)) {
            if(knownStyles[refIdCopy])
                result = knownStyles[refIdCopy];
            else {
                result = QSharedPointer<LanguageStyle>(new LanguageStyle());
                result->defaultId = refIdCopy;
                knownStyles[langId + ":" + id] = result;
            }
        }
    }
    if(result && id != "") {
        knownStyles[langId + ":" + id] = result;
    }
    xml.skipCurrentElement();
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
