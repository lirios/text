#ifndef LANGUAGECONTEXT_H
#define LANGUAGECONTEXT_H

#include <QHash>
#include <QRegExp>
#include "languagecontextelement.h"

class LanguageContext
{
public:
    LanguageContext();

    QString id;

    void include(LanguageContext *inc);

    QList<LanguageContextElement *> elements;
};

#endif // LANGUAGECONTEXT_H
