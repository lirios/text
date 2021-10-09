# Translations
file(GLOB LiriText_TRANSLATIONS "${CMAKE_SOURCE_DIR}/translations/app/*_*.ts")
qt5_add_translation(LiriText_QM_FILES ${LiriText_TRANSLATIONS})
if(WIN32)
    set(_translations_destdir "${CMAKE_CURRENT_BINARY_DIR}/translations")
elseif(APPLE)
    set(_translations_destdir "${CMAKE_CURRENT_BINARY_DIR}/Contents/Resources/data/translations")
elseif(LINUX OR BSD)
    set(_translations_destdir "${INSTALL_DATADIR}/liri-text/translations")
endif()
if(DEFINED _translations_destdir)
    install(FILES ${LiriText_QM_FILES} DESTINATION "${_translations_destdir}")
endif()

set(LiriText_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../data/icons/io.liri.Text.icns")
set_source_files_properties("${LiriText_ICON}" PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

set(LiriText_RC "${CMAKE_CURRENT_SOURCE_DIR}/../data/io.liri.Text.rc")

if(WIN32)
    set(LiriText_DEFINES -DRELATIVE_LANGUAGE_PATH="/language-specs/")
elseif(APPLE)
    set(LiriText_DEFINES -DRELATIVE_LANGUAGE_PATH="../Resources/data/language-specs/")
else()
    set(LiriText_DEFINES -DABSOLUTE_LANGUAGE_PATH="${CMAKE_INSTALL_PREFIX}/${INSTALL_DATADIR}/liri-text/language-specs/")
endif()

if(APPLE OR WIN32)
    set(LiriText_OUTPUT_NAME "LiriText")
else()
    set(LiriText_OUTPUT_NAME "liri-text")
endif()

liri_add_executable(LiriText
    OUTPUT_NAME
        "${LiriText_OUTPUT_NAME}"
    SOURCES
        documenthandler.cpp
        documenthandler.h
        highlightdata.cpp
        highlightdata.h
        historymanager.cpp
        historymanager.h
        languagecontextbase.cpp
        languagecontextbase.h
        languagecontextcontainer.cpp
        languagecontextcontainer.h
        languagecontext.cpp
        languagecontext.h
        languagecontextkeyword.cpp
        languagecontextkeyword.h
        languagecontextreference.cpp
        languagecontextreference.h
        languagecontextsimple.cpp
        languagecontextsimple.h
        languagecontextsubpattern.cpp
        languagecontextsubpattern.h
        languagedatabasemaintainer.cpp
        languagedatabasemaintainer.h
        languagedefaultstyles.cpp
        languagedefaultstyles.h
        languageloader.cpp
        languageloader.h
        languagemanager.cpp
        languagemanager.h
        languagemetadata.h
        lirisyntaxhighlighter.cpp
        lirisyntaxhighlighter.h
        main.cpp
        ${LiriText_ICON}
        ${LiriText_RC}
        ${LiriText_QM_FILES}
    RESOURCES
        text.qrc
    DEFINES
        #QT_NO_CAST_FROM_ASCII
        #QT_NO_FOREACH
        -DTEXT_VERSION="${PROJECT_VERSION}"
        -DUSER_LANGUAGE_PATH="/language-specs/"
        -DLANGUAGE_DB_VERSION=1
        ${LiriText_DEFINES}
    APPDATA
        "${CMAKE_CURRENT_SOURCE_DIR}/../data/io.liri.Text.appdata.xml"
    DESKTOP
        "${CMAKE_CURRENT_SOURCE_DIR}/../data/io.liri.Text.desktop"
    LIBRARIES
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Qml
        Qt5::Quick
        Qt5::QuickControls2
        Qt5::Sql
    GUI
)
set_target_properties(LiriText PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.liri.Text"
    MACOSX_BUNDLE_ICON_FILE "io.liri.Text"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

liri_finalize_executable(LiriText)
