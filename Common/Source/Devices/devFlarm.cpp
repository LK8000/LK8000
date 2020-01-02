/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "devFlarm.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "dlgIGCProgress.h"
#include "Util/Clamp.hpp"
#include "dlgFlarmIGCDownload.h"
#include <queue>
#include "Thread/Mutex.hpp"
#include "Thread/Cond.hpp"






DeviceDescriptor_t* CDevFlarm::m_pDevice=NULL;


void CDevFlarm::Install(DeviceDescriptor_t* d ) {
  StartupStore(_T("Flarm Drvier Install %s"), NEWLINE);
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = FlarmParse ; // ParseNMEA;
	d->Open = Open;
	d->Close = Close;
	d->Config = Config;
	d->ParseStream  = FlarmParseString;
}

namespace {
  std::queue<uint8_t> buffered_data;
  Mutex mutex;
  Cond cond;
  bool bFLARM_BinMode = false;
}

BOOL CDevFlarm::FlarmParse(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info) {
  if (IsInBinaryMode()) {
    if (strncmp("$PFLAU", sentence, 6) == 0) {
      StartupStore(TEXT("$PFLAU detected, disable binary mode!" ));
      SetBinaryModeFlag(false);
    }
    return true; // ignore all data ...
  }
  return false;
}

BOOL CDevFlarm::FlarmParseString(DeviceDescriptor_t* d, char *String, int len, NMEA_INFO *GPS_INFO) {
  if ((!d) || (!String) || (!len)) {
    return FALSE;
  }

  ScopeLock lock(mutex);

  if (!IsInBinaryMode()) {
    return FALSE;
  }

  for (int i = 0; i < len; i++) {
    buffered_data.push(String[i]);
  }
  cond.Broadcast();

  return  true;
}

/**
 * return true if received data is available.
 */
bool BlockReceived() {
  ScopeLock lock(mutex);
  return (!buffered_data.empty());
}

bool IsInBinaryMode() {
  ScopeLock lock(mutex);
  return bFLARM_BinMode;
}

bool SetBinaryModeFlag(bool bBinMode) {
  ScopeLock lock(mutex);
  bool OldVal = bFLARM_BinMode;
  bFLARM_BinMode = bBinMode;
  if(!bFLARM_BinMode) {
    // same as clear() but free allocated memory.
    buffered_data = std::queue<uint8_t>();
  }
  return OldVal;
}

uint8_t RecChar(DeviceDescriptor_t* d, uint8_t *inchar, uint16_t Timeout) {
  ScopeLock lock(mutex);

  while(buffered_data.empty()) {
    if(!cond.Wait(mutex, Timeout)) {
      return REC_TIMEOUT_ERROR;
    }
  }
  if(inchar) {
    *inchar = buffered_data.front();
  }
  buffered_data.pop();
  return REC_NO_ERROR;
}

BOOL CDevFlarm::Open(DeviceDescriptor_t* d) {
	m_pDevice = d;
	return TRUE;
}

BOOL CDevFlarm::Close (DeviceDescriptor_t* d) {
  if(IsInBinaryMode()) { // if FLARM is in Binary Mode?
    FlarmReboot(d);
  }
  m_pDevice = NULL;
  return TRUE;
}









CallBackTableEntry_t CDevFlarm::CallBackTable[]={
  EndCallBackEntry()
};

BOOL CDevFlarm::Config(DeviceDescriptor_t* d){
        if(m_pDevice != d) {
                StartupStore(_T("Flarm Config : Invalide device descriptor%s"), NEWLINE);
                return FALSE;
        }

        WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_DEVFLARM);
        if(wf) {

        WndButton *wBt = NULL;

        wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
        if(wBt){
                wBt->SetOnClickNotify(OnCloseClicked);
        }


        wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));
        if(wBt){
                wBt->SetOnClickNotify(OnIGCDownloadClicked);
        }

        wBt = (WndButton *)wf->FindByName(TEXT("cmdFlarmReboot"));
        if(wBt){
                wBt->SetOnClickNotify(OnRebootClicked);
        }
/*
        WndProperty* wp;
        wp = (WndProperty*)wf->FindByName(TEXT("prpFlarmId"));
        if (wp) {
        	wp->GetDataField()->SetAsString(_T("DD222"));

          wp->RefreshDisplay();
        }
*/



                wf->ShowModal();

                delete wf;
                wf=NULL;
        }
        return TRUE;
}




void CDevFlarm::OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}



void CDevFlarm::OnIGCDownloadClicked(WndButton* pWnd) {
	(void)pWnd;
LockFlightData();
bool bFlarmActive = GPS_INFO.FLARM_Available;
bool bInFlight    = CALCULATED_INFO.Flying;
UnlockFlightData();

	if(bInFlight)	{
	  MessageBoxX(MsgToken<2418>(), MsgToken<2397>(), mbOk);
	  return;
	}

	if(!bFlarmActive)	{
	  MessageBoxX(MsgToken<2401>(), MsgToken<2397>(), mbOk);
#ifdef NO_FAKE_FLARM
	  return;
#endif
     }
	if(m_pDevice) {
	  dlgIGCSelectListShowModal(m_pDevice);
	}
}


void CDevFlarm::OnRebootClicked(WndButton* pWnd) {
        (void)pWnd;

    StartupStore(TEXT("OnRebootClicked"));
    if(m_pDevice) {
        FlarmReboot(m_pDevice);
    }
}

BOOL CDevFlarm::FlarmReboot(DeviceDescriptor_t* d) {
    if (d && d->Com) {
        LeaveBinModeWithReset(d);
    }
    return TRUE;
}
