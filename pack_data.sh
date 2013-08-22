#!/bin/sh
rm data.ankizip
cd data
zip -r ../data.ankizip . -i '*.mtl' '*.mdl' '*.mesh' '*.scene' '*.ankitex' '*.tga' '*.png' '*.ttf' '*.ogg'
cd -
