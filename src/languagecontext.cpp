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

#include "languagecontext.h"
#include <QXmlStreamAttributes>
#include "languagecontextcontainer.h"
#include "languagecontextkeyword.h"
#include "languagecontextsimple.h"
#include "languagecontextsubpattern.h"
#include <QDebug>

LanguageContext::LanguageContext() :
    type(Undefined) { }

LanguageContext::LanguageContext(const LanguageContext &other) :
    LanguageContext() {

    type = other.type;
    styleId = other.styleId;
    base = other.base;
}

LanguageContext::~LanguageContext() { }

void LanguageContext::init(ElementType t) {
    type = t;
    switch (type) {
    case Simple:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextSimple());
        break;
    case Container:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextContainer());
        break;
    case SubPattern:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextSubPattern());
        break;
    case Keyword:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextKeyword());
        break;
    case Undefined:
        qDebug() << "Internal error in language loader";
        qDebug() << "Requested context of undefined type";
        Q_ASSERT(false);
    }
}

void LanguageContext::init(ElementType t, QXmlStreamAttributes attributes) {
    type = t;
    switch (type) {
    case Simple:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextSimple(attributes));
        break;
    case Container:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextContainer(attributes));
        break;
    case SubPattern:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextSubPattern(attributes));
        break;
    case Keyword:
        base = QSharedPointer<LanguageContextBase>(new LanguageContextKeyword(attributes));
        break;
    case Undefined:
        qDebug() << "Internal error in language loader";
        qDebug() << "Requested context of undefined type";
        Q_ASSERT(false);
    }
}

LanguageContext &LanguageContext::operator =(const LanguageContext &other) {
    type = other.type;
    styleId = other.styleId;
    base = other.base;
    return *this;
}
