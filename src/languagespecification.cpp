#include "languagespecification.h"
#include <iostream>

LanguageSpecification::LanguageSpecification() { }

void LanguageSpecification::mergeLanguage(const LanguageSpecification *other) {
    std::cerr << "Including " << other->name.toStdString() << "\n";
    for(QString id : other->contexts.keys()) {
        this->contexts.insert(other->name + ":" + id, other->contexts[id]);
    }
}
