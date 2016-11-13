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

    styles["def:comment"].setFontItalic(true);
    styles["def:comment"].setFontWeight(QFont::Light);
    styles["def:comment"].setForeground(QColor("green"));

    styles["def:shebang"].setFontItalic(true);
    styles["def:shebang"].setFontWeight(QFont::Normal);
    styles["def:shebang"].setForeground(QColor("green"));

    styles["def:doc-comment-element"].setFontItalic(false);
    styles["def:doc-comment-element"].setFontWeight(QFont::Normal);
    styles["def:doc-comment-element"].setForeground(QColor("green"));

    styles["def:constant"].setFontItalic(false);
    styles["def:constant"].setFontWeight(QFont::Medium);
    styles["def:constant"].setForeground(QColor("brown"));

    styles["def:special-char"].setFontItalic(false);
    styles["def:special-char"].setFontWeight(QFont::Medium);
    styles["def:special-char"].setForeground(QColor("orange"));

    styles["def:identifier"].setFontItalic(false);
    styles["def:identifier"].setFontWeight(QFont::Normal);
    styles["def:identifier"].setForeground(QColor("cyan"));

    styles["def:statement"].setFontItalic(false);
    styles["def:statement"].setFontWeight(QFont::Bold);
    styles["def:statement"].setForeground(QColor("black"));

    styles["def:type"].setFontItalic(false);
    styles["def:type"].setFontWeight(QFont::Bold);
    styles["def:type"].setForeground(QColor("blue"));

    styles["def:preprocessor"].setFontItalic(false);
    styles["def:preprocessor"].setFontWeight(QFont::Medium);
    styles["def:preprocessor"].setForeground(QColor("teal"));

    styles["def:error"].setFontItalic(false);
    styles["def:error"].setFontWeight(QFont::Medium);
    styles["def:error"].setBackground(QColor("red"));

    styles["def:warning"].setFontItalic(false);
    styles["def:warning"].setFontWeight(QFont::Medium);
    styles["def:warning"].setBackground(QColor("yellow"));

    styles["def:note"].setFontItalic(false);
    styles["def:note"].setFontWeight(QFont::Normal);
    styles["def:note"].setBackground(QColor("yellow"));

    styles["def:underlined"].setFontItalic(true);
    styles["def:underlined"].setFontWeight(QFont::Normal);
    styles["def:underlined"].setUnderlineStyle(QTextCharFormat::SingleUnderline);
}
