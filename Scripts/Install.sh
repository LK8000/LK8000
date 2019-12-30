#!/bin/sh

cp -Rv ../Common/Distribution/LK8000/* ~/LK8000

cp -v ../Common/Data/Language/DEFAULT_MENU.TXT ~/LK8000/_System

mkdir ~/LK8000/_Language

echo "empty file" > ~/LK8000/_Language/_LANGUAGE
echo "empty file" > ~/LK8000/_System/_Bitmaps/_BITMAPSH

cp -v ../Common/Data/Language/language.json ~/LK8000/_Language/language.json
cp -v ../Common/Data/Language/Translations/*.json ~/LK8000/_Language

cp ../Common/Data/Sounds/*.WAV ~/LK8000/_System/_Sounds

rm -fv ~/LK8000/_System/_Bitmaps/*.BMP

cp -Rv ../Distrib/LINUX/* ~/LK8000/
