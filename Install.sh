#!/bin/sh

cp -R ./Common/Distribution/LK8000/_Configuration ~/LK8000
cp -R ./Common/Distribution/LK8000/_Polars ~/LK8000
cp -R ./Common/Distribution/LK8000/_System ~/LK8000
cp ./Common/Data/Language/DEFAULT_MENU.TXT ~/LK8000/_System

mkdir ~/LK8000/_Language

echo "empty file" > ~/LK8000/_Language/_LANGUAGE
cp ./Common/Data/Language/ENG_HELP.TXT ~/LK8000/_Language/ENG_HELP.TXT
cp ./Common/Data/Language/ENG_MSG.TXT ~/LK8000/_Language/ENG_MSG.TXT
cp ./Common/Data/Language/ENGLISH.LNG ~/LK8000/_Language/ENGLISH.LNG
cp ./Common/Data/Language/Translations/*.TXT ~/LK8000/_Language
cp ./Common/Data/Language/Translations/*.LNG ~/LK8000/_Language

cp ./Common/Data/Sounds/*.WAV ~/LK8000/_System/_Sounds
