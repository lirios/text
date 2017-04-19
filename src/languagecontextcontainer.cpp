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

#include "languagecontextcontainer.h"
#include "languagecontext.h"

#include <QXmlStreamAttributes>

LanguageContextContainer::LanguageContextContainer() :
    includes({}) { }

LanguageContextContainer::LanguageContextContainer(QXmlStreamAttributes attributes) {
    if(attributes.hasAttribute("style-inside"))
        styleInside  = attributes.value("style-inside") == "true";
    if(attributes.hasAttribute("extend-parent"))
        extendParent = attributes.value("extend-parent") == "true";
    if(attributes.hasAttribute("end-at-line-end"))
        endAtLineEnd = attributes.value("end-at-line-end") == "true";
    if(attributes.hasAttribute("end-parent"))
        endParent = attributes.value("end-parent") == "true";
    if(attributes.hasAttribute("first-line-only"))
        firstLineOnly = attributes.value("first-line-only") == "true";
    if(attributes.hasAttribute("once-only"))
        onceOnly = attributes.value("once-only") == "true";
}

void LanguageContextContainer::markAsInUse() {
    if(m_inUse)
        return;
    LanguageContextBase::markAsInUse();
    for (const auto &inc : qAsConst(includes))
        inc->base->markAsInUse();
}

void LanguageContextContainer::prepareForRemoval(bool ignoreUsage) {
    if(!ignoreUsage && inUse())
        return;

    while (!includes.isEmpty()) {
        auto inc = includes.back();
        includes.pop_back();
        inc->base->prepareForRemoval(ignoreUsage);
    }
}
