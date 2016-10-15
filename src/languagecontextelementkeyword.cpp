#include "languagecontextelementkeyword.h"

LanguageContextElementKeyword::LanguageContextElementKeyword() :
    LanguageContextElement(ElementType::Keyword) { }

LanguageContextElementKeyword::LanguageContextElementKeyword(QString kw) :
    LanguageContextElement(ElementType::Keyword),
    keyword(kw) { }
