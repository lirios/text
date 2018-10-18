import qbs 1.0

Project {
    name: "Text"

    readonly property string version: "0.3.0"

    property bool useStaticAnalyzer: false

    property bool withFluid: qbs.targetOS.contains("windows") ||
                             qbs.targetOS.contains("macos") ||
                             qbs.targetOS.contains("android")

    minimumQbsVersion: "1.6"

    qbsSearchPaths: {
        var paths = [];
        if (withFluid)
            paths.push("fluid/qbs/shared");
        return paths;
    }

    references: [
        "src/src.qbs",
    ]

    SubProject {
        filePath: "fluid/fluid.qbs"

        Properties {
            condition: withFluid
            useSystemQbsShared: false
            autotestEnabled: false
            deploymentEnabled: false
            withDocumentation: false
            withDemo: false
        }
    }

    Product {
        name: "Text Documents"
        files: [
            "AUTHORS.md",
            "README.md",
            "LICENSE.*"
        ]
    }
}
