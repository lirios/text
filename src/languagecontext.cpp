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

#include "languagecontext.h"
#include "languagecontextreference.h"
#include <QXmlStreamAttributes>

LanguageContext::LanguageContext() :
    type(Undefined),
    simple(),
    m_inUse(false) { }

LanguageContext::LanguageContext(const LanguageContext &parent) {
    type = parent.type;
    switch (type) {
    case Simple:
        simple = parent.simple;
        break;
    case Container:
        container = parent.container;
        break;
    case SubPattern:
        subPattern = parent.subPattern;
        break;
    case Keyword:
        keyword = parent.keyword;
        break;
    }
}

LanguageContext::~LanguageContext() {
    switch (type) {
    case Simple:
        simple.clear();
        break;
    case Container:
        container.clear();
        break;
    case SubPattern:
        subPattern.clear();
        break;
    case Keyword:
        keyword.clear();
        break;
    }
}

void LanguageContext::init(ElementType t) {
    type = t;
    switch (type) {
    case Simple:
        simple = QSharedPointer<LanguageContextSimple>(new LanguageContextSimple());
        break;
    case Container:
        container = QSharedPointer<LanguageContextContainer>(new LanguageContextContainer());
        break;
    case SubPattern:
        subPattern = QSharedPointer<LanguageContextSubPattern>(new LanguageContextSubPattern());
        break;
    case Keyword:
        keyword = QSharedPointer<LanguageContextKeyword>(new LanguageContextKeyword());
        break;
    }
}

void LanguageContext::init(ElementType t, QXmlStreamAttributes attributes) {
    type = t;
    switch (type) {
    case Simple:
        simple = QSharedPointer<LanguageContextSimple>(new LanguageContextSimple(attributes));
        break;
    case Container:
        container = QSharedPointer<LanguageContextContainer>(new LanguageContextContainer(attributes));
        break;
    case SubPattern:
        subPattern = QSharedPointer<LanguageContextSubPattern>(new LanguageContextSubPattern(attributes));
        break;
    case Keyword:
        keyword = QSharedPointer<LanguageContextKeyword>(new LanguageContextKeyword(attributes));
        break;
    }
}

LanguageContext &LanguageContext::operator =(const LanguageContext &other) {
    type = other.type;
    switch (type) {
    case Simple:
        simple = other.simple;
        break;
    case Container:
        container = other.container;
        break;
    case SubPattern:
        subPattern = other.subPattern;
        break;
    case Keyword:
        keyword = other.keyword;
        break;
    }
    return *this;
}

void LanguageContext::markAsInUse() {
    if(m_inUse)
        return;

    m_inUse = true;
    if(type == Simple) {
        for (auto inc : simple->includes)
            inc->context->markAsInUse();
    }
    if(type == Container) {
        for (auto inc : container->includes)
            inc->context->markAsInUse();
    }
}

#define REMOVE_INCLUDES(ctx) \
    while (!ctx->includes.isEmpty()) { \
        auto inc = ctx->includes.back(); \
        ctx->includes.pop_back(); \
        inc->context->prepareForRemoval(ignoreUsage); \
    }

void LanguageContext::prepareForRemoval(bool ignoreUsage) {
    if(!ignoreUsage && inUse())
        return;

    if(type == Simple) {
        REMOVE_INCLUDES(simple)
    }
    if(type == Container) {
        REMOVE_INCLUDES(container)
    }
}
