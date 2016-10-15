#ifndef LANGUAGESPECIFICATION_H
#define LANGUAGESPECIFICATION_H

#include <QHash>
#include <QString>

#include "languagestyle.h"
#include "languagecontext.h"
#include "languagecontextelement.h"

class LanguageSpecification
{
public:
    LanguageSpecification();

    QString name;
    QHash<QString, LanguageStyle*> styles;
    QList<LanguageContextElement*> mainContext;
};

#endif // LANGUAGESPECIFICATION_H
