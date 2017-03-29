#CONFIG += use_gtksourceview_language_specs

win32|macx {
    DEFINES += RELATIVE_LANGUAGE_PATH=\\\"/language-specs/\\\"
} else: unix:!android {
    use_gtksourceview_language_specs {
        DEFINES += GTKSOURCEVIEW_LANGUAGE_PATH=\\\"/usr/share/gtksourceview-3.0/language-specs/\\\"
        message(Using GtkSourceView language specifications.)
    }
    
    DATA_INSTALL_PREFIX = $$LIRI_INSTALL_DATADIR/liri-text
    isEmpty(DATA_RUNTIME_PREFIX) {
        DEFINES += ABSOLUTE_LANGUAGE_PATH=\\\"$$DATA_INSTALL_PREFIX/language-specs/\\\"
    } else {
        DEFINES += ABSOLUTE_LANGUAGE_PATH=\\\"$$DATA_RUNTIME_PREFIX/language-specs/\\\"
    }

    syntax.path = $$DATA_INSTALL_PREFIX/language-specs
    syntax.files = data/language-specs/*.lang
    INSTALLS += syntax
} else {
    warning(Unsupported platform)
}

DEFINES += USER_LANGUAGE_PATH=\\\"/language-specs/\\\"
