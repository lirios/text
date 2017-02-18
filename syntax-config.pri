CONFIG += use_gtksourceview_language_specs

win32|macx {
    DEFINES += RELATIVE_LANGUAGE_PATH=\\\"/language-specs/\\\"
} else: unix:!android {
    use_gtksourceview_language_specs {
        DEFINES += GTKSOURCEVIEW_LANGUAGE_PATH=\\\"/usr/share/gtksourceview-3.0/language-specs/\\\"
    }

    DATA_PREFIX = $$PREFIX/share/liri-text
    DEFINES += ABSOLUTE_LANGUAGE_PATH=\\\"$$DATA_PREFIX/language-specs/\\\"

    INSTALLS += syntax
    syntax.path = $$DATA_PREFIX/language-specs
    syntax.files = data/language-specs/*
} else {
    warning(Unsupported platform)
}

DEFINES += USER_LANGUAGE_PATH=\\\"/language-specs/\\\"
