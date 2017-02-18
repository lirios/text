unix:!android {
    PREFIX=$$(PREFIX)
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    INSTALLS += target desktop

    desktop.path = $$PREFIX/share/applications
    desktop.files += liri-text.desktop

    target.path = $$PREFIX/bin
    target.files += liri-text
}

export(INSTALLS)
