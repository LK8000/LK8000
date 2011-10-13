/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#else
#include "wcecompat/ts_string.h"
#endif

#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "lk8000.h"

#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif


void FormatWarningString(int Type, TCHAR *Name , AIRSPACE_ALT Base, AIRSPACE_ALT Top, TCHAR *szMessageBuffer, TCHAR *szTitleBuffer )
{
  TCHAR BaseStr[512];
  TCHAR TopStr[512];

  switch (Type)
    {
    case RESTRICT:	  
	// LKTOKEN  _@M565_ = "Restricted" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M565_"))); break;
    case PROHIBITED:	  
	// LKTOKEN  _@M537_ = "Prohibited" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M537_"))); break;
    case DANGER:          
	// LKTOKEN  _@M213_ = "Danger Area" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M213_"))); break;
    case CLASSA:          
      _tcscpy(szTitleBuffer,TEXT("Class A")); break;
    case CLASSB:          
      _tcscpy(szTitleBuffer,TEXT("Class B")); break;
    case CLASSC:          
      _tcscpy(szTitleBuffer,TEXT("Class C")); break;
    case CLASSD:          
      _tcscpy(szTitleBuffer,TEXT("Class D")); break;
    case CLASSE:			
      _tcscpy(szTitleBuffer,TEXT("Class E")); break;
    case CLASSF:			
      _tcscpy(szTitleBuffer,TEXT("Class F")); break;
    case CLASSG:			
      _tcscpy(szTitleBuffer,TEXT("Class G")); break;
    case NOGLIDER:		
	// LKTOKEN  _@M464_ = "No Glider" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M464_"))); break;
    case CTR:					
      _tcscpy(szTitleBuffer,TEXT("CTR")); break;
    case WAVE:				
	// LKTOKEN  _@M794_ = "Wave" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M794_"))); break;
    case CLASSTMZ:            
      _tcscpy(szTitleBuffer,TEXT("TMZ")); break;
    default:					
	// LKTOKEN  _@M765_ = "Unknown" 
      _tcscpy(szTitleBuffer,gettext(TEXT("_@M765_")));
    }

  if(Base.FL == 0)
    {
      if (Base.AGL > 0) {
        _stprintf(BaseStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Base.AGL, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("AGL"));
      } else if (Base.Altitude > 0)
        _stprintf(BaseStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Base.Altitude, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("MSL"));
      else
        _stprintf(BaseStr,TEXT("GND"));
    }
  else
    {
      _stprintf(BaseStr,TEXT("FL %1.0f"),Base.FL );
    }

  if(Top.FL == 0)
    {
      if (Top.AGL > 0) {
        _stprintf(TopStr,TEXT("%1.0f%s %s"), 
                  ALTITUDEMODIFY * Top.AGL, 
                  Units::GetUnitName(Units::GetUserAltitudeUnit()),
                  TEXT("AGL"));
      } else {
	_stprintf(TopStr,TEXT("%1.0f%s %s"), ALTITUDEMODIFY * Top.Altitude, 
		  Units::GetUnitName(Units::GetUserAltitudeUnit()),
		  TEXT("MSL"));
      }
    }
  else
    {
      _stprintf(TopStr,TEXT("FL %1.0f"),Top.FL );
    }

  _stprintf(szMessageBuffer,TEXT("%s: %s\r\n%s: %s\r\n%s: %s\r\n"),
            szTitleBuffer, 
            Name, 
	// LKTOKEN  _@M729_ = "Top" 
            gettext(TEXT("_@M729_")),
            TopStr,
	// LKTOKEN  _@M128_ = "Base" 
            gettext(TEXT("_@M128_")),
            BaseStr 
            );
}



void ExtractDirectory(TCHAR *Dest, TCHAR *Source) {
  int len = _tcslen(Source);
  int found = -1;
  int i;
  if (len==0) {
    Dest[0]= 0;
    return;
  }
  for (i=0; i<len; i++) {
    if ((Source[i]=='/')||(Source[i]=='\\')) {
      found = i;
    }
  }
  for (i=0; i<=found; i++) {
    Dest[i]= Source[i];
  }
  Dest[i]= 0;
}


void StatusFileInit() {
  #if TESTBENCH
  StartupStore(TEXT(". StatusFileInit%s"),NEWLINE);
  #endif

  // DEFAULT - 0 is loaded as default, and assumed to exist
  StatusMessageData[0].key = TEXT("DEFAULT");
  StatusMessageData[0].doStatus = true;
  StatusMessageData[0].doSound = true; 
  StatusMessageData[0].sound = TEXT("IDR_WAV_DRIP");
  StatusMessageData_Size=1;
#ifdef VENTA_DEBUG_EVENT // VENTA- longer statusmessage delay in event debug mode
	StatusMessageData[0].delay_ms = 10000;  // 10 s
#else
    StatusMessageData[0].delay_ms = 2500; // 2.5 s
#endif

  // Load up other defaults - allow overwrite in config file
#include "Status_defaults.cpp"

}

void ReadStatusFile() {
  
  StartupStore(TEXT(". Loading status file%s"),NEWLINE);

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  FILE *fp=NULL;
  
  // Open file from registry
  GetRegistryString(szRegistryStatusFile, szFile1, MAX_PATH);
  ExpandLocalPath(szFile1);

  SetRegistryString(szRegistryStatusFile, TEXT("\0"));
  
  if (_tcslen(szFile1)>0)
    fp  = _tfopen(szFile1, TEXT("rt"));
  
  // Unable to open file
  if (fp == NULL)
    return;
  
  // TODO code: Safer sizes, strings etc - use C++ (can scanf restrict length?)
  TCHAR buffer[2049];	// Buffer for all
  TCHAR key[2049];	// key from scanf
  TCHAR value[2049];	// value from scanf
  int ms;				// Found ms for delay
  TCHAR **location;	// Where to put the data
  int found;			// Entries found from scanf
  bool some_data;		// Did we find some in the last loop...
  
  // Init first entry
  _init_Status(StatusMessageData_Size);
  some_data = false;
  
  /* Read from the file */
  while (
	 (StatusMessageData_Size < MAXSTATUSMESSAGECACHE)
	 && fgetws(buffer, 2048, fp)
	 && ((found = swscanf(buffer, TEXT("%[^#=]=%[^\n]\n"), key, value)) != EOF)
	 ) {
    // Check valid line? If not valid, assume next record (primative, but works ok!)
    if ((found != 2) || key[0] == '\0' || value == '\0') {
      
      // Global counter (only if the last entry had some data)
      if (some_data) {
	StatusMessageData_Size++;
	some_data = false;
	_init_Status(StatusMessageData_Size);
      }
      
    } else {
      
      location = NULL;
      
      if (wcscmp(key, TEXT("key")) == 0) {
	some_data = true;	// Success, we have a real entry
	location = &StatusMessageData[StatusMessageData_Size].key;
      } else if (wcscmp(key, TEXT("sound")) == 0) {
	StatusMessageData[StatusMessageData_Size].doSound = true;
	location = &StatusMessageData[StatusMessageData_Size].sound;
      } else if (wcscmp(key, TEXT("delay")) == 0) {
	if (swscanf(value, TEXT("%d"), &ms) == 1)
	  StatusMessageData[StatusMessageData_Size].delay_ms = ms;
      } else if (wcscmp(key, TEXT("hide")) == 0) {
	if (wcscmp(value, TEXT("yes")) == 0)
	  StatusMessageData[StatusMessageData_Size].doStatus = false;
      }
      
      // Do we have somewhere to put this && is it currently empty ? (prevent lost at startup)
      if (location && (wcscmp(*location, TEXT("")) == 0)) {
	// TODO code: this picks up memory lost from no entry, but not duplicates - fix.
	if (*location) {
	  // JMW fix memory leak
	  free(*location);
	}
	*location = StringMallocParse(value);
      }
    }
    
  }
  
  // How many we really got (blank next just in case)
  StatusMessageData_Size++;
  _init_Status(StatusMessageData_Size);
  
  // file was ok, so save it to registry
  ContractLocalPath(szFile1);
  SetRegistryString(szRegistryStatusFile, szFile1);
  
  fclose(fp);
}

// Create a blank entry (not actually used)
void _init_Status(int num) {
	StatusMessageData[num].key = TEXT("");
	StatusMessageData[num].doStatus = true;
	StatusMessageData[num].doSound = false;
	StatusMessageData[num].sound = TEXT("");
	StatusMessageData[num].delay_ms = 2500;  // 2.5 s
}


int propGetScaleList(double *List, size_t Size){

  TCHAR Buffer[128];
  TCHAR Name[] = TEXT("ScaleList");
  TCHAR *pWClast, *pToken;
  int   Idx = 0;
  double vlast=0;
  double val;

  ASSERT(List != NULL);
  ASSERT(Size > 0);

  SetRegistryString(TEXT("ScaleList"),
   TEXT("0.5,1,2,5,10,20,50,100,150,200,500,1000"));

  if (GetRegistryString(Name, Buffer, sizeof(Buffer)/sizeof(TCHAR)) == 0){

    pToken = strtok_r(Buffer, TEXT(","), &pWClast);
    
    while(Idx < (int)Size && pToken != NULL){
      val = _tcstod(pToken, NULL);
      if (Idx>0) {
        List[Idx] = (val+vlast)/2;
        Idx++;
      }
      List[Idx] = val;
      Idx++;
      vlast = val;
      pToken = strtok_r(NULL, TEXT(","), &pWClast);
    }
    
    return(Idx);
    
  } else {
    return(0);
  }
  
}

long GetUTCOffset(void) {
  return UTCOffset;
}


#if (WINDOWSPC<1)
#define GdiFlush() do { } while (0)
#endif


#if 0 // REMOVE ANIMATION
static RECT AnimationRectangle = {0,0,0,0};

void SetSourceRectangle(RECT fromRect) {
  AnimationRectangle = fromRect;
}


RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed)
{
    return AnimationRectangle;
}
#endif 



void CreateDirectoryIfAbsent(TCHAR *filename) {
  TCHAR fullname[MAX_PATH];

  LocalPath(fullname, filename);

  DWORD fattr = GetFileAttributes(fullname);

  if ((fattr != 0xFFFFFFFF) &&
      (fattr & FILE_ATTRIBUTE_DIRECTORY)) {
    // directory exists
  } else {
    CreateDirectory(fullname, NULL);
  }

}

bool FileExists(TCHAR *FileName){

  HANDLE hFile = CreateFileW(FileName, GENERIC_READ, 0, NULL,
                 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if( hFile == INVALID_HANDLE_VALUE)
    return(FALSE);

  CloseHandle(hFile);

  return(TRUE);
  
  /*
  FILE *file = _tfopen(FileName, _T("r"));
  if (file != NULL) {
    fclose(file);
    return(TRUE);
  }
  return FALSE;
  */
}

bool RotateScreen() {
#if (WINDOWSPC>0)
  return false;
#else 
  //
  // Change the orientation of the screen
  //
#if 0
  DEVMODE DeviceMode;
    
  memset(&DeviceMode, 0, sizeof(DeviceMode));
  DeviceMode.dmSize=sizeof(DeviceMode);
  DeviceMode.dmFields = DM_DISPLAYORIENTATION;
  DeviceMode.dmDisplayOrientation = DMDO_90; 
  //Put your desired position right here.

  if (DISP_CHANGE_SUCCESSFUL == 
      ChangeDisplaySettingsEx(NULL, &DeviceMode, NULL, CDS_RESET, NULL))
    return true;
  else
    return false;
#else
  return false;
#endif
#endif

}


int GetTextWidth(HDC hDC, TCHAR *text) {
  SIZE tsize;
  GetTextExtentPoint(hDC, text, _tcslen(text), &tsize);
  return tsize.cx;
}


void ExtTextOutClip(HDC hDC, int x, int y, TCHAR *text, int width) {
  int len = _tcslen(text);
  if (len <=0 ) {
    return;
  }
  SIZE tsize;
  GetTextExtentPoint(hDC, text, len, &tsize);
  RECT rc;
  rc.left = x;
  rc.top = y;
  rc.right = x + min(width,tsize.cx);
  rc.bottom = y + tsize.cy;

  ExtTextOut(hDC, x, y, /* ETO_OPAQUE | */ ETO_CLIPPED, &rc,
             text, len, NULL);
}

void UpdateConfBB(void) {

  ConfBB[0]=true; // thermal always on automatically
  ConfBB[1]=ConfBB1;
  ConfBB[2]=ConfBB2;
  ConfBB[3]=ConfBB3;
  ConfBB[4]=ConfBB4;
  ConfBB[5]=ConfBB5;
  ConfBB[6]=ConfBB6;
  ConfBB[7]=ConfBB7;
  ConfBB[8]=ConfBB8;
  ConfBB[9]=ConfBB9;

  if (ConfBB2==false && ConfBB3==false &&
      ConfBB4==false && ConfBB5==false &&
      ConfBB6==false && ConfBB7==false &&
      ConfBB8==false && ConfBB9==false)

		// we need at least one bottom bar stripe available (thermal apart)
		ConfBB[1]=true;

}

void UpdateConfIP(void) {

  // MAP MODE always available
  ConfIP[0][0]=true; 
  ConfIP[0][1]=true; 
  ConfMP[0]=true; // map mode

  // LKMODE_INFOMODE is 1
  ConfIP[1][0]=ConfIP11;
  ConfIP[1][1]=ConfIP12;
  ConfIP[1][2]=ConfIP13;
  ConfIP[1][3]=ConfIP14;
  ConfIP[1][4]=ConfIP15;
  ConfIP[1][5]=ConfIP16;

  // WPMODE
  ConfIP[2][0]=ConfIP21;
  ConfIP[2][1]=ConfIP22;
  ConfIP[2][2]=ConfIP23;
  ConfIP[2][3]=ConfIP24;

  // COMMONS
  ConfIP[3][0]=ConfIP31;
  ConfIP[3][1]=ConfIP32;
  ConfIP[3][2]=ConfIP33;

  // TRAFFIC always on if available
  ConfIP[4][0]=true;
  ConfIP[4][1]=true;
  ConfIP[4][2]=true;
  ConfMP[4]=true; // traffic mode

  // Check if we have INFOMODE
  if (ConfIP[1][0]==false && ConfIP[1][1]==false 
	&& ConfIP[1][2]==false && ConfIP[1][3]==false 
	&& ConfIP[1][4]==false && ConfIP[1][5]==false) {
	ConfMP[1]=false;
  } else
	ConfMP[1]=true;

  // Check if we have NEAREST pages
  if (ConfIP[2][0]==false && ConfIP[2][1]==false 
	&& ConfIP[2][2]==false && ConfIP[2][3]==false ) {
	ConfMP[2]=false;
  } else
	ConfMP[2]=true;

  // Check if we have COMMONS
  if (ConfIP[3][0]==false && ConfIP[3][1]==false && ConfIP[3][2]==false ) {
	ConfMP[3]=false;
  } else
	ConfMP[3]=true;

  /*
  // Verify that we have at least one menu
  if (ConfMP[1]==false && ConfMP[2]==false && ConfMP[3]==false ) {
	ConfIP[1][0]=true;
	ConfMP[1]=true;
  }
  */
  SetInitialModeTypes();

}

void SetInitialModeTypes(void) {

  // Update the initial values for each mapspace, keeping the first valid value. We search backwards.
  // INFOMODE 1  
  if (ConfIP[LKMODE_INFOMODE][IM_TRI]) ModeType[LKMODE_INFOMODE]=IM_TRI;
  if (ConfIP[LKMODE_INFOMODE][IM_CONTEST]) ModeType[LKMODE_INFOMODE]=IM_CONTEST;
  if (ConfIP[LKMODE_INFOMODE][IM_AUX]) ModeType[LKMODE_INFOMODE]=IM_AUX;
  if (ConfIP[LKMODE_INFOMODE][IM_TASK]) ModeType[LKMODE_INFOMODE]=IM_TASK;
  if (ConfIP[LKMODE_INFOMODE][IM_THERMAL]) ModeType[LKMODE_INFOMODE]=IM_THERMAL;
  if (ConfIP[LKMODE_INFOMODE][IM_CRUISE]) ModeType[LKMODE_INFOMODE]=IM_CRUISE;

  // WP NEAREST MODE 2  
  if (ConfIP[LKMODE_WP][WP_NEARTPS]) ModeType[LKMODE_WP]=WP_NEARTPS;
  if (ConfIP[LKMODE_WP][WP_LANDABLE]) ModeType[LKMODE_WP]=WP_LANDABLE;
  if (ConfIP[LKMODE_WP][WP_AIRPORTS]) ModeType[LKMODE_WP]=WP_AIRPORTS;

  // COMMONS MODE 3
  if (ConfIP[LKMODE_NAV][NV_HISTORY]) ModeType[LKMODE_WP]=NV_HISTORY;
  if (ConfIP[LKMODE_NAV][NV_COMMONS]) ModeType[LKMODE_WP]=NV_COMMONS;


}


void RestartCommPorts() {

  StartupStore(TEXT(". RestartCommPorts%s"),NEWLINE);

  LockComm();

  devClose(devA());
  devClose(devB());

  NMEAParser::Reset();

  devInit(TEXT(""));

  UnlockComm();

}


void TriggerGPSUpdate()
{
  GpsUpdated = true;
  SetEvent(dataTriggerEvent);
}

// This is currently doing nothing.
void TriggerVarioUpdate()
{
}



bool Debounce(void) {
  static DWORD fpsTimeLast= 0;
  DWORD fpsTimeThis = ::GetTickCount();
  DWORD dT = fpsTimeThis-fpsTimeLast;

  if (dT>(unsigned int)debounceTimeout) {
    fpsTimeLast = fpsTimeThis;
    return true;
  } else {
    return false;
  }
}

//
// Let's get rid of BOOOOls soon!!!
bool BOOL2bool(BOOL a) {
  if (a==TRUE) return true;
  return false;
}

