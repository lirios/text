load(liri_deployment)

TEMPLATE = app
TARGET = liri-text

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
    src/languagedatabasemaintainer.cpp \
    src/languagecontextbase.cpp

RESOURCES += \
    src/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

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
    src/languagedatabasemaintainer.h \
    src/languagecontextbase.h

TRANSLATIONS = \
    src/translations/ar.ts \
    src/translations/da.ts \
    src/translations/de.ts \
    src/translations/es.ts \
    src/translations/es_419.ts \
    src/translations/fr.ts \
    src/translations/it.ts \
    src/translations/ja.ts \
    src/translations/nl.ts \
    src/translations/pt_BR.ts \
    src/translations/pt_PT.ts \
    src/translations/ru.ts \
    src/translations/zh_HK.ts

lupdate_only{
    SOURCES = \
        src/qml/* \
        src/*
    TRANSLATIONS += \
        src/translations/C.ts
}

unix:!android {
    target.path = $$LIRI_INSTALL_BINDIR
    INSTALLS += target
}

ICON    = data/icons/io.liri.Text.icns
ICON_RC = data/icons/io.liri.Text.ico

unix:!android:!mac {
    ICONS_SIZES = 16 22 24 32 48 64 128 192 256
    for(size, ICONS_SIZES) {
        eval(icon$${size}.files = data/icons/$${size}x$${size}/io.liri.Text.png)
        eval(icon$${size}.path = $$LIRI_INSTALL_PREFIX/share/icons/hicolor/$${size}x$${size}/apps)
        INSTALLS += icon$${size}
    }
    iconSVG.files = data/icons/io.liri.Text.svg
    iconSVG.path = $$LIRI_INSTALL_PREFIX/share/icons/hicolor/scalable/apps
    INSTALLS += iconSVG

    desktop.files = data/io.liri.Text.desktop
    desktop.path = $$LIRI_INSTALL_APPLICATIONSDIR
    INSTALLS += desktop

    appdata.files = data/io.liri.Text.appdata.xml
    appdata.path = $$LIRI_INSTALL_APPDATADIR
    INSTALLS += appdata
}
