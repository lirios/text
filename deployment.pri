unix:!android {
    PREFIX=$$(PREFIX)
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    INSTALLS += target desktop icons

    icons.path = $$PREFIX/share/icons/hicolor
    icons.files += data/icons/*

    desktop.path = $$PREFIX/share/applications
    desktop.files += data/io.liri.Text.desktop

    target.path = $$PREFIX/bin
    target.files += liri-text
}

export(INSTALLS)
