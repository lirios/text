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

#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QMimeType>

class QDir;
class LanguageManager : public QObject
{
    Q_OBJECT
public:
    static void init();
    static QString pathForId(QString id);
    static QString pathForMimetype(QMimeType mimetype, QString filename);
public slots:
    static void close();
protected:
    static void initDB();
    static void updateDB();
private:
    static QList<QDir> specsDirs;
};

#endif // LANGUAGEMANAGER_H
