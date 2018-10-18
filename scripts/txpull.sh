#!/bin/bash

# Fetch translations
tx pull --force --all --minimum-perc=5

# Commit to git
if [ "$CI" = "true" ]; then
    AUTHOR="Liri CI <ci@liri.io>"
    SUBJECT="Automatic merge of Transifex translations"
    git config --global push.default simple
    git add --verbose src/translations/liri-text_*.ts
    git commit --author="$AUTHOR" --message="$SUBJECT"
    git push
fi
