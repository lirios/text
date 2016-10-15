#ifndef LANGUAGECONTEXTELEMENTSIMPLE_H
#define LANGUAGECONTEXTELEMENTSIMPLE_H

#include <QList>
#include <QRegExp>
#include "languagecontextelement.h"

class LanguageContextElementSimple : public LanguageContextElement
{
public:
    LanguageContextElementSimple();
    QRegExp match;
    QList<LanguageContextElement *> includes;
};

#endif // LANGUAGECONTEXTELEMENTSIMPLE_H
