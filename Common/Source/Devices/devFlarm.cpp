/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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
#include "OS/Sleep.h"
#include "dlgFlarmIGCDownload.h"







PDeviceDescriptor_t CDevFlarm::m_pDevice=NULL;








bool CDevFlarm::Register(){
	return devRegister(GetName(), cap_baro_alt|cap_vario, &Install);
}

BOOL CDevFlarm::Install( PDeviceDescriptor_t d ) {
  StartupStore(_T("Flarm Drvier Install %s"), NEWLINE);
	_tcscpy(d->Name, GetName());
	d->ParseNMEA = FlarmParse ; // ParseNMEA;
	d->Open = Open;
	d->Close = Close;
	d->IsGPSSource = GetFalse;
	d->IsBaroSource = GetTrue;
	d->Config = Config;
	d->ParseStream  = FlarmParseString;

	return(TRUE);
}


uint8_t RingBuff[REC_BUFFERSIZE+1];
volatile  uint16_t InCnt=0;
volatile  uint16_t OutCnt=0;
static bool recEnable = true;



BOOL CDevFlarm::FlarmParse(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{
  if(IsInBinaryMode ())
  {
    if (_tcsncmp(_T("$PFLAU"), sentence, 6) == 0)
    {
	  StartupStore(TEXT("$PFLAU detected, disable binary mode!" ));
	  SetBinaryModeFlag (false);
    }
  }
  return false;
}


BOOL CDevFlarm::FlarmParseString(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO)
{
if(d == NULL) return 0;
if(String == NULL) return 0;
if(len == 0) return 0;

int cnt=0;

if(recEnable)
  while (cnt < len)
  {
    RingBuff[InCnt++] = (TCHAR) String[cnt++];
    InCnt %= REC_BUFFERSIZE;
  } //  (cnt < len)

return  true;
}

bool BlockReceived(void)
{
	if(OutCnt == InCnt)
	  return false;
	else
	 return true;
}

uint8_t RecChar( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout)
{

  uint16_t TimeCnt =0;
  uint8_t Tmp;
  while(OutCnt == InCnt)
  {
    Poco::Thread::sleep(1);
    Poco::Thread::Thread::yield();
    if(TimeCnt++ > Timeout)
    {
      {StartupStore(TEXT("REC_TIMEOUT_ERROR" ));}
      return REC_TIMEOUT_ERROR;
    }
  }
  Tmp = RingBuff[OutCnt++];
  OutCnt %= REC_BUFFERSIZE;
  if(inchar)
    *inchar = Tmp;

  return REC_NO_ERROR;
}



BOOL CDevFlarm::Open( PDeviceDescriptor_t d) {
	m_pDevice = d;

	return TRUE;
}

BOOL CDevFlarm::Close (PDeviceDescriptor_t d) {

  LockFlightData();
  if(IsInBinaryMode()) // if FLARM in Bin Modet?
  {
    if(d != NULL)
	  FlarmReboot(d);
  }
  UnlockFlightData();
  m_pDevice = NULL;

  return TRUE;
}




CallBackTableEntry_t CDevFlarm::CallBackTable[]={
  EndCallBackEntry()
};

BOOL CDevFlarm::Config(PDeviceDescriptor_t d){
  if(m_pDevice != d) {
    StartupStore(_T("Flarm Config : Invalide device descriptor%s"), NEWLINE);
    return FALSE;
  }

  WndForm* wf = dlgLoadFromXML(CallBackTable, IDR_XML_DEVFLARM);
  if(wf)
  {

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

    WndProperty* wp;
    wp = (WndProperty*)wf->FindByName(TEXT("prpFlarmId"));
    if (wp) {
      TCHAR szFlarmID[16];
      _stprintf(szFlarmID, _T("%06lX")  ,dwFlarmID[ d->PortNumber]);
      wp->GetDataField()->SetAsString(szFlarmID);
      wp->RefreshDisplay();
    }

    wf->ShowModal();


    if (wp) {
      _stscanf(wp->GetDataField()->GetAsString(),TEXT("%lx"), &dwFlarmID[ d->PortNumber] );
    }

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
	  MessageBoxX(MsgToken(2418), MsgToken(2397), mbOk);
	  return;
	}

	if(!bFlarmActive)	{
	  MessageBoxX(MsgToken(2401), MsgToken(2397), mbOk);
	  return;
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

BOOL CDevFlarm::FlarmReboot(PDeviceDescriptor_t d) {
    if (d && d->Com) {
        LeaveBinModeWithReset(d);
        d->Com->WriteString(TEXT("$PFLAR,0*55\r\n"));
        StartupStore(TEXT("$PFLAR,0*55\r\n"));
        LockFlightData();
        GPS_INFO.FLARM_Available = false;
        UnlockFlightData();
    }
    return TRUE;
}
