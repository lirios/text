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

#include "languagecontextcontainer.h"

#include <QXmlStreamAttributes>

LanguageContextContainer::LanguageContextContainer() :
    LanguageContext(LanguageContext::Container) { }

LanguageContextContainer::LanguageContextContainer(QXmlStreamAttributes attributes) :
    LanguageContextContainer() {

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

LanguageContextContainer::~LanguageContextContainer() {
    resolveCircularDeps(this);
}
