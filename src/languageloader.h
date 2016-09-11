#ifndef LANGUAGELOADER_H
#define LANGUAGELOADER_H

#include <QString>
#include <QXmlStreamReader>

class LanguageLoader
{
public:
    LanguageLoader();
    void loadFromFile(QString path);
private:
    void parseMetadata(QXmlStreamReader *xml);
    void parseContext(QXmlStreamReader *xml);
};

#endif // LANGUAGELOADER_H
