/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgProfiles.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"


static WndForm *wf=NULL;
static short profilemode=0;
static TCHAR profilesuffix[10];

extern void LKAircraftSave(const TCHAR *szFile);
extern void LKPilotSave(const TCHAR *szFile);
extern void LKDeviceSave(const TCHAR *szFile);

static void OnSaveExistingClicked(WndButton* pWnd) {

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

  wp->OnLButtonDown((POINT){0,0});

  dfe = (DataFieldFileReader*) wp->GetDataField();


  file_index = dfe->GetAsInteger();

  if (file_index>0) {
	_tcscpy(file_name,dfe->GetAsString());
	if(MessageBoxX(file_name, 
		// LKTOKEN  _@M509_ = "Overwrite profile?" 
		MsgToken(509), 
		mbYesNo) == IdYes) {

		switch (profilemode) {
			case 0:
				LKProfileSave(dfe->GetPathFile());
				break;
			case 1:
                        	LKPilotSave(dfe->GetPathFile());
				break;
			case 2:
                        	LKAircraftSave(dfe->GetPathFile());
				break;
			case 3:
                        	LKDeviceSave(dfe->GetPathFile());
				break;
			default:
				return;
		}
		// LKTOKEN  _@M535_ = "Profile saved!" 
		MessageBoxX(MsgToken(535),_T(""), mbOk);
		return;
	}
  	dfe->Set(0);
  } 
}

static void OnSaveNewClicked(WndButton* pWnd) {

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


  _tcscat(profile_name, profilesuffix);
  LocalPath(file_name,TEXT(LKD_CONF));
  _tcscat(file_name,TEXT(DIRSEP));
  _tcscat(file_name,profile_name);

  dfe->Lookup(file_name);
  file_index = dfe->GetAsInteger();

  if (file_index==0) {
	_stprintf(tmptext, TEXT("%s: %s"), 
		// LKTOKEN  _@M458_ = "New profile" 
		MsgToken(458), 
		profile_name);

	if(MessageBoxX(tmptext, 
		// LKTOKEN  _@M579_ = "Save ?" 
		MsgToken(579), 
		mbYesNo) == IdYes) {
		switch (profilemode) {
			case 0:
				LKProfileSave(file_name);
				break;
			case 1:
                        	LKPilotSave(file_name);
				break;
			case 2:
                        	LKAircraftSave(file_name);
				break;
			case 3:
                        	LKDeviceSave(file_name);
				break;
			default:
				return;
		}
		dfe->addFile(profile_name, file_name);

		MessageBoxX(
		// LKTOKEN  _@M535_ = "Profile saved!" 
		MsgToken(535), 
		_T(""), mbOk);

  		dfe->Set(0);
		return;
	}
  }

  if (file_index>0) {
	_stprintf(tmptext, TEXT("%s: %s"), 
	// LKTOKEN  _@M533_ = "Profile already exists" 
		MsgToken(533), 	
		profile_name);

	if (CheckClubVersion() ) {
		MessageBoxX(tmptext,
		// LKTOKEN  _@M162_ = "Cannot overwrite!" 
		MsgToken(162),
		mbOk);
	} else {
		if(MessageBoxX(tmptext, 
		// LKTOKEN  _@M510_ = "Overwrite?" 
		MsgToken(510), 
		mbYesNo) == IdYes) {

			switch (profilemode) {
				case 0:
					LKProfileSave(file_name);
					break;
				case 1:
	                        	LKPilotSave(file_name);
					break;
				case 2:
	                        	LKAircraftSave(file_name);
					break;
				case 3:
	                        	LKDeviceSave(file_name);
					break;
				default:
					return;
			}
			MessageBoxX(
			// LKTOKEN  _@M535_ = "Profile saved!" 
			MsgToken(535),
			_T(""), mbOk);
			return;
		}
	}
  	dfe->Set(0);
  }
}

static void OnCloseClicked(WndButton* pWnd) {
        wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnSaveNewClicked),
  ClickNotifyCallbackEntry(OnSaveExistingClicked),
  EndCallBackEntry()
};


void dlgProfilesShowModal(short mode){

  wf = NULL;
  profilemode=mode;

  TCHAR filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgProfiles.xml"));
  wf = dlgLoadFromXML(CallBackTable, filename, TEXT("IDR_XML_PROFILES"));

  if (!wf) return;

  ((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetOnClickNotify(OnCloseClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSaveExisting"))) ->SetOnClickNotify(OnSaveExistingClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdSaveNew"))) ->SetOnClickNotify(OnSaveNewClicked);


  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpFile"));
  if (wp) {
    wp->SetVisible(false);
	DataFieldFileReader* dfe;
	dfe = (DataFieldFileReader*)wp->GetDataField();

	TCHAR tsuff[10];
	switch (profilemode) {
		case 0:
			_stprintf(profilesuffix,_T("%s"),_T(LKS_PRF));
			_stprintf(tsuff,_T("*%s"),_T(LKS_PRF));
			break;
		case 1:
			_stprintf(profilesuffix,_T("%s"),_T(LKS_PILOT));
			_stprintf(tsuff,_T("*%s"),_T(LKS_PILOT));
			wf->SetCaption(MsgToken(1784)); // Aircraft profiles
			break;
		case 2:
			wf->SetCaption(MsgToken(1783)); // Pilot profiles
			_stprintf(profilesuffix,_T("%s"),_T(LKS_AIRCRAFT));
			_stprintf(tsuff,_T("*%s"),_T(LKS_AIRCRAFT));
			break;
		case 3:
			wf->SetCaption(MsgToken(1819)); // Device profiles
			_stprintf(profilesuffix,_T("%s"),_T(LKS_DEVICE));
			_stprintf(tsuff,_T("*%s"),_T(LKS_DEVICE));
			break;
		default:
			return;
	}

	dfe->ScanDirectoryTop(_T(LKD_CONF),tsuff);
  	dfe->Set(0);
  }
 
  wf->ShowModal();

  delete wf;

  wf = NULL;

}
