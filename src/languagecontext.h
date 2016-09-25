#ifndef LANGUAGECONTEXT_H
#define LANGUAGECONTEXT_H

#include <QHash>
#include <QRegExp>

class LanguageContext
{
public:
    LanguageContext();
    QList<LanguageContext*> includes;
    QHash<QString, QString> keywords;
};

#endif // LANGUAGECONTEXT_H
