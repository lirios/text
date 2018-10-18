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
    Depends { name: "Qt"; submodules: ["core", "widgets", "qml", "quick", "quickcontrols2", "sql"] }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "LiriTranslations" }

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

    Qt.core.resourcePrefix: "/"
    Qt.core.resourceSourceBase: sourceDirectory

    files: [
        "*.cpp",
        "*.h",
        "../data/icons/io.liri.Text.icns",
        "../data/io.liri.Text.rc",
    ]

    Group {
        name: "Resource Data"
        files: [
            "qml/Main.qml",
            "qml/RecentFilesPage.qml",
            "qml/EditPage.qml",
            "qml/FileGridView.qml",
            "qml/SearchOverlay.qml",
            "resources/icon.png",
        ]
        fileTags: ["qt.core.resource_data"]
    }

    Group {
        name: "Translations"
        files: ["translations/*.ts"]
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.binDir
        qbs.installSourceBase: destinationDirectory
        fileTagsFilter: isBundle ? ["bundle.content"] : ["application"]
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
        files: ["../data/io.liri.Text.desktop.in"]
        fileTags: ["liri.desktop.template"]
    }

    Group {
        name: "Desktop File Translations"
        files: ["translations/io.liri.Text_*.desktop"]
        fileTags: ["liri.desktop.translations"]
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.applicationsDir
        fileTagsFilter: "liri.desktop.file"
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
        prefix: "../data/icons/hicolor/"
        files: ["**/*.png", "**/*.svg"]
        qbs.install: true
        qbs.installSourceBase: prefix
        qbs.installDir: lirideployment.dataDir + "/icons/hicolor"
    }

    Group {
        fileTagsFilter: "qm"
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return "translations";
            else if (qbs.targetOS.contains("macos"))
                return "Contents/Resources/data/translations";
            else
                return dataInstallDir + "/translations";
        }
    }
}
