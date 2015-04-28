#!/bin/bash
# Saves the executables for later use
# by Paolo

PLACE=/lk/SAVED

NOW=`date +%y%m%d_%H%M`

test -d $PLACE || {\
  echo
  echo -n "Folder $PLACE does not exist. Create [y/n]?  " 
  read a
  case $a in
    y|Y) ;;
    n|N|*) echo "NOT SAVED"; exit  ;;
  esac

  mkdir $PLACE || {\
    echo "We got problems!"
    echo "CANNOT CREATE FOLDER <$PLACE> . NOT SAVED."
    exit
  }
  mkdir $PLACE/PC 
  mkdir $PLACE/PNA
  mkdir $PLACE/LINUX

  echo "Folder <$PLACE> created."
}

test -r LK8000-LINUX && {\
  test -d $PLACE/LINUX || {\
    echo "NO LINUX SUBFOLDER! ABORTED."; exit
  }
  cp LK8000-LINUX $PLACE/LINUX/$NOW
  echo "Linux executable saved as <$PLACE/LINUX/$NOW>"
}
test -r LK8000-PC.exe && {\
  test -d $PLACE/PC || {\
    echo "NO PC SUBFOLDER! ABORTED."; exit
  }
  cp LK8000-PC.exe $PLACE/PC/${NOW}.exe
  echo "PC executable saved as <$PLACE/PC/${NOW}.exe>"
}
test -r LK8000-PNA.exe && {\
  test -d $PLACE/PNA || {\
    echo "NO PNA SUBFOLDER! ABORTED."; exit
  }
  cp LK8000-PNA.exe $PLACE/PNA/${NOW}.exe
  echo "PNA executable saved as <$PLACE/PNA/${NOW}.exe>"
}

echo All done. Goodbye.

