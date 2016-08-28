#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>

class LanguageLoader
{
public:
    LanguageLoader();
    void loadFromFile(QString path);
};

#endif // LANGUAGELOADER_H
