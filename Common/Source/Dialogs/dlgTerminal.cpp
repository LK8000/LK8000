/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "TraceThread.h"
#include <ctype.h>
#include "dlgTools.h"
#include "ComCheck.h"
#include "resource.h"


static WndForm *wf=NULL;
static WndListFrame *wTTYList=NULL;
static WndOwnerDrawFrame *wTTYListEntry = NULL;

static TCHAR tmps[100];


#define INTERLINE NIBLSCALE(2);

static bool OnTimerNotify() {
    wTTYList->Redraw();
    return true;
}


static void OnPaintListItem(WindowControl * Sender, LKSurface& Surface){
  (void)Sender;

  unsigned int hline, hframe, numlines;
  short active;

  Surface.SetTextColor(RGB_BLACK);
  Surface.SelectObject(LK8GenericVar02Font);
  active=ComCheck_ActivePort; // can change in thread
  if (active<0) return;

  hline = Surface.GetTextHeight(TEXT("M"))+INTERLINE;
  hframe=wTTYListEntry->GetHeight();
  numlines=hframe/hline -1;
  if (numlines>=CC_NUMBUFLINES) numlines=CC_NUMBUFLINES-1;

  unsigned int y=0, first, last;

  _stprintf(tmps,_T("[ Rx=%ld ErrRx=%ld Tx=%ld ErrTx=%ld ]"),
      ComPortRx[active],ComPortErrRx[active],ComPortTx[active],ComPortErrTx[active]);
  Surface.DrawText(0, 0, tmps);
  y+=hline;


  if (ComCheck_Reset>=0 || (ComCheck_LastLine==0 && ComCheck_BufferFull==false)) {
      _stprintf(tmps,_T("%s"),MsgToken(1872)); // NO DATA RECEIVED
      Surface.DrawText(0, y, tmps);
      return;
  }

  last=ComCheck_LastLine;
  if (ComCheck_BufferFull==false && ComCheck_LastLine<=numlines) {
      first=0;
  } else {
      int dif = ComCheck_LastLine-numlines;
      if (dif>=0)
          first=dif;
      else
          first= (CC_NUMBUFLINES-1) + dif;
  }

  for (unsigned int n=0, curr=first; n<numlines; n++) { 
      BUGSTOP_LKASSERT(curr<CC_NUMBUFLINES);
      if (curr>=CC_NUMBUFLINES) break;
      Surface.DrawTextClip(0, y, ComCheckBuffer[curr], wTTYListEntry->GetWidth());
      y+=hline;
      if (curr==last) break;
      if (++curr>(CC_NUMBUFLINES-1)) curr=0;
  }

}


static bool stopped=false;

static void OnPort1Clicked(WndButton* pWnd) {
  // Name is available only in Fly mode, not inited in SIM mode because no devices, and not inited if disabled
  _stprintf(tmps,_T("%s: %s (%s)"),MsgToken(1871),MsgToken(232), 
     _tcslen(DeviceList[0].Name)>0?DeviceList[0].Name:MsgToken(1600));
  wf->SetCaption(tmps);
  ComCheck_ActivePort=0; // needed
  ComCheck_Reset=0;
  wf->SetTimerNotify(500, OnTimerNotify);
  ((WndButton *)wf->FindByName(TEXT("cmdSelectStop")))->SetCaption(MsgToken(670)); // Stop
  stopped=false;
}

static void OnPort2Clicked(WndButton* pWnd) {
  _stprintf(tmps,_T("%s: %s (%s)"),MsgToken(1871),MsgToken(233), 
      _tcslen(DeviceList[1].Name)>0?DeviceList[1].Name:MsgToken(1600));
  wf->SetCaption(tmps);
  ComCheck_ActivePort=1; // needed
  ComCheck_Reset=1;
  wf->SetTimerNotify(500, OnTimerNotify);
  ((WndButton *)wf->FindByName(TEXT("cmdSelectStop")))->SetCaption(MsgToken(670)); // Stop
  stopped=false;
}

static void OnStopClicked(WndButton* pWnd) {
  stopped=!stopped;
  wf->SetCaption(tmps);
  if (stopped) {
      _stprintf(tmps,_T("%s: %s %s"),MsgToken(1871), MsgToken(670),
           ComCheck_ActivePort==0?MsgToken(232):MsgToken(233));
      wf->SetCaption(tmps);
      wf->SetTimerNotify(0, NULL);
      ((WndButton *)wf->FindByName(TEXT("cmdSelectStop")))->SetCaption(MsgToken(1200)); // Start
  } else {
      if (ComCheck_ActivePort==0) {
          _stprintf(tmps,_T("%s: %s (%s)"),MsgToken(1871),MsgToken(232), 
              _tcslen(DeviceList[0].Name)>0?DeviceList[0].Name:MsgToken(1600));

      } else {
          _stprintf(tmps,_T("%s: %s (%s)"),MsgToken(1871),MsgToken(233), 
              _tcslen(DeviceList[1].Name)>0?DeviceList[1].Name:MsgToken(1600));
      }
      wf->SetCaption(tmps);
      wf->SetTimerNotify(500, OnTimerNotify);
      ((WndButton *)wf->FindByName(TEXT("cmdSelectStop")))->SetCaption(MsgToken(670)); // Stop
  }
}

static void OnTTYCloseClicked(WndButton* pWnd) {

  ComCheck_ActivePort=-1; 
  wf->SetModalResult(mrCancel);
}


static CallBackTableEntry_t CallBackTable[]={
  OnPaintCallbackEntry(OnPaintListItem),
  EndCallBackEntry()
};


void dlgTerminal(int portnumber) {

  SHOWTHREAD(_T("dlgTerminal"));

  wf = dlgLoadFromXML(CallBackTable, 
                        ScreenLandscape ? TEXT("dlgTerminal_L.xml") : TEXT("dlgTerminal_P.xml"), 
                        ScreenLandscape ? IDR_XML_TERMINAL_L : IDR_XML_TERMINAL_P);

  if (!wf) return;


  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnTTYCloseClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSelectPort1")))->SetOnClickNotify(OnPort1Clicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSelectPort2")))->SetOnClickNotify(OnPort2Clicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSelectStop")))->SetOnClickNotify(OnStopClicked);

  wTTYList = (WndListFrame*)wf->FindByName(TEXT("frmTTYList"));
  LKASSERT(wTTYList!=NULL);
  wTTYList->SetWidth(wf->GetWidth()-NIBLSCALE(2));

  wTTYListEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmTTYListEntry"));
  LKASSERT(wTTYListEntry!=NULL);
  wTTYListEntry->SetWidth(wf->GetWidth()-NIBLSCALE(2));

  switch(portnumber) {
      case 0:
          OnPort1Clicked(NULL);
          break;
      case 1:
          OnPort2Clicked(NULL);
          break;
      default:
          break;
  }



  wf->ShowModal();

  ComCheck_ActivePort=-1;
  delete wf;
  wf = NULL;
  return;

}
