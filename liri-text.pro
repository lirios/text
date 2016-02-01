TEMPLATE = app

QT += qml quick
QT += KCoreAddons
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/sessionmanager.cpp \
    src/documenthandler.cpp

RESOURCES += \
    src/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/sessionmanager.h \
    src/documenthandler.h
