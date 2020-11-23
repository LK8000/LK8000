[![License: GPL-2.0](
   https://img.shields.io/github/license/LK8000/LK8000.svg)](
   https://github.com/LK8000/LK8000/blob/master/LICENSE)
[![Website: www.lk8000.it](
   https://img.shields.io/badge/website-www.lk8000.it-blue)](
   https://www.lk8000.it)
[![Forum: postfrontal.com](
   https://img.shields.io/badge/forum-postfrontal.com-blue)](
   https://www.postfrontal.com/forum/default.asp?CAT_ID=11)
[![Translation status](
   https://hosted.weblate.org/widgets/lk8000/-/svg-badge.svg)](
   https://hosted.weblate.org/engage/lk8000/)

# WELCOME TO THE LK8000 DEVELOPMENT HUB
THIS IS THE DEVELOPEMENT BRANCH.

---
## Build Targets supported
```
TARGET=[PPC2003|PNA|PC|PCX64|LINUX|KOBO|PI]
```
for a Linux build, if `install` is added to the command line, LK8000-LINUX and all files needed to run the software will be installed into $HOME/LK8000

for a PC build, add `distrib` to the command line and all files needed to run the softwre will be installed into LK8000/Distrib/PC/LK8000

---
## Build Options supported

### All Targets :
```
DEBUG=[y|n]  
   default = "n"
   if y is specified, no optimzation is made and debug info are included.
```
### LINUX Target :
```
OPENGL=[y|n]
   default = "y" if libgl is available
   if "n" is specified, libSDL with Memory rendering is used.

USE_EGL=[y|n]
   default = "y" if "libgl" and "libegl" are available

GLES=[y|n]
   default = "n"

GLES2=[y|n]
   default = "n"

USE_SDL=[y|n]
   default = "y" if "libegl" not available or OPENGL=n

ENABLE_MESA_KMS=[y|n]
   "n" by default,
   if "y" is specified, use GBM/DRM instead of X11
```

Do you want LINUX greyscaled like on Kobo? 
```
make -j4 TARGET=LINUX OPENGL=n GREYSCALE=y DITHER=y clean
make -j4 TARGET=LINUX OPENGL=n GREYSCALE=y DITHER=y install
```
Notice the `clean` that will also remove the Distrib folder, which is needed
to rebuild bitmaps for use without opengl

### KOBO Target :
```
KOBO=<kobo rootfs directory>
   default = "/opt/kobo-rootfs"
```

### Raspberry Pi 2
```
dependencies : 
   zlib1g-dev
   libzzip-dev
   libpng-dev
   libfreetype6-dev
   libgeographic-dev
   libboost-dev
   libinput-dev
   libudev-dev
   libglm-dev
   libsndfile1-dev
   libasound2-dev
   xsltproc
   imagemagick

PI=<rPi rootfs directory>
   required only to cross compile.
```

### Desktop Target ( LINUX or PC ) :
```
FULLSCREEN=[y|n]
   default = "n"
```

---
## Prebuild toolchain

Kobo & WinCE toolchain for Debian host can be found here : 
http://lk8000.it/toolchain/

a Doker image is also available here : 
https://hub.docker.com/r/lk8000/lk8000/tags
