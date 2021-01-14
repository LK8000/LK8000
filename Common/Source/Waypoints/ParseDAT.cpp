/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "utils/tokenizer.h"


static int CheckFlags(TCHAR *temp);
static  double CalculateAngle(TCHAR *temp);
extern int globalFileNum;



// This is converting DAT Winpilot
int ParseDAT(TCHAR *String,WAYPOINT *Temp)
{
  TCHAR *Number;
  TCHAR *pToken;
  TCHAR TempString[READLINE_LENGTH];

  _tcscpy(TempString, String);
  // 20060513:sgi added wor on a copy of the string, do not modify the
  // source string, needed on error messages

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_DAT;

  Temp->FileNum = globalFileNum;

  lk::tokenizer<TCHAR> tok(TempString);

  // ExtractParameter(TempString,ctemp,0);
  if ((pToken = tok.Next({_T(',')})) == NULL)
    return FALSE;
  Temp->Number = _tcstol(pToken, &Number, 10);

  //ExtractParameter(TempString,ctemp,1); //Latitude
  if ((pToken = tok.Next({_T(',')})) == NULL)
    return FALSE;
  Temp->Latitude = CalculateAngle(pToken);

  if((Temp->Latitude > 90) || (Temp->Latitude < -90))
    {
      return FALSE;
    }

  //ExtractParameter(TempString,ctemp,2); //Longitude
  if ((pToken = tok.Next({_T(',')})) == NULL)
    return FALSE;
  Temp->Longitude  = CalculateAngle(pToken);
  if((Temp->Longitude  > 180) || (Temp->Longitude  < -180))
    {
      return FALSE;
    }

  //ExtractParameter(TempString,ctemp,3); //Altitude
  if ((pToken = tok.Next({_T(',')})) == NULL)
    return FALSE;
  Temp->Altitude = ReadAltitude(pToken);
  if (Temp->Altitude == -9999){
    return FALSE;
  }

  //ExtractParameter(TempString,ctemp,4); //Flags
  if ((pToken = tok.Next({_T(',')})) == NULL)
    return FALSE;
  Temp->Flags = CheckFlags(pToken);

  //ExtractParameter(TempString,ctemp,5); // Name
  if ((pToken = tok.Next({_T(',')})) == NULL)
    return FALSE;

  // guard against overrun
  if (_tcslen(pToken)>NAME_SIZE) {
    pToken[NAME_SIZE-1]= _T('\0');
  }

  _tcscpy(Temp->Name, pToken);
  int i;
  for (i=_tcslen(Temp->Name)-1; i>1; i--) {
    if (Temp->Name[i]==' ') {
      Temp->Name[i]=0;
    } else {
      break;
    }
  }

  //ExtractParameter(TempString,ctemp,6); // Comment
  // DAT Comment
  if ((pToken = tok.Next({_T(',')})) != NULL) {
    SetWaypointComment(*Temp, pToken);
  } else {
    SetWaypointComment(*Temp, _T(""));
  }

  if(Temp->Altitude <= 0) {
    WaypointAltitudeFromTerrain(Temp);
  }

  if (Temp->Details) {
    free(Temp->Details);
  }

  return TRUE;
}

  /*
void ExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber)
{
  int index = 0;
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength        = 0;

  StringLength = _tcslen(Source);

  while( (CurrentFieldNumber < DesiredFieldNumber) && (index < StringLength) )
    {
      if ( Source[ index ] == ',' )
        {
          CurrentFieldNumber++;
        }
      index++;
    }

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (index < StringLength)    &&
             (Source[ index ] != ',') &&
             (Source[ index ] != 0x00) )
        {
          Destination[dest_index] = Source[ index ];
          index++; dest_index++;
        }
      Destination[dest_index] = '\0';
    }
  // strip trailing spaces
  for (int i=dest_index-1; i>0; i--) {
    if (Destination[i]==' ') {
      Destination[i]= '\0';
    } else return;
  }
}
*/

static double CalculateAngle(TCHAR *temp)
{
  TCHAR *Colon;
  const TCHAR *Stop;
  double Degrees, Mins;

  Colon = _tcschr(temp,':');

  if(!Colon)
    {
      return -9999;
    }

  *Colon = _T('\0');
  Colon ++;

  Degrees = StrToDouble(temp, &Stop);
  Mins = StrToDouble(Colon, &Stop);
  if (Stop && *Stop == ':') {
    Stop++;
    Mins += (StrToDouble(Stop, &Stop)/60.0);
  }

  Degrees += (Mins/60);

  if( Stop && ((*Stop == 'N') || (*Stop == 'E')))
    {
    }
  else if( Stop && ((*Stop == 'S') || (*Stop == 'W')))
    {
      Degrees *= -1;
    }
  else
    {
      return -9999;
    }

  return Degrees;
}



static int CheckFlags(TCHAR *temp)
{
  int Flags = 0;

  if(_tcschr(temp,'A')) Flags += AIRPORT;
  if(_tcschr(temp,'T')) Flags += TURNPOINT;
  if(_tcschr(temp,'L')) Flags += LANDPOINT;
  if(_tcschr(temp,'H')) Flags += HOME;
  if(_tcschr(temp,'S')) Flags += START;
  if(_tcschr(temp,'F')) Flags += FINISH;
  if(_tcschr(temp,'R')) Flags += RESTRICTED;
  if(_tcschr(temp,'W')) Flags += WAYPOINTFLAG;

  return Flags;
}
