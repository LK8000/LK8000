#!/bin/bash
# Build LK8000 for Linux only, then copy executable to Common/Distribution/LK8000.
# Usage: ./build-linux.sh
set -e
SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPTDIR"
make -j$(nproc) TARGET=LINUX "$@"

EXE="$SCRIPTDIR/LK8000-LINUX"
DEST="$SCRIPTDIR/Common/Distribution/LK8000/LK8000-LINUX"
DESTDIR="$(dirname "$DEST")"

if [[ ! -f "$EXE" ]]; then
  echo "Errore: eseguibile non trovato: $EXE" >&2
  exit 1
fi

mkdir -p "$DESTDIR"
cp -f "$EXE" "$DEST"
echo "Copiato $EXE -> $DEST"

# Avvia LK8000-LINUX simulando il Kobo Glo HD in landscape (1448x1072).
# Usa i parametri -x/-y della riga di comando per forzare la risoluzione.
echo "Avvio LK8000-LINUX in landscape (Kobo Glo HD 1448x1072)..."
"$EXE" -x=1448 -y=1072 || echo "Esecuzione LK8000-LINUX terminata con codice $?"
