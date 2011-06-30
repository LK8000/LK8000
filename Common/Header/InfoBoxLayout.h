#ifndef INFOBOXLAYOUT_H
#define INFOBOXLAYOUT_H

#include "StdAfx.h"
#include "Sizes.h"

class InfoBoxLayout {
 public:
#if USEIBOX
  static bool fullscreen;
  static bool landscape;
  static bool square;
#endif
  static double dscale;
  static bool IntScaleFlag;
  static int scale;
#if USEIBOX
  static void CreateInfoBoxes(RECT rc);
  static void DestroyInfoBoxes(void);
  static int InfoBoxGeometry;
  static int ControlWidth, ControlHeight, TitleHeight;
#endif
  static void ScreenGeometry(RECT rc);
  static void Paint(void);
 private:
#if USEIBOX
  static void GetInfoBoxPosition(int i, RECT rc, 
				 int *x, int *y,
				 int *sizex, int *sizey);
  static void GetInfoBoxSizes(RECT rc);
#endif
};

class ButtonLabel {
 public:
  static int ButtonLabelGeometry;
  static HWND hWndButtonWindow[NUMBUTTONLABELS];
  static bool ButtonVisible[NUMBUTTONLABELS];
  static bool ButtonDisabled[NUMBUTTONLABELS];
  static void CreateButtonLabels(RECT rc);
  static void AnimateButton(int i);
  static void SetFont(HFONT Font);
  static void Destroy();
  static void SetLabelText(int index, const TCHAR *text);
  static bool CheckButtonPress(HWND pressedwindow);
  static void GetButtonPosition(int i, RECT rc, 
				int *x, int *y,
				int *sizex, int *sizey);
};

#endif
