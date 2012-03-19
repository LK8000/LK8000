/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Modeltype.h"


#ifdef PNA

//
//	Check if the model type is encoded in the executable file name
//
//  GlobalModelName is a global variable, shown during startup and used for printouts only.
//  In order to know what model you are using, GlobalModelType is used.
// 
//  This "smartname" facility is used to override the registry/config Model setup to force
//  a model type to be used, just in case. The model types may not follow strictly those in
//  config menu, nor be updated. Does'nt hurt though.
//
void SmartGlobalModelType() {

	GlobalModelType=MODELTYPE_PNA;	// default for ifdef PNA by now!

	if ( GetGlobalModelName() ) 
	{
		ConvToUpper(GlobalModelName);
	
		if ( !_tcscmp(GlobalModelName,_T("PNA"))) {
					GlobalModelType=MODELTYPE_PNA_PNA;
					_tcscpy(GlobalModelName,_T("GENERIC") );
		}
		else 
			if ( !_tcscmp(GlobalModelName,_T("HP31X")))	{
					GlobalModelType=MODELTYPE_PNA_HP31X;
			}
		else	
			if ( !_tcscmp(GlobalModelName,_T("PN6000"))) {
					GlobalModelType=MODELTYPE_PNA_PN6000;
			}
		else	
			if ( !_tcscmp(GlobalModelName,_T("MIO"))) {
					GlobalModelType=MODELTYPE_PNA_MIO;
			}
		else
			if ( !_tcscmp(GlobalModelName,_T("FUNTREK"))) {
					GlobalModelType=MODELTYPE_PNA_FUNTREK;
			}
		else
			if ( !_tcscmp(GlobalModelName,_T("ROYALTEK3200"))) {
					GlobalModelType=MODELTYPE_PNA_ROYALTEK3200;
			}
		else
			_tcscpy(GlobalModelName,_T("UNKNOWN") );
	} else	
		_tcscpy(GlobalModelName, _T("UNKNOWN") );
}


// Parse a MODELTYPE value and set the equivalent model name. 
// If the modeltype is invalid or not yet handled, assume that
// the user changed it in the registry or in the profile, and 
// correct the error returning false: this will force a Generic Type.

bool SetModelName(DWORD Temp) {
  switch (Temp) {
  case MODELTYPE_PNA_PNA:
    _tcscpy(GlobalModelName,_T("GENERIC")); 
    return true;
    break;
  case MODELTYPE_PNA_HP31X:
    _tcscpy(GlobalModelName,_T("HP31X"));
    return true;
    break;
  case MODELTYPE_PNA_PN6000:
    _tcscpy(GlobalModelName,_T("PN6000"));
    return true;
  case MODELTYPE_PNA_MIO:
    _tcscpy(GlobalModelName,_T("MIO"));
    return true;
  case  MODELTYPE_PNA_MEDION_P5:
    _tcscpy(GlobalModelName,_T("MEDION P5"));
    return true;
  case MODELTYPE_PNA_NOKIA_500:
    _tcscpy(GlobalModelName,_T("NOKIA500"));
    return true;
  case MODELTYPE_PNA_NAVIGON:
    _tcscpy(GlobalModelName,_T("NAVIGON"));
    return true;
  case MODELTYPE_PNA_FUNTREK:
    _tcscpy(GlobalModelName,_T("FUNTREK"));
    return true;
  case MODELTYPE_PNA_ROYALTEK3200:
    _tcscpy(GlobalModelName,_T("ROYALTEK3200"));
    return true;
  default:
    _tcscpy(GlobalModelName,_T("UNKNOWN"));
    return false;
  }

}


//
// A special case for preloading a modeltype directly from 
// Default profile, at the very beginning of runtime.
//
bool LoadModelFromProfile()
{

  TCHAR tmpTbuf[MAX_PATH*2];
  char  tmpbuf[MAX_PATH*2];

  LocalPath(tmpTbuf,_T(LKD_CONF));
  _tcscat(tmpTbuf,_T("\\"));
  _tcscat(tmpTbuf,_T(LKPROFILE));

  #if TESTBENCH
  StartupStore(_T("... Searching modeltype inside default profile <%s>%s"),tmpTbuf,NEWLINE);
  #endif

  FILE *fp=NULL;
  fp = _tfopen(tmpTbuf, _T("rb"));
  if(fp == NULL) {
	StartupStore(_T("... No default profile found%s"),NEWLINE);
	return false;
  }

  while (fgets(tmpbuf, (MAX_PATH*2)-1, fp) != NULL ) {

	if (strlen(tmpbuf)<21) continue;

	if (strncmp(tmpbuf,"AppInfoBoxModel",15) == 0) { // MUST MATCH!  szRegistryAppInfoBoxModel
		int val=atoi(&tmpbuf[16]);
		GlobalModelType=val;
		SetModelName(val);
		#if TESTBENCH
		StartupStore(_T("... ModelType found: <%s> val=%d%s"),GlobalModelName,GlobalModelType,NEWLINE);
		#endif
		fclose(fp);
		return true;
	}
  }
  #if TESTBENCH
  StartupStore(_T("... Modeltype not found in profile, probably Generic PNA is used.\n"));
  #endif
  fclose(fp);
  return false;
}

bool SetModelType() {

  DWORD Temp=0;

  Temp=Appearance.InfoBoxModel;
  
  if ( SetModelName(Temp) != true ) {
	GlobalModelType=MODELTYPE_PNA_PNA;
	_tcscpy(GlobalModelName,_T("GENERIC"));
	return false;
  } else {
	GlobalModelType = Temp;
  }
  
  StartupStore(_T(". SetModelType: Name=<%s> Type=%d%s"),GlobalModelName, GlobalModelType,NEWLINE);
  return true;
}



#endif // PNA



