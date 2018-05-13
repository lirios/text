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
      , {"xml:namespace", QTextCharFormat()}
    };

    styles[QStringLiteral("def:comment")].setFontItalic(true);
    styles[QStringLiteral("def:comment")].setFontWeight(QFont::Light);
    styles[QStringLiteral("def:comment")].setForeground(palette[QStringLiteral("green")]);

    styles[QStringLiteral("def:shebang")].setFontItalic(false);
    styles[QStringLiteral("def:shebang")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:shebang")].setForeground(palette[QStringLiteral("green")]);

    styles[QStringLiteral("def:doc-comment-element")].setFontItalic(false);
    styles[QStringLiteral("def:doc-comment-element")].setFontWeight(QFont::Normal);
    styles[QStringLiteral("def:doc-comment-element")].setForeground(palette[QStringLiteral("green")]);

    styles[QStringLiteral("def:constant")].setFontItalic(false);
    styles[QStringLiteral("def:constant")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:constant")].setForeground(palette[QStringLiteral("deepOrange")]);

    styles[QStringLiteral("def:special-char")].setFontItalic(false);
    styles[QStringLiteral("def:special-char")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:special-char")].setForeground(palette[QStringLiteral("orange")]);

    styles[QStringLiteral("def:identifier")].setFontItalic(false);
    styles[QStringLiteral("def:identifier")].setFontWeight(QFont::Normal);
    styles[QStringLiteral("def:identifier")].setForeground(palette[QStringLiteral("deepPurple")]);

    styles[QStringLiteral("def:statement")].setFontItalic(false);
    styles[QStringLiteral("def:statement")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:statement")].setForeground(palette[QStringLiteral("indigo")]);

    styles[QStringLiteral("def:type")].setFontItalic(false);
    styles[QStringLiteral("def:type")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:type")].setForeground(palette[QStringLiteral("blue")]);

    styles[QStringLiteral("def:preprocessor")].setFontItalic(false);
    styles[QStringLiteral("def:preprocessor")].setFontWeight(QFont::Normal);
    styles[QStringLiteral("def:preprocessor")].setForeground(palette[QStringLiteral("teal")]);

    styles[QStringLiteral("def:error")].setFontItalic(false);
    styles[QStringLiteral("def:error")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:error")].setBackground(palette[QStringLiteral("red")]);

    styles[QStringLiteral("def:warning")].setFontItalic(false);
    styles[QStringLiteral("def:warning")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("def:warning")].setBackground(palette[QStringLiteral("amber")]);

    styles[QStringLiteral("def:note")].setFontItalic(false);
    styles[QStringLiteral("def:note")].setFontWeight(QFont::Normal);
    styles[QStringLiteral("def:note")].setBackground(palette[QStringLiteral("yellow")]);

    styles[QStringLiteral("def:underlined")].setFontItalic(true);
    styles[QStringLiteral("def:underlined")].setFontWeight(QFont::Normal);
    styles[QStringLiteral("def:underlined")].setUnderlineStyle(QTextCharFormat::SingleUnderline);

    styles[QStringLiteral("xml:namespace")].setFontItalic(false);
    styles[QStringLiteral("xml:namespace")].setFontWeight(QFont::Medium);
    styles[QStringLiteral("xml:namespace")].setForeground(palette[QStringLiteral("purple")]);
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
