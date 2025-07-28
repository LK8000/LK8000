/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWeatherStDetails.cpp,v 1.0 2020/05/30 20:08:00 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"
#include "WindowControls.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "resource.h"
#include "Sound/Sound.h"

#define MAX_LEN 80


static WndForm *wf=NULL;

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}


static CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnCloseClicked),
  EndCallbackEntry()
};

void dlgWeatherStDetails(int indexid) {
  wf = dlgLoadFromXML(CallBackTable, IDR_XML_WEATHERSTDETAILS);

  if (!wf) return;
  
  WndProperty* wp;
  TCHAR buffer[80];
  FANET_WEATHER Station;
  TCHAR StationName[MAX_LEN];
  StationName[0] = 0; //zero-termination of String;
  LockFlightData();
  memcpy( &Station, &GPS_INFO.FANET_Weather[indexid], sizeof(FANET_WEATHER));
  GetFanetName(Station.ID, GPS_INFO, StationName);
  UnlockFlightData();

  TCHAR caption[MAX_LEN + 10];
  if(_tcslen(StationName) == 0)
    _stprintf(caption, TEXT("%X"),Station.ID);
  else
    _stprintf(caption, TEXT("%s [%X]"),StationName,Station.ID);
  wf->SetCaption(caption);


  double Distance, Bear;
  DistanceBearing( GPS_INFO.Latitude,GPS_INFO.Longitude, Station.Latitude,  Station.Longitude, &Distance, &Bear);
  wp = wf->FindByName<WndProperty>(TEXT("prpBearing"));
  if (wp) {
    _stprintf(buffer, TEXT(" %d%s"), iround(Bear),MsgToken<2179>());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpDistance"));
  if (wp) {
    _stprintf(buffer,_T("%.1f %s"), Units::ToDistance(Distance), Units::GetDistanceName());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpWSpeed"));
  if (wp) {
    _stprintf(buffer,_T("%.1f | %.1f %s"),
                Units::ToWindSpeed(Station.windSpeed),
                Units::ToWindSpeed(Station.windGust),
                Units::GetName(Units::GetHorizontalSpeedUnit()));

    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpWBearing"));
  if (wp) {
    _stprintf(buffer,_T("%.1f %s"),Station.windDir, MsgToken<2179>());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpTemp"));
  if (wp) {
    _stprintf(buffer,_T("%.1f %s"),Station.temp, MsgToken<2180>());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpHum"));
  if (wp) {
    _stprintf(buffer,_T("%.1f %%"),Station.hum);
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpBaro"));
  if (wp) {
    _stprintf(buffer, _T("%.2f %s"), Units::ToPressure(Station.pressure), Units::GetPressureName());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpCharge"));
  if (wp) {
    _stprintf(buffer,_T("%.1f %%"),Station.Battery);
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }


  wf->ShowModal();
  delete wf;
  wf = NULL;
  return;
}