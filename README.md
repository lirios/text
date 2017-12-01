Liri Text
=========

[![license](https://img.shields.io/github/license/lirios/text.svg)](https://github.com/lirios/text/blob/develop/LICENSE)
[![GitHub release](https://img.shields.io/github/release/lirios/text.svg)](https://github.com/lirios/text/releases)
[![Build Status](https://img.shields.io/travis/lirios/text/develop.svg)](https://travis-ci.org/lirios/text)
[![Snap Status](https://build.snapcraft.io/badge/lirios/text.svg)](https://build.snapcraft.io/user/lirios/text)
[![GitHub issues](https://img.shields.io/github/issues/lirios/text.svg)](https://github.com/lirios/text/issues)
[![Maintenance](https://img.shields.io/maintenance/yes/2017.svg)](https://github.com/lirios/text/commits/develop)

Liri Text is a cross-platform text editor made in accordance with Material Design.

## Dependencies

* Qt >= 5.8.0 with at least the following modules is required:
  * [qtbase](http://code.qt.io/cgit/qt/qtbase.git)
  * [qtdeclarative](http://code.qt.io/cgit/qt/qtdeclarative.git)
  * [qtquickcontrols2](http://code.qt.io/cgit/qt/qtquickcontrols2.git)
* [Fluid](https://github.com/lirios/fluid) built from develop branch
* [SQLite](https://www.sqlite.org/) >= 3.7.15

## Build

Liri Text uses [Qbs](http://doc.qt.io/qbs/) as build system.

If you haven't already, start by setting up a `qt5` profile for `qbs`:

```sh
qbs setup-toolchains --type gcc /usr/bin/g++ gcc
qbs setup-qt $(which qmake) qt5 # make sure that qmake is in PATH
qbs config profiles.qt5.baseProfile gcc
```

Then, from the root of the repository, run:

```sh
qbs -d build -j $(nproc) profile:qt5
```

You may want to add `modules.qbs.installPrefix:/install/path` option (it's set to /usr/local by default) to qbs call above.

Now, you can install Liri Text:

```sh
qbs install -d build --install-root / # use sudo if necessary
```

If you're packaging the app, replace `/` with install root suitable for your packaging system.

## Credits

Many thanks to ...

* [Corbin Crutchley](https://github.com/crutchcorn) for creating the application icon
* Everyone who contributed to the project directly via Git, listed in [AUTHORS.md](AUTHORS.md)

## License

This application is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
