TEMPLATE = app

QT += qml quick sql
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/documenthandler.cpp \
    src/historymanager.cpp \
    src/languageloader.cpp \
    src/languagestyle.cpp \
    src/lirisyntaxhighlighter.cpp \
    src/highlightdata.cpp \
    src/languagecontext.cpp \
    src/languagecontextcontainer.cpp \
    src/languagecontextkeyword.cpp \
    src/languagecontextsubpattern.cpp \
    src/languagecontextsimple.cpp \
    src/languagedefaultstyles.cpp \
    src/languagemanager.cpp

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
    src/languagestyle.h \
    src/lirisyntaxhighlighter.h \
    src/highlightdata.h \
    src/languagecontext.h \
    src/languagecontextcontainer.h \
    src/languagecontextkeyword.h \
    src/languagecontextsimple.h \
    src/languagecontextsubpattern.h \
    src/languagedefaultstyles.h \
    src/languagemanager.h \
    src/languagemetadata.h

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

    CONFIG += use_gtksourceview_language_specs
    use_gtksourceview_language_specs {
        DEFINES += GTKSOURCEVIEW_LANGUAGE_SPECS=\\\"/usr/share/gtksourceview-3.0/language-specs/\\\"
    }

    DATA_PREFIX = $$PREFIX/share/liri-text
}

DEFINES += LIRI_LANGUAGE_SPECS=\\\"$$DATA_PREFIX/language-specs/\\\"
