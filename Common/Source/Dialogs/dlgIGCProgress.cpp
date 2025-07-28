/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

extern void  StopIGCRead(void);
extern void  EOS_StopIGCRead(void);
extern void  OnAbort_IGC_FileRead(void);

static bool OnIGCProgressTimer(WndForm* pWnd);
static bool bClose = false;
#define MAX_STATUS_TXT_LEN  127
TCHAR m_szTmp[MAX_STATUS_TXT_LEN] =_T("...");

static bool OnIGCProgressTimer(WndForm *pWnd) {
  if (pWnd) {
      pWnd->SetTimerNotify(0, NULL); 
      WindowControl *wText = pWnd->FindByName(TEXT("frmIGCText"));
      if (wText) {
        wText->SetCaption(m_szTmp);
     wText->Redraw();
#ifndef USE_GDI
     main_window->Refresh();
#endif
        pWnd->SetTimerNotify(500, OnIGCProgressTimer); 
      }

    if (bClose)
      pWnd->SetModalResult(mrOK);
  }
  return true;
}


static void OnIGCSplashPaint(WndOwnerDrawFrame * Sender, LKSurface& Surface) {
    LKBitmap SplashBitmap;
    if(!SplashBitmap) {
        SplashBitmap = LoadSplash(_T("LKSTART"));
    }
    DrawSplash(Surface, Sender->GetClientRect(), SplashBitmap);

}

static void OnIGCProgressPaint(WndOwnerDrawFrame * Sender, LKSurface& Surface) {

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

CallBackTableEntry_t IGCProgressCallBackTable[] = {
    CallbackEntry(OnIGCSplashPaint),
    CallbackEntry(OnIGCProgressPaint),
    CallbackEntry(OnIGCAbortClicked),
    EndCallbackEntry()
};

void dlgIGCProgressShowModal(void){
    
	bClose = false;

    WndForm* _WndForm = dlgLoadFromXML(IGCProgressCallBackTable, ScreenLandscape ? IDR_XML_IGC_PROGRESS_P : IDR_XML_IGC_PROGRESS_L);

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
//        _WndForm->Show();
//        _WndForm->Redraw();
        _WndForm->SetTimerNotify(200, OnIGCProgressTimer); // check for end of download every 200ms
        _WndForm->ShowModal();
        delete _WndForm;
    }
}



void CloseIGCProgressDialog() {
  _stprintf(m_szTmp, TEXT("..."));
    bClose = true;

}

void CreateIGCProgressDialog() {
   dlgIGCProgressShowModal();
}


void IGCProgressDialogText(const TCHAR *text) {
	_tcsncpy(m_szTmp,text,MAX_STATUS_TXT_LEN - 1);
  m_szTmp[MAX_STATUS_TXT_LEN - 1] = _T('\0');
}


 void OnIGCAbortClicked(WndButton* pWnd) {

    (void)pWnd;

	StartupStore(_T("Abort pressed%s"), NEWLINE);


	if (MessageBoxX(MsgToken<2417>(), MsgToken<2398>(), mbYesNo) == IdYes) // _@M2417_ "Abort Download ?"
	{
	  CloseIGCProgressDialog();
	  StopIGCRead();
      EOS_StopIGCRead();
	  OnAbort_IGC_FileRead();
	}

}


