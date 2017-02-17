TEMPLATE = app

QT += qml quick quickcontrols2 sql
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/documenthandler.cpp \
    src/historymanager.cpp \
    src/languageloader.cpp \
    src/lirisyntaxhighlighter.cpp \
    src/highlightdata.cpp \
    src/languagecontext.cpp \
    src/languagecontextcontainer.cpp \
    src/languagecontextkeyword.cpp \
    src/languagecontextsubpattern.cpp \
    src/languagecontextsimple.cpp \
    src/languagedefaultstyles.cpp \
    src/languagemanager.cpp \
    src/languagecontextreference.cpp \
    src/languagedatabasemaintainer.cpp

RESOURCES += \
    src/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
include(syntax-config.pri)

HEADERS += \
    src/documenthandler.h \
    src/historymanager.h \
    src/languageloader.h \
    src/lirisyntaxhighlighter.h \
    src/highlightdata.h \
    src/languagecontext.h \
    src/languagecontextcontainer.h \
    src/languagecontextkeyword.h \
    src/languagecontextsimple.h \
    src/languagecontextsubpattern.h \
    src/languagedefaultstyles.h \
    src/languagemanager.h \
    src/languagemetadata.h \
    src/languagecontextreference.h \
    src/languagedatabasemaintainer.h

TRANSLATIONS = \
    src/translations/ru_RU.ts

lupdate_only{
    SOURCES = \
        src/qml/*
}

unix {
}
