#ifndef LANGUAGESPECIFICATION_H
#define LANGUAGESPECIFICATION_H

#include <QHash>
#include <QString>

#include "languagestyle.h"
#include "languagecontext.h"

class LanguageSpecification
{
public:
    LanguageSpecification();
    inline LanguageContext *getMainContext() { return contexts[name]; }
    void mergeLanguage(const LanguageSpecification *other);

    QString name;
    QHash<QString, LanguageStyle*> styles;
    QHash<QString, LanguageContext*> contexts;
};

#endif // LANGUAGESPECIFICATION_H
