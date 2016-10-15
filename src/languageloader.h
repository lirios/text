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
    LanguageSpecification *loadById(QString name);
    LanguageSpecification *loadFromFile(QString path);
protected:
    void parseMetadata(LanguageSpecification *lang, QXmlStreamReader *xml);
    LanguageContext *parseContext(QXmlStreamReader &xml, QString langId);
    QHash<QString, LanguageContext *> knownContexts;
};

#endif // LANGUAGELOADER_H
