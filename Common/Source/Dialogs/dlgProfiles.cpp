/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgProfiles.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <aygshell.h>

#include "Logger.h"
#include "McReady.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "LKProfiles.h"

extern void SettingsEnter();
extern void SettingsLeave();

static WndForm *wf=NULL;

static void OnSaveExistingClicked(WindowControl * Sender) {
  (void)Sender;

  int file_index; 
  TCHAR file_name[MAX_PATH];
  WndProperty* wp;
  DataFieldFileReader *dfe;

  if ( CheckClubVersion() ) {
	ClubForbiddenMsg();
	return;
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;

  HWND hwnd = wp->GetHandle();
  SendMessage(hwnd,WM_LBUTTONDOWN,0,0);
  dfe = (DataFieldFileReader*) wp->GetDataField();


  file_index = dfe->GetAsInteger();

  if (file_index>0) {
	_tcscpy(file_name,dfe->GetAsString());
	if(MessageBoxX(hWndMapWindow, file_name, 
	// LKTOKEN  _@M509_ = "Overwrite profile?" 
		gettext(TEXT("_@M509_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {
		#if OLDPROFILES
		WriteProfile(dfe->GetPathFile());
		#else
		LKProfileSave(dfe->GetPathFile());
		#endif
	// LKTOKEN  _@M535_ = "Profile saved!" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M535_")),_T(""), MB_OK|MB_ICONEXCLAMATION);
		return;
	}
  	dfe->Set(0);
  } 

}

static void OnSaveNewClicked(WindowControl * Sender) {
  (void)Sender;

  int file_index; 
  TCHAR file_name[MAX_PATH];
  TCHAR profile_name[MAX_PATH];
  TCHAR tmptext[MAX_PATH];
  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;
  dfe = (DataFieldFileReader*) wp->GetDataField();

  _tcscpy(profile_name,_T(""));
  dlgTextEntryShowModal(profile_name, 13); // max length including termination 0

  if (_tcslen(profile_name)<=0) return;


  _tcscat(profile_name, TEXT(LKS_PRF));
  LocalPath(file_name,TEXT(LKD_CONF));
  _tcscat(file_name,TEXT("\\"));
  _tcscat(file_name,profile_name);

  dfe->Lookup(file_name);
  file_index = dfe->GetAsInteger();

  if (file_index==0) {
	_stprintf(tmptext, TEXT("%s: %s"), 
	// LKTOKEN  _@M458_ = "New profile" 
		gettext(TEXT("_@M458_")), 
		profile_name);

	if(MessageBoxX(hWndMapWindow, tmptext, 
	// LKTOKEN  _@M579_ = "Save ?" 
		gettext(TEXT("_@M579_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {
		#if OLDPROFILES
		WriteProfile(file_name);
		#else
		LKProfileSave(file_name);
		#endif
		dfe->addFile(profile_name, file_name);

		MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M535_ = "Profile saved!" 
		gettext(TEXT("_@M535_")), 
		_T(""), MB_OK|MB_ICONEXCLAMATION);

  		dfe->Set(0);
		return;
	}
  }

  if (file_index>0) {
	_stprintf(tmptext, TEXT("%s: %s"), 
	// LKTOKEN  _@M533_ = "Profile already exists" 
		gettext(TEXT("_@M533_")), 	
		profile_name);

	if (CheckClubVersion() ) {
		MessageBoxX(hWndMapWindow, tmptext,
	// LKTOKEN  _@M162_ = "Cannot overwrite!" 
		gettext(TEXT("_@M162_")),
		MB_OK|MB_ICONEXCLAMATION);
	} else {
		if(MessageBoxX(hWndMapWindow, tmptext, 
	// LKTOKEN  _@M510_ = "Overwrite?" 
		gettext(TEXT("_@M510_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {

			#if OLDPROFILES
			WriteProfile(file_name);
			#else
			LKProfileSave(file_name);
			#endif
			MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M535_ = "Profile saved!" 
			gettext(TEXT("_@M535_")),
			_T(""), MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}
  	dfe->Set(0);
  }


}

static void OnCloseClicked(WindowControl * Sender){
(void)Sender;
        wf->SetModalResult(mrOK);
}


#if 0 // UNUSED in 3.0
static void OnLoadClicked(WindowControl * Sender){

 TCHAR file_name[MAX_PATH];

  WndProperty* wp;
  DataFieldFileReader *dfe;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (!wp) return;

  HWND hwnd = wp->GetHandle();
  SendMessage(hwnd,WM_LBUTTONDOWN,0,0);
  dfe = (DataFieldFileReader*) wp->GetDataField();

  int file_index = dfe->GetAsInteger();
  if (file_index>0) {
	_tcscpy(file_name,dfe->GetAsString());

	if(MessageBoxX(hWndMapWindow, file_name, 
	// LKTOKEN  _@M397_ = "Load this profile?" 
		gettext(TEXT("_@M397_")), 
		MB_YESNO|MB_ICONQUESTION) == IDYES) {
		SettingsEnter();
		#if OLDPROFILE
		ReadProfile(dfe->GetPathFile());
		WAYPOINTFILECHANGED=true;
		#else
		LKProfileLoad(dfe->GetPathFile());
		#endif
		SettingsLeave();
		MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M534_ = "Profile loaded!" 
		gettext(TEXT("_@M534_")),
		_T(""), MB_OK|MB_ICONEXCLAMATION);
		return;
	}
  	dfe->Set(0);
  }
  
}
#endif


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnSaveNewClicked),
  DeclareCallBackEntry(OnSaveExistingClicked),
//   DeclareCallBackEntry(OnLoadClicked), // UNUSED in 3.0
  DeclareCallBackEntry(NULL)
};


void dlgProfilesShowModal(void){

  wf = NULL;

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgProfiles.xml"));
  wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_PROFILES"));

  if (!wf) return;

  //ASSERT(wf!=NULL);

  ((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetOnClickNotify(OnCloseClicked);
// ((WndButton *)wf->FindByName(TEXT("cmdLoad"))) ->SetOnClickNotify(OnLoadClicked);  Unused in 3.0
  ((WndButton *)wf->FindByName(TEXT("cmdSaveExisting"))) ->SetOnClickNotify(OnSaveExistingClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSaveNew"))) ->SetOnClickNotify(OnSaveNewClicked);



  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (wp) {
	DataFieldFileReader* dfe;
	dfe = (DataFieldFileReader*)wp->GetDataField();

	TCHAR suff[10];
	_stprintf(suff,_T("*%S"),LKS_PRF);
	dfe->ScanDirectoryTop(_T(LKD_CONF),suff);
  	dfe->Set(0);
  }

  wf->ShowModal();

  delete wf;

  wf = NULL;

}
