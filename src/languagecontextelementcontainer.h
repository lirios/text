#ifndef LANGUAGECONTEXTELEMENTCONTAINER_H
#define LANGUAGECONTEXTELEMENTCONTAINER_H

#include <QList>
#include <QRegExp>
#include "languagecontextelement.h"

class LanguageContextElementContainer : public LanguageContextElement
{
public:
    LanguageContextElementContainer();
    QString startPattern;
    QString endPattern;
    QList<LanguageContextElement *> includes;
};

#endif // LANGUAGECONTEXTELEMENTCONTAINER_H
