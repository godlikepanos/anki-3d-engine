#!/bin/sh
rm data.ankizip
cd data
zip -r ../data.ankizip . -i '*.mtl' '*.mdl' '*.mesh' '*.scene' '*.ankitex' '*.tga' '*.ttf' '*.ogg' '*.glsl' '*.ankipart'
cd -
