#!/bin/bash 

## Resample WAV files
output="../Common/Data/Sounds/converted/"
if [ ! -d $output ]; then
  mkdir $output;
fi


find ../Common/Data/Sounds/ -name '*.WAV' | while IFS=$'n' read -r FILE
  do
    outputfile="$(basename "$FILE")"
    echo "Processing $outputfile"
    sox -S "$FILE" -r 22050 -b 16 "$output/$outputfile"

    mv "$output/$outputfile" "$FILE"

done

find ../Common/Data/Sounds/ -name '*.wav' | while IFS=$'n' read -r FILE
  do
    outputfile="$(basename "$FILE")"
    echo "Processing $outputfile"
    sox -S "$FILE" -r 22050 -b 16 "$output/$outputfile"

    mv "$output/$outputfile" "$FILE"

done

rm -rf $output


output="../Common/Distribution/LK8000/_System/_Sounds/converted/"
if [ ! -d $output ]; then
  mkdir $output;
fi

find ../Common/Distribution/LK8000/_System/_Sounds -name '*.WAV' | while IFS=$'n' read -r FILE
  do
    outputfile="$(basename "$FILE")"
    echo "Processing $outputfile"
    sox -S "$FILE" -r 22050 -b 16 "$output/$outputfile"

    mv "$output/$outputfile" "$FILE"

done

rm -rf $output

