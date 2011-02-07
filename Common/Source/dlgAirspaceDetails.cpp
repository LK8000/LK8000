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
#else
static int index_circle = -1;
static int index_area = -1;
#endif
static WndForm *wf=NULL;


#ifdef LKAIRSPACE
static void OnAcknowledgeClicked(WindowControl * Sender){
  (void)Sender;

  if (airspace == NULL) return;
  const TCHAR *Name = airspace->Name();
  if (Name) {
    UINT answer;
    answer = MessageBoxX(hWndMapWindow,
			 Name,
			  // LKTOKEN  _@M51_ = "Acknowledge for day?" 
			 gettext(TEXT("_@M51_")),
			 MB_YESNOCANCEL|MB_ICONQUESTION);
    if (answer == IDYES) {
	  if (airspace) AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, false, airspace, true);
      wf->SetModalResult(mrOK);
    } else if (answer == IDNO) {
      // this will cancel a daily ack
	  if (airspace) AirspaceWarnListAdd(&GPS_INFO, &CALCULATED_INFO, true, airspace, true);
      wf->SetModalResult(mrOK);
    }
  }
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
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};

static double FLAltRounded(double alt) {
  int f = iround(alt/10)*10;
  return (double)f;
}

#ifdef LKAIRSPACE
static void SetValues(const CAirspace *airspace) {

    WndProperty* wp;
  TCHAR buffer[80];
  TCHAR buffer2[80];

  const int atype = airspace->Type();
  const AIRSPACE_ALT* top = airspace->Top();
  const AIRSPACE_ALT* base = airspace->Base();
  const TCHAR *name = airspace->Name();
  bool inside = airspace->Inside(GPS_INFO.Longitude, GPS_INFO.Latitude);
  double bearing;
  double range = airspace->Range(GPS_INFO.Longitude, GPS_INFO.Latitude, bearing);

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
void dlgAirspaceDetails(const CAirspace *airspace) {
  if (airspace == NULL) {
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

  SetValues(airspace);

  wf->ShowModal();

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
