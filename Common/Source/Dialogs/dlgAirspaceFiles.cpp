/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceFiles.cpp,v 1.1 2018/12/12 15:16:00 root Exp root $
*/
#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"


static void getVariables(WndForm * pForm) {
  TCHAR tmp[MAX_PATH];

  for (unsigned int i=0 ; i < NO_AS_FILES; i++) {
    _stprintf(tmp,_T("prpFile%1u"),i+1);
    WndProperty* wp = pForm->FindByName<WndProperty>(tmp);
    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
        const TCHAR* str = dfe->GetPathFile();
        if(!str) {
          str = _T("");
        }
        if(_tcscmp(szAirspaceFile[i], str)!= 0) {
          lk::strcpy(szAirspaceFile[i], str);
          AIRSPACEFILECHANGED= true;
        }
      }
    }
  }
}


static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      getVariables(pForm);

      bool bIdenti = false;
      for (unsigned int i=0 ; i < NO_AS_FILES-1; i++) {
        if (_tcslen(szAirspaceFile[i])> 0) {
          for (unsigned int j=(i+1) ; j < NO_AS_FILES; j++) {
            TCHAR tmp[MAX_PATH];
            if (_tcslen(szAirspaceFile[j])> 0) {
              if (_tcscmp(szAirspaceFile[i],szAirspaceFile[j])==0) {
                _sntprintf(tmp, MAX_PATH, _T("%s %u %s %u %s!"), 
                                  MsgToken<2338>(), // _@M2338_ "Airspace Files"
                                  i+1,
                                  MsgToken<2345>() , //_@M2345_ "and"
                                  j+1,
                                  MsgToken<2346>() //_@M2346_ "are identical"
                                );
                MessageBoxX(  tmp, MsgToken<356>(),  mbOk) ;  // _@M356_ "Information"
                bIdenti = true;
              }
            }
          }
        }
      }
      if (bIdenti) {
        return;
      }
      pForm->SetModalResult(mrOK);
    }
  }
}


static void setVariables(WndForm * pForm) {
  TCHAR tmp[100];

  for (unsigned int i=0 ; i < NO_AS_FILES; i++) {
    _stprintf(tmp, _T("prpFile%1u"), i + 1);
    auto wp = pForm->FindByName<WndProperty>(tmp);

    if (wp) {
      DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
      if(dfe) {
        const TCHAR* suffix_filters[] = {
          _T(LKS_AIRSPACES),
          _T(LKS_OPENAIP),
          _T(LKS_OPENAIR)
        };
        dfe->ScanDirectoryTop(_T(LKD_AIRSPACES), suffix_filters);
        dfe->Lookup(szAirspaceFile[i]);
      }
      wp->RefreshDisplay();
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnCloseClicked),
  EndCallbackEntry()
};


void dlgAirspaceFilesShowModal(){
  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_FILES_L : IDR_XML_FILES_P));
  if (!pForm) return;

  setVariables(pForm.get());
  pForm->SetCaption(MsgToken<2338>()); // _@M2338_ "Airspace Files" 
  pForm->ShowModal();
}
