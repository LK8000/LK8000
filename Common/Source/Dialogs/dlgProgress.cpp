/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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

extern void LoadSplash(LKSurface& Surface, const TCHAR *splashfile);

static void OnSplashPaint(WindowControl * Sender, LKSurface& Surface) {
    LoadSplash(Surface, _T("LKSTART"));
}

static void OnProgressPaint(WindowControl * Sender, LKSurface& Surface) {
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
  InflateRect(&PrintAreaR, -NIBLSCALE(2), -NIBLSCALE(2));
  Surface.Rectangle(PrintAreaR.left,PrintAreaR.top,PrintAreaR.right,PrintAreaR.bottom);

  Surface.SetTextColor(RGB_WHITE);
  Surface.SetBackgroundTransparent();

  InflateRect(&PrintAreaR, -NIBLSCALE(2), -NIBLSCALE(2));
  
  const TCHAR* text = Sender->GetCaption();
  Surface.DrawText(text,_tcslen(text), &PrintAreaR, DT_VCENTER|DT_SINGLELINE);

  Surface.SelectObject(ohB);
  Surface.SelectObject(ohP);
  Surface.SelectObject(oldFont);
   
}

CallBackTableEntry_t CallBackTable[] = {
    OnPaintCallbackEntry(OnSplashPaint),
    OnPaintCallbackEntry(OnProgressPaint),
    EndCallBackEntry()
};

dlgProgress::dlgProgress() {
    
	TCHAR filename[MAX_PATH];
	LocalPathS(filename, TEXT("dlgProgress.xml"));
    
    _WndForm = dlgLoadFromXML(CallBackTable, filename, TEXT("IDR_XML_PROGRESS"));
    assert(_WndForm);
    if(_WndForm) {
        WindowControl* wSplash = _WndForm->FindByName(TEXT("frmSplash")); 
        if(wSplash) {
            wSplash->SetWidth(_WndForm->GetWidth());
        }
        WindowControl* wText = _WndForm->FindByName(TEXT("frmText")); 
        if(wText) {
            wText->SetWidth(_WndForm->GetWidth());
        }
        _WndForm->Show();
        _WndForm->Redraw();
    }
}

dlgProgress::~dlgProgress() {
    delete _WndForm; 
}

void dlgProgress::SetProgressText(const TCHAR* szText) {
    WindowControl* wText = _WndForm->FindByName(TEXT("frmText")); 
    if(wText) {
        wText->SetCaption(szText);
        wText->Redraw();
#ifndef USE_GDI
        MainWindow.Refresh();
#endif
    }
}
