TEMPLATE = app

QT += qml quick
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/documenthandler.cpp \
    src/historymanager.cpp \
    src/languageloader.cpp \
    src/languagespecification.cpp \
    src/languagecontext.cpp \
    src/languagestyle.cpp \
    src/languagecontextelement.cpp \
    src/languagecontextelementkeyword.cpp \
    src/lirisyntaxhighlighter.cpp \
    src/languagecontextelementsimple.cpp \
    src/languagecontextelementcontainer.cpp \
    src/languagecontextelementsubpattern.cpp \
    src/highlightdata.cpp

RESOURCES += \
    src/qml.qrc \
    icons/icons.qrc

include(material/material.pri)

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/documenthandler.h \
    src/historymanager.h \
    src/languageloader.h \
    src/languagespecification.h \
    src/languagecontext.h \
    src/languagestyle.h \
    src/languagecontextelement.h \
    src/languagecontextelementkeyword.h \
    src/lirisyntaxhighlighter.h \
    src/languagecontextelementsimple.h \
    src/languagecontextelementcontainer.h \
    src/languagecontextelementsubpattern.h \
    src/highlightdata.h

TRANSLATIONS = \
    src/translations/ru_RU.ts

lupdate_only{
    SOURCES = \
        src/qml/*
}

DISTFILES += \
    icons.yml

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    INSTALLS += target desktop
    desktop.path = $$PREFIX/share/applications
    desktop.files += liri-text.desktop
    target.path = $$PREFIX/bin
    target.files += liri-text
}
