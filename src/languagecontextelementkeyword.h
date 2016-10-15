#ifndef LANGUAGECONTEXTELEMENTKEYWORD_H
#define LANGUAGECONTEXTELEMENTKEYWORD_H

#include "languagecontextelement.h"
#include <QString>

class LanguageContextElementKeyword : public LanguageContextElement
{
public:
    LanguageContextElementKeyword();
    LanguageContextElementKeyword(QString kw);

    QString keyword;
};

#endif // LANGUAGECONTEXTELEMENTKEYWORD_H
