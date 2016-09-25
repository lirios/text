#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>
#include <QXmlStreamReader>
#include <QHash>

#include "languagespecification.h"

class LanguageLoader
{
public:
    LanguageLoader();
    static LanguageSpecification *loadById(QString name);
    static LanguageSpecification *loadFromFile(QString path);
protected:
    static void parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml);
    static QString parseContext(LanguageSpecification *lang, QXmlStreamReader *xml);
    static QHash<QString, LanguageSpecification> contexts;
};

#endif // LANGUAGELOADER_H
