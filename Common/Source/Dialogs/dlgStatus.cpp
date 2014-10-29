/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStatus.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "Waypointparser.h"
#include "Logger.h"
#include "LKProcess.h"
#include "Dialogs.h"
#include "device.h"

extern BOOL extGPSCONNECT;
extern NMEAParser nmeaParser1;
extern NMEAParser nmeaParser2;


static WndForm *wf=NULL;
static bool multi_page = false;
static int status_page = 0;
static WndFrame *wStatus0=NULL;
static WndFrame *wStatus1=NULL;
static WndFrame *wStatus2=NULL;
static WndFrame *wStatus3=NULL;
static WndFrame *wStatus4=NULL;
static WndFrame *wStatus5=NULL;

#define NUMPAGES 6

// #define FASTPAGE 1  // DISABLED 

static void NextPage(int Step){
  status_page += Step;
  if (status_page>=NUMPAGES) { status_page=0; }
  if (status_page<0) { status_page=NUMPAGES-1; }
  switch(status_page) {
  case 0:
	// LKTOKEN  _@M661_ = "Status: Aircraft" 
    wf->SetCaption(gettext(TEXT("_@M661_")));
    break;
  case 1:
    if (SIMMODE) {
	TCHAR sysmode[100];
	_stprintf(sysmode,_T("%s (%s)"),gettext(TEXT("_@M664_")),gettext(TEXT("_@M1211_")) );
    	wf->SetCaption(sysmode);
    } else {
	// LKTOKEN  _@M664_ = "Status: System" 
    	wf->SetCaption(gettext(TEXT("_@M664_")));
#if FLARMDEADLOCK
    	if( GPS_INFO.FLARM_SW_Version  < 0.01)
    	{
    	  if(nmeaParser1.isFlarm)
            devRequestFlarmVersion(devA());
    	  else
    	    if(nmeaParser2.isFlarm)
              devRequestFlarmVersion(devB());
    	}
#endif
    }
    break;
  case 2:
	// LKTOKEN  _@M665_ = "Status: Task" 
    wf->SetCaption(gettext(TEXT("_@M665_")));
    break;
  case 3:
	// LKTOKEN  _@M663_ = "Status: Rules" 
    wf->SetCaption(gettext(TEXT("_@M663_")));
    break;
  case 4:
	// LKTOKEN  _@M666_ = "Status: Times" 
    wf->SetCaption(gettext(TEXT("_@M666_")));
    break;
  case 5:
	// LKTOKEN  _@M662_ = "Status: Ext.Device" 
    wf->SetCaption(gettext(TEXT("_@M662_")));
    break;
  }
  wStatus0->SetVisible(status_page == 0);
  wStatus1->SetVisible(status_page == 1); 
  wStatus2->SetVisible(status_page == 2); 
  wStatus3->SetVisible(status_page == 3); 
  wStatus4->SetVisible(status_page == 4); 
  wStatus5->SetVisible(status_page == 5); 

}


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
	(void)lParam;
	(void)Sender;

  switch(wParam & 0xffff){
    case VK_LEFT:
    case '6':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case VK_RIGHT:
    case '7':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wf->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}

#if FASTPAGE
static int TouchKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
	(void)lParam;

 //StartupStore(_T("...... TouchContext=%d\n"),TouchContext);

 // Example on how to use TouchContext. 
 // X,Y are always relative to the frame you are clicking on! So they are not much
 // usable because they respect the xml frames, windows, button relative coordinates.
 // int X,Y;
 // X = LOWORD(lParam); Y = HIWORD(lParam);
 // StartupStore(_T("...... Context=%d X=%d Y=%d\n"),TouchContext,X,Y);

 // We can detect a double click, and use it to force -for example- a window close.
 // But it will not always work because clicking on a readonly button field will 
 // return MOUSEMOVE for some reason. *to detect, return 1 always!*
 // if (TouchContext==TCX_PROC_DOUBLECLICK) wf->SetModalResult(mrOK); 

 // So in this Status context we shall advance to next page when we have touched
 // on the screen but NOT on a button. We have Close, Next, Prev buttons.
 // We shall read TC=PROC_DOWN when clicking on any window position with no button field.
 // We shall read TC=MOUSEMOVE   when clicking on readonly fields
 // We shall NOT read TC=DOUBLECLICK when as above, but done quickly, because we return 0;
 // 

 // UPDATE: on PNAs this does not work. set on hold because we must investigate.
 // Read-only boxes are creating problems, readouts of 12  (BUTTON_UP)!! This should
 // not happen, because those boxes are not buttons. 
 if (Y<32) return(1);

 if (TouchContext< TCX_PROC_UP) {
   NextPage(+1);
   return 0;
 }

 return (1);

}

#endif // FASTPAGE

static void OnNextClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(-1);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(NULL)
};

static bool first = true;

static void UpdateValuesSystem() {

  static int extGPSCONNECT_last = extGPSCONNECT;
  static bool NAVWarning_last = GPS_INFO.NAVWarning;
  static int SatellitesUsed_last = GPS_INFO.SatellitesUsed;
  static int VarioAvailable_last = GPS_INFO.VarioAvailable;
  static int FLARM_Available_last = GPS_INFO.FLARM_Available;
  static bool LoggerActive_last = LoggerActive;
  static bool DeclaredToDevice_last = DeclaredToDevice;
  static double SupplyBatteryVoltage_last = GPS_INFO.SupplyBatteryVoltage;
  static int PDABatteryPercent_last = PDABatteryPercent;
  static int batterybank_last = GPS_INFO.ExtBatt_Bank;
  static double extbatt1_voltage = GPS_INFO.ExtBatt1_Voltage;
  static double extbatt2_voltage = GPS_INFO.ExtBatt2_Voltage;

  //static double FLARM_HW_Version = GPS_INFO.FLARM_HW_Version;
  //static double FLARM_SW_Version = GPS_INFO.FLARM_SW_Version;


  if (first ||
      (extGPSCONNECT_last != extGPSCONNECT) ||
      (NAVWarning_last != GPS_INFO.NAVWarning) ||
      (SatellitesUsed_last != GPS_INFO.SatellitesUsed) ||
      (VarioAvailable_last != GPS_INFO.VarioAvailable) ||
      (FLARM_Available_last != GPS_INFO.FLARM_Available) ||
      (LoggerActive_last != LoggerActive) ||
      (DeclaredToDevice_last != DeclaredToDevice) ||
      (SupplyBatteryVoltage_last != GPS_INFO.SupplyBatteryVoltage) ||
      (batterybank_last != GPS_INFO.ExtBatt_Bank) ||
      (extbatt1_voltage != GPS_INFO.ExtBatt1_Voltage) ||
      (extbatt2_voltage != GPS_INFO.ExtBatt2_Voltage) ||
      // (FLARM_HW_Version != GPS_INFO.FLARM_HW_Version) ||
      // (FLARM_SW_Version != GPS_INFO.FLARM_SW_Version) ||
      (PDABatteryPercent_last != PDABatteryPercent)) {
    first = false;

    extGPSCONNECT_last = extGPSCONNECT;
    NAVWarning_last = GPS_INFO.NAVWarning;
    SatellitesUsed_last = GPS_INFO.SatellitesUsed;
    VarioAvailable_last = GPS_INFO.VarioAvailable;
    FLARM_Available_last = GPS_INFO.FLARM_Available;
    LoggerActive_last = LoggerActive;
    DeclaredToDevice_last = DeclaredToDevice;
    SupplyBatteryVoltage_last = GPS_INFO.SupplyBatteryVoltage;
    PDABatteryPercent_last = PDABatteryPercent;
    batterybank_last = GPS_INFO.ExtBatt_Bank;
    extbatt1_voltage = GPS_INFO.ExtBatt1_Voltage;
    extbatt2_voltage = GPS_INFO.ExtBatt2_Voltage;
    // FLARM_HW_Version = GPS_INFO.FLARM_HW_Version;
    // FLARM_SW_Version = GPS_INFO.FLARM_SW_Version;
  } else {
    return;
  }
  
  TCHAR Temp[80];
  TCHAR Temp2[80];

  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpGPS"));
  if (wp) {
    if (extGPSCONNECT) {
      if (GPS_INFO.NAVWarning) {
	// LKTOKEN  _@M303_ = "Fix invalid" 
        wp->SetText(gettext(TEXT("_@M303_")));
      } else {
        if (GPS_INFO.SatellitesUsed==0) {
	// LKTOKEN  _@M471_ = "No fix" 
          wp->SetText(gettext(TEXT("_@M471_")));
        } else {
	// LKTOKEN  _@M31_ = "3D fix" 
          wp->SetText(gettext(TEXT("_@M31_")));
        }
      }
      wp->RefreshDisplay();

      wp = (WndProperty*)wf->FindByName(TEXT("prpNumSat"));
      if (wp) {
        if (GPS_INFO.SatellitesUsed >= 0) {  // known numer of sats
          _stprintf(Temp,TEXT("%d"),GPS_INFO.SatellitesUsed);
        } else { // valid but unknown number of sats
          _stprintf(Temp,TEXT(">3"));
        }
	if (nmeaParser1.activeGPS==true)
		_tcscat(Temp,_T("  (Dev:A)"));
	else
		_tcscat(Temp,_T("  (Dev:B)"));
        wp->SetText(Temp);
        wp->RefreshDisplay();
      }
    } else {
	// LKTOKEN  _@M240_ = "Disconnected" 
      wp->SetText(gettext(TEXT("_@M240_")));
      wp->RefreshDisplay();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVario"));
  if (wp) {
    if (GPS_INFO.VarioAvailable) {
	// LKTOKEN  _@M199_ = "Connected" 
#ifdef DEVICE_SERIAL	
     if(GPS_INFO.HardwareId >0)
     {
    	  TCHAR sDevice[32]={0};
 		if((pDevSecondaryBaroSource != NULL))
           if(!(pDevSecondaryBaroSource->Disabled) &&
              (pDevSecondaryBaroSource->Name != NULL))
           {
             _stprintf(sDevice, TEXT("%s"), pDevSecondaryBaroSource->Name  );
           }

 		if((pDevPrimaryBaroSource != NULL))
           if(!(pDevPrimaryBaroSource->Disabled) &&
              (pDevPrimaryBaroSource->Name != NULL))
           {
             _stprintf(sDevice, TEXT("%s"), pDevPrimaryBaroSource->Name );
           }

	  _stprintf(Temp,TEXT("%s (%i)"),sDevice, GPS_INFO.HardwareId);
		wp->SetText(Temp);
     }
     else
#endif     
	   wp->SetText(gettext(TEXT("_@M199_")));
    } else {
	// LKTOKEN  _@M240_ = "Disconnected" 
      wp->SetText(gettext(TEXT("_@M240_")));
    }
    wp->RefreshDisplay();
  }

  if (wp) {
    wp = (WndProperty*)wf->FindByName(TEXT("prpFLARM"));
    if (GPS_INFO.FLARM_Available) {
	// LKTOKEN  _@M199_ = "Connected" 

#if FLARMDEADLOCK
      if(FLARM_SW_Version > 0.0)
      {
    //	StartupStore(_T("STATUS: Flarm Version: %4.2f/%4.2f\n"),FLARM_SW_Version, FLARM_HW_Version);
		_stprintf(Temp,TEXT("OK (%4.2f/%4.2f) "),FLARM_SW_Version, FLARM_HW_Version);
    	wp->SetText(Temp);
      }
      else
#endif
        wp->SetText(gettext(TEXT("_@M199_")));
    } else {
	// LKTOKEN  _@M240_ = "Disconnected" 
      wp->SetText(gettext(TEXT("_@M240_")));
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLogger"));
  if (wp) {
    if (LoggerGActive()) {
      if (LoggerActive) {
	// LKTOKEN  _@M494_ = "ON (G)" 
        wp->SetText(gettext(TEXT("_@M494_")));
      } else {
	// LKTOKEN  _@M489_ = "OFF (G)" 
        wp->SetText(gettext(TEXT("_@M489_")));
      }
    }
    else { // no G Record
      if (LoggerActive) {
	// LKTOKEN  _@M495_ = "ON (no G)" 
        wp->SetText(gettext(TEXT("_@M495_")));
      } else {
	// LKTOKEN  _@M490_ = "OFF (no G)" 
        wp->SetText(gettext(TEXT("_@M490_")));
      }
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDeclared"));
  if (wp) {
    if (DeclaredToDevice) {
	// LKTOKEN  _@M827_ = "Yes" 
      wp->SetText(gettext(TEXT("_@M827_")));
    } else {
      wp->SetText(gettext(TEXT("_@M890_"))); // No
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVersion"));
  if (wp) {
      TCHAR softversion[100];
#ifndef LKCOMPETITION
      _stprintf(softversion,_T("%s.%s"),_T(LKVERSION), _T(LKRELEASE));
#else
      _stprintf(softversion,_T("%s.%s Competition"),_T(LKVERSION), _T(LKRELEASE));
#endif
      wp->SetText(softversion);
      wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBattBank"));
  if (wp) {
    _stprintf(Temp,TEXT("%d"),GPS_INFO.ExtBatt_Bank);
    wp->SetText(Temp);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpBatt1Volt"));
  if (wp) {
	if (GPS_INFO.ExtBatt1_Voltage>=1000)
		_stprintf(Temp,TEXT("%.0f%%"),GPS_INFO.ExtBatt1_Voltage-1000);
	else
		_stprintf(Temp,TEXT("%.2f V"),GPS_INFO.ExtBatt1_Voltage);
    wp->SetText(Temp);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpBatt2Volt"));
  if (wp) {
	if (GPS_INFO.ExtBatt1_Voltage>=1000)
		_stprintf(Temp,TEXT("%.0f%%"),GPS_INFO.ExtBatt2_Voltage-1000);
	else
		_stprintf(Temp,TEXT("%.2f V"),GPS_INFO.ExtBatt2_Voltage);
    wp->SetText(Temp);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBattery"));
  if (wp) {
    _tcscpy(Temp,TEXT("\0"));
#if (WINDOWSPC<1)
    _stprintf(Temp2,TEXT("%d%% "), PDABatteryPercent);
    _tcscat(Temp, Temp2);
#endif
    if (GPS_INFO.SupplyBatteryVoltage == 0) {
      _tcscpy(Temp2,TEXT("\0"));
    } else {
      _stprintf(Temp2,TEXT("%.1f V"),GPS_INFO.SupplyBatteryVoltage);
    }
    _tcscat(Temp, Temp2);

    wp->SetText(Temp);
    wp->RefreshDisplay();
  }
}


static void UpdateValuesTimes(void) {
  WndProperty *wp;
  TCHAR Temp[1000];
  double sunsettime;
  int sunsethours;
  int sunsetmins;

  sunsettime = DoSunEphemeris(GPS_INFO.Longitude,
                              GPS_INFO.Latitude);
  sunsethours = (int)sunsettime;
  sunsetmins = (int)((sunsettime-sunsethours)*60);

  wp = (WndProperty*)wf->FindByName(TEXT("prpSunset"));
  if (wp) {
    _stprintf(Temp, TEXT("%02d:%02d"), sunsethours,sunsetmins);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLocalTime"));
  if (wp) {
    Units::TimeToText(Temp, (int)DetectCurrentTime());
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTakeoffTime"));
  if (wp) {
    if (CALCULATED_INFO.FlightTime>0) {
      Units::TimeToText(Temp, 
                        (int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLandingTime"));
  if (wp) {
    if (!CALCULATED_INFO.Flying) {
      Units::TimeToText(Temp, 
                        (int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime
                                              +CALCULATED_INFO.FlightTime)));
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFlightTime"));
  if (wp) {
    if (CALCULATED_INFO.FlightTime > 0){
      Units::TimeToText(Temp, (int)CALCULATED_INFO.FlightTime);
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

}

static int nearest_waypoint= -1;


static void UpdateValuesFlight(void) {
  WndProperty *wp;
  TCHAR Temp[1000];
  double bearing;
  double distance;
  TCHAR sCoordinate[32]={0};
  TCHAR sBaroDevice[32]={0};

  Units::CoordinateToString(GPS_INFO.Longitude, GPS_INFO.Latitude, sCoordinate, sizeof(sCoordinate)-1);

  wp = (WndProperty *)wf->FindByName(TEXT("prpCoordinate"));
  if(wp) {
	  wp->SetText(sCoordinate);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
  if (wp) {

    _stprintf(sBaroDevice, TEXT("GPS"));
    if(EnableNavBaroAltitude)
    {
	  if(GPS_INFO.BaroAltitudeAvailable)
	  {
		_stprintf(sBaroDevice, TEXT("BARO"));

		if((pDevSecondaryBaroSource != NULL))
          if((! pDevSecondaryBaroSource->Disabled) &&
             (pDevSecondaryBaroSource->IsBaroSource)&&
             pDevSecondaryBaroSource->Name != NULL)
          {
            _stprintf(sBaroDevice, TEXT("%s"), pDevSecondaryBaroSource->Name );
          }

		if((pDevPrimaryBaroSource != NULL))
          if((! pDevPrimaryBaroSource->Disabled) &&
             (pDevPrimaryBaroSource->IsBaroSource)&&
             pDevPrimaryBaroSource->Name != NULL)
          {
            _stprintf(sBaroDevice, TEXT("%s"), pDevPrimaryBaroSource->Name );
          }
		if(GPS_INFO.haveRMZfromFlarm)
		 _stprintf(sBaroDevice, TEXT("FLARM"));
	  }
    }
	_stprintf(Temp, TEXT("%.0f%s (%s)"),
	                    CALCULATED_INFO.NavAltitude*ALTITUDEMODIFY,
	                    Units::GetAltitudeName(),
	                    sBaroDevice);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMaxHeightGain"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              CALCULATED_INFO.MaxHeightGain*ALTITUDEMODIFY, 
              Units::GetAltitudeName());
    wp->SetText(Temp);
  }

  if (nearest_waypoint>=0) {

    DistanceBearing(GPS_INFO.Latitude,
                    GPS_INFO.Longitude,
                    WayPointList[nearest_waypoint].Latitude,
                    WayPointList[nearest_waypoint].Longitude,
                    &distance,
                    &bearing);

    wp = (WndProperty*)wf->FindByName(TEXT("prpNear"));
    if (wp) {
      wp->SetText(WayPointList[nearest_waypoint].Name);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpBearing"));
    if (wp) {
      _stprintf(Temp, TEXT("%d%s"), iround(bearing),gettext(_T("_@M2179_")));
      wp->SetText(Temp);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpDistance"));
    if (wp) {
      TCHAR DistanceText[MAX_PATH];
      Units::FormatUserDistance(distance,DistanceText, 10);
      wp->SetText(DistanceText);
    }

  } else {
    wp = (WndProperty*)wf->FindByName(TEXT("prpNear"));
    if (wp) {
      wp->SetText(TEXT("-"));
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpBearing"));
    if (wp) {
      wp->SetText(TEXT("-"));
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpDistance"));
    if (wp) {
      wp->SetText(TEXT("-"));
    }
  }

}


static void UpdateValuesRules(void) {
  WndProperty *wp;
  TCHAR Temp[80];

  wp = (WndProperty*)wf->FindByName(TEXT("prpValidStart"));
  if (wp) {
    if (CALCULATED_INFO.ValidStart) {
	// LKTOKEN  _@M677_ = "TRUE" 
      wp->SetText(gettext(TEXT("_@M677_")));
    } else {
	// LKTOKEN  _@M278_ = "FALSE" 
      wp->SetText(gettext(TEXT("_@M278_")));
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpValidFinish"));
  if (wp) {
    if (CALCULATED_INFO.ValidFinish) {
	// LKTOKEN  _@M677_ = "TRUE" 
      wp->SetText(gettext(TEXT("_@M677_")));
    } else {
	// LKTOKEN  _@M278_ = "FALSE" 
      wp->SetText(gettext(TEXT("_@M278_")));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartTime"));
  if (wp) {
    if (CALCULATED_INFO.TaskStartTime>0) {
      Units::TimeToText(Temp, (int)TimeLocal((int)(CALCULATED_INFO.TaskStartTime)));
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartSpeed"));
  if (wp) {
    if (CALCULATED_INFO.TaskStartTime>0) {
      _stprintf(Temp, TEXT("%.0f %s"), 
                TASKSPEEDMODIFY*CALCULATED_INFO.TaskStartSpeed, 
                Units::GetTaskSpeedName());
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }
  // StartMaxHeight, StartMaxSpeed;

  //  double start_h;
  LockTaskData();

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartPoint"));

  if (ValidTaskPoint(0)) {
    //    start_h = WayPointList[Task[0].Index].Altitude;
    if (wp) {
      wp->SetText(WayPointList[Task[0].Index].Name);
    }
  } else {
    //    start_h = 0;
    if (wp) {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeight"));
  if (wp) {
    if (CALCULATED_INFO.TaskStartTime>0) {
      _stprintf(Temp, TEXT("%.0f %s"), 
                (CALCULATED_INFO.TaskStartAltitude)*ALTITUDEMODIFY, 
                Units::GetAltitudeName());
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishAlt"));
  if (wp) {
    double finish_min = FAIFinishHeight(&GPS_INFO, &CALCULATED_INFO, -1);
    _stprintf(Temp, TEXT("%.0f %s"), 
              finish_min*ALTITUDEMODIFY, 
              Units::GetAltitudeName());
    wp->SetText(Temp);
  }

  UnlockTaskData();
}


static void UpdateValuesTask(void) {
  WndProperty *wp;
  TCHAR Temp[80];

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskTime"));
  Units::TimeToText(Temp, (int)AATTaskLength*60);
  if (wp) {
    if (!AATEnabled) {
      wp->SetVisible(false);
    } else {
      wp->SetText(Temp);
    }
  }

  double dd = CALCULATED_INFO.TaskTimeToGo;
  if (CALCULATED_INFO.TaskStartTime>0.0) {
    dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpETETime"));
  if (wp) {
    Units::TimeToText(Temp, (int)dd);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRemainingTime"));
  if (wp) {
    Units::TimeToText(Temp, (int)CALCULATED_INFO.TaskTimeToGo);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskDistance"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), DISTANCEMODIFY*
              (CALCULATED_INFO.TaskDistanceToGo
               +CALCULATED_INFO.TaskDistanceCovered), 
              Units::GetDistanceName());
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRemainingDistance"));
  if (wp) {
    if (AATEnabled) {
      _stprintf(Temp, TEXT("%.0f %s"), 
                DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance, 
                Units::GetDistanceName());
    } else {
      _stprintf(Temp, TEXT("%.0f %s"), 
                DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo, 
                Units::GetDistanceName());
    }
    wp->SetText(Temp);
  }

  double d1 = 
    (CALCULATED_INFO.TaskDistanceToGo
     +CALCULATED_INFO.TaskDistanceCovered)/dd;
  // TODO bug: this fails for OLC

  wp = (WndProperty*)wf->FindByName(TEXT("prpEstimatedSpeed"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              TASKSPEEDMODIFY*d1, Units::GetTaskSpeedName());
    wp->SetText(Temp);
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAverageSpeed"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeed, 
              Units::GetTaskSpeedName());
    wp->SetText(Temp);
  }
}


static int OnTimerNotify(WindowControl * Sender) {
  (void)Sender;

  static short i=0;
  if(i++ % 2 == 0) return 0;

  UpdateValuesSystem();
  UpdateValuesFlight();

  return 0;
}


void dlgStatusShowModal(int start_page){

  if (start_page==-1) {
    multi_page = true;
    status_page = max(0,min(NUMPAGES-1,status_page));
  } else {
    status_page = start_page;
    multi_page = false;
  }

  first = true;

  TCHAR filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgStatus.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_STATUS"));

  if (!wf) return;

  #if FASTPAGE
  // LButtonUp notify will intercept ALL touchscreen and then ALSO process them 
  // normally, such as in buttons. For this reason we must use TouchContext.
  wf->SetLButtonUpNotify(TouchKeyDown);
  #endif

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wStatus0    = ((WndFrame *)wf->FindByName(TEXT("frmStatusFlight")));
  wStatus1    = ((WndFrame *)wf->FindByName(TEXT("frmStatusSystem")));
  wStatus2    = ((WndFrame *)wf->FindByName(TEXT("frmStatusTask")));
  wStatus3    = ((WndFrame *)wf->FindByName(TEXT("frmStatusRules")));
  wStatus4    = ((WndFrame *)wf->FindByName(TEXT("frmStatusTimes")));
  wStatus5    = ((WndFrame *)wf->FindByName(TEXT("frmStatusExtDevice")));

  //ASSERT(wStatus0!=NULL);
  //ASSERT(wStatus1!=NULL);
  //ASSERT(wStatus2!=NULL);
  //ASSERT(wStatus3!=NULL);
  //ASSERT(wStatus4!=NULL);
  //ASSERT(wStatus5!=NULL);

  wf->SetTimerNotify(OnTimerNotify);

  if (!multi_page) {
    WndButton *wb;
    wb = ((WndButton *)wf->FindByName(TEXT("cmdNext")));
    if (wb != NULL) {
      wb->SetVisible(false);
    }
    wb = ((WndButton *)wf->FindByName(TEXT("cmdPrev")));
    if (wb != NULL) {
      wb->SetVisible(false);
    }
  }

  nearest_waypoint = FindNearestWayPoint(GPS_INFO.Longitude,
                                         GPS_INFO.Latitude,
                                         100000.0, true); // big range limit

  UpdateValuesSystem();
  UpdateValuesFlight();
  UpdateValuesTask();
  UpdateValuesRules();
  UpdateValuesTimes();

  NextPage(0); // just to turn proper pages on/off

  wf->ShowModal();

  delete wf;

  wf = NULL;

}

