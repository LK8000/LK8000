/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceDetails.cpp,v 8.3 2010/12/13 12:23:03 root Exp root $
*/

#include "StdAfx.h"
#include <aygshell.h>
#include "XCSoar.h"
#include "Statistics.h"
#include "externs.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "Airspace.h"
#include "AirspaceWarning.h"
#ifdef LKAIRSPACE
#include "LKAirspace.h"
#endif

#include "utils/heapcheck.h"

#ifdef LKAIRSPACE
static CAirspace *airspace = NULL;
static CAirspace airspace_copy;
#else
static int index_circle = -1;
static int index_area = -1;
#endif
static WndForm *wf=NULL;

static void SetValues(void);

#ifdef LKAIRSPACE
static void OnFlyClicked(WindowControl * Sender){
  (void)Sender;

  if (airspace == NULL) return;
  if (wf == NULL) return;

  #if 0 // We dont ask for confirmation, but we might change our mind!
  UINT answer;
  if (airspace_copy.Flyzone()) {
	// LKTOKEN _@M1273_ "Set as NOFLY zone?"
	answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1273_")), MB_YESNO|MB_ICONQUESTION);
  } else {
	// LKTOKEN _@M1272_ "Set as FLY zone?"
	answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1272_")), MB_YESNO|MB_ICONQUESTION);
  }
  if (answer == IDYES) {
	CAirspaceManager::Instance().AirspaceFlyzoneToggle(*airspace);
	SetValues();
	// wf->SetModalResult(mrOK);
  }
  #endif

  CAirspaceManager::Instance().AirspaceFlyzoneToggle(*airspace);
  SetValues();
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));

}
#endif

#ifdef LKAIRSPACE
static void OnAcknowledgeClicked(WindowControl * Sender){
  (void)Sender;

  if (airspace == NULL) return;
  if (wf == NULL) return;

  #if 0  // We dont ask anymore to the user for enable/disable confirmation
  UINT answer;
  if (!airspace_copy.Enabled()) {
  
    // LKTOKEN  _@M1280_ "Enable this airspace?"
    answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1280_")),  MB_YESNO|MB_ICONQUESTION);
    if (answer == IDYES) {
      // this will cancel a daily ack
      CAirspaceManager::Instance().AirspaceEnable(*airspace);
      wf->SetModalResult(mrOK);
    }
  } else {
    // LKTOKEN  _@M1284_ "Disable this airspace?" 
    answer = MessageBoxX(hWndMapWindow, airspace_copy.Name(), gettext(TEXT("_@M1284_")),	MB_YESNO|MB_ICONQUESTION);
    if (answer == IDYES) {
      CAirspaceManager::Instance().AirspaceDisable(*airspace);
      wf->SetModalResult(mrOK);
    }
  }
  #endif

  if (airspace_copy.Enabled()) 
      CAirspaceManager::Instance().AirspaceDisable(*airspace);
  else 
      CAirspaceManager::Instance().AirspaceEnable(*airspace);

  SetValues();
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));


}
#else
static void OnAcknowledgeClicked(WindowControl * Sender){
  (void)Sender;

  TCHAR *Name = NULL;
  if (index_circle>=0) {
    Name = AirspaceCircle[index_circle].Name;
  } else if (index_area>=0) {
    Name = AirspaceArea[index_area].Name;
  }
  if (Name) {
    UINT answer;
    answer = MessageBoxX(hWndMapWindow,
			 Name,
	// LKTOKEN  _@M51_ = "Acknowledge for day?" 
			 gettext(TEXT("_@M51_")),
			 MB_YESNOCANCEL|MB_ICONQUESTION);
    if (answer == IDYES) {
      if (index_circle>=0) {
	AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, false, true, 
			    index_circle, true);
      } else if (index_area>=0) {
	AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, false, false, 
			    index_area, true);
      }
      wf->SetModalResult(mrOK);
    } else if (answer == IDNO) {
      // this will cancel a daily ack
      if (index_circle>=0) {
	AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, true, true, 
			    index_circle, true);
      } else if (index_area>=0) {
	AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, true, false, 
			    index_area, true);
      }
      wf->SetModalResult(mrOK);
    }
  }
}
#endif

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAcknowledgeClicked),
#ifdef LKAIRSPACE
  DeclareCallBackEntry(OnFlyClicked),
#endif
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};
#ifndef LKAIRSPACE
static double FLAltRounded(double alt) {
  int f = iround(alt/10)*10;
  return (double)f;
}
#endif
#ifdef LKAIRSPACE
static void SetValues() {

  if (airspace==NULL) return;

  airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(airspace);

  WndProperty* wp;
  WndButton *wb;
  TCHAR buffer[80];
  TCHAR buffer2[80];

  int bearing;
  int hdist;
  int vdist;
  bool inside = CAirspaceManager::Instance().AirspaceCalculateDistance( airspace, &hdist, &bearing, &vdist);
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpName"));
  if (wp) {
    wp->SetText(airspace_copy.Name());
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpType"));
  if (wp) {
	if (airspace_copy.Flyzone()) {
	  wsprintf(buffer,TEXT("%s %s"), CAirspaceManager::Instance().GetAirspaceTypeText(airspace_copy.Type()), gettext(TEXT("FLY")));
	} else {
	  wsprintf(buffer,TEXT("%s %s"), CAirspaceManager::Instance().GetAirspaceTypeText(airspace_copy.Type()), gettext(TEXT("NOFLY")));
	}
	wp->SetText( buffer );
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpTop"));
  if (wp) {
	CAirspaceManager::Instance().GetAirspaceAltText(buffer, sizeof(buffer)/sizeof(buffer[0]), airspace_copy.Top());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBase"));
  if (wp) {
	CAirspaceManager::Instance().GetAirspaceAltText(buffer, sizeof(buffer)/sizeof(buffer[0]), airspace_copy.Base());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRange"));
  if (wp) {
    Units::FormatUserDistance(abs(hdist), buffer, 20);
    if (inside) {
	  // LKTOKEN  _@M359_ = "Inside" 
      wp->SetCaption(gettext(TEXT("_@M359_")));
	  // LKTOKEN _@M1257_ "to leave"
	  wsprintf(buffer2, TEXT("%s %d")TEXT(DEG)TEXT(" %s"), buffer, iround(bearing), gettext(TEXT("_@M1257_")));
    } else {
	  // LKTOKEN _@M1258_ "to enter"
	  wsprintf(buffer2, TEXT("%s %d")TEXT(DEG)TEXT(" %s"), buffer, iround(bearing), gettext(TEXT("_@M1258_")));
	}
    wp->SetText(buffer2);
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpWarnLevel"));
  if (wp) {
	  switch (airspace_copy.WarningLevel()) {
		default:
		  // LKTOKEN _@M765_ "Unknown"
		  wp->SetText(gettext(TEXT("_@M765_")));
		  break;
		  
		case awNone:
		  // LKTOKEN _@M479_ "None"
  		  wp->SetText(gettext(TEXT("_@M479_")));
		  break;

		case awYellow:
			// LKTOKEN _@M1255_ "YELLOW WARNING"
			wp->SetText(gettext(TEXT("_@M1255_")));
		  break;
		
		case awRed:
			// LKTOKEN _@M1256_ "RED WARNING"
			wp->SetText(gettext(TEXT("_@M1256_")));
		  break;
	  }//sw
	  wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAckLevel"));
  if (wp) {
      if (airspace_copy.Enabled()) {
        switch (airspace_copy.WarningAckLevel()) {
          default:
            // LKTOKEN _@M765_ "Unknown"
            wp->SetText(gettext(TEXT("_@M765_")));
            break;
            
          case awNone:
            // LKTOKEN _@M479_ "None"
            wp->SetText(gettext(TEXT("_@M479_")));
            break;

          case awYellow:
              // LKTOKEN _@M1267_ "Yellow acknowledged"
              wp->SetText(gettext(TEXT("_@M1267_")));
            break;
          
          case awRed:
              // LKTOKEN _@M1268_ "Red acknowledged"
              wp->SetText(gettext(TEXT("_@M1268_")));
            break;

        }//sw
      } else {
          // LKTOKEN _@M1269_ "Disabled"
          wp->SetText(gettext(TEXT("_@M1269_")));
      }
	  wp->RefreshDisplay();
  }

  wb = (WndButton*)wf->FindByName(TEXT("cmdFly"));
  if (wb) {
	if (airspace_copy.Flyzone()) {
	  // LKTOKEN _@M1271_ "NOFLY"
	  wb->SetCaption(gettext(TEXT("_@M1271_")));
	} else {
	  // LKTOKEN _@M1270_ "FLY"
	  wb->SetCaption(gettext(TEXT("_@M1270_")));
	}
	wb->Redraw();
  }


  wb = (WndButton*)wf->FindByName(TEXT("cmdAcknowledge"));
  if (wb) {
    if (airspace_copy.Enabled()) {
      // LKTOKEN _@M1283_ "Disable"
      wb->SetCaption(gettext(TEXT("_@M1283_")));
    } else {
      // LKTOKEN _@M1282_ "Enable"
      wb->SetCaption(gettext(TEXT("_@M1282_")));
    }
    wb->Redraw();
  }


}

#else

static void SetValues(void) {
  int atype = 0;
  AIRSPACE_ALT* top = NULL;
  AIRSPACE_ALT* base = NULL;
  TCHAR *name = 0;
  WndProperty* wp;
  TCHAR buffer[80];
  TCHAR buffer2[80];
  bool inside = false;
  double range = 0.0;
  double bearing;

  if (index_area >=0) {
    atype = AirspaceArea[index_area].Type;
    top = &AirspaceArea[index_area].Top;
    base = &AirspaceArea[index_area].Base;
    name = AirspaceArea[index_area].Name;
    inside = InsideAirspaceArea(GPS_INFO.Longitude, GPS_INFO.Latitude, 
				index_area);
    range = 
      RangeAirspaceArea(GPS_INFO.Longitude, GPS_INFO.Latitude, 
			index_area, &bearing);
  }
  if (index_circle >=0) {
    atype = AirspaceCircle[index_circle].Type;
    top = &AirspaceCircle[index_circle].Top;
    base = &AirspaceCircle[index_circle].Base;
    name = AirspaceCircle[index_circle].Name;
    inside = InsideAirspaceCircle(GPS_INFO.Longitude, GPS_INFO.Latitude, 
				  index_circle);
    range = 
      RangeAirspaceCircle(GPS_INFO.Longitude, GPS_INFO.Latitude, 
			  index_circle);

    DistanceBearing(GPS_INFO.Latitude,
		    GPS_INFO.Longitude,
		    AirspaceCircle[index_circle].Latitude, 
		    AirspaceCircle[index_circle].Longitude,
		    NULL, &bearing);
    if (inside) {
      bearing = AngleLimit360(bearing+180);
    }
  }

  if (range<0) {
    range = -range;
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpName"));
  if (wp) {
    wp->SetText(name);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpType"));
  if (wp) {
    switch (atype) {
    case RESTRICT:
	// LKTOKEN  _@M565_ = "Restricted" 
      wp->SetText(gettext(TEXT("_@M565_"))); break;
    case PROHIBITED:
	// LKTOKEN  _@M537_ = "Prohibited" 
      wp->SetText(gettext(TEXT("_@M537_"))); break;
    case DANGER:
	// LKTOKEN  _@M213_ = "Danger Area" 
      wp->SetText(gettext(TEXT("_@M213_"))); break;
    case CLASSA:
      wp->SetText(TEXT("Class A")); break;
    case CLASSB:
      wp->SetText(TEXT("Class B")); break;
    case CLASSC:
      wp->SetText(TEXT("Class C")); break;
    case CLASSD:
      wp->SetText(TEXT("Class D")); break;
    case CLASSE:
      wp->SetText(TEXT("Class E")); break;
    case CLASSF:
      wp->SetText(TEXT("Class F")); break;
    case CLASSG:
      wp->SetText(TEXT("Class G")); break;
    case NOGLIDER:
	// LKTOKEN  _@M464_ = "No Glider" 
      wp->SetText(gettext(TEXT("_@M464_"))); break;
    case CTR:
      wp->SetText(TEXT("CTR")); break;
    case WAVE:
	// LKTOKEN  _@M794_ = "Wave" 
      wp->SetText(gettext(TEXT("_@M794_"))); break;
    default:
	// LKTOKEN  _@M765_ = "Unknown" 
      wp->SetText(gettext(TEXT("_@M765_")));
    }
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpTop"));
  if (wp) {
    switch (top->Base){
    case abUndef:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("%.0f[m] %.0f[ft] [?]"), 
		  (top->Altitude), 
		  (top->Altitude*TOFEET));
      } else {
	_stprintf(buffer, TEXT("%.0f ft [?]"), 
		  (top->Altitude*TOFEET));
      }
      break;
    case abMSL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("%.0f[m] %.0f[ft] MSL"), 
		  top->Altitude, top->Altitude*TOFEET);
      } else {
	_stprintf(buffer, TEXT("%.0f ft MSL"), 
		  top->Altitude*TOFEET);
      }
      break;
    case abAGL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("%.0f[m] %.0f[ft] AGL"), 
		  top->AGL, top->AGL*TOFEET);
      } else {
	_stprintf(buffer, TEXT("%.0f ft AGL"), 
		  top->AGL*TOFEET);
      }
      break;
    case abFL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("FL%.0f (%.0f[m] %.0f[ft])"), 
		  top->FL, FLAltRounded(top->Altitude), 
		  FLAltRounded(top->Altitude*TOFEET));
      } else {
	_stprintf(buffer, TEXT("FL%.0f (%.0f ft)"), 
		  top->FL, FLAltRounded(top->Altitude*TOFEET));
      }
      break;
    }
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBase"));
  if (wp) {
    switch (base->Base){
    case abUndef:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("%.0f[m] %.0f[ft] [?]"), 
		  base->Altitude, base->Altitude*TOFEET);
      } else {
	_stprintf(buffer, TEXT("%.0f ft [?]"), 
		  base->Altitude*TOFEET);
      }
      break;
    case abMSL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("%.0f[m] %.0f[ft] MSL"), 
		  base->Altitude, base->Altitude*TOFEET);
      } else {
	_stprintf(buffer, TEXT("%.0f ft MSL"), 
		  base->Altitude*TOFEET);
      }
      break;
    case abAGL:
      if (base->Altitude == 0) {
        _stprintf(buffer, TEXT("SFC"));
      } else {
	if (Units::GetUserAltitudeUnit() == unMeter) {
	  _stprintf(buffer, TEXT("%.0f[m] %.0f[ft] AGL"), 
		    base->AGL, base->AGL*TOFEET);
	} else {
	  _stprintf(buffer, TEXT("%.0f ft AGL"), 
		    base->AGL*TOFEET);
	}
      }
      break;
    case abFL:
      if (Units::GetUserAltitudeUnit() == unMeter) {
	_stprintf(buffer, TEXT("FL %.0f (%.0f[m] %.0f[ft])"), 
		  base->FL, FLAltRounded(base->Altitude), 
		  FLAltRounded(base->Altitude*TOFEET));
      } else {
	_stprintf(buffer, TEXT("FL%.0f (%.0f ft)"), 
		  base->FL, FLAltRounded(base->Altitude*TOFEET));
      }
      break;
    }
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRange"));
  if (wp) {
    if (inside) {
	// LKTOKEN  _@M359_ = "Inside" 
      wp->SetCaption(gettext(TEXT("_@M359_")));
    }
    Units::FormatUserDistance(range, buffer, 20);
    _stprintf(buffer2, TEXT(" %d")TEXT(DEG), iround(bearing));
    _tcscat(buffer, buffer2);
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
}
#endif

#ifdef LKAIRSPACE
void dlgAirspaceDetails(CAirspace *airspace_to_show) {
  if (airspace != NULL) {
    return;
  }

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgAirspaceDetails.xml"));
  wf = dlgLoadFromXML(CallBackTable,
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_AIRSPACEDETAILS"));

  if (!wf) return;
  airspace = airspace_to_show;
  SetValues();

  wf->ShowModal();
  airspace = NULL;
  delete wf;
  wf = NULL;
  return;
}
#else
void dlgAirspaceDetails(int the_circle, int the_area) {
  index_circle = the_circle;
  index_area = the_area;
  if ((index_area<=0) && (index_circle <=0)) {
    return;
  }

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgAirspaceDetails.xml"));
  wf = dlgLoadFromXML(CallBackTable,
		      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_AIRSPACEDETAILS"));

  if (!wf) return;

  ASSERT(wf!=NULL);

  SetValues();

  wf->ShowModal();

  delete wf;
  wf = NULL;
  return;
}
#endif

/*


			distance,
                    Units::GetDistanceName()

  wp = (WndProperty*)wf->FindByName(TEXT("prpDistance"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(distance);
    wp->GetDataField()->SetUnits(Units::GetDistanceName());
    wp->RefreshDisplay();
  }

*/
