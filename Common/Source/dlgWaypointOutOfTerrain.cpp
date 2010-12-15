/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWaypointOutOfTerrain.cpp,v 8.2 2010/12/13 16:58:13 root Exp root $
*/

#include "StdAfx.h"

#include "Statistics.h"

#include "externs.h"
#include "Units.h"
#include "Waypointparser.h"

#include "dlgTools.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;

static void OnYesClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(wpTerrainBoundsYes);
}

static void OnYesAllClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(wpTerrainBoundsYesAll);
}

static void OnNoClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(wpTerrainBoundsNo);
}

static void OnNoAllClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(wpTerrainBoundsNoAll);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnYesClicked),
  DeclareCallBackEntry(OnYesAllClicked),
  DeclareCallBackEntry(OnNoClicked),
  DeclareCallBackEntry(OnNoAllClicked),
  DeclareCallBackEntry(NULL)
};

int dlgWaypointOutOfTerrain(TCHAR *Message){

  WndFrame* wfrm;
  int res = 0;

#ifdef HAVEEXCEPTIONS
  __try{
#endif

    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWaypointOutOfTerrain.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
		        
                        filename, 
		        hWndMainWindow,
		        TEXT("IDR_XML_WAYPOINTTERRAIN"));

    if (wf) {

    
      wfrm = (WndFrame*)wf->FindByName(TEXT("frmWaypointOutOfTerrainText"));

      wfrm->SetCaption(Message);
      wfrm->SetCaptionStyle(
          DT_EXPANDTABS
        | DT_CENTER
        | DT_NOCLIP
        | DT_WORDBREAK);


      res = wf->ShowModal();
      delete wf;

    }

    wf = NULL;

#ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER ){

    res = 0; 
    // ToDo: log that problem

  };
#endif

  return(res);

}

