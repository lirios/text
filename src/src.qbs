import qbs 1.0

QtGuiApplication {
    readonly property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
    readonly property string dataInstallDir: lirideployment.dataDir + "/liri-text"

    name: "liri-text"
    targetName: {
        if (qbs.targetOS.contains("windows"))
            return "LiriText";
        return name;
    }
    consoleApplication: false

    Depends { name: "lirideployment" }
    Depends { name: "Qt"; submodules: ["widgets", "qml", "quick", "quickcontrols2", "sql"] }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }

    bundle.identifierPrefix: "io.liri"
    bundle.identifier: "io.liri.Text"
    bundle.infoPlist: ({"CFBundleIconFile": "io.liri.Text"})

    cpp.defines: {
        var defines = base.concat([
            "TEXT_VERSION=" + project.version,
            'USER_LANGUAGE_PATH="/language-specs/"',
            "LANGUAGE_DB_VERSION=1"
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
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("darwin") && !qbs.targetOS.contains("android")
        name: "Desktop File"
        files: ["../data/*.desktop"]
        qbs.install: true
        qbs.installDir: lirideployment.applicationsDir
    }

    Group {
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("darwin") && !qbs.targetOS.contains("android")
        name: "AppStream Metadata"
        files: ["../data/*.appdata.xml"]
        qbs.install: true
        qbs.installDir: lirideployment.appDataDir
    }

    Group {
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("darwin") && !qbs.targetOS.contains("android")
        name: "Icons"
        prefix: "../data/icons/"
        files: ["**/*.png", "**/*.svg"]
        qbs.install: true
        qbs.installSourceBase: prefix
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor"
    }
}
