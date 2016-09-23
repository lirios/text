#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>
#include <QXmlStreamReader>

#include "languagespecification.h"

class LanguageLoader
{
public:
    LanguageLoader();
    LanguageSpecification *loadFromFile(QString path);
protected:
    void parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml);
    void parseContext(LanguageSpecification *lang, QXmlStreamReader *xml);
};

#endif // LANGUAGELOADER_H
