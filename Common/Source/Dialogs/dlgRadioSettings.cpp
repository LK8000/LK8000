/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWindSettings.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "Dialogs.h"
#include "TraceThread.h"

#include "externs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "resource.h"
#include "NavFunctions.h"
#include "Util/TruncateString.hpp"
#include "Radio.h"

static WndForm *wf=NULL;

static WndButton *wpnewActive = nullptr;
static WndButton *wpnewActiveFreq = nullptr;
static WndButton *wpnewPassive = nullptr;
static WndButton *wpnewPassiveFreq= nullptr;
static WndButton *wpnewDual = nullptr;
static WndButton *wpnewVol= nullptr;

static int ActiveRadioIndex=-1;
static int PassiveRadioIndex=-1;
static int SqCnt=0;
static int HoldOff = 0;
#define HOLDOFF_TIME  1 // x 0.5s
#define VOL 0
#define SQL 1
#define STX 0x02

static unsigned char VolMode  = 0;
static unsigned char lVolume  = 6;
static unsigned char lSquelch = 3;



#define DEVICE_NAME_LEN 12

static void OnCancelClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static int OnRemoteUpdate(void)
{

  if(RadioPara.Changed)
  {

    TCHAR Name[250];
 
    TCHAR ActiveName[DEVICE_NAME_LEN+8];
		CopyTruncateString(ActiveName, DEVICE_NAME_LEN, RadioPara.ActiveName);

    if(RadioPara.TX)
      _stprintf(Name,_T(">%s<"),ActiveName);
    else
      if(RadioPara.RX_active)
        _stprintf(Name,_T("<%s>"),ActiveName);
      else
        if(RadioPara.ActiveValid)
          _stprintf(Name,_T("[%s]"),ActiveName);
        else
          _stprintf(Name,_T("%s"),ActiveName);
      
    if(wpnewActive)
      wpnewActive->SetCaption(Name);
    _stprintf(Name,_T("%6.03f"),RadioPara.ActiveFrequency);
    if(wpnewActiveFreq)
      wpnewActiveFreq->SetCaption(Name);
   
    TCHAR PassiveName[DEVICE_NAME_LEN+8];
		CopyTruncateString(PassiveName, DEVICE_NAME_LEN, RadioPara.PassiveName );

    if(RadioPara.RX_standy)
      _stprintf(Name,_T("<%s>"),PassiveName);
    else
      if(RadioPara.PassiveValid)
        _stprintf(Name,_T("[%s]"),PassiveName);
      else
        _stprintf(Name,_T("%s"),PassiveName);
    
    if(wpnewPassive)
     wpnewPassive->SetCaption(Name);
    _stprintf(Name,_T("%6.03f"),RadioPara.PassiveFrequency);
    if(wpnewPassiveFreq)
     wpnewPassiveFreq->SetCaption(Name);
/*
        if( lSquelch !=  RadioPara.Squelch)
        {
              VolMode = SQL;
              SqCnt =0;
        }
*/
        if( lVolume !=  RadioPara.Volume)
              VolMode = VOL;
        lSquelch =  RadioPara.Squelch;
        lVolume =  RadioPara.Volume;
        if(wpnewVol)
        {
          if(VolMode == VOL)
          {
            if(RadioPara.VolValid)
              _stprintf(Name,_T("V[%i]"),RadioPara.Volume);
            else
              _stprintf(Name,_T("V %i"),RadioPara.Volume);
          }
          else
          {
            if(RadioPara.SqValid)
              _stprintf(Name,_T("S [%i]"),RadioPara.Squelch);
            else
              _stprintf(Name,_T("S %i"),RadioPara.Squelch);
          }
          wpnewVol->SetCaption(Name);
        }

        if(RadioPara.Dual)
          if(RadioPara.DualValid)
            _stprintf(Name,_T("[Dual Off]"));
          else
            _stprintf(Name,_T("Dual Off"));
        else
          if(RadioPara.DualValid) 
            _stprintf(Name,_T("[Dual On]"));
          else
            _stprintf(Name,_T("Dual On"));            
        if(wpnewDual)
              wpnewDual->SetCaption(Name);

      RadioPara.Changed =FALSE;
      return 1;
    }
    return 0;
}

static int OnUpdate(void) {
  TCHAR Name[DEVICE_NAME_LEN+8];

	CopyTruncateString(Name, DEVICE_NAME_LEN, RadioPara.ActiveName);
	if(wpnewActive) 
		wpnewActive->SetCaption(Name);
	_stprintf(Name,_T("%7.3f"),RadioPara.ActiveFrequency);
	if(wpnewActiveFreq)
		wpnewActiveFreq->SetCaption(Name);

	CopyTruncateString(Name, DEVICE_NAME_LEN, RadioPara.PassiveName);		
	if(wpnewPassive)
		wpnewPassive->SetCaption(Name);
	_stprintf(Name,_T("%7.3f"),RadioPara.PassiveFrequency);
	if(wpnewPassiveFreq)
		wpnewPassiveFreq->SetCaption(Name);


    if(wpnewVol)
    {
        if(VolMode == VOL)   {
           _stprintf(Name,_T("V%i"),lVolume);
           wpnewVol->SetCaption(Name);
        }    else      {
        _stprintf(Name,_T("S%i"),lSquelch);
          wpnewVol->SetCaption(Name);
        }
    }

    WindowControl* wAuto = wf->FindByName(TEXT("cmdAutoActive"));
    if(bAutoActive) {
      wAuto->SetCaption(MsgToken(1324));   //  M1324 "B>"
    } else  {
      wAuto->SetCaption(_T(""));
    }

    wAuto = wf->FindByName(TEXT("cmdAutoPassive"));
    if(bAutoPassiv) {
      wAuto->SetCaption(MsgToken(1324));   //  M1324 "B>"
    } else  {
      wAuto->SetCaption(_T(""));
    }


return 0;
}


static void OnDualButton(WndButton* pWnd){
TCHAR Name[250];

    RadioPara.Dual = !RadioPara.Dual;
    devPutRadioMode((int)RadioPara.Dual);
    if(RadioPara.Dual)
      _stprintf(Name,_T("Dual Off"));
    else
      _stprintf(Name,_T("Dual On"));
    if(wpnewDual)
       wpnewDual->SetCaption(Name);

}


static void OnActiveButton(WndButton* pWnd){

  if (HoldOff ==0)
  {
    int res = dlgSelectWaypoint(1, 3);
    if(res > RESWP_END )
    if(ValidWayPoint(res))
    {
      double  Frequency = StrToDouble(WayPointList[res].Freq,NULL);
      if(!ValidFrequency(Frequency))
      {
        MessageBoxX(MsgToken(2490), MsgToken(2494), mbOk); //   "_@M002490_": "Invalid radio frequency/channel input!",
      }
      else
      {
        devPutFreqActive(Frequency, WayPointList[res].Name);
    	_tcscpy(RadioPara.ActiveName, WayPointList[res].Name);
        RadioPara.ActiveFrequency = Frequency;
        ActiveRadioIndex = res;  
        OnUpdate();
        HoldOff = HOLDOFF_TIME;
      }
    }
  }
}


static void OnPassiveButton(WndButton* pWnd){
  if (HoldOff ==0)
  {
   int res = dlgSelectWaypoint(1, 3);

   if(res > RESWP_END )
     if(ValidWayPoint(res))
    {
      double Frequency = StrToDouble(WayPointList[res].Freq,NULL);
      if(!ValidFrequency(Frequency))
      {
        MessageBoxX(MsgToken(2490), MsgToken(2494), mbOk); //    "_@M002490_": "Invalid radio frequency/channel input!",
      }
      else
      { 
        devPutFreqStandby(Frequency, WayPointList[res].Name);
        _tcscpy(RadioPara.PassiveName, WayPointList[res].Name);
        RadioPara.PassiveFrequency = Frequency;
        PassiveRadioIndex = res;
        OnUpdate();
        HoldOff = HOLDOFF_TIME;
      }
      MessageBoxX(MsgToken(2490), MsgToken(2494), mbOk); //   "_@M002490_": "Invalid radio frequency/channel input!",
    }
  }
}



static void OnActiveFreq(WndButton* pWnd){
TCHAR	szFreq[20];
_stprintf(szFreq, _T("%7.3f"),RadioPara.ActiveFrequency);

    dlgNumEntryShowModal(szFreq,8);
    double Frequency = StrToDouble(szFreq,NULL);
    while(Frequency > 1000.0)
	   Frequency /=10;
    if(!ValidFrequency(Frequency))
    {        
      MessageBoxX(MsgToken(2490), MsgToken(2494), mbOk); //   "_@M002490_": "Invalid radio frequency/channel input!",
    }
    else
    {
      UpdateStationName(RadioPara.ActiveName, Frequency);
      devPutFreqActive(Frequency,RadioPara.ActiveName);
      OnUpdate();
    }

}

static void OnPassiveFreq(WndButton* pWnd){
TCHAR	szFreq[20] ;
_stprintf(szFreq,  _T("%7.3f"),RadioPara.PassiveFrequency);

   dlgNumEntryShowModal(szFreq,8);

   double Frequency = StrToDouble(szFreq,NULL);
   while(Frequency > 1000)
	   Frequency /=10;

    if(!ValidFrequency(Frequency))
    {        
      MessageBoxX(MsgToken(2490), MsgToken(2494), mbOk); //    "_@M002490_": "Invalid radio frequency/channel input!",
    }
    else
    {
      UpdateStationName(RadioPara.PassiveName, Frequency);
      devPutFreqStandby(Frequency,RadioPara.PassiveName);
      OnUpdate();
    }
   
}


static void OnRadioActiveAutoClicked(WndButton* pWnd){
  if(bAutoActive) {
	  bAutoActive = false;
  } else {
		bAutoActive = true;


	int Idx = SearchBestStation();
	if ( ValidWayPoint(Idx))
	{
		double fFreq = StrToDouble(WayPointList[Idx].Freq,NULL);
		devPutFreqActive(	fFreq , WayPointList[Idx].Name);
	}

  }
  OnUpdate();
}


static void OnRadioStandbyAutoClicked(WndButton* pWnd)
{
  if(bAutoPassiv) {
	  bAutoPassiv = false;
  } else {
	  bAutoPassiv = true;
		
	int Idx = SearchBestStation();
	if ( ValidWayPoint(Idx))
	{
		double fFreq = StrToDouble(WayPointList[Idx].Freq,NULL);
		devPutFreqStandby(	fFreq , WayPointList[Idx].Name);
	}

  }
  OnUpdate();
}


static void OnExchange(WndButton* pWnd){
int tmp;
TCHAR szTempStr[NAME_SIZE+1];
double fTmp;
// if (HoldOff ==0)
 {
   tmp =   ActiveRadioIndex;
   ActiveRadioIndex = PassiveRadioIndex;
   PassiveRadioIndex = tmp;
   devPutFreqSwap();
    fTmp =   RadioPara.ActiveFrequency;
    RadioPara.ActiveFrequency = RadioPara.PassiveFrequency;
    RadioPara.PassiveFrequency=  fTmp;
    _tcscpy( szTempStr,  RadioPara.ActiveName);
    _tcscpy(  RadioPara.ActiveName, RadioPara.PassiveName);
    _tcscpy(  RadioPara.PassiveName, szTempStr);
    OnUpdate();
  }
}

static void SendVolSq(void){
  if (HoldOff ==0)
  {

  }
}


static void OnMuteButton(WndButton* pWnd){

    switch (VolMode)
    {
      default:
      case VOL: VolMode = SQL;  SqCnt =0; break;
      case SQL: VolMode = VOL;  SqCnt =11;break;
    }
    OnUpdate();
}


static void OnVolUpButton(WndButton* pWnd){

    if(VolMode == VOL)
    {
        if(lVolume < 20)
          lVolume += 1;
        if(lVolume > 20) lVolume = 20;
        if (HoldOff ==0)
        {
          devPutVolume(lVolume);
          HoldOff = HOLDOFF_TIME;
        }
    }
    else
    {
      if(lSquelch < 10)
        lSquelch += 1;
      if(lSquelch > 10) lSquelch = 10;
      SqCnt =0;
      if (HoldOff ==0)
      {
        devPutSquelch(lSquelch);
        HoldOff = HOLDOFF_TIME;
      }
    }
    OnUpdate();
}



static void OnVolDownButton(WndButton* pWnd){

  if(VolMode == VOL)
  {
	if(lVolume > 1)
	  lVolume -= 1;
	if(lVolume < 1)
	  lVolume = 1;
	if (HoldOff ==0)
	{
	  devPutVolume(lVolume);
	  HoldOff = HOLDOFF_TIME;
	}
  }
  else
  {
	if(lSquelch > 1)
		lSquelch -= 1;
	if(lSquelch < 1) lSquelch = 1;
	SqCnt =0;
	  if (HoldOff ==0)
	  {
	      devPutSquelch(lSquelch);
	      HoldOff = HOLDOFF_TIME;
	  }
  }

  OnUpdate();
  SendVolSq();
  HoldOff = HOLDOFF_TIME;
}



static bool OnTimerNotify(WndForm* pWnd) {

  if(VolMode != VOL)
    SqCnt++;

  if(SqCnt > 10)
  {
    VolMode = VOL;
    SqCnt = 0;
    OnUpdate();
  }
  if (HoldOff >0)
    HoldOff--;
  else
    OnRemoteUpdate();


  return 0;
}


static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnDualButton),
  ClickNotifyCallbackEntry(OnActiveButton),
  ClickNotifyCallbackEntry(OnActiveFreq),
  ClickNotifyCallbackEntry(OnPassiveFreq),
  ClickNotifyCallbackEntry(OnPassiveButton),
  ClickNotifyCallbackEntry(OnMuteButton),
  ClickNotifyCallbackEntry(OnCancelClicked),
  ClickNotifyCallbackEntry(OnRadioActiveAutoClicked),
  ClickNotifyCallbackEntry(OnRadioStandbyAutoClicked),
  ClickNotifyCallbackEntry(OnExchange),
  ClickNotifyCallbackEntry(OnVolUpButton),
  ClickNotifyCallbackEntry(OnVolDownButton),
  EndCallBackEntry()
};


void dlgRadioSettingsShowModal(void){
  SHOWTHREAD(_T("dlgRadioSettingsShowModal"));


  wf = dlgLoadFromXML(CallBackTable, IDR_XML_RADIOSETTINGS );
  if (!wf) return;

  VolMode = VOL; // start with volume

  wpnewActive = (WndButton*)wf->FindByName(TEXT("cmdActive"));
  LKASSERT( wpnewActive !=NULL);

  wpnewActiveFreq = (WndButton*)wf->FindByName(TEXT("cmdActiveFreq"));
  LKASSERT( wpnewActiveFreq !=NULL);

  wpnewPassive  = (WndButton*)wf->FindByName(TEXT("cmdPassive"));
  LKASSERT(   wpnewPassive   !=NULL)

  wpnewPassiveFreq = (WndButton*)wf->FindByName(TEXT("cmdPassiveFreq"));
  LKASSERT(   wpnewPassiveFreq   !=NULL)

  wpnewVol  = (WndButton*)wf->FindByName(TEXT("cmdVol"));
  LKASSERT(   wpnewVol   !=NULL)

  wpnewDual  = (WndButton*)wf->FindByName(TEXT("cmdDual"));
  LKASSERT(   wpnewDual   !=NULL)

  wf->SetTimerNotify(300, OnTimerNotify);

  OnUpdate();
  wf->ShowModal();


  wpnewActive = nullptr;
  wpnewActiveFreq = nullptr;
  wpnewPassive = nullptr;
  wpnewPassiveFreq = nullptr;
  wpnewDual = nullptr;
  wpnewVol = nullptr;

  delete wf;
  wf = nullptr;

}
