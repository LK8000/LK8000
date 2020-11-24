/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "externs.h"
#include "FlarmIdFile.h"
#include "DoInits.h"
#include "utils/stl_utils.h"


FlarmIdFile::FlarmIdFile(void)
{
  TCHAR flarmIdFileName[MAX_PATH] = TEXT("\0");

  LocalPath(flarmIdFileName, _T(LKD_CONF), _T(LKF_FLARMNET));

  FILE*	hFile = _tfopen(flarmIdFileName, TEXT("rt"));
  if (hFile == NULL) {
    LocalPath(flarmIdFileName, _T(LKD_CONF), _T("data.fln"));
	hFile = _tfopen(flarmIdFileName, TEXT("rt"));
	if (hFile == NULL) return;
  }

  fseek (hFile , 0 , SEEK_END);
  long int fileLength = ftell (hFile);
  fseek (hFile , 7 , SEEK_SET);

  int itemCount = 0;

  while( ( fileLength - ftell(hFile)) > 87) {
	FlarmId *flarmId = new FlarmId;

	_tcscpy(flarmId->id,_T(""));
	_tcscpy(flarmId->name,_T(""));
	_tcscpy(flarmId->airfield,_T(""));
	_tcscpy(flarmId->type,_T(""));
	_tcscpy(flarmId->reg,_T(""));
	_tcscpy(flarmId->cn,_T(""));
	_tcscpy(flarmId->freq,_T(""));

	GetItem(hFile, flarmId);
	flarmIds[flarmId->GetId()] = flarmId;
	itemCount++;
  }
  FlarmNetCount=itemCount;
  fclose(hFile);
}

FlarmIdFile::~FlarmIdFile(void)
{
  for(auto& item : flarmIds) {
    delete item.second;
  }
  flarmIds.clear();
}

void FlarmIdFile::GetItem(FILE* hFile, FlarmId *flarmId)
{
  GetAsString(hFile, FLARMID_SIZE_ID-1, flarmId->id);
  GetAsString(hFile, FLARMID_SIZE_NAME-1, flarmId->name);
  GetAsString(hFile, FLARMID_SIZE_AIRFIELD-1, flarmId->airfield);
  GetAsString(hFile, FLARMID_SIZE_TYPE-1, flarmId->type);
  GetAsString(hFile, FLARMID_SIZE_REG-1, flarmId->reg);
  GetAsString(hFile, MAXFLARMCN, flarmId->cn);
  GetAsString(hFile, FLARMID_SIZE_FREQ-1, flarmId->freq);
  //SetFilePointer(hFile, 1, NULL, FILE_CURRENT) ;

  int i = 0;
  while(i < FLARMID_SIZE_REG && flarmId->reg[i] != 0) {
      if (flarmId->reg[i] == _T(' ')) flarmId->reg[i] = 0;
      i++;
  }

  i = 0;
  while(i < MAXFLARMCN && flarmId->cn[i] != 0) {
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



void FlarmIdFile::GetAsString(FILE* hFile, int charCount, TCHAR *res)
{
  LKASSERT((charCount * 2)<=100);
  unsigned bytesToRead = std::min(charCount * 2, 100);

  char bytes[100];

  fread(bytes, 1, bytesToRead, (FILE*)hFile);

  TCHAR *curChar = res;
  for (unsigned z = 0; z < bytesToRead && (z+1) < std::size(bytes) ; z += 2)
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
      if(_tcscmp(itemTemp->cn, cn) == 0)
	{
	  return itemTemp;
	}
      ++iterFind;
    }

  return NULL;
}

long FlarmId::GetId()
{
  unsigned int res;

  _stscanf(id, TEXT("%6x"), &res);

  return res;
};
