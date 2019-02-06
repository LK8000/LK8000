/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "FlarmIdFile.h"
#include "utils/zzip_stream.h"

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
  LocalPath(filename,TEXT(LKD_CONF),_T(LKF_FLARMIDS));

  #if TESTBENCH
  StartupStore(TEXT("... OpenFLARMDetails: <%s>%s"),filename,NEWLINE);
  #endif

  zzip_stream stream(filename, "rt");
  if( !stream ) {
	#if TESTBENCH
	StartupStore(_T("... No flarm details local file found%s"),NEWLINE);
	#endif
	return;
  }

  TCHAR line[READLINE_LENGTH];
  while (stream.read_line(line)) {
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
    StartupStore(_T(". Local IDFLARM, found %d IDs%s"),NumberOfFLARMNames,NEWLINE);
  }
}


void SaveFLARMDetails(void)
{
  TCHAR filename[MAX_PATH];
  LocalPath(filename,TEXT(LKD_CONF),_T(LKF_FLARMIDS)); // 091103

  FILE * stream = _tfopen(filename,_T("wt"));
  if(stream) {
    for (int z = 0; z < NumberOfFLARMNames; z++) {
      _ftprintf(stream, TEXT("%lx=%s\n"), FLARM_Names[z].ID,FLARM_Names[z].Name);
    }
    fclose(stream);
    StartupStore(_T("... Saved %d FLARM names%s"),NumberOfFLARMNames,NEWLINE);
  } else {
    StartupStore(_T("-- Cannot save FLARM details, error --\n"));
  }
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

bool AddFlarmLookupItem(int id, TCHAR *name, bool saveFile) {
    bool bRet = false;
    int index = LookupSecondaryFLARMId(id);

#ifdef DEBUG_LKT
    StartupStore(_T("... LookupSecondary id=%d result index=%d\n"), id, index);
#endif
    if (index == -1) {
        if (NumberOfFLARMNames < MAXFLARMLOCALS) { // 100322
            // create new record
            index = NumberOfFLARMNames++;
        }
    }

    if(index != -1) {
        FLARM_Names[index].ID = id;
        LK_tcsncpy(FLARM_Names[index].Name, name, MAXFLARMNAME);
        bRet = true;
    }

    if (bRet && saveFile) {
        SaveFLARMDetails();
    }
    return bRet;
}
