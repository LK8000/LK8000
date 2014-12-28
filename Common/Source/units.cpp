/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: units.cpp,v 8.5 2010/12/13 17:37:35 root Exp root $
*/

//default       EU   UK   US   AUS
//altitude      m    ft   ft   m
//verticalspeed m/s  kts  kts  kts
//wind speed    km/  kts  mp   kts
//IAS           km/  kts  mp   kts
//distance      km   nm   ml   nm


#include "externs.h"


CoordinateFormats_t Units::CoordinateFormat;

UnitDescriptor_t Units::UnitDescriptors[] ={
  {NULL,         1,          0},
  {TEXT("km"),   0.001,      0},
  {TEXT("nm"),   0.00053996, 0},
  {TEXT("mi"),   0.0006214,  0},
  {TEXT("kh"), 0.0036,     0}, // 091219
  {TEXT("kt"),   0.001944,   0},
  {TEXT("mh"),  0.002237,   0},
  {TEXT("ms"),  1.0,        0},
  {TEXT("fm"),  3.281*60.0, 0}, 
  {TEXT("m"),    1.0,        0},
  {TEXT("ft"),   3.281,      0},
  {TEXT("K"),    1,          0},
  {NULL,   1.0,       -273.15}, // name is contruct by GetUnitName()
  {NULL,   1.8,       -459.67}, // name is contruct by GetUnitName()
  {TEXT("fs"),  3.281,      0} // 100128
};

Units_t Units::UserDistanceUnit = unKiloMeter;
Units_t Units::UserAltitudeUnit = unMeter;
Units_t Units::UserHorizontalSpeedUnit = unKiloMeterPerHour;
Units_t Units::UserVerticalSpeedUnit = unMeterPerSecond;
Units_t Units::UserWindSpeedUnit = unKiloMeterPerHour;
Units_t Units::UserTaskSpeedUnit = unKiloMeterPerHour;

void Units::LongitudeToDMS(double Longitude,
                           int *dd,
                           int *mm,
                           int *ss,
                           bool *east) {

  int sign = Longitude<0 ? 0 : 1;
  Longitude = fabs(Longitude);

  *dd = (int)Longitude;
  Longitude = (Longitude - (*dd)) * 60.0;
  *mm = (int)(Longitude);
  Longitude = (Longitude - (*mm)) * 60.0;
  *ss = (int)(Longitude + 0.5);
  if (*ss >= 60)
    {
      (*mm)++;
      (*ss) -= 60;
    }
  if ((*mm) >= 60)
    {
      (*dd)++;
      (*mm)-= 60;
    }
  *east = (sign==1);
}


void Units::LatitudeToDMS(double Latitude, 
                          int *dd,
                          int *mm,
                          int *ss,
                          bool *north) {

  int sign = Latitude<0 ? 0 : 1;
  Latitude = fabs(Latitude);

  *dd = (int)Latitude;
  Latitude = (Latitude - (*dd)) * 60.0;
  *mm = (int)(Latitude);
  Latitude = (Latitude - (*mm)) * 60.0;
  *ss = (int)(Latitude + 0.5);
  if (*ss >= 60) {
    (*mm)++;
    (*ss) -= 60;
  }
  if ((*mm) >= 60) {
    (*dd)++;
    (*mm) -= 60;
  }
  *north = (sign==1);
}

extern void LatLonToUtmWGS84 (int& utmXZone, char& utmYZone, double& easting, double& northing, double lat, double lon);

bool Units::CoordinateToString(double Longitude, double Latitude, TCHAR *Buffer, size_t size) {
	if(CoordinateFormat == cfUTM) {
		int utmzone=0;
		char utmchar=0;
		double easting=0, northing=0;
		LatLonToUtmWGS84 ( utmzone, utmchar, easting, northing, Latitude, Longitude );
		_sntprintf(Buffer, size, _T("UTM %d%c  %.0f  %.0f"), utmzone, utmchar, easting, northing);
	} else {
		TCHAR sLongitude[16] = {0};
		TCHAR sLatitude[16] = {0};

		Units::LongitudeToString(Longitude, sLongitude, sizeof(sLongitude)-1);
		Units::LatitudeToString(Latitude, sLatitude, sizeof(sLatitude)-1);

		_sntprintf(Buffer,size,_T("%s  %s"), sLatitude, sLongitude);
	}
	return true;
}


bool Units::LongitudeToString(double Longitude, TCHAR *Buffer, size_t size){
  (void)size;
  TCHAR EW[] = TEXT("WE");
  int dd, mm, ss;

  int sign = Longitude<0 ? 0 : 1;
  Longitude = fabs(Longitude);


  switch(CoordinateFormat){
    case cfDDMMSS:
      dd = (int)Longitude;
      Longitude = (Longitude - dd) * 60.0;
      mm = (int)(Longitude);
      Longitude = (Longitude - mm) * 60.0;
      ss = (int)(Longitude + 0.5);
      if (ss >= 60)
        {
          mm++;
          ss -= 60;
        }
      if (mm >= 60)
        {
          dd++;
          mm -= 60;
        }
      _stprintf(Buffer, TEXT("%c%03d%s%02d'%02d\""), EW[sign], dd, gettext(_T("_@M2179_")), mm, ss);
    break;
    case cfDDMMSSss:
      dd = (int)Longitude;
      Longitude = (Longitude - dd) * 60.0;
      mm = (int)(Longitude);
      Longitude = (Longitude - mm) * 60.0;
      _stprintf(Buffer, TEXT("%c%03d%s%02d'%05.2f\""), EW[sign], dd, gettext(_T("_@M2179_")), mm, Longitude);
    break;
    case cfDDMMmmm:
      dd = (int)Longitude;
      Longitude = (Longitude - dd) * 60.0;
      _stprintf(Buffer, TEXT("%c%03d%s%06.3f'"), EW[sign], dd, gettext(_T("_@M2179_")), Longitude);
    break;
    case cfDDdddd:
      _stprintf(Buffer, TEXT("%c%08.4f%s"), EW[sign], Longitude, gettext(_T("_@M2179_")));
    break;
    case cfUTM:
	_tcscpy(Buffer,_T(""));
	break;
    default:
	LKASSERT(0);
    break;
  }

  return(true);

}


bool Units::LatitudeToString(double Latitude, TCHAR *Buffer, size_t size){
  (void)size;
  TCHAR EW[] = TEXT("SN");
  int dd, mm, ss;

  int sign = Latitude<0 ? 0 : 1;
  Latitude = fabs(Latitude);

  switch(CoordinateFormat){
    case cfDDMMSS:
      dd = (int)Latitude;
      Latitude = (Latitude - dd) * 60.0;
      mm = (int)(Latitude);
      Latitude = (Latitude - mm) * 60.0;
      ss = (int)(Latitude + 0.5);
      if (ss >= 60) {
        mm++;
        ss -= 60;
      }
      if (mm >= 60) {
        dd++;
        mm -= 60;
      }
      _stprintf(Buffer, TEXT("%c%02d%s%02d'%02d\""), EW[sign], dd, gettext(_T("_@M2179_")), mm, ss);
    break;
    case cfDDMMSSss:
      dd = (int)Latitude;
      Latitude = (Latitude - dd) * 60.0;
      mm = (int)(Latitude);
      Latitude = (Latitude - mm) * 60.0;
      _stprintf(Buffer, TEXT("%c%02d%s%02d'%05.2f\""), EW[sign], dd, gettext(_T("_@M2179_")), mm, Latitude);
    break;
    case cfDDMMmmm:
      dd = (int)Latitude;
      Latitude = (Latitude - dd) * 60.0;
      _stprintf(Buffer, TEXT("%c%02d%s%06.3f'"), EW[sign], dd, gettext(_T("_@M2179_")), Latitude);
    break;
    case cfDDdddd:
      _stprintf(Buffer, TEXT("%c%07.4f%s"), EW[sign], Latitude, gettext(_T("_@M2179_")));
    break;
    case cfUTM:
	_tcscpy(Buffer,_T(""));
	break;
    default:
	LKASSERT(0);
    break;
  }

  return(true);

}

const TCHAR *Units::GetUnitName(Units_t Unit) {
    //  return(gettext(UnitDescriptors[Unit].Name)); 
    // JMW adjusted this because units are pretty standard internationally
    // so don't need different names in different languages.
    if (!UnitDescriptors[Unit].Name) {
        switch (Unit) {
            case unUndef:
            case unKiloMeter:
            case unNauticalMiles:
            case unStatuteMiles:
            case unKiloMeterPerHour:
            case unKnots:
            case unStatuteMilesPerHour:
            case unMeterPerSecond:
            case unFeetPerMinutes:
            case unMeter:
            case unFeet:
            case unFligthLevel:
            case unKelvin:
                break;          
            case unGradCelcius:
                UnitDescriptors[Unit].Name = gettext(_T("_@2180"));
                break;
            case unGradFahrenheit:
                UnitDescriptors[Unit].Name = gettext(_T("_@2181"));
                break;
        }
    }
    return UnitDescriptors[Unit].Name;
}

Units_t Units::GetUserDistanceUnit(void){
  return(UserDistanceUnit);
}

Units_t Units::SetUserDistanceUnit(Units_t NewUnit){
  Units_t last = UserDistanceUnit;
  if (UserDistanceUnit != NewUnit){
    UserDistanceUnit = NewUnit;
    NotifyUnitChanged();
  }
  return(last);
}

Units_t Units::GetUserAltitudeUnit(void){
  return(UserAltitudeUnit);
}

Units_t Units::GetUserInvAltitudeUnit(void){ // 100126
  return(UserAltitudeUnit==unFeet?unMeter:unFeet);
}

Units_t Units::SetUserAltitudeUnit(Units_t NewUnit){
  Units_t last = UserAltitudeUnit;
  if (UserAltitudeUnit != NewUnit){
    UserAltitudeUnit = NewUnit;
    NotifyUnitChanged();
  }
  return(last);
}

Units_t Units::GetUserHorizontalSpeedUnit(void){
  return(UserHorizontalSpeedUnit);
}

Units_t Units::SetUserHorizontalSpeedUnit(Units_t NewUnit){
  Units_t last = UserHorizontalSpeedUnit;
  if (UserHorizontalSpeedUnit != NewUnit){
    UserHorizontalSpeedUnit = NewUnit;
    NotifyUnitChanged();
  }
  return(last);
}

Units_t Units::GetUserTaskSpeedUnit(void){
  return(UserTaskSpeedUnit);
}

Units_t Units::SetUserTaskSpeedUnit(Units_t NewUnit){
  Units_t last = UserTaskSpeedUnit;
  if (UserTaskSpeedUnit != NewUnit){
    UserTaskSpeedUnit = NewUnit;
    NotifyUnitChanged();
  }
  return(last);
}

Units_t Units::GetUserVerticalSpeedUnit(void){
  return(UserVerticalSpeedUnit);
}

Units_t Units::SetUserVerticalSpeedUnit(Units_t NewUnit){
  Units_t last = UserVerticalSpeedUnit;
  if (UserVerticalSpeedUnit != NewUnit){
    UserVerticalSpeedUnit = NewUnit;
    NotifyUnitChanged();
  }
  return(last);
}

Units_t Units::GetUserWindSpeedUnit(void){
  return(UserWindSpeedUnit);
}

Units_t Units::SetUserWindSpeedUnit(Units_t NewUnit){
  Units_t last = UserWindSpeedUnit;
  if (UserWindSpeedUnit != NewUnit){
    UserWindSpeedUnit = NewUnit;
    NotifyUnitChanged();
  }
  return(last);
}

Units_t Units::GetUserUnitByGroup(UnitGroup_t UnitGroup){
  switch(UnitGroup){
    case ugNone:
    return(unUndef);
    case ugDistance:
    return(GetUserDistanceUnit());
    case ugAltitude:
    return(GetUserAltitudeUnit());
    case ugHorizontalSpeed:
    return(GetUserHorizontalSpeedUnit());
    case ugVerticalSpeed:
    return(GetUserVerticalSpeedUnit());
    case ugWindSpeed:
    return(GetUserWindSpeedUnit());
    case ugTaskSpeed:
    return(GetUserTaskSpeedUnit());
    case ugInvAltitude:
    return(GetUserInvAltitudeUnit());
    default:
      return(unUndef);
  }
}


void Units::NotifyUnitChanged(void){
  // todo

  if (SPEEDMODIFY==TOMPH) {
    SetUserHorizontalSpeedUnit(unStatuteMilesPerHour);
    SetUserWindSpeedUnit(unStatuteMilesPerHour);
  }
  if (SPEEDMODIFY==TOKNOTS) {
    SetUserHorizontalSpeedUnit(unKnots);
    SetUserWindSpeedUnit(unKnots);
  }
  if (SPEEDMODIFY==TOKPH) {
    SetUserHorizontalSpeedUnit(unKiloMeterPerHour);
    SetUserWindSpeedUnit(unKiloMeterPerHour);
  }

  if (DISTANCEMODIFY == TOMILES) {
    SetUserDistanceUnit(unStatuteMiles);
  }
  if (DISTANCEMODIFY == TONAUTICALMILES) {
    SetUserDistanceUnit(unNauticalMiles);
  }
  if (DISTANCEMODIFY == TOKILOMETER) {
    SetUserDistanceUnit(unKiloMeter);
  }

  if (ALTITUDEMODIFY == TOFEET) {
    SetUserAltitudeUnit(unFeet);
  }
  if (ALTITUDEMODIFY == TOMETER) {
    SetUserAltitudeUnit(unMeter);
  }

  if (LIFTMODIFY==TOKNOTS) {
    SetUserVerticalSpeedUnit(unKnots);
  }
  if (LIFTMODIFY==TOMETER) {
    SetUserVerticalSpeedUnit(unMeterPerSecond);
  }
#if 1
  if (LIFTMODIFY==TOFEETPERMINUTE) { // 100128
    SetUserVerticalSpeedUnit(unFeetPerMinutes);
  }
#endif

  if (TASKSPEEDMODIFY==TOMPH) {
    SetUserTaskSpeedUnit(unStatuteMilesPerHour);
  }
  if (TASKSPEEDMODIFY==TOKNOTS) {
    SetUserTaskSpeedUnit(unKnots);
  }
  if (TASKSPEEDMODIFY==TOKPH) {
    SetUserTaskSpeedUnit(unKiloMeterPerHour);
  }

}

const TCHAR *Units::GetHorizontalSpeedName(){
  return(GetUnitName(GetUserHorizontalSpeedUnit()));
}

const TCHAR *Units::GetVerticalSpeedName(){
  return(GetUnitName(GetUserVerticalSpeedUnit()));
}

const TCHAR *Units::GetDistanceName(){
  return(GetUnitName(GetUserDistanceUnit()));
}

const TCHAR *Units::GetAltitudeName(){
  return(GetUnitName(GetUserAltitudeUnit()));
}

const TCHAR *Units::GetInvAltitudeName(){ // 100126
  return(GetUnitName(GetUserInvAltitudeUnit()));
}

const TCHAR *Units::GetTaskSpeedName(){
  return(GetUnitName(GetUserTaskSpeedUnit()));
}

bool Units::FormatUserAltitude(double Altitude, TCHAR *Buffer, size_t size){

  int prec;
  TCHAR sTmp[32];
  UnitDescriptor_t *pU = &UnitDescriptors[UserAltitudeUnit]; 

  Altitude = Altitude * pU->ToUserFact; // + pU->ToUserOffset;

//  prec = 4-log10(Altitude);
//  prec = max(prec, 0);
  prec = 0;

  _stprintf(sTmp, TEXT("%.*f%s"), prec, Altitude, GetUnitName(UserAltitudeUnit));

  if (_tcslen(sTmp) < size-1){
    _tcscpy(Buffer, sTmp);
    return(true);
  } else {
    LK_tcsncpy(Buffer, sTmp, size-1);
    return(false);
  }

}

#if 100126
// I don't think it is a good idea to change, even if for a short time, a global variable at all effect here,
// just to be able to call the function above with a parameter missing!
bool Units::FormatAlternateUserAltitude(double Altitude, TCHAR *Buffer, size_t size){
  Units_t useUnit;
  TCHAR sTmp[32];

  if (UserAltitudeUnit == unMeter)
	useUnit=unFeet;
  else
	useUnit=unMeter;

  UnitDescriptor_t *pU = &UnitDescriptors[useUnit]; 
  Altitude = Altitude * pU->ToUserFact;
  _stprintf(sTmp, TEXT("%.*f%s"), 0, Altitude, GetUnitName(useUnit));

  if (_tcslen(sTmp) < size-1){
	_tcscpy(Buffer, sTmp);
	return(true);
  } else {
	LK_tcsncpy(Buffer, sTmp, size-1);
	return(false);
  }
}

#else
bool Units::FormatAlternateUserAltitude(double Altitude, TCHAR *Buffer, size_t size){
  Units_t saveUnit = UserAltitudeUnit;
  bool res;

  if (saveUnit == unMeter)
    UserAltitudeUnit = unFeet;
  if (saveUnit == unFeet)
    UserAltitudeUnit = unMeter;

  res = FormatUserAltitude(Altitude, Buffer, size);

  UserAltitudeUnit = saveUnit;

  return(res);

}
#endif

bool Units::FormatUserArrival(double Altitude, TCHAR *Buffer, size_t size){

  int prec;
  TCHAR sTmp[32];
  UnitDescriptor_t *pU = &UnitDescriptors[UserAltitudeUnit];

  Altitude = Altitude * pU->ToUserFact; // + pU->ToUserOffset;

//  prec = 4-log10(Altitude);
//  prec = max(prec, 0);
  prec = 0;

  _stprintf(sTmp, TEXT("%+.*f%s"), prec, Altitude, GetUnitName(UserAltitudeUnit));

  if (_tcslen(sTmp) < size-1){
    _tcscpy(Buffer, sTmp);
    return(true);
  } else {
    LK_tcsncpy(Buffer, sTmp, size-1);
    return(false);
  }

}

bool Units::FormatUserDistance(double Distance, TCHAR *Buffer, size_t size){

  int prec;
  double value;
  TCHAR sTmp[32];
  UnitDescriptor_t *pU = &UnitDescriptors[UserDistanceUnit];

  value = Distance * pU->ToUserFact; // + pU->ToUserOffset;

  if (value >= 100)
    prec = 0;
  else if (value > 10)
    prec = 1;
  else if (value > 1)
    prec = 2;
  else {
    prec = 3;
    if (UserDistanceUnit == unKiloMeter){
      prec = 0;
      pU = &UnitDescriptors[unMeter];
      value = Distance * pU->ToUserFact;
    }
    if (UserDistanceUnit == unNauticalMiles 
        || UserDistanceUnit == unStatuteMiles) {
      pU = &UnitDescriptors[unFeet];
      value = Distance * pU->ToUserFact;
      if (value<1000) {
        prec = 0;
      } else {
        prec = 1;
        pU = &UnitDescriptors[UserDistanceUnit];
        value = Distance* pU->ToUserFact;
      }
    }
  }

  _stprintf(sTmp, TEXT("%.*f%s"), prec, value, GetUnitName(UserDistanceUnit));

  if (_tcslen(sTmp) < size-1){
    _tcscpy(Buffer, sTmp);
    return(true);
  } else {
    LK_tcsncpy(Buffer, sTmp, size-1);
    return(false);
  }

}

bool Units::FormatUserMapScale(Units_t *Unit, double Distance, TCHAR *Buffer, size_t size){

  int prec;
  double value;
  TCHAR sTmp[32];
  UnitDescriptor_t *pU = &UnitDescriptors[UserDistanceUnit];

  if (Unit != NULL)
    *Unit = UserDistanceUnit;

  value = Distance * pU->ToUserFact; // + pU->ToUserOffset;

  if (value >= 9.999)
    prec = 0;
  else if (value >= 0.999)
    prec = 1;
  else {
    prec = 2;
    if (UserDistanceUnit == unKiloMeter){
      prec = 0;
      if (Unit != NULL)
        *Unit = unMeter;
      pU = &UnitDescriptors[unMeter];
      value = Distance * pU->ToUserFact;
    }
    if ((UserDistanceUnit == unNauticalMiles || UserDistanceUnit == unStatuteMiles) && (value < 0.160)) {
      prec = 0;
      if (Unit != NULL)
        *Unit = unFeet;
      pU = &UnitDescriptors[unFeet];
      value = Distance * pU->ToUserFact;
    }
  }

  _stprintf(sTmp, TEXT("%.*f%s"), prec, value, GetUnitName(UserDistanceUnit));

  if (_tcslen(sTmp) < size-1){
    _tcscpy(Buffer, sTmp);
    return(true);
  } else {
    LK_tcsncpy(Buffer, sTmp, size-1);
    return(false);
  }

}


double Units::ToUserAltitude(double Altitude){
  UnitDescriptor_t *pU = &UnitDescriptors[UserAltitudeUnit];

  Altitude = Altitude * pU->ToUserFact; // + pU->ToUserOffset;
  
  return(Altitude);
}

double Units::ToSysAltitude(double Altitude){
  UnitDescriptor_t *pU = &UnitDescriptors[UserAltitudeUnit];

  Altitude = Altitude / pU->ToUserFact; // + pU->ToUserOffset;
  
  return(Altitude);
}


double Units::ToUserDistance(double Distance){
  UnitDescriptor_t *pU = &UnitDescriptors[UserDistanceUnit];

  Distance = Distance * pU->ToUserFact; // + pU->ToUserOffset;
  
  return(Distance);
}

double Units::ToSysDistance(double Distance){
  UnitDescriptor_t *pU = &UnitDescriptors[UserDistanceUnit];

  Distance = Distance / pU->ToUserFact; // + pU->ToUserOffset;
  
  return(Distance);
}

void Units::TimeToText(TCHAR* text, int d) {
  int hours, mins;
  bool negative = (d<0);
  int dd = abs(d) % (3600*24);
  hours = (dd/3600);
  mins = (dd/60-hours*60);
  hours = hours % 24;
  if (negative) {
    _stprintf(text, TEXT("-%02d:%02d"),		  
              hours, mins);
  } else {
    _stprintf(text, TEXT("%02d:%02d"),		  
              hours, mins);
  }
}

void Units::TimeToTextSimple(TCHAR* text, int d) {
  int hours, mins;
  bool negative = (d<0);
  int dd = abs(d) % (3600*24);
  hours = (dd/3600);
  mins = (dd/60-hours*60);
  hours = hours % 24;
  if (negative) {
    _stprintf(text, TEXT("-%02d%02d"),		  
              hours, mins);
  } else {
    _stprintf(text, TEXT("%02d%02d"),		  
              hours, mins);
  }
}

// Not for displaying a clock time, good for a countdown
// will display either
// Returns true if hours, false if minutes 
bool Units::TimeToTextDown(TCHAR* text, int d) {
  int hours, mins, seconds;
  bool negative = (d<0);
  int dd = abs(d) % (3600*24);
  bool ishours;
  hours = (dd/3600);
  mins = (dd/60-hours*60);
  hours = hours % 24;
  seconds = (dd-mins*60-hours*3600);

  if (hours==0) {
	_stprintf(text, TEXT("%02d:%02d"), mins, seconds);
	ishours=false;
  } else {
	_stprintf(text, TEXT("%02d:%02d"), hours, mins);
	ishours=true;
  }

  if (negative) {
	TCHAR t[20];
	_stprintf(t,TEXT("-%s"),text); 
	_tcscpy(text,t);
  }
  return ishours;

}

// LK8000 
void Units::TimeToTextS(TCHAR* text, int d) {
  int hours, mins, seconds;
  bool negative = (d<0);
  int dd = abs(d) % (3600*24);
  hours = (dd/3600);
  mins = (dd/60-hours*60);
  seconds = (dd-mins*60-hours*3600);
  hours = hours % 24;
  if (negative) {
	if (hours>0) {
		hours = -hours;
	} else if (mins>0) {
		mins = -mins;
	} else {
		seconds = -seconds;
	}
	_stprintf(text, TEXT("-%d:%02d:%02d"),  hours, mins, seconds);
  } else {
	_stprintf(text, TEXT("%d:%02d:%02d"),  hours, mins, seconds);
  }
}

