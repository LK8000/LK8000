#ifndef FLARMIDFILE_H
#define FLARMIDFILE_H

#include <map>
#include <stdio.h>

#define FLARMID_SIZE_ID		7
#define FLARMID_SIZE_NAME	22
#define FLARMID_SIZE_AIRFIELD	22
#define FLARMID_SIZE_TYPE	22
#define FLARMID_SIZE_REG	8
#define FLARMID_SIZE_FREQ	8

class FlarmId
{
public:
  TCHAR id[FLARMID_SIZE_ID+1];
  TCHAR name[FLARMID_SIZE_NAME+1];
  TCHAR airfield[FLARMID_SIZE_AIRFIELD+1];
  TCHAR type[FLARMID_SIZE_TYPE+1];
  TCHAR reg[FLARMID_SIZE_REG+1];
  TCHAR cn[MAXFLARMCN+1];
  TCHAR freq[FLARMID_SIZE_FREQ+1];
  long GetId();
};

typedef FlarmId* FlarmIdptr;
typedef std::map< long, FlarmIdptr > FlarmIdMap;


class FlarmIdFile
{
private:
  FlarmIdMap flarmIds;
  void GetAsString(FILE* hFile, int charCount, TCHAR *res);
  void GetItem(FILE* hFile, FlarmId *flarmId);
public:
  FlarmIdFile(void);
  ~FlarmIdFile(void);
  FlarmId* GetFlarmIdItem(long id);
  FlarmId* GetFlarmIdItem(TCHAR *cn);
};

#endif
