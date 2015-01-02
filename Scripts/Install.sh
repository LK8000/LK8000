#!/bin/sh

cp -Rv ../Common/Distribution/LK8000/* ~/LK8000

cp -v ../Common/Data/Language/DEFAULT_MENU.TXT ~/LK8000/_System

mkdir ~/LK8000/_Language

echo "empty file" > ~/LK8000/_Language/_LANGUAGE
echo "empty file" > ~/LK8000/_System/_Bitmaps/_BITMAPSH

cp -v ../Common/Data/Language/ENG_HELP.TXT ~/LK8000/_Language/ENG_HELP.TXT
cp -v ../Common/Data/Language/ENG_MSG.TXT ~/LK8000/_Language/ENG_MSG.TXT
cp -v ../Common/Data/Language/ENGLISH.LNG ~/LK8000/_Language/ENGLISH.LNG
cp -v ../Common/Data/Language/Translations/*.TXT ~/LK8000/_Language
cp -v ../Common/Data/Language/Translations/*.LNG ~/LK8000/_Language

cp ../Common/Data/Sounds/*.WAV ~/LK8000/_System/_Sounds

rm -fv ~/LK8000/_System/_Bitmaps/*.BMP

cp -Rv ../Distrib/LINUX/* ~/LK8000/
