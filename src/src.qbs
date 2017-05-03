import qbs 1.0

QtGuiApplication {
    name: "liri-text"

    Depends { name: "Qt"; submodules: ["qml", "quick", "quickcontrols2", "sql"] }

    cpp.defines: {
        var defines = base.concat([
            "TEXT_VERSION=" + project.version,
            'USER_LANGUAGE_PATH="/language-specs/"'
        ]);
        if (qbs.hostOS.contains("windows"))
            defines.push('RELATIVE_LANGUAGE_PATH="/language-specs/"');
        else if (qbs.hostOS.contains("macos"))
            defines.push('RELATIVE_LANGUAGE_PATH="../Resources/language-specs/"');
        else if (qbs.hostOS.contains("linux"))
            defines.push('ABSOLUTE_LANGUAGE_PATH="' + qbs.installRoot + '/share/liri-text/language-specs/"');
        return defines;
    }

    Group {
        name: "Sources"
        files: [
            "*.cpp",
            "*.h",
            "*.qrc"
        ]
    }

    Group {
        name: "QML Files"
        files: ["*.qml"]
        prefix: "qml/"
    }

    Group {
        name: "Translations"
        files: ["translations/*.ts"]
    }

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return "";
            else if (qbs.targetOS.contains("darwin"))
                return "Contents/MacOS";
            else
                return "bin";
        }
        fileTagsFilter: product.type
    }

    Group {
        name: "Language Specs"
        files: ["../data/language-specs/*.lang"]
        qbs.install: true
        qbs.installDir: {
            if (qbs.hostOS.contains("windows"))
                return "language-specs";
            else if (qbs.hostOS.contains("macos"))
                return "Contents/Resources/data/language-specs";
            else if (qbs.hostOS.contains("linux"))
                return "share/liri-text/language-specs";
        }
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Desktop File"
        files: ["../data/*.desktop"]
        qbs.install: true
        qbs.installDir: "share/applications"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "AppStream Metadata"
        files: ["../data/*.appdata.xml"]
        qbs.install: true
        qbs.installDir: "share/appdata"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 16x16"
        files: ["../data/icons/16x16/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/16x16/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 22x22"
        files: ["../data/icons/22x22/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/22x22/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 24x24"
        files: ["../data/icons/24x24/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/24x24/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 32x32"
        files: ["../data/icons/32x32/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/32x32/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 48x48"
        files: ["../data/icons/48x48/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/48x48/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 64x64"
        files: ["../data/icons/64x64/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/64x64/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 128x128"
        files: ["../data/icons/128x128/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/128x128/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 192x192"
        files: ["../data/icons/192x192/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/192x192/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon 256x256"
        files: ["../data/icons/256x256/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/256x256/apps"
    }

    Group {
        condition: qbs.hostOS.contains("linux")
        name: "Icon scalable"
        files: ["../data/icons/io.liri.Text.svg"]
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/scalable/apps"
    }

    Group {
         name: "macOS (icons)"
         condition: qbs.targetOS.contains("macos")
         files: ["../data/icons/io.liri.Text.icns"]
         qbs.install: true
         qbs.installDir: "Contents/Resources"
     }
}
