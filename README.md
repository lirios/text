Liri Text
=========

[![License](https://img.shields.io/github/license/lirios/text.svg)](https://github.com/lirios/text/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/lirios/text.svg)](https://github.com/lirios/text/releases)
[![Build Status](https://img.shields.io/travis/lirios/text/master.svg)](https://travis-ci.org/lirios/text)
[![Build status](https://ci.appveyor.com/api/projects/status/29p7qve6esu7ln22/branch/master?svg=true)](https://ci.appveyor.com/project/plfiorini/text/branch/master)
[![Snap Status](https://build.snapcraft.io/badge/lirios/text.svg)](https://build.snapcraft.io/user/lirios/text)
[![GitHub issues](https://img.shields.io/github/issues/lirios/text.svg)](https://github.com/lirios/text/issues)
[![Maintained](https://img.shields.io/maintenance/yes/2018.svg)](https://github.com/lirios/text/commits/master)

Liri Text is a cross-platform text editor made in accordance with Material Design.

## Dependencies

Qt >= 5.10.0 with at least the following modules is required:

 * [qtbase](http://code.qt.io/cgit/qt/qtbase.git)
 * [qtdeclarative](http://code.qt.io/cgit/qt/qtdeclarative.git)
 * [qtquickcontrols2](http://code.qt.io/cgit/qt/qtquickcontrols2.git)

The following modules and their dependencies are required:

 * [qbs](http://code.qt.io/cgit/qbs/qbs.git) >= 1.9.0
 * [qbs-shared](https://github.com/lirios/qbs-shared.git) >= 1.2.0
 * [fluid](https://github.com/lirios/fluid.git) >= 1.0.0
 * [SQLite](https://www.sqlite.org/) >= 3.7.15

## Installation

We use the [Qbs](http://doc.qt.io/qbs/) build system.

If you want to learn more, please read the [Qbs manual](http://doc.qt.io/qbs/index.html),
especially the [setup guide](http://doc.qt.io/qbs/configuring.html) and how to install artifacts
from the [installation guide](http://doc.qt.io/qbs/installing-files.html).

If you haven't already, start by setting up a `qt5` profile for `qbs`:

```sh
qbs setup-toolchains --type gcc /usr/bin/g++ gcc
qbs setup-qt $(which qmake) qt5 # make sure that qmake is in PATH
qbs config profiles.qt5.baseProfile gcc
```

Then, from the root of the repository, run:

```sh
qbs -d build -j $(nproc) profile:qt5 # use sudo if necessary
```

To the `qbs` call above you can append additional configuration parameters:

 * `modules.lirideployment.prefix:/path/to/prefix` where most files are installed (default: `/usr/local`)
 * `modules.lirideployment.dataDir:path/to/lib` where data files are installed (default: `/usr/local/share`)
 * `modules.lirideployment.libDir:path/to/lib` where libraries are installed (default: `/usr/local/lib`)
 * `modules.lirideployment.qmlDir:path/to/qml` where QML plugins are installed (default: `/usr/local/lib/qml`)
 * `modules.lirideployment.pluginsDir:path/to/plugins` where Qt plugins are installed (default: `/usr/local/lib/plugins`)
 * `modules.lirideployment.qbsModulesDir:path/to/qbs` where Qbs modules are installed (default: `/usr/local/share/qbs/modules`)

See [lirideployment.qbs](https://github.com/lirios/qbs-shared/blob/develop/modules/lirideployment/lirideployment.qbs)
for more deployment-related parameters.

You can also append the following options to the last line:

 * `projects.Text.useStaticAnalyzer:true` to enable the Clang static analyzer.

## Credits

Many thanks to ...

 * [Corbin Crutchley](https://github.com/crutchcorn) for creating the application icon
 * Everyone who contributed to the project directly via Git, listed in [AUTHORS.md](AUTHORS.md)

## License

This application is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
