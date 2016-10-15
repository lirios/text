#ifndef LANGUAGECONTEXTELEMENT_H
#define LANGUAGECONTEXTELEMENT_H

#include "languagestyle.h"

class LanguageContextElement
{
public:
    enum ElementType {
        Simple
      , Container
      , Keyword
    } type;

    LanguageContextElement();
    LanguageContextElement(ElementType t);
};

#endif // LANGUAGECONTEXTELEMENT_H
