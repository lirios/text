#!/bin/bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
    # Configure qbs
    qbs setup-toolchains --detect
    qbs setup-qt /usr/local/opt/qt/bin/qmake travis-qt5
    qbs config profiles.travis-qt5.baseProfile xcode-macosx-x86_64
    qbs config preferences.qbsSearchPaths $(pwd)/fluid/qbs/shared

    # Build and install Fluid
    cd fluid
    sudo qbs -d build -j $(sysctl -n hw.ncpu) profile:travis-qt5 modules.qbs.installRoot:/usr/local modules.lirideployment.prefix:/opt/qt modules.lirideployment.qmlDir:/opt/qt/qml project.withDocumentation:false project.withDemo:false
    sudo rm -fr build
    cd ../

    # Build app
    cd ../
    qbs -d build -j $(sysctl -n hw.ncpu) profile:travis-qt5
else
    # Build qbs
    git clone -b v1.9.0 https://code.qt.io/qbs/qbs.git qbs-src
    cd qbs-src && qmake CONFIG+=exceptions -r qbs.pro && make -j$(nproc) && sudo make install && cd ..

    # Configure qbs
    qbs setup-toolchains --type gcc /usr/bin/g++ gcc
    qbs setup-qt $(which qmake) travis-qt5
    qbs config profiles.travis-qt5.baseProfile gcc
    qbs config preferences.qbsSearchPaths $(pwd)/fluid/qbs/shared

    # Build and install Fluid
    git clone --recursive https://github.com/lirios/fluid.git
    cd fluid
    sudo $(which qbs) -d build -j $(nproc) profile:travis-qt5 modules.qbs.installRoot:/opt modules.qbs.installPrefix:qt58 modules.lirideployment.qmlDir:qml project.withDocumentation:false project.withDemo:false
    sudo rm -fr build

    # Build and install app
    cd ../
    mkdir -p appdir
    qbs -d build -j $(nproc) profile:travis-qt5 modules.qbs.installRoot:appdir modules.qbs.installPrefix:usr
fi
