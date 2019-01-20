/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgIGCProgress.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 23 novembre 2014, 17:36
 */


#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "LKObjects.h"
#include "resource.h"
#include "Draw/LoadSplash.h"
#include "dlgTools.h"
#include "dlgIGCProgress.h"


static LKBitmap SplashBitmap;

static void OnIGCSplashPaint(WindowControl * Sender, LKSurface& Surface) {
    if(!SplashBitmap) {
        SplashBitmap = LoadSplash(_T("LKSTART"));
    }
    DrawSplash(Surface, Sender->GetClientRect(), SplashBitmap);
}

static void OnIGCProgressPaint(WindowControl * Sender, LKSurface& Surface) {
  RECT PrintAreaR = Sender->GetClientRect();
    
  const auto oldFont = Surface.SelectObject(MapWindowBoldFont);

  Surface.FillRect(&PrintAreaR, LKBrush_Petrol);

  // Create text area

  // we cannot use LKPen here because they are not still initialised for startup menu. no problem
  LKPen hP(PEN_SOLID,NIBLSCALE(1),RGB_GREEN);
  auto ohP = Surface.SelectObject(hP);
  const auto ohB = Surface.SelectObject(LKBrush_Petrol);
  Surface.Rectangle(PrintAreaR.left,PrintAreaR.top,PrintAreaR.right,PrintAreaR.bottom);
  Surface.SelectObject(ohP);
  hP.Release();

  hP.Create(PEN_SOLID,NIBLSCALE(1),RGB_BLACK);
  ohP = Surface.SelectObject(hP);
  Surface.SelectObject(LK_HOLLOW_BRUSH);
  InflateRect(&PrintAreaR, -NIBLSCALE(2), -NIBLSCALE(2));
  Surface.Rectangle(PrintAreaR.left,PrintAreaR.top,PrintAreaR.right,PrintAreaR.bottom);

  Surface.SetTextColor(RGB_WHITE);
  Surface.SetBackgroundTransparent();

  InflateRect(&PrintAreaR, -NIBLSCALE(2), -NIBLSCALE(2));
  
  const TCHAR* text = Sender->GetCaption();
  Surface.DrawText(text, &PrintAreaR, DT_VCENTER|DT_SINGLELINE);

  Surface.SelectObject(ohB);
  Surface.SelectObject(ohP);
  Surface.SelectObject(oldFont);
   
}

CallBackTableEntry_t IGCCallBackTable[] = {
    OnPaintCallbackEntry(OnIGCSplashPaint),
    OnPaintCallbackEntry(OnIGCProgressPaint),
    ClickNotifyCallbackEntry(OnIGCAbortClicked),
    EndCallBackEntry()
};

class dlgIGCProgress final  {
public:
    dlgIGCProgress();
    ~dlgIGCProgress();
    
    dlgIGCProgress( const dlgIGCProgress& ) = delete;
    dlgIGCProgress& operator=( const dlgIGCProgress& ) = delete;

    void SetIGCProgressText(const TCHAR* szText);

private:
    WndForm* _WndForm;
};

dlgIGCProgress::dlgIGCProgress() {
    
    _WndForm = dlgLoadFromXML(IGCCallBackTable, ScreenLandscape ? IDR_XML_IGC_PROGRESS_L : IDR_XML_IGC_PROGRESS_P);
    LKASSERT(_WndForm);
    if(_WndForm) {
        WindowControl* wSplash = _WndForm->FindByName(TEXT("frmIGCSplash")); 
        if(wSplash) {
            wSplash->SetWidth(_WndForm->GetWidth());
            wSplash->SetHeight(_WndForm->GetHeight());
        }
        WindowControl* wText = _WndForm->FindByName(TEXT("frmIGCText")); 
        if(wText) {
            wText->SetWidth(_WndForm->GetWidth());
            wText->SetTop(_WndForm->GetHeight() - wText->GetHeight());
        }
        _WndForm->Show();
        _WndForm->Redraw();
    }
}

dlgIGCProgress::~dlgIGCProgress() {
    delete _WndForm; 
}

void dlgIGCProgress::SetIGCProgressText(const TCHAR* szText) {
    WindowControl* wText = _WndForm->FindByName(TEXT("frmIGCText")); 
    if(wText) {
        wText->SetCaption(szText);
        wText->Redraw();
#ifndef USE_GDI
        MainWindow.Refresh();
#endif
    }
}

static dlgIGCProgress* pWndIGCProgress = NULL;

void CloseIGCProgressDialog() {
    delete pWndIGCProgress;
    pWndIGCProgress = NULL;
    SplashBitmap.Release();
}

void CreateIGCProgressDialog(const TCHAR* text) {
    if(!pWndIGCProgress) {
        pWndIGCProgress = new dlgIGCProgress();
    } 
    if(pWndIGCProgress) {
        pWndIGCProgress->SetIGCProgressText(text);
    }
}


void IGCProgressDialogText(const TCHAR* text) {

    if(pWndIGCProgress) {
        pWndIGCProgress->SetIGCProgressText(text);
    }
}

extern void   StopIGCRead(void);

 void OnIGCAbortClicked(WndButton* pWnd) {
TCHAR Tmp[200 ];
    (void)pWnd;

  StartupStore(_T("Abort pressed%s"), NEWLINE);
       _stprintf(Tmp, _T("%s ?"),MsgToken(2359));  // _@M2359_ "Abort Dolanload"
        if (MessageBoxX(Tmp, Tmp, mbOk) == IdOk)
        {
          StopIGCRead();
        }
    
}


