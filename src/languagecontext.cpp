#include "languagecontext.h"

LanguageContext::LanguageContext() {

}

void LanguageContext::include(LanguageContext *inc) {
    for (LanguageContextElement *el : inc->elements) {
        elements.append(el);
    }
}
