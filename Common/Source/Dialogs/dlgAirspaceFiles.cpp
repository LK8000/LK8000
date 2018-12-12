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


static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}





static void setVariables(void) {
  WndProperty *wp;
  static  TCHAR temptext[MAX_PATH];
   TCHAR tmp[80];



  for (unsigned int i=0 ; i < NO_AS_FILES; i++)
  {
    _tcscpy(temptext,szAirspaceFile[i]);
    _stprintf(tmp,_T("prpFile%1u"),i+1);
    wp = (WndProperty*)wf->FindByName(tmp);

    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
        dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), _T("*" LKS_AIRSPACES));
        dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), _T("*" LKS_OPENAIP));
        dfe->Lookup(temptext);
      }
      _stprintf(tmp,_T("%s %1u "), MsgToken(2339),i+1);  // _@M2339_ "Airspace File "
      wp->SetCaption(tmp);
      wp->RefreshDisplay();
    }
  }
}



static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  EndCallBackEntry()
};


void dlgAirspaceFilesShowModal(void){
  static  TCHAR temptext[MAX_PATH];
   TCHAR tmp[80];
  WndProperty *wp;



  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_FILES_L : IDR_XML_FILES_P);
  if (!wf) return;

  setVariables();
  wf->SetCaption(MsgToken(2338)); // _@M2338_ "Airspace Files" 
  wf->ShowModal();


  for (unsigned int i=0 ; i < NO_AS_FILES; i++)
  {
    _tcscpy(temptext,szAirspaceFile[i]);
    _stprintf(tmp,_T("prpFile%1u"),i+1);
    wp = (WndProperty*)wf->FindByName(tmp);
    if (wp) {
      DataFieldFileReader* dfe = (DataFieldFileReader*)wp->GetDataField();
      _tcscpy(temptext, dfe->GetPathFile());
      if (_tcscmp(temptext,szAirspaceFile[i])) {
        _tcscpy(szAirspaceFile[i],temptext);
        AIRSPACEFILECHANGED= true;
      }
    }
  }



  delete wf;
  wf = NULL;

}
