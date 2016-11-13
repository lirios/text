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

#include "languagedefaultstyles.h"

LanguageDefaultStyles::LanguageDefaultStyles() {
    styles = {
        {"def:comment", QTextCharFormat()}
      , {"def:shebang", QTextCharFormat()}
      , {"def:doc-comment-element", QTextCharFormat()}
      , {"def:constant", QTextCharFormat()}
      , {"def:special-char", QTextCharFormat()}
      , {"def:identifier", QTextCharFormat()}
      , {"def:statement", QTextCharFormat()}
      , {"def:type", QTextCharFormat()}
      , {"def:preprocessor", QTextCharFormat()}
      , {"def:error", QTextCharFormat()}
      , {"def:warning", QTextCharFormat()}
      , {"def:note", QTextCharFormat()}
      , {"def:underlined", QTextCharFormat()}
    };
}
