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
    styles["def:comment"].setForeground(palette["green"]);

    styles["def:shebang"].setFontItalic(false);
    styles["def:shebang"].setFontWeight(QFont::Medium);
    styles["def:shebang"].setForeground(palette["green"]);

    styles["def:doc-comment-element"].setFontItalic(false);
    styles["def:doc-comment-element"].setFontWeight(QFont::Normal);
    styles["def:doc-comment-element"].setForeground(palette["green"]);

    styles["def:constant"].setFontItalic(false);
    styles["def:constant"].setFontWeight(QFont::Medium);
    styles["def:constant"].setForeground(palette["deepOrange"]);

    styles["def:special-char"].setFontItalic(false);
    styles["def:special-char"].setFontWeight(QFont::Medium);
    styles["def:special-char"].setForeground(palette["orange"]);

    styles["def:identifier"].setFontItalic(false);
    styles["def:identifier"].setFontWeight(QFont::Normal);
    styles["def:identifier"].setForeground(palette["cyan"]);

    styles["def:statement"].setFontItalic(false);
    styles["def:statement"].setFontWeight(QFont::Medium);
    styles["def:statement"].setForeground(palette["indigo"]);

    styles["def:type"].setFontItalic(false);
    styles["def:type"].setFontWeight(QFont::Medium);
    styles["def:type"].setForeground(palette["blue"]);

    styles["def:preprocessor"].setFontItalic(false);
    styles["def:preprocessor"].setFontWeight(QFont::Normal);
    styles["def:preprocessor"].setForeground(palette["teal"]);

    styles["def:error"].setFontItalic(false);
    styles["def:error"].setFontWeight(QFont::Medium);
    styles["def:error"].setBackground(palette["red"]);

    styles["def:warning"].setFontItalic(false);
    styles["def:warning"].setFontWeight(QFont::Medium);
    styles["def:warning"].setBackground(palette["amber"]);

    styles["def:note"].setFontItalic(false);
    styles["def:note"].setFontWeight(QFont::Normal);
    styles["def:note"].setBackground(palette["yellow"]);

    styles["def:underlined"].setFontItalic(true);
    styles["def:underlined"].setFontWeight(QFont::Normal);
    styles["def:underlined"].setUnderlineStyle(QTextCharFormat::SingleUnderline);
}

const QHash<QString, QColor> LanguageDefaultStyles::palette = {
    {"red",         QColor(QRgb(0xF44336))}
  , {"pink",        QColor(QRgb(0xE91E63))}
  , {"purple",      QColor(QRgb(0x9C27B0))}
  , {"deepPurple",  QColor(QRgb(0x673AB7))}
  , {"indigo",      QColor(QRgb(0x3F51B5))}
  , {"blue",        QColor(QRgb(0x2196F3))}
  , {"lightBlue",   QColor(QRgb(0x03A9F4))}
  , {"cyan",        QColor(QRgb(0x00BCD4))}
  , {"teal",        QColor(QRgb(0x009688))}
  , {"green",       QColor(QRgb(0x4CAF50))}
  , {"lightGreen",  QColor(QRgb(0x8BC34A))}
  , {"lime",        QColor(QRgb(0xCDDC39))}
  , {"yellow",      QColor(QRgb(0xFFEB3B))}
  , {"amber",       QColor(QRgb(0xFFC107))}
  , {"orange",      QColor(QRgb(0xFF9800))}
  , {"deepOrange",  QColor(QRgb(0xFF5722))}
  , {"brown",       QColor(QRgb(0x795548))}
  , {"grey",        QColor(QRgb(0x9E9E9E))}
  , {"blueGrey",    QColor(QRgb(0x607D8B))}
  , {"black",       QColor(QRgb(0x000000))}
  , {"white",       QColor(QRgb(0xFFFFFF))}
};
