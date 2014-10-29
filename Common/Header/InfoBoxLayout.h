#ifndef INFOBOXLAYOUT_H
#define INFOBOXLAYOUT_H



class ButtonLabel {
 public:
  static int ButtonLabelGeometry;
  static HWND hWndButtonWindow[NUMBUTTONLABELS];
  static bool ButtonVisible[NUMBUTTONLABELS];
  static bool ButtonDisabled[NUMBUTTONLABELS];
  static void CreateButtonLabels(RECT rc);
  static void AnimateButton(int i);
  static void SetFont(const LKFont& Font);
  static void Destroy();
  static void SetLabelText(int index, const TCHAR *text);
  static bool CheckButtonPress(HWND pressedwindow);
  static void GetButtonPosition(int i, RECT rc, 
				int *x, int *y,
				int *sizex, int *sizey);
};

#endif
