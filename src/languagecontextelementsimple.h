#ifndef LANGUAGECONTEXTELEMENTSIMPLE_H
#define LANGUAGECONTEXTELEMENTSIMPLE_H

#include <QList>
#include "languagecontextelement.h"

class LanguageContextElementSimple : public LanguageContextElement
{
public:
    LanguageContextElementSimple();
    QString matchPattern;
    QList<LanguageContextElement *> includes;
};

#endif // LANGUAGECONTEXTELEMENTSIMPLE_H
