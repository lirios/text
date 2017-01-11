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
#include "languagemanager.h"

LanguageLoader::LanguageLoader() { }

LanguageLoader::LanguageLoader(QSharedPointer<LanguageDefaultStyles> defaultStyles) {
    for (auto styleId : defaultStyles->styles.keys()) {
        knownStyles[styleId] = QSharedPointer<LanguageStyle>();
    }
}

LanguageLoader::~LanguageLoader() {
    for (auto contextRef : knownContexts) {
        if(!contextRef->context->inUse())
            contextRef->context->prepareForRemoval();
    }
}

QSharedPointer<LanguageContextReference> LanguageLoader::loadMainContextById(QString id) {
    qDebug() << "Loading" << id;
    QString path = LanguageManager::pathForId(id);
    return loadMainContext(path);
}

QSharedPointer<LanguageContextReference> LanguageLoader::loadMainContextByMimeType(QMimeType mimeType, QString filename) {
    QString path = LanguageManager::pathForMimeType(mimeType, filename);
    return loadMainContext(path);
}

QSharedPointer<LanguageContextReference> LanguageLoader::loadMainContext(QString path) {
    QFile file(path);
    QString langId;
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language") {
                    langId = xml.attributes().value("id").toString();
                    languageDefaultOptions   [langId] = QRegularExpression::OptimizeOnFirstUsageOption;
                    languageLeftWordBoundary [langId] = "\\b";
                    languageRightWordBoundary[langId] = "\\b";
                }
                if(xml.name() == "styles")
                    parseStyles(xml, langId);
                if(xml.name() == "default-regex-options")
                    parseDefaultRegexOptions(xml, langId);
                if(xml.name() == "keyword-char-class")
                    parseWordCharClass(xml, langId);
                if(xml.name() == "definitions")
                    parseDefinitions(xml, langId);
            }
        }
    }
    file.close();
    QString contextId = langId + ":" + langId;
    if(knownContexts.keys().contains(contextId)) {
        auto mainContext = knownContexts[contextId];
        mainContext->context->markAsInUse();
        return mainContext;
    }
    else
        return QSharedPointer<LanguageContextReference>();
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

void LanguageLoader::loadDefinitionsAndStylesById(QString id) {
    QString path = LanguageManager::pathForId(id);
    loadDefinitionsAndStyles(path);
}

void LanguageLoader::loadDefinitionsAndStyles(QString path) {
    QString langId;
    QFile file(path);
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language")
                    langId = xml.attributes().value("id").toString();
                if(xml.name() == "styles")
                    parseStyles(xml, langId);
                if(xml.name() == "default-regex-options")
                    parseDefaultRegexOptions(xml, langId);
                if(xml.name() == "keyword-char-class")
                    parseWordCharClass(xml, langId);
                if(xml.name() == "definitions")
                    parseDefinitions(xml, langId);
            }
        }
    }
    file.close();
}

void LanguageLoader::parseMetadata(QXmlStreamReader &xml, LanguageMetadata &metadata) {
    while (!(xml.name() == "metadata" && xml.isEndElement())) {
        xml.readNext();
        if(xml.name() == "property") {
            QStringRef pName = xml.attributes().value("name");
            if(pName == "mimetypes")
                metadata.mimeTypes = xml.readElementText();
            if(pName == "globs")
                metadata.globs = xml.readElementText();
            // Note: metadata can also have line-comment and block-comment properties
        }
    }
}

void LanguageLoader::parseStyles(QXmlStreamReader &xml, QString langId) {
    while (!(xml.name() == "styles" && xml.isEndElement())) {
        xml.readNext();
        if(xml.name() == "style")
            parseStyle(xml, langId);
    }
}

void LanguageLoader::parseDefinitions(QXmlStreamReader &xml, QString langId) {
    while (!(xml.name() == "definitions" && xml.isEndElement())) {
        xml.readNext();
        if(xml.name() == "define-regex")
            parseDefineRegex(xml, langId);
        if(xml.name() == "context")
            parseContext(xml, langId);
        if(xml.name() == "replace")
            parseReplace(xml, langId);
    }
}

QSharedPointer<LanguageContextReference> LanguageLoader::parseContext(QXmlStreamReader &xml, QString langId, QXmlStreamAttributes additionalAttributes) {
    QSharedPointer<LanguageContextReference> result;
    QXmlStreamAttributes contextAttributes = xml.attributes();
    contextAttributes += additionalAttributes;
    QString id = contextAttributes.value("id").toString();
    if(!result && id != "" && knownContexts.keys().contains(langId + ":" + id))
        result = knownContexts[langId + ":" + id];
    else
        result = QSharedPointer<LanguageContextReference>(new LanguageContextReference());
    if(contextAttributes.hasAttribute("ref")) {
        QStringRef refId = contextAttributes.value("ref");
        if(refId.contains(':') && !knownContexts.keys().contains(refId.toString())) {
            loadDefinitionsAndStylesById(refId.left(refId.indexOf(':')).toString());
        }
        QString refIdCopy = refId.toString();
        if(!refIdCopy.contains(':'))
            refIdCopy = langId + ":" + refIdCopy;
        if(knownContexts.keys().contains(refIdCopy)) {
            if(contextAttributes.hasAttribute("original")) {
                result->context = originalContexts[refIdCopy]->context;
                result->style = originalContexts[refIdCopy]->style;
            } else {
                result->context = knownContexts[refIdCopy]->context;
                result->style = knownContexts[refIdCopy]->style;
            }
        } else {
            // Predefinition
            knownContexts[refIdCopy] = result;
            originalContexts[refIdCopy] = result;
        }
    }

    if(id != "") {
        knownContexts[langId + ":" + id] = result;
        originalContexts[langId + ":" + id] = result;
    }

    QString kwPrefix = "\\%[", kwSuffix = "\\%]";

    QString styleId = "";
    if(contextAttributes.hasAttribute("style-ref")) {
        QStringRef styleIdRef = xml.attributes().value("style-ref");
        if(styleIdRef.contains(':') && !knownStyles.keys().contains(styleIdRef.toString()))
            loadDefinitionsAndStylesById(styleIdRef.left(styleIdRef.indexOf(':')).toString());
        styleId = styleIdRef.toString();
        if(!styleId.contains(':'))
            styleId = langId + ":" + styleId;
    }
    applyStyleToContext(result, styleId);
    if(contextAttributes.hasAttribute("ignore-style"))
        result->style.clear();

    if(contextAttributes.hasAttribute("sub-pattern")) {
        if(result->context->type != LanguageContext::SubPattern)
            result->context->init(LanguageContext::SubPattern, contextAttributes);
    }

    xml.readNext();
    while (!(xml.name() == "context" && xml.isEndElement())) {
        if(xml.name() == "start") {
            if(result->context->type != LanguageContext::Container)
                result->context->init(LanguageContext::Container, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->container->start = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.readElementText() :
                                                                                            escapeNonExtended( xml.readElementText() ),
                                             options | QRegularExpression::ExtendedPatternSyntaxOption, langId);
        }
        if(xml.name() == "end") {
            if(result->context->type != LanguageContext::Container)
                result->context->init(LanguageContext::Container, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->container->end = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.readElementText() :
                                                                                          escapeNonExtended( xml.readElementText() ),
                                           options | QRegularExpression::ExtendedPatternSyntaxOption, langId);
        }
        if(xml.name() == "match") {
            if(result->context->type != LanguageContext::Simple)
                result->context->init(LanguageContext::Simple, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->simple->match = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.readElementText() :
                                                                                         escapeNonExtended( xml.readElementText() ),
                                          options | QRegularExpression::ExtendedPatternSyntaxOption, langId);
        }
        if(xml.name() == "prefix") {
            /* According to https://developer.gnome.org/gtksourceview/stable/lang-reference.html
             * prefix is a regex in form of define-regex, which means it can have it's own regex options.
             * Howether, in practice none of prebundled languages have them.
             * Futhermore, making prefix an isolated group breaks highlighting for some languages.
             * Following these considerations, prefixes and suffixes are taken in their original form.
             */
            kwPrefix = xml.readElementText();
        }
        if(xml.name() == "suffix") {
            kwSuffix = xml.readElementText();
        }
        if(xml.name() == "keyword") {
            if(result->context->type != LanguageContext::Container)
                result->context->init(LanguageContext::Container, contextAttributes);

            auto inc = QSharedPointer<LanguageContextReference>(new LanguageContextReference);
            inc->context->init(LanguageContext::Keyword, contextAttributes);
            applyStyleToContext(inc, styleId);

            auto options = parseRegexOptions(xml, langId);
            inc->context->keyword->keyword = resolveRegex(kwPrefix + xml.readElementText() + kwSuffix, options, langId);
            result->context->container->includes.append(inc);
        }
        if(xml.name() == "include") {
            xml.readNext();
            while (!(xml.name() == "include" && xml.isEndElement())) {
                if(xml.name() == "context") {
                    if(result->context->type == LanguageContext::Undefined)
                        result->context->init(LanguageContext::Container, contextAttributes);

                    if(result->context->type == LanguageContext::Simple) {
                        auto inc = parseContext(xml, langId);
                        if(inc)
                            result->context->simple->includes.append(inc);
                    } else if(result->context->type == LanguageContext::Container) {
                        QXmlStreamAttributes childrenAttributes;
                        if(result->context->container->start.pattern() == "" && contextAttributes.hasAttribute("once-only"))
                            childrenAttributes += QXmlStreamAttribute("once-only", contextAttributes.value("once-only").toString());
                        auto inc = parseContext(xml, langId, childrenAttributes);

                        if(inc)
                            result->context->container->includes.append(inc);
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

QSharedPointer<LanguageStyle> LanguageLoader::parseStyle(QXmlStreamReader &xml, QString langId) {
    auto result = QSharedPointer<LanguageStyle>();
    QString id = xml.attributes().value("id").toString();
    if(xml.attributes().hasAttribute("map-to")) {
        QStringRef refId = xml.attributes().value("map-to");
        if(refId.contains(':') && !knownStyles.keys().contains(refId.toString())) {
            loadDefinitionsAndStylesById(refId.left(refId.indexOf(':')).toString());
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
    auto result = languageDefaultOptions[langId];
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
    auto options = parseRegexOptions(xml, langId);
    knownRegexes[id] = applyOptionsToSubRegex(xml.readElementText(), options);
}

void LanguageLoader::parseWordCharClass(QXmlStreamReader &xml, QString langId) {
    QString charClass = xml.readElementText();
    languageLeftWordBoundary [langId] = QStringLiteral("(?<!%1)(?=%1)").arg(charClass);
    languageRightWordBoundary[langId] = QStringLiteral("(?<=%1)(?!%1)").arg(charClass);
}

void LanguageLoader::parseReplace(QXmlStreamReader &xml, QString langId) {
    QString id = xml.attributes().value("id").toString();
    QString refId = xml.attributes().value("ref").toString();
    if(!id.contains(':'))
        id.prepend(langId + ":");
    if(!refId.contains(':'))
        refId.prepend(langId + ":");
    if(knownContexts.keys().contains(id) && knownContexts.keys().contains(refId)) {
        *knownContexts[id].data() = *knownContexts[refId].data();
    }
    xml.readNext();
}

QRegularExpression LanguageLoader::resolveRegex(QString pattern, QRegularExpression::PatternOptions options, QString langId) {
    QString resultPattern = pattern;

    for (QString id : knownRegexes.keys()) {
        resultPattern = resultPattern.replace("\\%{" + id + "}", knownRegexes[id]);
    }
    resultPattern = resultPattern.replace("\\%[", languageLeftWordBoundary [langId]);
    resultPattern = resultPattern.replace("\\%]", languageRightWordBoundary[langId]);
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

void LanguageLoader::applyStyleToContext(QSharedPointer<LanguageContextReference> context, QString styleId) {
    if(knownStyles.keys().contains(styleId)) {
        if(knownStyles[styleId])
            context->style = knownStyles[styleId];
        else {
            context->style = QSharedPointer<LanguageStyle>(new LanguageStyle());
            context->style->defaultId = styleId;
        }
    }
}
