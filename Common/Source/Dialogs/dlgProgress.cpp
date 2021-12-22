/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgProgress.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 23 novembre 2014, 17:36
 */

#include "externs.h"
#include "dlgProgress.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "LKObjects.h"
#include "resource.h"
#include "Draw/LoadSplash.h"

class dlgProgress final {
public:
    dlgProgress();
    ~dlgProgress();

    dlgProgress( const dlgProgress& ) = delete;
    dlgProgress& operator=( const dlgProgress& ) = delete;    

    void SetProgressText(const TCHAR* szText);

private:

    void OnSplashPaint(WindowControl * Sender, LKSurface& Surface);
    static void OnProgressPaint(WindowControl * Sender, LKSurface& Surface);

    std::unique_ptr<WndForm> _WndForm;
    LKBitmap _SplashBitmap;
};

dlgProgress::dlgProgress() {

    _SplashBitmap = LoadSplash(_T("LKSTART"));

    using std::placeholders::_1;
    using std::placeholders::_2;

    CallBackTableEntry_t CallBackTable[] = {
        callback_entry("OnSplashPaint", std::bind(&dlgProgress::OnSplashPaint, this, _1, _2)),
        callback_entry("OnProgressPaint", dlgProgress::OnProgressPaint),
        EndCallBackEntry()
    };

    _WndForm.reset(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_PROGRESS_L : IDR_XML_PROGRESS_P));
    if(_WndForm) {
        WindowControl* wSplash = _WndForm->FindByName(TEXT("frmSplash")); 
        if(wSplash) {
            wSplash->SetWidth(_WndForm->GetWidth());
            wSplash->SetHeight(_WndForm->GetHeight());
        }
        WindowControl* wText = _WndForm->FindByName(TEXT("frmText")); 
        if(wText) {
            wText->SetWidth(_WndForm->GetWidth());
            wText->SetTop(_WndForm->GetHeight() - wText->GetHeight());
        }
        _WndForm->Show();
        _WndForm->Redraw();
    }
}

dlgProgress::~dlgProgress() {

}

void dlgProgress::SetProgressText(const TCHAR* szText) {
    if(_WndForm) {
        WindowControl* wText = _WndForm->FindByName(TEXT("frmText")); 
        if(wText) {
            wText->SetCaption(szText);
            wText->Redraw();
#ifndef USE_GDI
            main_window->Refresh();
#endif
        }
    }
}

void dlgProgress::OnSplashPaint(WindowControl * Sender, LKSurface& Surface) {
    DrawSplash(Surface, Sender->GetClientRect(), _SplashBitmap);
}

void dlgProgress::OnProgressPaint(WindowControl * Sender, LKSurface& Surface) {
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

static dlgProgress* pWndProgress = nullptr;

void CloseProgressDialog() {
    delete pWndProgress;
    pWndProgress = nullptr;
}

void CreateProgressDialog(const TCHAR* text) {
    if(!pWndProgress) {
        pWndProgress = new dlgProgress();
    } 
    if(pWndProgress) {
        pWndProgress->SetProgressText(text);
    }
}
