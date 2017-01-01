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

QSharedPointer<LanguageContextContainer> LanguageLoader::loadMainContextById(QString id) {
    qDebug() << "Loading" << id;
    QString path = LanguageManager::pathForId(id);
    return loadMainContext(path);
}

QSharedPointer<LanguageContextContainer> LanguageLoader::loadMainContextByMimeType(QMimeType mimeType, QString filename) {
    QString path = LanguageManager::pathForMimeType(mimeType, filename);
    return loadMainContext(path);
}

// TODO: fix loading of json, perl and ruby
QSharedPointer<LanguageContextContainer> LanguageLoader::loadMainContext(QString path) {
    QFile file(path);
    QString langId;
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language") {
                    langId = xml.attributes().value("id").toString();
                    languageDefaultOptions[langId] = QRegularExpression::OptimizeOnFirstUsageOption;
                }
                if(xml.name() == "define-regex")
                    parseDefineRegex(xml, langId);
                if(xml.name() == "context")
                    parseContext(xml, langId);
                if(xml.name() == "style")
                    parseStyle(xml, langId);
                if(xml.name() == "default-regex-options")
                    parseDefaultRegexOptions(xml, langId);
            }
        }
    }
    file.close();
    QString contextId = langId + ":" + langId;
    if(knownContexts.keys().contains(contextId))
        return knownContexts[contextId]->staticCast<LanguageContextContainer>();
    else
        return QSharedPointer<LanguageContextContainer>();
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
    QXmlStreamAttributes contextAttributes = xml.attributes();
    QString id = contextAttributes.value("id").toString();
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

    QString kwPrefix = "\\%[", kwSuffix = "\\%]";

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
            *result.data() = QSharedPointer<LanguageContext>(new LanguageContextSubPattern(contextAttributes));
    }

    xml.readNext();
    while (!(xml.name() == "context" && xml.isEndElement())) {
        if(xml.name() == "start") {
            if(!*result.data())
                *result.data() = QSharedPointer<LanguageContext>(new LanguageContextContainer(contextAttributes));

            QSharedPointer<LanguageContextContainer> container = result->staticCast<LanguageContextContainer>();
            QRegularExpression::PatternOptions options = parseRegexOptions(xml, langId);
            xml.readNext();
            container->start = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.text().toString() :
                                                                                            escapeNonExtended( xml.text().toString() ),
                                             options | QRegularExpression::ExtendedPatternSyntaxOption);
            xml.readNext();
        }
        if(xml.name() == "end") {
            if(!*result.data())
                *result = QSharedPointer<LanguageContext>(new LanguageContextContainer(contextAttributes));

            QSharedPointer<LanguageContextContainer> container = result->staticCast<LanguageContextContainer>();
            QRegularExpression::PatternOptions options = parseRegexOptions(xml, langId);
            xml.readNext();
            container->end = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.text().toString() :
                                                                                          escapeNonExtended( xml.text().toString() ),
                                           options | QRegularExpression::ExtendedPatternSyntaxOption);
            xml.readNext();
        }
        if(xml.name() == "match") {
            if(!*result.data())
                *result.data() = QSharedPointer<LanguageContext>(new LanguageContextSimple(contextAttributes));

            QSharedPointer<LanguageContextSimple> simple = result->staticCast<LanguageContextSimple>();
            QRegularExpression::PatternOptions options = parseRegexOptions(xml, langId);
            xml.readNext();
            simple->match = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.text().toString() :
                                                                                         escapeNonExtended( xml.text().toString() ),
                                          options | QRegularExpression::ExtendedPatternSyntaxOption);
            xml.readNext();
        }
        if(xml.name() == "prefix") {
            /* According to https://developer.gnome.org/gtksourceview/stable/lang-reference.html
             * prefix is a regex in form of define-regex, which means it can have it's own regex options.
             * Howether, in practice none of prebundled languages have them.
             * Futhermore, making prefix an isolated group breaks highlighting for some languages.
             * Following these considerations, prefixes and suffixes are taken in their original form.
             */
            xml.readNext();
            kwPrefix = xml.text().toString();
            xml.readNext();
        }
        if(xml.name() == "suffix") {
            xml.readNext();
            kwSuffix = xml.text().toString();
            xml.readNext();
        }
        if(xml.name() == "keyword") {
            if(!*result.data())
                *result.data() = QSharedPointer<LanguageContext>(new LanguageContextKeyword(contextAttributes));

            QSharedPointer<LanguageContextKeyword> kw = result->staticCast<LanguageContextKeyword>();
            QRegularExpression::PatternOptions options = parseRegexOptions(xml, langId);
            xml.readNext();
            kw->keywords.append(resolveRegex(kwPrefix + xml.text().toString() + kwSuffix, options));
            xml.readNext();
        }
        if(xml.name() == "include") {
            xml.readNext();
            while (!(xml.name() == "include" && xml.isEndElement())) {
                if(xml.name() == "context") {
                    ContextDPtr inc = parseContext(xml, langId);
                    if(!*result.data())
                        *result.data() = QSharedPointer<LanguageContext>(new LanguageContextContainer(contextAttributes));

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

QRegularExpression::PatternOptions LanguageLoader::parseRegexOptions(QXmlStreamReader &xml, QString langId) {
    QRegularExpression::PatternOptions result = languageDefaultOptions[langId];
    if(xml.attributes().hasAttribute("case-sensitive")) {
        bool caseInsensitive = xml.attributes().value("case-sensitive") == "false";
        if(caseInsensitive)
            result |= QRegularExpression::CaseInsensitiveOption;
        else
            result &= ~QRegularExpression::CaseInsensitiveOption;
    }
    if(xml.attributes().hasAttribute("extended")) {
        bool extended = xml.attributes().value("extended") == "true";
        if(extended)
            result |= QRegularExpression::ExtendedPatternSyntaxOption;
        else
            result &= ~QRegularExpression::ExtendedPatternSyntaxOption;
    }
    if(xml.attributes().hasAttribute("dupnames")) {
        // Not supported
    }
    return result;
}

void LanguageLoader::parseDefaultRegexOptions(QXmlStreamReader &xml, QString langId) {
    languageDefaultOptions[langId] = parseRegexOptions(xml, langId);
    xml.readNext();
}

void LanguageLoader::parseDefineRegex(QXmlStreamReader &xml, QString langId) {
    QString id = xml.attributes().value("id").toString();
    QRegularExpression::PatternOptions options = parseRegexOptions(xml, langId);
    xml.readNext();
    knownRegexes[id] = applyOptionsToSubRegex(xml.text().toString(), options);
    xml.readNext();
}

QRegularExpression LanguageLoader::resolveRegex(QString pattern, QRegularExpression::PatternOptions options) {
    QString resultPattern = pattern;

    for (QString id : knownRegexes.keys()) {
        resultPattern = resultPattern.replace("\\%{" + id + "}", knownRegexes[id]);
    }
    resultPattern = resultPattern.replace("\\%[", "\\b");
    resultPattern = resultPattern.replace("\\%]", "\\b");
    return QRegularExpression(resultPattern, options);
}

QString LanguageLoader::escapeNonExtended(QString pattern) {
    return pattern.replace('#', "\\#").replace(' ', "\\ ");
}

QString LanguageLoader::applyOptionsToSubRegex(QString pattern, QRegularExpression::PatternOptions options) {
    QString result = pattern;
    if((options & QRegularExpression::ExtendedPatternSyntaxOption) == 0)
        result = escapeNonExtended(result);
    if((options & QRegularExpression::CaseInsensitiveOption) != 0)
        result = result.prepend("(?:(?i)").append(")");
    else
        result = result.prepend("(?:(?-i)").append(")");
    return result;
}
