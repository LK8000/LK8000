/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "externs.h"
#include "FlarmIdFile.h"
#include "DoInits.h"

//
// This HAS TO BE FIXED some day! We must not init this class with such action.
// We are getting here before even startup, this is the place where localpath is executed first
//

FlarmIdFile::FlarmIdFile(void)
{
  TCHAR path[MAX_PATH];

  LKSound(_T("LK_CONNECT.WAV"));

  TCHAR flarmIdFileName[MAX_PATH] = TEXT("\0");
  if (SIMMODE) return;

  LocalPath(path);

  wsprintf(flarmIdFileName, TEXT("%s\\%s\\%s"), path, TEXT(LKD_CONF), TEXT(LKF_FLARMNET));

  FILE*	hFile = _wfopen(flarmIdFileName, TEXT("rt"));
  if (hFile == NULL) {
	wsprintf(flarmIdFileName, TEXT("%s\\%s\\data.fln"), path, TEXT(LKD_CONF));
	hFile = _wfopen(flarmIdFileName, TEXT("rt"));
	if (hFile == NULL) return;
  }

  DWORD fileLength;
	
  fseek (hFile , 0 , SEEK_END);
  fileLength = ftell (hFile);
  fseek (hFile , 7 , SEEK_SET);

  int itemCount = 0;

  while( ( (signed)fileLength - ftell(hFile)) > 87) {
	FlarmId *flarmId = new FlarmId;
	GetItem(hFile, flarmId);
	flarmIds[flarmId->GetId()] = flarmId;
	itemCount++;
  }
  FlarmNetCount=itemCount;
  fclose(hFile);
}

FlarmIdFile::~FlarmIdFile(void)
{
}

void FlarmIdFile::GetItem(HANDLE hFile, FlarmId *flarmId)
{
  GetAsString(hFile, 6, flarmId->id);
  GetAsString(hFile, 21, flarmId->name);
  GetAsString(hFile, 21, flarmId->airfield);
  GetAsString(hFile, 21, flarmId->type);
  GetAsString(hFile, 7, flarmId->reg);
  GetAsString(hFile, MAXFLARMCN, flarmId->cn);
  GetAsString(hFile, 7, flarmId->freq);
  //SetFilePointer(hFile, 1, NULL, FILE_CURRENT) ;

  int i = 0;
  int maxSize = sizeof(flarmId->reg) / sizeof(TCHAR);
  while(flarmId->reg[i] != 0 && i < maxSize) {
      if (flarmId->reg[i] == _T(' ')) flarmId->reg[i] = 0;
      i++;
  }

  i = 0;
  maxSize = sizeof(flarmId->cn) / sizeof(TCHAR);
  while(flarmId->cn[i] != 0 && i < maxSize) {
      if (flarmId->cn[i] == _T(' ')) flarmId->cn[i] = 0;
      i++;
  }

  // Add a valid CN if missing. Ex: D-6543 = D43
  if (_tcslen(flarmId->cn) == 0 ) {
	int reglen=_tcslen(flarmId->reg);
	if (reglen >=3) {
		flarmId->cn[0] = flarmId->reg[0];
		flarmId->cn[1] = flarmId->reg[reglen-2];
		flarmId->cn[2] = flarmId->reg[reglen-1];
		flarmId->cn[3] = _T('\0');
	}
  }

  fseek((FILE*)hFile, 1, SEEK_CUR);
}



void FlarmIdFile::GetAsString(HANDLE hFile, int charCount, TCHAR *res)
{
  int bytesToRead = charCount * 2;
  char bytes[100];
  //DWORD bytesRead; 

  //ReadFile(hFile, bytes, bytesToRead, &bytesRead, NULL);
  fread(bytes, 1, bytesToRead, (FILE*)hFile);
    	
  TCHAR *curChar = res;
  for (int z = 0; z < bytesToRead; z += 2)
    {
      char tmp[3];
      tmp[0] = bytes[z];
      tmp[1] = bytes[z+1];
      tmp[2] = 0;

      int i;
      sscanf(tmp, "%2x", &i);

      *curChar = (unsigned char)i;
      curChar ++;
        
    }     
  *curChar = 0;
		
}
FlarmId* FlarmIdFile::GetFlarmIdItem(long id)
{	
  FlarmIdMap::iterator iterFind = flarmIds.find(id);
  if( iterFind != flarmIds.end() )
    {
      return flarmIds[id];		
    }
	
  return NULL;
}

FlarmId* FlarmIdFile::GetFlarmIdItem(TCHAR *cn)
{
  FlarmId *itemTemp = NULL;
  FlarmIdMap::iterator iterFind = flarmIds.begin();
  while( iterFind != flarmIds.end() )
    {
      itemTemp = (FlarmId*)(iterFind->second );
      if(wcscmp(itemTemp->cn, cn) == 0)
	{
	  return itemTemp;
	}	
      iterFind++;
    }
	
  return NULL;
}

long FlarmId::GetId() 
{ 
  long res;

  swscanf(id, TEXT("%6x"), &res);

  return res;
};


