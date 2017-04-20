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

#include "languagecontextsimple.h"
#include "languagecontext.h"

#include <QXmlStreamAttributes>

LanguageContextSimple::LanguageContextSimple() { }

LanguageContextSimple::LanguageContextSimple(const QXmlStreamAttributes &attributes) {
    if(attributes.hasAttribute(QStringLiteral("extend-parent")))
        extendParent  = attributes.value(QStringLiteral("extend-parent"))   == "true";

    if(attributes.hasAttribute(QStringLiteral("end-parent")))
        endParent     = attributes.value(QStringLiteral("end-parent"))      == "true";

    if(attributes.hasAttribute(QStringLiteral("first-line-only")))
        firstLineOnly = attributes.value(QStringLiteral("first-line-only")) == "true";

    if(attributes.hasAttribute(QStringLiteral("once-only")))
        onceOnly      = attributes.value(QStringLiteral("once-only"))       == "true";
}
