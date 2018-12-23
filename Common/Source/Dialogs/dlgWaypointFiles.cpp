/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceWarningParams.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"



static WndForm *wf=NULL;


static void getVariables(void) {
  WndProperty *wp;

   TCHAR tmp[MAX_PATH];

  for (unsigned int i=0 ; i < NO_WP_FILES; i++)
  {
    _stprintf(tmp,_T("prpFile%1u"),i+1);
    wp = (WndProperty*)wf->FindByName(tmp);

    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
    	  if(dfe->GetAsDisplayString() != NULL)
            _sntprintf(szWaypointFile[i], MAX_PATH ,_T("%s"), dfe->GetAsDisplayString());
    	  else
    		  szWaypointFile[i][0] = {'\0'};
      }
    }
  }
}


static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    bool bIdenti = false;
    getVariables();
    for (unsigned int i=0 ; i < NO_WP_FILES-1; i++)
    {
      if((szWaypointFile[i] != NULL) &&  (_tcslen (szWaypointFile[i])> 0))
      for (unsigned int j=(i+1) ; j < NO_WP_FILES; j++)
      {
        TCHAR tmp[MAX_PATH];
        if((szWaypointFile[j] != NULL) && (_tcslen (szWaypointFile[j])> 0))
        {
		  if(_tcscmp(szWaypointFile[i],szWaypointFile[j])==0)
		  {
		   _sntprintf(tmp, MAX_PATH, _T("%s %u %s %u %s!"), MsgToken(2340), // _@M2340_ "Waypoint Files"
														    i+1,
															MsgToken(2345) , //_@M2345_ "and"
															j+1,
															MsgToken(2346) //_@M2346_ "are identical"
														 );
		    MessageBoxX(  tmp, MsgToken(356),  mbOk) ;  // _@M356_ "Information"
		    bIdenti = true;
		  }
        }
      }
    }
    if( bIdenti) return ;


    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}




static void setVariables(void) {
WndProperty *wp;
TCHAR temptext[MAX_PATH];
TCHAR tmp[MAX_PATH];

  for (unsigned int i=0 ; i < NO_WP_FILES; i++)
  {
    _tcscpy(temptext,szWaypointFile[i]);
    _sntprintf(tmp,MAX_PATH,_T("prpFile%1u"),i+1);
    wp = (WndProperty*)wf->FindByName(tmp);

    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
          dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_WINPILOT));
          dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_XCSOAR));
          dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_CUP));
          dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_WP_COMPE));
          dfe->ScanDirectoryTop(_T(LKD_WAYPOINTS), _T("*" LKS_OPENAIP));
          dfe->Lookup(temptext);



        _sntprintf(tmp,MAX_PATH,_T("%s %1u "), MsgToken(2341),i+1);  // _@M2341_ "Waypoint File"
        wp->SetCaption(tmp);
      }
      wp->RefreshDisplay();
    }
  }
}



static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgWaypointFilesShowModal(void){
TCHAR temptext[MAX_PATH];
TCHAR tmp[MAX_PATH];
WndProperty *wp;



  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_FILES_L : IDR_XML_FILES_P);
  if (!wf) return;

  setVariables();
  wf->SetCaption(MsgToken(2340)); 
  wf->ShowModal();


  for (unsigned int i=0 ; i < NO_WP_FILES; i++)
  {
    _tcscpy(temptext,szWaypointFile[i]);
    _sntprintf(tmp,MAX_PATH,_T("prpFile%1u"),i+1);
    wp = (WndProperty*)wf->FindByName(tmp);
    if (wp) {
      DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
      _tcscpy(temptext, dfe->GetPathFile());
      if (_tcscmp(temptext,szWaypointFile[i])) {
        _tcscpy(szWaypointFile[i],temptext);
        WAYPOINTFILECHANGED= true;
      }
    }
  }


  delete wf;
  wf = NULL;

}
