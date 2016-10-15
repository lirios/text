#ifndef LANGUAGECONTEXTELEMENT_H
#define LANGUAGECONTEXTELEMENT_H

#include "languagestyle.h"

class LanguageContextElement
{
public:
    enum ElementType {
        Simple
      , Container
      , SubPattern
      , Keyword
    } type;

    LanguageContextElement(ElementType t);
};

#endif // LANGUAGECONTEXTELEMENT_H
