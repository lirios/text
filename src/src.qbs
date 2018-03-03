import qbs 1.0

QtGuiApplication {
    readonly property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
    readonly property string dataInstallDir: lirideployment.dataDir + "/liri-text"

    name: "liri-text"
    consoleApplication: false

    Depends { name: "lirideployment" }
    Depends { name: "Qt"; submodules: ["qml", "quick", "quickcontrols2", "sql"] }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }

    bundle.identifierPrefix: "io.liri"
    bundle.identifier: "io.liri.Text"
    bundle.infoPlist: ({"CFBundleIconFile": "io.liri.Text"})

    cpp.defines: {
        var defines = base.concat([
            "TEXT_VERSION=" + project.version,
            'USER_LANGUAGE_PATH="/language-specs/"'
        ]);
        if (qbs.targetOS.contains("windows"))
            defines.push('RELATIVE_LANGUAGE_PATH="/language-specs/"');
        else if (qbs.targetOS.contains("macos"))
            defines.push('RELATIVE_LANGUAGE_PATH="../Resources/language-specs/"');
        else if (qbs.targetOS.contains("linux"))
            defines.push('ABSOLUTE_LANGUAGE_PATH="' + dataInstallDir + '/language-specs/"');
        return defines;
    }

    files: [
        "*.cpp",
        "*.h",
        "*.qrc",
        "../data/icons/io.liri.Text.icns",
        "../data/io.liri.Text.rc",
    ]

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
        qbs.installDir: lirideployment.binDir
        qbs.installSourceBase: destinationDirectory
        fileTagsFilter: isBundle ? ["bundle.content"] : product.type
    }

    Group {
        name: "Language Specs"
        files: ["../data/language-specs/*.lang"]
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return "language-specs";
            else if (qbs.targetOS.contains("macos"))
                return "Contents/Resources/data/language-specs";
            else if (qbs.targetOS.contains("linux"))
                return dataInstallDir + "/language-specs";
        }
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Desktop File"
        files: ["../data/*.desktop"]
        qbs.install: true
        qbs.installDir: lirideployment.applicationsDir
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "AppStream Metadata"
        files: ["../data/*.appdata.xml"]
        qbs.install: true
        qbs.installDir: lirideployment.appDataDir
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 16x16"
        files: ["../data/icons/16x16/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/16x16/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 22x22"
        files: ["../data/icons/22x22/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/22x22/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 24x24"
        files: ["../data/icons/24x24/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/24x24/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 32x32"
        files: ["../data/icons/32x32/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/32x32/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 48x48"
        files: ["../data/icons/48x48/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/48x48/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 64x64"
        files: ["../data/icons/64x64/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/64x64/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 128x128"
        files: ["../data/icons/128x128/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/128x128/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 192x192"
        files: ["../data/icons/192x192/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/192x192/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon 256x256"
        files: ["../data/icons/256x256/io.liri.Text.png"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/256x256/apps"
    }

    Group {
        condition: qbs.targetOS.contains("linux")
        name: "Icon scalable"
        files: ["../data/icons/io.liri.Text.svg"]
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor/scalable/apps"
    }
}
