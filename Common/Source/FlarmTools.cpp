/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "FlarmIdFile.h" 

FlarmIdFile *file=NULL; 

int NumberOfFLARMNames = 0;


typedef struct {
  long ID;
  TCHAR Name[MAXFLARMNAME+1];
} FLARM_Names_t;

FLARM_Names_t FLARM_Names[MAXFLARMLOCALS+1];

void CloseFLARMDetails() {
  int i;
  for (i=0; i<NumberOfFLARMNames; i++) {
    //    free(FLARM_Names[i]);
  }
  NumberOfFLARMNames = 0;
}


void OpenFLARMDetails() {

  static FlarmIdFile flarmidfile;
  file=&flarmidfile;
  LKASSERT(file!=NULL);

  StartupStore(_T(". FLARMNET database, found %d IDs%s"),FlarmNetCount,NEWLINE);

  if (NumberOfFLARMNames) {
    CloseFLARMDetails(); 
  }

  TCHAR filename[MAX_PATH];
  LocalPath(filename,TEXT(LKD_CONF));
  _tcscat(filename,_T("\\"));
  _tcscat(filename,_T(LKF_FLARMIDS));

  #if TESTBENCH  
  StartupStore(TEXT(". OpenFLARMDetails: <%s>%s"),filename,NEWLINE);
  #endif

  HANDLE hFile = CreateFile(filename,GENERIC_READ,0,NULL,
			    OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE) {
	#if TESTBENCH
	StartupStore(_T("... No flarm details local file found%s"),NEWLINE);
	#endif
	return;
  }

  TCHAR line[READLINE_LENGTH];
  while (ReadString(hFile,READLINE_LENGTH, line)) {
    long id;
    TCHAR Name[MAX_PATH];

    if (_stscanf(line, TEXT("%lx=%s"), &id, Name) == 2) {
      if (AddFlarmLookupItem(id, Name, false) == false)
	{
	  break; // cant add anymore items !
	}
    }
  }

  if (NumberOfFLARMNames>0) {
	_stprintf(filename,_T(". Local FLARM IDs found: %d%s"),NumberOfFLARMNames,NEWLINE);
	StartupStore(filename);
  }

  CloseHandle(hFile);

}


void SaveFLARMDetails(void)
{
  DWORD bytesWritten;
  TCHAR filename[MAX_PATH];
  LocalPath(filename,TEXT(LKD_CONF)); // 091103
  _tcscat(filename,_T("\\"));
  _tcscat(filename,_T(LKF_FLARMIDS));
  
  HANDLE hFile = CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if( hFile == INVALID_HANDLE_VALUE) {
	// StartupStore(_T("-- Cannot save FLARM details, error --\n"));
	return;
  }
  
  TCHAR wsline[READLINE_LENGTH];
  char cline[READLINE_LENGTH];
  
  for (int z = 0; z < NumberOfFLARMNames; z++)
    {   
      _stprintf(wsline, TEXT("%lx=%s\r\n"), FLARM_Names[z].ID,FLARM_Names[z].Name);
#ifdef UNICODE
      WideCharToMultiByte( CP_ACP, 0, wsline,
			   _tcslen(wsline)+1,
			   cline,
			   READLINE_LENGTH, NULL, NULL);
      
      WriteFile(hFile, cline, strlen(cline), &bytesWritten, NULL);
#else
      UNUSED(cline);
      WriteFile(hFile, wsline, strlen(wsline), &bytesWritten, NULL);
#endif      
    }
  _stprintf(filename,_T("... Saved %d FLARM names%s"),NumberOfFLARMNames,NEWLINE);
  StartupStore(filename);
  CloseHandle(hFile);
}


int LookupSecondaryFLARMId(int id)
{
  for (int i=0; i<NumberOfFLARMNames; i++) 
    {
      if (FLARM_Names[i].ID == id) 
	{
	  return i;
	}
    }
  return -1;
}

int LookupSecondaryFLARMId(TCHAR *cn)
{
  for (int i=0; i<NumberOfFLARMNames; i++) 
    {
      if (_tcscmp(FLARM_Names[i].Name, cn) == 0) 
	{
	  return i;
	}
    }
  return -1;
}

// returns Name or Cn to be used
TCHAR* LookupFLARMCn(long id) {
  
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(id);
  if (index != -1)
    {
      return FLARM_Names[index].Name;
    }
  
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file->GetFlarmIdItem(id);
  if (flarmId != NULL)
    {
      return flarmId->cn;
    }
  return NULL;
}

TCHAR* LookupFLARMDetails(long id) {
  
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(id);
  if (index != -1)
    {
      return FLARM_Names[index].Name;
    }
  
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file->GetFlarmIdItem(id);
  if (flarmId != NULL)
    {
      // return flarmId->cn;
      return flarmId->reg;
    }
  return NULL;
}

// Used by TeamCode, to select a CN and get back the Id
int LookupFLARMDetails(TCHAR *cn) 
{
  // try to find flarm from userFile
  int index = LookupSecondaryFLARMId(cn);
  if (index != -1)
    {
      return FLARM_Names[index].ID;
    }
  
  // try to find flarm from FLARMNet.org File
  FlarmId* flarmId = file->GetFlarmIdItem(cn);
  if (flarmId != NULL)
    {
      return flarmId->GetId();
    }
  return 0;
}


bool AddFlarmLookupItem(int id, TCHAR *name, bool saveFile)
{
  int index = LookupSecondaryFLARMId(id);

  #ifdef DEBUG_LKT
  StartupStore(_T("... LookupSecondary id=%d result index=%d\n"),id,index);
  #endif
  if (index == -1)
    {
      if (NumberOfFLARMNames < MAXFLARMLOCALS)  // 100322 
	{
	  // create new record
	  FLARM_Names[NumberOfFLARMNames].ID = id;
	  LK_tcsncpy(FLARM_Names[NumberOfFLARMNames].Name, name,MAXFLARMNAME); // was 20 100322
	  NumberOfFLARMNames++;
	  SaveFLARMDetails();
	  return true;
	}
    }
  else
    {
      // modify existing record
      FLARM_Names[index].ID = id;
      LK_tcsncpy(FLARM_Names[index].Name, name,MAXFLARMNAME);
      if (saveFile)
	{
	  SaveFLARMDetails();
	}
      return true;
    }
  return false;
}


