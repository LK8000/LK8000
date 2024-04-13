/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgProfiles.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"
#include "utils/printf.h"

static WndForm *wf=NULL;
static short profilemode=0;
static TCHAR profilesuffix[10];

static void OnSaveExistingClicked(WndButton* pWnd) {

  int file_index;
  WndProperty* wp;
  DataFieldFileReader *dfe;

  if ( CheckClubVersion() ) {
	ClubForbiddenMsg();
	return;
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpFile"));
  if (!wp) return;

  wp->OnLButtonDown((POINT){0,0});

  dfe = (DataFieldFileReader*) wp->GetDataField();


  file_index = dfe->GetAsInteger();

  if (file_index>0) {
	if(MessageBoxX(dfe->GetAsString(),
		// LKTOKEN  _@M509_ = "Overwrite profile?"
		MsgToken<509>(),
		mbYesNo) == IdYes) {

                TCHAR file_name[MAX_PATH];
                LocalPath(file_name,TEXT(LKD_CONF), dfe->GetPathFile());
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
		// LKTOKEN  _@M535_ = "Profile saved!"
		MessageBoxX(MsgToken<535>(),_T(""), mbOk);
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

  wp = wf->FindByName<WndProperty>(TEXT("prpFile"));
  if (!wp) return;
  dfe = (DataFieldFileReader*) wp->GetDataField();

  _tcscpy(profile_name,_T(""));
  dlgTextEntryShowModal(profile_name, 13); // max length including termination 0

  if (_tcslen(profile_name)<=0) return;


  _tcscat(profile_name, profilesuffix);
  LocalPath(file_name,TEXT(LKD_CONF), profile_name);
  dfe->Lookup(profile_name);
  file_index = dfe->GetAsInteger();

  if (file_index==0) {
	lk::snprintf(tmptext, TEXT("%s: %s"),
		// LKTOKEN  _@M458_ = "New profile"
		MsgToken<458>(),
		profile_name);

	if(MessageBoxX(tmptext,
		// LKTOKEN  _@M579_ = "Save ?"
		MsgToken<579>(),
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
		dfe->addFile(profile_name, profile_name);

		MessageBoxX(
		// LKTOKEN  _@M535_ = "Profile saved!"
		MsgToken<535>(),
		_T(""), mbOk);

		dfe->Set(0);
		return;
	}
  }

  if (file_index>0) {
	lk::snprintf(tmptext, TEXT("%s: %s"),
	// LKTOKEN  _@M533_ = "Profile already exists"
		MsgToken<533>(),
		profile_name);

	if (CheckClubVersion() ) {
		MessageBoxX(tmptext,
		// LKTOKEN  _@M162_ = "Cannot overwrite!"
		MsgToken<162>(),
		mbOk);
	} else {
		if(MessageBoxX(tmptext,
		// LKTOKEN  _@M510_ = "Overwrite?"
		MsgToken<510>(),
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
			MsgToken<535>(),
			_T(""), mbOk);
			return;
		}
	}
	dfe->Set(0);
  }
}

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
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

  wf = dlgLoadFromXML(CallBackTable, IDR_XML_PROFILES);

  if (!wf) return;

  (wf->FindByName<WndButton>(TEXT("cmdClose"))) ->SetOnClickNotify(OnCloseClicked);
  (wf->FindByName<WndButton>(TEXT("cmdSaveExisting"))) ->SetOnClickNotify(OnSaveExistingClicked);
  (wf->FindByName<WndButton>(TEXT("cmdSaveNew"))) ->SetOnClickNotify(OnSaveNewClicked);


  WndProperty* wp;
  wp = wf->FindByName<WndProperty>(TEXT("prpFile"));
  if (wp) {
    wp->SetVisible(false);
    DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(wp->GetDataField());
    if(dfe) {
	  switch (profilemode) {
		case 0:
			_stprintf(profilesuffix,_T("%s"),_T(LKS_PRF));
            dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_PRF));
			break;
		case 1:
			wf->SetCaption(MsgToken<1784>()); // Aircraft profiles
			_stprintf(profilesuffix,_T("%s"),_T(LKS_PILOT));
            dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_PILOT));
			break;
		case 2:
			wf->SetCaption(MsgToken<1783>()); // Pilot profiles
			_stprintf(profilesuffix,_T("%s"),_T(LKS_AIRCRAFT));
            dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_AIRCRAFT));
			break;
		case 3:
			wf->SetCaption(MsgToken<1819>()); // Device profiles
			_stprintf(profilesuffix,_T("%s"),_T(LKS_DEVICE));
            dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_DEVICE));
			break;
		default:
			return;
	  }
	  dfe->Set(0);
    }
  }

  wf->ShowModal();

  delete wf;

  wf = NULL;

}
