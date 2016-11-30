#!/bin/sh
#


rm -rf app/src/main/res

mkdir -p app/src/main/res/drawable

ln -rfs  ../Common/Data/Bitmaps/LKicon.png app/src/main/res/drawable/icon.png
ln -rfs  ../Common/Data/Bitmaps/LKicon.png app/src/main/res/drawable/notification_icon.png
