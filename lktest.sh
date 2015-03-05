#!/bin/sh
# PV 26.2.2015
# Test LK8000 running on different screen resolutions and geometries

xeq=./LK8000-LINUX

while :
do
tput clear
echo ----------------------------------
echo TEST LK8000 RESOLUTIONS-GEOMETRIES
echo ----------------------------------
echo
echo "      (L)       GEOMETRY 4:3        (P) "
echo " 1  ..  320x240            6  ..  240x320"
echo " 2  ..  640x480            7  ..  480x640"
echo " 3  ..  800x600            8  ..  600x800"
echo " 4  .. 1024x768            9  ..  768x1024"
echo " 5  ..  400x300           10  ..  300x400"
echo 
echo "     (L)       GEOMETRY 5:3        (P) "
echo " 11 ..  400x240           16  ..  240x400"
echo " 12 ..  800x480           17  ..  480x800"
echo " 13 ..  960x576"
echo " 14 ..  480x288           19  ..  288x480"
echo " 15 ..  720x432           20  ..  432x720"
echo
echo "     (L)       GEOMETRY 16:9        (P)  "
echo " 21 ..  480x272           26  ..  272x480"
echo " 22 ..  960x544           "
echo " 23 ..  720x408           28  ..  480x720"
echo " 24 .. 1920x1080"
echo
echo -n "Select or Q to quit: "
read a

case $a in
  1) $xeq -x=320 -y=240 ;;
  2) $xeq -x=640 -y=480 ;;
  3) $xeq -x=800 -y=600 ;;
  4) $xeq -x=1024 -y=768 ;;
  5) $xeq -x=400 -y=300 ;;
  6) $xeq -x=240 -y=320 ;;
  7) $xeq -x=480 -y=640 ;;
  8) $xeq -x=600 -y=800 ;;
  9) $xeq -x=768 -y=1024 ;;
  10) $xeq -x=300 -y=400 ;;

  11) $xeq -x=400 -y=240 ;;
  12) $xeq -x=800 -y=480 ;;
  13) $xeq -x=960 -y=576 ;;
  14) $xeq -x=480 -y=288 ;;
  15) $xeq -x=720 -y=432 ;;

  16) $xeq -x=240 -y=400 ;;
  17) $xeq -x=480 -y=800 ;;
  19) $xeq -x=288 -y=480 ;;
  20) $xeq -x=432 -y=720 ;;

  21) $xeq -x=480 -y=272 ;;
  22) $xeq -x=960 -y=544 ;;
  23) $xeq -x=720 -y=408 ;;
  24) $xeq -x=1920 -y=1080 ;;

  26) $xeq -x=272 -y=480 ;;
  28) $xeq -x=480 -y=720 ;;


 x|q|X|Q) exit  ;;
 *) echo "Error, press return and try again"; read b;;

esac


done






