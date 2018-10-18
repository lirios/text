#!/bin/bash

LUPDATE=${LUPDATE-lupdate}

###
# Update source translation files
###

appdir=src
mkdir -p $appdir/translations
$LUPDATE $appdir -ts -no-obsolete $appdir/translations/liri-text.ts

tx push --source --no-interactive
tx push --translations --no-interactive
