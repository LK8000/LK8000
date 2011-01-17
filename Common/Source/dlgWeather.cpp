/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWeather.cpp,v 8.2 2010/12/13 17:30:25 root Exp root $
*/
#include "StdAfx.h"

#include "Statistics.h"

#include "externs.h"
#include "Units.h"
#include "Process.h"
#include "RasterTerrain.h"
#include "dlgTools.h"
#include "Port.h"
#include "InfoBoxLayout.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;
extern int TimeLocal(int d);

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void OnDisplayItemData(DataField *Sender, 
                              DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      Sender->Set(RasterTerrain::render_weather);
    break;
    case DataField::daPut: 
    case DataField::daChange:
      RasterTerrain::render_weather = Sender->GetAsInteger();
    break;
  }
}


static void RASPGetTime(DataField *Sender) {
  DataFieldEnum* dfe;
  dfe = (DataFieldEnum*)Sender;
  int index=0;
  for (int i=0; i<MAX_WEATHER_TIMES; i++) {
    if (RASP.weather_available[i]) {
      if (RASP.weather_time == i) {
        Sender->Set(index);
      }
      index++;
    }
  }
}

static void RASPSetTime(DataField *Sender) {
  int index = 0;
  if (Sender->GetAsInteger()==0) {
    RASP.weather_time = 0;
    return;
  }
  for (int i=0; i<MAX_WEATHER_TIMES; i++) {
    if (RASP.weather_available[i]) {
      if (index == Sender->GetAsInteger()) {
        RASP.weather_time = i;
      }
      index++;
    }
  }
}

static void OnTimeData(DataField *Sender, 
                       DataField::DataAccessKind_t Mode){

  switch(Mode){
    case DataField::daGet:
      RASPGetTime(Sender);
    break;
    case DataField::daPut: 
    case DataField::daChange:
      RASPSetTime(Sender);
    break;
  }

}


void OnWeatherHelp(WindowControl * Sender){
  WndProperty *wp = (WndProperty*)Sender;
  int type = wp->GetDataField()->GetAsInteger();
  TCHAR caption[80], shelp[20];
	// LKTOKEN  _@M813_ = "Weather parameters" 
  _stprintf(caption, gettext(TEXT("_@M813_")));

  _stprintf(shelp,_T("_@H%d_"),type+650);
  dlgHelpShowModal(caption, shelp);

}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnTimeData), 
  DeclareCallBackEntry(OnDisplayItemData), 
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnWeatherHelp),
  DeclareCallBackEntry(NULL)
};


void dlgWeatherShowModal(void){

  if (!InfoBoxLayout::landscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWeather_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WEATHER_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWeather.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WEATHER"));
  }

  WndProperty* wp;
    
  if (wf) {

    wp = (WndProperty*)wf->FindByName(TEXT("prpTime"));
    if (wp) {
      DataFieldEnum* dfe;
      dfe = (DataFieldEnum*)wp->GetDataField();
      dfe->addEnumText(TEXT("Now"));
      for (int i=1; i<MAX_WEATHER_TIMES; i++) {
        if (RASP.weather_available[i]) {
          TCHAR timetext[10];
          _stprintf(timetext,TEXT("%04d"), RASP.IndexToTime(i));
          dfe->addEnumText(timetext);
        }
      }

      RASPGetTime(dfe);

      wp->RefreshDisplay();
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpDisplayItem"));
    DataFieldEnum* dfe;
    if (wp) {
      dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M708_ = "Terrain" 
      dfe->addEnumText(gettext(TEXT("_@M708_")));

      TCHAR Buffer[20];
      for (int i=1; i<=15; i++) {
        RASP.ItemLabel(i, Buffer);
        if (_tcslen(Buffer)) {
          dfe->addEnumText(Buffer);
        }
      }
      dfe->Set(RasterTerrain::render_weather);
      wp->RefreshDisplay();
    }

    wf->ShowModal();

    wp = (WndProperty*)wf->FindByName(TEXT("prpTime"));
    if (wp) {
      DataFieldEnum* dfe;
      dfe = (DataFieldEnum*)wp->GetDataField();
      RASPSetTime(dfe);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpDisplayItem"));
    if (wp) {
      RasterTerrain::render_weather = 
        wp->GetDataField()->GetAsInteger();
    }

    delete wf;
  }
  wf = NULL;
}

