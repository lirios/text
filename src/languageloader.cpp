/*
 * Copyright © 2016-2017 Andrew Penkrat
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
#include "languagemanager.h"
#include "languagecontextcontainer.h"
#include "languagecontextkeyword.h"
#include "languagecontextsimple.h"
#include "languagecontextsubpattern.h"

LanguageLoader::LanguageLoader() { }

LanguageLoader::LanguageLoader(QSharedPointer<LanguageDefaultStyles> defaultStyles) {
    for (auto styleId = defaultStyles->styles.keyBegin(), end = defaultStyles->styles.keyEnd(); styleId != end; ++styleId) {
        m_themeStyles += *styleId;
        m_styleMap[*styleId] = *styleId;
    }
}

LanguageLoader::~LanguageLoader() {
    for (const auto &contextRef : qAsConst(m_originalContexts)) {
        if(contextRef->context->base) {
            contextRef->context->base->prepareForRemoval();
        }
    }
}

QSharedPointer<LanguageContext> LanguageLoader::loadMainContextById(const QString &id) {
    QString path = LanguageManager::getInstance()->pathForId(id);
    return loadMainContext(path);
}

QSharedPointer<LanguageContext> LanguageLoader::loadMainContextByMimeType(const QMimeType &mimeType, const QString &filename) {
    QString path = LanguageManager::getInstance()->pathForMimeType(mimeType, filename);
    return loadMainContext(path);
}

QSharedPointer<LanguageContext> LanguageLoader::loadMainContext(const QString &path) {
    QFile file(path);
    QString langId;
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language") {
                    langId = xml.attributes().value(QStringLiteral("id")).toString();
                    m_languageDefaultOptions   [langId] = QRegularExpression::OptimizeOnFirstUsageOption;
                    m_languageLeftWordBoundary [langId] = QStringLiteral("\\b");
                    m_languageRightWordBoundary[langId] = QStringLiteral("\\b");
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
    if(m_knownContexts.contains(contextId)) {
        auto mainContext = buildContextTree(m_knownContexts[contextId]);
        mainContext->base->markAsInUse();
        return mainContext;
    }
    else
        return QSharedPointer<LanguageContext>();
}

LanguageMetadata LanguageLoader::loadMetadata(const QString &path) {
    LanguageMetadata result;
    QFile file(path);
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language") {
                    result.id = xml.attributes().value(QStringLiteral("id")).toString();
                    if(xml.attributes().hasAttribute(QStringLiteral("_name"))) // Translatable
                        result.name = xml.attributes().value(QStringLiteral("_name")).toString();
                    else
                        result.name = xml.attributes().value(QStringLiteral("name")).toString();
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

void LanguageLoader::loadDefinitionsAndStylesById(const QString &id) {
    QString path = LanguageManager::getInstance()->pathForId(id);
    loadDefinitionsAndStyles(path);
}

void LanguageLoader::loadDefinitionsAndStyles(const QString &path) {
    QString langId;
    QFile file(path);
    if(file.open(QFile::ReadOnly)) {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement()) {
                if(xml.name() == "language") {
                    langId = xml.attributes().value(QStringLiteral("id")).toString();
                    m_languageDefaultOptions   [langId] = QRegularExpression::OptimizeOnFirstUsageOption;
                    m_languageLeftWordBoundary [langId] = QStringLiteral("\\b");
                    m_languageRightWordBoundary[langId] = QStringLiteral("\\b");
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
}

void LanguageLoader::parseMetadata(QXmlStreamReader &xml, LanguageMetadata &metadata) {
    while (!(xml.name() == "metadata" && xml.isEndElement())) {
        xml.readNext();
        if(xml.name() == "property") {
            QStringRef pName = xml.attributes().value(QStringLiteral("name"));
            if(pName == "mimetypes")
                metadata.mimeTypes = xml.readElementText();
            if(pName == "globs")
                metadata.globs = xml.readElementText();
            // Note: metadata can also have line-comment and block-comment properties
        }
    }
}

void LanguageLoader::parseStyles(QXmlStreamReader &xml, const QString &langId) {
    while (!(xml.name() == "styles" && xml.isEndElement())) {
        xml.readNext();
        if(xml.name() == "style")
            parseStyle(xml, langId);
    }
}

void LanguageLoader::parseDefinitions(QXmlStreamReader &xml, const QString &langId) {
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

QSharedPointer<LanguageContextReference> LanguageLoader::parseContext(QXmlStreamReader &xml, const QString &langId, const QXmlStreamAttributes &additionalAttributes) {
    QSharedPointer<LanguageContextReference> result(new LanguageContextReference());
    QXmlStreamAttributes contextAttributes = xml.attributes();
    contextAttributes += additionalAttributes;

    if(contextAttributes.hasAttribute(QStringLiteral("ref"))) {
        QString refId = contextAttributes.value(QStringLiteral("ref")).toString();
        if(refId.contains(':') && !m_knownContexts.contains(refId))
            loadDefinitionsAndStylesById(refId.left(refId.indexOf(':')));
        if(!refId.contains(':'))
            refId = langId + ":" + refId;

        result->refId = refId;
        result->originalRef = contextAttributes.hasAttribute(QStringLiteral("original"));
        result->ignoreStyle = contextAttributes.hasAttribute(QStringLiteral("ignore-style"));
    } else {
        result->context = QSharedPointer<LanguageContext>(new LanguageContext());
    }

    if(contextAttributes.hasAttribute(QStringLiteral("style-ref"))) {
        QString styleId = xml.attributes().value(QStringLiteral("style-ref")).toString();
        if(styleId.contains(':') && !m_styleMap.contains(styleId))
            loadDefinitionsAndStylesById(styleId.left(styleId.indexOf(':')));
        if(!styleId.contains(':'))
            styleId = langId + ":" + styleId;

        if(contextAttributes.hasAttribute(QStringLiteral("ref")))
            result->styleOverwrite = styleId;
        else
            result->context->styleId = styleId;
    }

    if(contextAttributes.hasAttribute(QStringLiteral("ref"))) {
        // If it's a reference context, we're already done
        xml.skipCurrentElement();
        return result;
    }

    if(contextAttributes.hasAttribute(QStringLiteral("id"))) {
        QString id = langId + ":" + contextAttributes.value(QStringLiteral("id")).toString();
        // Known context could've already been set by replace tag
        if(!m_knownContexts.contains(id))
            m_knownContexts[id] = result;
        m_originalContexts [id] = result;
    }

    if(contextAttributes.hasAttribute(QStringLiteral("sub-pattern"))) {
        if(result->context->type != LanguageContext::SubPattern)
            result->context->init(LanguageContext::SubPattern, contextAttributes);

        xml.skipCurrentElement();
        return result;
    }

    QString kwPrefix = QStringLiteral("\\%["),
            kwSuffix = QStringLiteral("\\%]");

    xml.readNext();
    while (!(xml.name() == "context" && xml.isEndElement())) {
        if(xml.name() == "start") {
            if(result->context->type != LanguageContext::Container)
                result->context->init(LanguageContext::Container, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->base.staticCast<LanguageContextContainer>()->start = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.readElementText() :
                                                                                                             escapeNonExtended( xml.readElementText() ),
                                                              options | QRegularExpression::ExtendedPatternSyntaxOption, langId);
            result->context->base.staticCast<LanguageContextContainer>()->includesOnly = false;
        }
        if(xml.name() == "end") {
            if(result->context->type != LanguageContext::Container)
                result->context->init(LanguageContext::Container, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->base.staticCast<LanguageContextContainer>()->end = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.readElementText() :
                                                                                                           escapeNonExtended( xml.readElementText() ),
                                                            options | QRegularExpression::ExtendedPatternSyntaxOption, langId);
        }
        if(xml.name() == "match") {
            if(result->context->type != LanguageContext::Simple)
                result->context->init(LanguageContext::Simple, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->base.staticCast<LanguageContextSimple>()->match = resolveRegex((options & QRegularExpression::ExtendedPatternSyntaxOption) != 0 ? xml.readElementText() :
                                                                                                          escapeNonExtended( xml.readElementText() ),
                                                           options | QRegularExpression::ExtendedPatternSyntaxOption, langId);
        }
        if(xml.name() == "prefix") {
            /* According to https://developer.gnome.org/gtksourceview/stable/lang-reference.html
             * prefix is a regex in form of define-regex, which means it can have it's own regex options.
             * However, in practice none of prebundled languages has them.
             * Furthermore, making prefix an isolated group breaks highlighting for some languages.
             * Following these considerations, prefixes and suffixes are taken in their original form.
             */
            kwPrefix = xml.readElementText();
        }
        if(xml.name() == "suffix") {
            kwSuffix = xml.readElementText();
        }
        if(xml.name() == "keyword") {
            if(result->context->type != LanguageContext::Keyword)
                result->context->init(LanguageContext::Keyword, contextAttributes);

            auto options = parseRegexOptions(xml, langId);
            result->context->base.staticCast<LanguageContextKeyword>()->keywords += resolveRegex(kwPrefix + xml.readElementText() + kwSuffix, options, langId);
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
                            result->includes.append(inc);
                    } else if(result->context->type == LanguageContext::Container) {
                        QXmlStreamAttributes childrenAttributes;
                        if(result->context->base.staticCast<LanguageContextContainer>()->start.pattern() == QLatin1String("") && contextAttributes.hasAttribute(QStringLiteral("once-only")))
                            childrenAttributes += QXmlStreamAttribute(QStringLiteral("once-only"), contextAttributes.value(QStringLiteral("once-only")).toString());
                        auto inc = parseContext(xml, langId, childrenAttributes);

                        if(inc)
                            result->includes.append(inc);
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

void LanguageLoader::parseStyle(QXmlStreamReader &xml, const QString &langId) {
    QString id = langId + ":" + xml.attributes().value(QStringLiteral("id")).toString();

    QString mapId;
    if(!m_themeStyles.contains(id) && xml.attributes().hasAttribute(QStringLiteral("map-to"))) {
        QString refId = xml.attributes().value(QStringLiteral("map-to")).toString();
        if(refId.contains(':') && !m_styleMap.contains(refId)) {
            loadDefinitionsAndStylesById(refId.left(refId.indexOf(':')));
        }
        if(!refId.contains(':'))
            refId = langId + ":" + refId;
        if(m_styleMap.contains(refId))
            mapId = m_styleMap[refId];
        else
            mapId = refId;
    } else
        mapId = id;

    for (auto key = m_styleMap.keyBegin(), end = m_styleMap.keyEnd(); key != end; ++key) {
        if(m_styleMap[*key] == id)
            m_styleMap[*key] = mapId;
    }
    m_styleMap[id] = mapId;

    xml.skipCurrentElement();
}

QRegularExpression::PatternOptions LanguageLoader::parseRegexOptions(QXmlStreamReader &xml, const QString &langId) {
    auto result = m_languageDefaultOptions[langId];
    if(xml.attributes().hasAttribute(QStringLiteral("case-sensitive"))) {
        bool caseInsensitive = xml.attributes().value(QStringLiteral("case-sensitive")) == "false";
        if(caseInsensitive)
            result |= QRegularExpression::CaseInsensitiveOption;
        else
            result &= ~QRegularExpression::CaseInsensitiveOption;
    }
    if(xml.attributes().hasAttribute(QStringLiteral("extended"))) {
        bool extended = xml.attributes().value(QStringLiteral("extended")) == "true";
        if(extended)
            result |= QRegularExpression::ExtendedPatternSyntaxOption;
        else
            result &= ~QRegularExpression::ExtendedPatternSyntaxOption;
    }
    if(xml.attributes().hasAttribute(QStringLiteral("dupnames"))) {
        qDebug() << "Unsupported dupnames attribute";
    }
    return result;
}

void LanguageLoader::parseDefaultRegexOptions(QXmlStreamReader &xml, const QString &langId) {
    m_languageDefaultOptions[langId] = parseRegexOptions(xml, langId);
    xml.readNext();
}

void LanguageLoader::parseDefineRegex(QXmlStreamReader &xml, const QString &langId) {
    QString id = xml.attributes().value(QStringLiteral("id")).toString();
    auto options = parseRegexOptions(xml, langId);
    m_knownRegexes[id] = applyOptionsToSubRegex(xml.readElementText(), options);
}

void LanguageLoader::parseWordCharClass(QXmlStreamReader &xml, const QString &langId) {
    QString charClass = xml.readElementText();
    m_languageLeftWordBoundary [langId] = QStringLiteral("(?<!%1)(?=%1)").arg(charClass);
    m_languageRightWordBoundary[langId] = QStringLiteral("(?<=%1)(?!%1)").arg(charClass);
}

void LanguageLoader::parseReplace(QXmlStreamReader &xml, const QString &langId) {
    QString id = xml.attributes().value(QStringLiteral("id")).toString();
    QString refId = xml.attributes().value(QStringLiteral("ref")).toString();
    if(!id.contains(':'))
        id.prepend(langId + ":");
    if(!refId.contains(':'))
        refId.prepend(langId + ":");

    m_knownContexts[id] = QSharedPointer<LanguageContextReference>(new LanguageContextReference());
    m_knownContexts[id]->refId = refId;

    xml.readNext();
}

QRegularExpression LanguageLoader::resolveRegex(const QString &pattern, QRegularExpression::PatternOptions options, const QString &langId) {
    QString resultPattern = pattern;

    for (auto id = m_knownRegexes.keyBegin(), end = m_knownRegexes.keyEnd(); id != end; ++id) {
        resultPattern = resultPattern.replace("\\%{" + *id + "}", m_knownRegexes[*id]);
    }
    resultPattern = resultPattern.replace(QLatin1String("\\%["), m_languageLeftWordBoundary [langId]);
    resultPattern = resultPattern.replace(QLatin1String("\\%]"), m_languageRightWordBoundary[langId]);
    return QRegularExpression(resultPattern, options);
}

QString LanguageLoader::escapeNonExtended(const QString &pattern) {
    QString escaped = pattern;
    escaped.replace('#', QLatin1String("\\#"));
    escaped.replace(' ', QLatin1String("\\ "));
    return escaped;
}

QString LanguageLoader::applyOptionsToSubRegex(const QString &pattern, QRegularExpression::PatternOptions options) {
    QString result = pattern;
    if((options & QRegularExpression::ExtendedPatternSyntaxOption) == 0)
        result = escapeNonExtended(result);
    if((options & QRegularExpression::CaseInsensitiveOption) != 0)
        result = result.prepend("(?:(?i)").append(")");
    else
        result = result.prepend("(?:(?-i)").append(")");
    return result;
}

QSharedPointer<LanguageContext> LanguageLoader::buildContextTree(QSharedPointer<LanguageContextReference> reference) {
    QSharedPointer<LanguageContext> result;
    if(reference->context)
        result = reference->context;
    else {
        QString refId = reference->refId;
        if(refId.endsWith(QLatin1String(":*"))) {
            refId = refId.left(refId.length() - 2);
        }
        QSharedPointer<LanguageContext> refContext;
        if(reference->originalRef)
            refContext = buildContextTree(m_originalContexts[refId]);
        else
            refContext = buildContextTree(m_knownContexts[refId]);

        if(reference->refId.endsWith(QLatin1String(":*"))) {
            result = QSharedPointer<LanguageContext>(new LanguageContext());
            result->init(LanguageContext::Container);
            auto container = result->base.staticCast<LanguageContextContainer>();
            container->includesOnly = true;
            container->includes = refContext->base.staticCast<LanguageContextContainer>()->includes;
        } else
            result = QSharedPointer<LanguageContext>(new LanguageContext(*refContext));

        bool styleOverwrite = false;
        QString styleId;
        if(!reference->styleOverwrite.isEmpty()) {
            styleOverwrite = true;
            styleId = reference->styleOverwrite;
        }
        if(reference->ignoreStyle) {
            styleOverwrite = true;
            styleId.clear();
        }
        if(styleOverwrite)
            result->styleId = styleId;
    }

    while (reference->includes.size() > 0) {
        auto inc = reference->includes.first();
        reference->includes.removeFirst();

        if(result->type == LanguageContext::Container)
            result->base.staticCast<LanguageContextContainer>()->includes.append(buildContextTree(inc));
        if(result->type == LanguageContext::Simple)
            result->base.staticCast<LanguageContextSimple>()->includes.append(buildContextTree(inc));
    }

    return result;
}
