#ifndef LANGUAGECONTEXTELEMENTCONTAINER_H
#define LANGUAGECONTEXTELEMENTCONTAINER_H

#include <QList>
#include <QRegExp>
#include "languagecontextelement.h"

class LanguageContextElementContainer : public LanguageContextElement
{
public:
    LanguageContextElementContainer();
    QRegExp start;
    QRegExp end;
    QList<LanguageContextElement *> includes;
};

#endif // LANGUAGECONTEXTELEMENTCONTAINER_H
