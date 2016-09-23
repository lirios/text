#ifndef LANGUAGESPECIFICATION_H
#define LANGUAGESPECIFICATION_H

#include <QHash>
#include <QString>

class LanguageSpecification
{
public:
    LanguageSpecification();

    QHash<QRegExp, QString> keywords;
};

#endif // LANGUAGESPECIFICATION_H
