import qbs 1.0

Project {
    name: "Text"

    readonly property string version: "0.2.0"

    property bool useStaticAnalyzer: false

    minimumQbsVersion: "1.6"

    references: [
        "src/src.qbs",
    ]
}
