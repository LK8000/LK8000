/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: InfoBox.h,v 8.1 2009/06/21 13:29:13 venta Exp root $
*/
#if USEIBOX

#include "Units.h"
#include "lk8000.h"
#include "Dialogs.h"

#define BORDERTOP    (1<<bkTop)
#define BORDERRIGHT  (1<<bkRight)
#define BORDERBOTTOM (1<<bkBottom)
#define BORDERLEFT   (1<<bkLeft)
#define BORDERTAB    (1<<(bkLeft+1))

#define TITLESIZE    32
#define VALUESIZE    32
#define COMMENTSIZE  32


class InfoBox{
 public:
    static COLORREF redColor;
    static COLORREF blueColor;
    static COLORREF inv_redColor;
    static COLORREF inv_blueColor;
    static COLORREF amberColor;

// Not really used, tested and dropped. But useful for the future
    static COLORREF yellowColor;
    static COLORREF greenColor;
    static COLORREF magentaColor;
    static COLORREF inv_yellowColor;
    static COLORREF inv_greenColor;
    static COLORREF inv_magentaColor;
  private:

    int mX;
    int mY;
    int mWidth;
    int mHeight;
    HWND mParent;
    HWND mHWnd;
    HDC  mHdc;
    HDC  mHdcTemp;
    HDC mHdcBuf;
    int  mBorderKind;
    COLORREF mColorBack;
    COLORREF mColorFore;
    COLORREF mColorTitle;
    COLORREF mColorTitleBk;
    COLORREF mColorValue;
    COLORREF mColorValueBk;
    COLORREF mColorComment;
    COLORREF mColorCommentBk;

    COLORREF mColorRed;
    COLORREF mColorBlue;

    bool mTitleChanged;

    HBRUSH mhBrushBk;
    HBRUSH mhBrushBkSel;
    HPEN mhPenBorder;
    HPEN mhPenSelector;
    TCHAR mTitle[TITLESIZE+1];
    TCHAR mValue[VALUESIZE+1];
    TCHAR mComment[COMMENTSIZE+1];
    Units_t mValueUnit;
    HFONT  *mphFontTitle;
    HFONT  *mphFontValue;
    HFONT  *mphFontComment;
    HFONT  *valueFont;
    FontHeightInfo_t *mpFontHeightTitle;
    FontHeightInfo_t *mpFontHeightValue;
    FontHeightInfo_t *mpFontHeightComment;
    bool   mHasFocus;
    RECT   recTitle;
    RECT   recValue;
    RECT   recComment;
    HBITMAP mhBitmapUnit;
    HBITMAP mBufBitMap;
    POINT  mBitmapUnitPos;
    POINT  mBitmapUnitSize;

    int color;
    int colorBottom;
    int colorTop;
    int mBorderSize;
    int mUnitBitmapKind;
    bool mVisible;
	bool mSmallerFont; 

    void InitializeDrawHelpers(void);
    void PaintTitle(void);
    void PaintValue(void);
    void PaintComment(void);
    void PaintSelector(void);

    // LRESULT CALLBACK InfoBoxWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  public:
    void Paint(void);
    void PaintFast(void);
    void PaintInto(HDC mHdcDest, int xoff, int yoff, int width, int height);

    Units_t SetValueUnit(Units_t Value);
    void SetTitle(TCHAR *Value);
    void SetValue(TCHAR *Value);
    void SetComment(TCHAR *Value);
	void SetSmallerFont(bool smallerFont);

    void SetFocus(bool Value);
    bool SetVisible(bool Value);

    int GetBorderKind(void);
    int SetBorderKind(int Value);

    HWND GetHandle(void);
    HWND GetParent(void);

    void SetColor(int Value);
    void SetColorBottom(int Value);
    void SetColorTop(int Value);

    InfoBox(HWND Parent, int X, int Y, int Width, int Height);
    ~InfoBox(void);

    HDC GetHdcBuf(void);

};

#endif // USEIBOX
