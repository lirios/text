#!/bin/bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
    macdeployqt liri-text.app -dmg -qmldir=. -verbose=2
    curl --upload-file ./liri-text.dmg https://transfer.sh/Liri_Text-git-$(date +%Y%m%d-%H%M%S)-$(git rev-parse --short HEAD).dmg
else
    sudo make INSTALL_ROOT=../appdir install ; sudo chown -R $USER ../appdir ; find ../appdir/
    cd ../
    wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" 
    chmod a+x linuxdeployqt*.AppImage
    unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
    ./linuxdeployqt*.AppImage ./appdir/usr/share/applications/*.desktop -bundle-non-qt-libs -qmldir=. -verbose=2
    rm ./appdir/io.liri.Text.png # Workaround for linuxedeloyqt bug
    ./linuxdeployqt*.AppImage ./appdir/usr/share/applications/*.desktop -appimage -qmldir=. -verbose=2
    find ./appdir -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq
    curl --upload-file ./Liri_Text*.AppImage https://transfer.sh/Liri_Text-git-$(date +%Y%m%d-%H%M%S)-$(git rev-parse --short HEAD)-x86_64.AppImage
fi
