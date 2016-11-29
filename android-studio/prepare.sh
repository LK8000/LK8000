#!/bin/sh
#


rm -rf app/src/main/res

mkdir -p app/src/main/res/drawable app/src/main/assets/language/ 

ln -rfs  ../android/res/values app/src/main/res/values
ln -rfs  ../android/res/xml app/src/main/res/xml
ln -rfs  ../Common/Data/Bitmaps/LKicon.png app/src/main/res/drawable/icon.png
ln -rfs  ../Common/Data/Bitmaps/LKicon.png app/src/main/res/drawable/notification_icon.png
