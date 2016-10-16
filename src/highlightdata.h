#ifndef HIGHLIGHTDATA_H
#define HIGHLIGHTDATA_H

#include <QTextBlockUserData>
#include "languagecontextelementcontainer.h"

class HighlightData : public QTextBlockUserData
{
public:
    HighlightData();
    ~HighlightData();

    QList<LanguageContextElementContainer *> containers;
};

#endif // HIGHLIGHTDATA_H
