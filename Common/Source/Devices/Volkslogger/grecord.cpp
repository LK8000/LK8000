/***********************************************************************
**
**   grecord.cpp
**
**   This file is part of libkfrgcs.
**
************************************************************************
**
**   Copyright (c):  2002 by Garrecht Ingenieurgesellschaft
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id: grecord.cpp,v 1.1 2007/09/14 17:13:23 jwharington Exp $
**
***********************************************************************/

#include <stdio.h>
#include <string.h>

#include "grecord.h"
#include "vlapihlp.h"
#include "vlapityp.h"

#include "utils/heapcheck.h"


// base-64 functions
//
char *byte_bas64(byte *b) {
 const char *const base64tab = "0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ`abcdefghijklmnopqrstuvwxyz";
 static char bas64ar[5];
  bas64ar[0] = base64tab[   b[0] >> 2                           ];
  bas64ar[1] = base64tab[ ((b[0] & 0x03) << 4) | (b[1] >> 4)    ];
  bas64ar[2] = base64tab[ ((b[1] & 0x0f) << 2) | (b[2] >> 6)    ];
  bas64ar[3] = base64tab[   b[2] & 0x3f                         ];
  bas64ar[4] = 0;
  return bas64ar;
}

byte b64b(char c) {
  if ((c >= '0') && (c <= '9'))
    return(c-'0');
  else if ((c >= '@') && (c <= 'Z'))
    return(c-'@'+10);
  else if ((c >= '`') && (c <= 'z'))
    return(c-'`'+37);
  else
    return(0);
}

void bas64_byte(byte *b, char *c) {
  b[0] = ( b64b(c[0]) << 2)         | (b64b(c[1]) >> 4);
  b[1] = ((b64b(c[1]) & 0x0f) << 4) | (b64b(c[2]) >> 2);
  b[2] = ((b64b(c[2]) & 0x03) << 6) | (b64b(c[3])     );
}


// g-record functions
//
GRECORD::GRECORD(FILE *ausgabedatei) {
  strcpy(grecord, "");
  tricnt = 0;
  gcnt   = 0;
  memset(ba,0xff,3);
  ausgabe = ausgabedatei;
}

void GRECORD::update(byte b) {
  ba[tricnt++] = b;
  if (tricnt == 3) {
  tricnt = 0;
  strcat(grecord,byte_bas64(ba));
  memset(ba,0xff,3);
  gcnt++;
    if (gcnt == 18) {
      gcnt = 0;
      fprintf(ausgabe,"G%s\n",grecord);
      strcpy(grecord, "");
    }
  }
}

void GRECORD::_final (void) {
  if (tricnt || gcnt) {
    strcat(grecord,byte_bas64(ba));
    fprintf(ausgabe,"G%s\n",grecord);
  }
}









/*
DATA-GCS:
  - Bin�rblock beim Logger anfordern und im Speicher ablegen
* - Bin�rblock ins IGC-Format konvertieren
* - IGC-Datei abspeichern
  - Bin�rblock im radix-64-Format als G-Records an IGC-Datei anh�ngen

VALI-GCS:
  - IGC-Datei laden und ohne die nicht vom Logger stammenden Datens�tze
    und Whitespaces in temp1.igc abspeichern
  - G-Records aus IGC-Datei laden von radix-64 in Bin�rblock umwandeln
* - Bin�rblock ins IGC-Format konvertieren
*   und speichern in Datei temp2.igc
  - Sicherheitscheck:
    Dateien temp1 und temp2 vergleichen
    Signatur �berpr�fen

* kann f�r DATA- und VALI-Programm genutzt werden



Ben�tigte Funktionen: (D=f�r DATA, V=f�r VALI, P=schon programmiert)
DV P
x  x - Verzeichnis der Fl�ge auslesen
x  x - Bin�rblock(Flug) vom Logger lesen
xx   - Bin�rblock ins IGC-Format konvertieren dabei IGC-Datei abspeichern
x    - Dateiname nach IGC-Vorschrift generieren
xx   - Datei kopieren
 x   - Signatur in Bin�rblock �berpr�fen
x  x - Bin�rblock in GR64 konvertieren und anh�ngen
 x   - GR64 laden, in Bin�rblock umwandeln und im Speicher ablegen
     - IGC-Datei laden und alle nicht vom Logger stammenden Datens�tze
       ausfiltern, die Datei dann wieder als temp-Datei abspeichern

*/


/*
Den String *st mit dem Zeichen f auf L�nge l erweitern, sofern er nicht
schon die gew�nschte L�nge hat
*/
static char *strexpnd(char *st, char f, unsigned int l) {
 unsigned int i,ll;
  ll = strlen(st);
  if (ll > l) st[l] = 0;
  if (ll < l) for(i=ll; i<l; i++) {st[i] = f; st[i+1] = 0;};
  return(st);
}



/*
Filtern einer Zeile:
  - IGC-Zeichenfilter
  - Falls Datensatzkennzeichen nicht zu den vom Logger stammenden
    Datens�tzen (A,B,C,D,E,F,HF,I,J,K,LGCS-Datensatz) geh�rt,
    Leerzeile zur�ckliefern, sonst den gefilterten String
*/
char *filterline(char *st) {
  strtrim(st);
  if (!(
      (st[0]=='A')
   || (st[0]=='B')
   || (st[0]=='C')
   || (st[0]=='D')
   || (st[0]=='E')
   || (st[0]=='F')
   || (st[0]=='G')
   || ((st[0]=='H') && (st[1]=='F'))
   || (st[0]=='I')
   || (st[0]=='J')
   || (st[0]=='K')
   || ((st[0]=='L') && (st[1]=='G') && (st[2]=='C') && (st[3]=='S'))
   )) st[0] = 0;
  return st;
}


/*
Zeile aus *datei lesen und �ber IGC-linefilter laufen lassen
*/
char *fgetline(char *st, size_t si, FILE *datei) {
 char *stat;
  if ( (stat = fgets(st,si,datei)) != 0)
    filterline(st);
  return stat;
}



void print_g_record(FILE *datei, lpb puffer, int32 puflen) {
 int32 i;
 GRECORD g1(datei);
  for(i=0; i<puflen; i++)
    g1.update(puffer[i]);
  g1._final();
}




/*
Aus Datei *dateiname die G-Records extrahieren (nur als zusammenh�ngender
Block), von radix-64 in Bin�r umwandeln und in *puffer speichern.
Pufferl�nge puflen ist angegeben, um ein �berschreiben nicht zum Puffer
geh�render Bereiche zu verhindern
*/
int get_g_record(TCHAR *dateiname, lpb puffer, unsigned long puflen) {
 unsigned long i = 0;
 int	       j;
 const int     zeilemax = 79;
 char          zeile[zeilemax];
 FILE	       *datei;
 byte	       bin[3];
 char          *stat;
  if ((datei = _tfopen(dateiname, _T("rt"))) == NULL)
    return -1;
  while ((stat=fgetline(zeile,sizeof(zeile),datei)) != 0) {
    if (strcmp(zeile,"") == 0) continue;
    if (zeile[0] == 'G') break;
  }
  if (stat) do {
    if (strcmp(zeile,"") == 0) continue;
    if (zeile[0] != 'G') break;
    strexpnd(zeile,'z',73);

    for(j=1; j<73; j=j+4) {
      bas64_byte(bin,&zeile[j]);
      //_fmemcpy(&puffer[i],bin,3);
	  puffer[i] = bin[0];
	  puffer[i+1] = bin[1];
	  puffer[i+2] = bin[2];
      i += 3;
      if (i+3 > puflen) break;
    }
  } while (fgetline(zeile,sizeof(zeile),datei) != 0);
  fclose(datei); return 0;
}


// Eine IGC-Datei von allen Zeilen befreien, die vom Pilot oder OO legal zur
// Datei hinzugef�gt worden sein k�nnten
// Speichern der "cleanen" Datei
void clean_igcfile(TCHAR *quelldateiname, TCHAR *zieldateiname) {
 FILE *quelle;
 FILE *ziel;
 const int zeilemax = 79;
 char zeile[zeilemax];
  quelle = _tfopen(quelldateiname, _T("rt"));
  ziel = _tfopen(zieldateiname, _T("wt"));
  while ((fgetline(zeile,sizeof(zeile),quelle)) != 0) {
    if ( (zeile[0]) && (zeile[0] != 'G'))
      fprintf(ziel,"%s\n",zeile);
  }
  fclose(quelle);
  fclose(ziel);
}
