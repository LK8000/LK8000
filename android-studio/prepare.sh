#!/bin/sh
#


rm -rf app/src/main/res
rm -rf app/src/main/java

mkdir -p app/src/main/res/drawable app/src/main/java/org/LK8000 app/src/main/java/ioio/lib/android

ln -rfs ../android/AndroidManifest.xml app/src/main/AndroidManifest.xml
ln -rfs ../android/custom_rules.xml app/src/main/custom_rules.xml

ln -rfs ../android/src/*.java app/src/main/java/org/LK8000

ln -rfs  ../android/ioio/software/IOIOLib/src/ioio/lib/api app/src/main/java/ioio/lib/api
ln -rfs  ../android/ioio/software/IOIOLib/src/ioio/lib/spi app/src/main/java/ioio/lib/spi
ln -rfs  ../android/ioio/software/IOIOLib/src/ioio/lib/util app/src/main/java/ioio/lib/util
ln -rfs  ../android/ioio/software/IOIOLib/src/ioio/lib/impl app/src/main/java/ioio/lib/impl
ln -rfs  ../android/ioio/software/IOIOLib/target/android/src/ioio/lib/spi app/src/main/java/ioio/lib/spi2
ln -rfs  ../android/ioio/software/IOIOLib/target/android/src/ioio/lib/util/android/ContextWrapperDependent.java app/src/main/java/ioio/
ln -rfs  ../android/ioio/software/IOIOLibAccessory/src/ioio/lib/android/accessory app/src/main/java/ioio/lib/android/accessory
ln -rfs  ../android/ioio/software/IOIOLibBT/src/ioio/lib/android/bluetooth app/src/main/java/ioio/lib/android/bluetooth
ln -rfs  ../android/ioio/software/IOIOLibAndroidDevice/src/ioio/lib/android/device app/src/main/java/ioio/lib/android/device

ln -rfs  ../android/res/values app/src/main/res/values
ln -rfs  ../android/res/xml app/src/main/res/xml
ln -rfs  ../Common/Data/Bitmaps/LKicon.png app/src/main/res/drawable/icon.png
ln -rfs  ../Common/Data/Bitmaps/LKicon.png app/src/main/res/drawable/notification_icon.png
