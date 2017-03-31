/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: AirfieldDetails.cpp,v 8.3 2010/12/10 22:13:23 root Exp root $
*/


#include "externs.h"
#include "Dialogs/dlgProgress.h"
#include "AirfieldDetails.h"
#include "utils/openzip.h"

ZZIP_FILE* OpenAirfieldDetails() {

  ZZIP_FILE * fp = nullptr;

  if (_tcslen(szAirfieldFile)>0) {
    TCHAR zfilename[MAX_PATH];
    LocalPath(zfilename, _T(LKD_WAYPOINTS), szAirfieldFile);
    fp = openzip(zfilename, "rb");
    StartupStore(_T(". open AirfieldFile %s <%s> "), fp ? _T("") : _T("FAILED"),  szAirfieldFile);
  }


  return fp;
}


void CloseAirfieldDetails(ZZIP_FILE* fp) {
  if (fp) {
    zzip_fclose(fp);
  }
};


void LookupAirfieldDetail(TCHAR *Name, TCHAR *Details) {
  TCHAR UName[100];
  TCHAR NameA[100];
  TCHAR NameB[100];
  TCHAR NameC[100];
  TCHAR NameD[100];
  TCHAR TmpName[100];
  bool isHome, isPreferred, isLandable;

  for(unsigned i=NUMRESWP;i<WayPointList.size();++i) {

	_tcscpy(UName, WayPointList[i].Name);

	CharUpper(UName); // WP name
	CharUpper(Name);  // AIR name  If airfields name was not uppercase it was not recon

	_stprintf(NameA,TEXT("%s A/F"),Name);
	_stprintf(NameB,TEXT("%s AF"),Name);
	_stprintf(NameC,TEXT("%s A/D"),Name);
	_stprintf(NameD,TEXT("%s AD"),Name);

	isHome=false;
	isPreferred=false;
    isLandable = (((WayPointList[i].Flags & AIRPORT) == AIRPORT) ||
                 ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT));

	_stprintf(TmpName,TEXT("%s=HOME"),UName);
	if ( (_tcscmp(Name, TmpName)==0) )  isHome=true;

  // Only bother checking whether it's preferred if it's landable.
  if (isLandable) {
	_stprintf(TmpName,TEXT("%s=PREF"),UName);
	if ( (_tcscmp(Name, TmpName)==0) )  isPreferred=true;
	_stprintf(TmpName,TEXT("%s=PREFERRED"),UName);
	if ( (_tcscmp(Name, TmpName)==0) )  isPreferred=true;
  }

	if ( isHome==true ) {
      if (isLandable) WayPointCalc[i].Preferred = true;
	  HomeWaypoint = i;
	  AirfieldsHomeWaypoint = i; // make it survive a reset..
	}
	if ( isPreferred==true ) {
	  WayPointCalc[i].Preferred = true;
	}

	if ((_tcscmp(UName, Name)==0)
	    ||(_tcscmp(UName, NameA)==0)
	    ||(_tcscmp(UName, NameB)==0)
	    ||(_tcscmp(UName, NameC)==0)
	    ||(_tcscmp(UName, NameD)==0)
	    || isHome || isPreferred )
	  {
	    if (_tcslen(Details) >0 ) { // avoid setting empty details
	      if (WayPointList[i].Details) {
		free(WayPointList[i].Details);
	      }
	      WayPointList[i].Details = (TCHAR*)malloc((_tcslen(Details)+1)*sizeof(TCHAR));
	      if (WayPointList[i].Details != NULL) _tcscpy(WayPointList[i].Details, Details);
	    }
	    return;
	  }
    }
}


#define DETAILS_LENGTH 5000

/*
 * fix: if empty lines, do not set details for the waypoint
 * fix: remove CR from text appearing as a spurious char in waypoint details
 */
void ParseAirfieldDetails(ZZIP_FILE* fp) {

  assert(fp);
  if(!fp) {
    return;
  }

  TCHAR TempString[READLINE_LENGTH+1];
  TCHAR CleanString[READLINE_LENGTH+1];
  TCHAR Details[DETAILS_LENGTH+1];
  TCHAR Name[201];

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;
  CleanString[0]=0;

  bool inDetails = false;
  bool hasDetails = false;
  int i, n;
  unsigned int j;

  charset cs = charset::unknown;
  while(ReadString(fp,READLINE_LENGTH,TempString, cs))
    {
      if(TempString[0]=='[') { // Look for start

	if (inDetails) {
	  LookupAirfieldDetail(Name, Details);
	  Details[0]= 0;
	  Name[0]= 0;
	  hasDetails=false;
	}

	// extract name
	for (i=1; i<200; i++) {
	  if (TempString[i]==']') {
	    break;
	  }
	  Name[i-1]= TempString[i];
	}
	Name[i-1]= 0;

	inDetails = true;

      } else {
	// VENTA3: append text to details string
	if (inDetails)  // BUGFIX 100711
	for (j=0; j<_tcslen(TempString); j++ ) {
	  if ( TempString[j] > 0x20 ) {
	    hasDetails = true;
	    break;
	  }
	}
	// first hasDetails set TRUE for rest of details
	if (hasDetails==true) {

	  // Remove carriage returns
	  for (j=0, n=0; j<_tcslen(TempString); j++) {
	    if ( TempString[j] == 0x0d ) continue;
	    CleanString[n++]=TempString[j];
	  }
	  CleanString[n]='\0';

	  if (_tcslen(Details)+_tcslen(CleanString)+3<DETAILS_LENGTH) {
	    _tcscat(Details,CleanString);
	    _tcscat(Details,TEXT("\r\n"));
	  }
	}
      }
    }

  if (inDetails) {
    LookupAirfieldDetail(Name, Details);
  }

}


void ReadAirfieldFile() {
  #if TESTBENCH
  StartupStore(TEXT(". ReadAirfieldFile"));
  #endif

	// LKTOKEN  _@M400_ = "Loading Waypoint Notes File..."
  CreateProgressDialog(MsgToken(400));

  ZZIP_FILE* fp = OpenAirfieldDetails();
  if(fp) {
    ParseAirfieldDetails(fp);
    CloseAirfieldDetails(fp);
  }
}
