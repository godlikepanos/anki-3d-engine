#!/bin/sh
rm data.ankizip
cd assets
zip -r ../data.ankizip . -i '*.ankimtl' '*.ankimdl' '*.ankimesh' '*.ankiscene' '*.ankitex' '*.tga' '*.ttf' '*.ogg' '*.glsl' '*.ankipart' '*.lua'
cd -
