TEMPLATE = app

QT += qml quick
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/documenthandler.cpp \
    src/historymanager.cpp

RESOURCES += \
    src/qml.qrc \
    icons/icons.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/documenthandler.h \
    src/historymanager.h

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
