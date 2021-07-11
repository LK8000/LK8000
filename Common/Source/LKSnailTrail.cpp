/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKSnailTrail.cpp $
*/

#include "externs.h"

#include "LKObjects.h"
#include "ScreenGeometry.h"
#include <functional>
#include "Asset.hpp"
using std::placeholders::_1;

static short ApplySnailResize(short defaultsize,short change) {

  if (change!=MAXSNAILRESIZE) {
      int psign=1; short i;
      if (change>MAXSNAILRESIZE) {
          psign=1;
          i=change-MAXSNAILRESIZE; // 6>>1 , 10>>5
      } else {
          psign=-1;
          i=MAXSNAILRESIZE-change; // 0>>5 , 4>>1
      }
      unsigned int u=  (double)(defaultsize*i) / 8.0; 
      short newsize=defaultsize + (u*psign);
      if (newsize<1) newsize=1;
      
      return newsize;
  }

  return defaultsize;
}

void SnailTrail_Create(void) {

  #if TESTBENCH
  StartupStore(_T("... SnailTrail_Create\n"));
  #endif

  //
  // THE SNAIL TRAIL 
  //
  // Default sizes on 320x240 practically equivalent to 9,4,3 on V5 480x272 (5" lcd)
  // That's because the trail was tuned to be visible on a 3.5" screen at 320x240 (114dpi)
  // and 480x272 on a 5" is almost the same (110dpi)
  //
  #define SNAIL_SIZE0  8.00  // 8
  #define SNAIL_SIZE5  3.60  // 4
  #define SNAIL_SIZE7  2.65  // 3
  #define SNAIL_SIZEN  6.00  // fixed color snail for low zoom map

  unsigned int iSnailSizes[NUMSNAILCOLORS+1];
  LKColor      hSnailColours[NUMSNAILCOLORS+1];

  extern float ScreenPixelRatio;
  
  int tmpsize= iround(SNAIL_SIZE0 * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[0]=%d ratio=%f\n"),tmpsize,ScreenPixelRatio);
  #endif
  tmpsize=ApplySnailResize(tmpsize,SnailScale);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[0]=%d scale=%d\n"),tmpsize,SnailScale-MAXSNAILRESIZE);
  #endif
  iSnailSizes[0]= tmpsize;
  iSnailSizes[1]= tmpsize;
  iSnailSizes[2]= tmpsize;
  iSnailSizes[3]= tmpsize;
  iSnailSizes[4]= tmpsize;
  iSnailSizes[10]= tmpsize;
  iSnailSizes[11]= tmpsize;
  iSnailSizes[12]= tmpsize;
  iSnailSizes[13]= tmpsize;
  iSnailSizes[14]= tmpsize;

  tmpsize= iround(SNAIL_SIZE5 * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[5]=%d\n"),tmpsize);
  #endif
  tmpsize=ApplySnailResize(tmpsize,SnailScale);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[5]=%d scale=%d\n"),tmpsize,SnailScale-MAXSNAILRESIZE);
  #endif
  iSnailSizes[5]= tmpsize;
  iSnailSizes[6]= tmpsize;
  iSnailSizes[9]= tmpsize;
  iSnailSizes[8]= tmpsize;

  tmpsize= iround(SNAIL_SIZE7 * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[7]=%d\n"),tmpsize);
  #endif
  tmpsize=ApplySnailResize(tmpsize,SnailScale);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[7]=%d scale=%d\n"),tmpsize,SnailScale-MAXSNAILRESIZE);
  #endif
  iSnailSizes[7]= tmpsize;

  tmpsize= iround(SNAIL_SIZEN * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". (N) SNAIL[15]=%d\n"),tmpsize);
  #endif
  tmpsize=ApplySnailResize(tmpsize,SnailScale);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[15]=%d scale=%d\n"),tmpsize,SnailScale-MAXSNAILRESIZE);
  #endif
  iSnailSizes[15]= tmpsize;


  if (!IsDithered()||IsEinkColored()) {

    // COLORED SNAIL TRAIL
    //
    hSnailColours[0] = RGB_BLACK;
    hSnailColours[1] = RGB_INDIGO;
    hSnailColours[2] = RGB_INDIGO;
    hSnailColours[3] = RGB_BLUE;
    hSnailColours[4] = RGB_BLUE;
    hSnailColours[5] = RGB_LAKE;
    hSnailColours[6] = RGB_LAKE;
    hSnailColours[7] = RGB_GREY;
    hSnailColours[8] = RGB_GREEN;
    hSnailColours[9] = RGB_GREEN;
    hSnailColours[10] = RGB_ORANGE;
    hSnailColours[11] = RGB_ORANGE;
    hSnailColours[12] = RGB_RED;
    hSnailColours[13] = RGB_RED;
    hSnailColours[14] = RGB_DARKRED;

    hSnailColours[15] = RGB_PETROL; // low zoom fixed color

    for (int i = 0; i < NUMSNAILCOLORS + 1; i++) {
      MapWindow::hSnailPens[i].Create(PEN_SOLID, iSnailSizes[i], hSnailColours[i]);
    }

  } else {

    // DITHERED SNAIL TRAIL
    //
    hSnailColours[0] = RGB_GREY;
    hSnailColours[1] = RGB_GREY;
    hSnailColours[2] = RGB_GREY;
    hSnailColours[3] = RGB_GREY;
    hSnailColours[4] = RGB_GREY;
    hSnailColours[5] = RGB_GREY;
    hSnailColours[6] = RGB_GREY;
    hSnailColours[7] = RGB_WHITE;
    hSnailColours[8] = RGB_BLACK;
    hSnailColours[9] = RGB_RED;
    hSnailColours[10] = RGB_RED;
    hSnailColours[11] = RGB_RED;
    hSnailColours[12] = RGB_BLACK;
    hSnailColours[13] = RGB_BLACK;
    hSnailColours[14] = RGB_BLACK;
    hSnailColours[15] = RGB_BLACK;

    MapWindow::hSnailPens[0].Create(PEN_DASH, iSnailSizes[0], hSnailColours[0]);
    MapWindow::hSnailPens[1].Create(PEN_DASH, iSnailSizes[1], hSnailColours[1]);
    MapWindow::hSnailPens[2].Create(PEN_DASH, iSnailSizes[2], hSnailColours[2]);
    MapWindow::hSnailPens[3].Create(PEN_DASH, iSnailSizes[3], hSnailColours[3]);
    MapWindow::hSnailPens[4].Create(PEN_DASH, iSnailSizes[4], hSnailColours[4]);
    MapWindow::hSnailPens[5].Create(PEN_DASH, iSnailSizes[5], hSnailColours[5]);
    MapWindow::hSnailPens[6].Create(PEN_DASH, iSnailSizes[6], hSnailColours[6]);
    MapWindow::hSnailPens[7].Create(PEN_SOLID, iSnailSizes[7], hSnailColours[7]);
    MapWindow::hSnailPens[8].Create(PEN_SOLID, iSnailSizes[8], hSnailColours[8]);
    MapWindow::hSnailPens[9].Create(PEN_SOLID, iSnailSizes[9], hSnailColours[9]);
    MapWindow::hSnailPens[10].Create(PEN_SOLID, iSnailSizes[10], hSnailColours[10]);
    MapWindow::hSnailPens[11].Create(PEN_SOLID, iSnailSizes[11], hSnailColours[11]);
    MapWindow::hSnailPens[12].Create(PEN_SOLID, iSnailSizes[12], hSnailColours[12]);
    MapWindow::hSnailPens[13].Create(PEN_SOLID, iSnailSizes[13], hSnailColours[13]);
    MapWindow::hSnailPens[14].Create(PEN_SOLID, iSnailSizes[14], hSnailColours[14]);
    MapWindow::hSnailPens[15].Create(PEN_SOLID, iSnailSizes[15], hSnailColours[15]);

  }  // DITHERED

}



void SnailTrail_Delete(void) {

  #if TESTBENCH
  StartupStore(_T("... SnailTrail_Delete\n"));
  #endif

  std::for_each(std::begin(MapWindow::hSnailPens), std::end(MapWindow::hSnailPens), std::bind(&LKPen::Release, _1) );

}
