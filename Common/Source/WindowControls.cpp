/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindowControls.cpp,v 8.10 2010/12/13 01:17:08 root Exp root $
*/

#include "StdAfx.h"
#include "tchar.h"
#include <stdio.h>
#include "Defines.h" // VENTA3
#include "options.h"
#include "WindowControls.h"
#ifndef ALTAIRSYNC
#include "Message.h"
#include "MapWindow.h"
#include "InfoBoxLayout.h"
#endif
#include "Utils.h"
#include "compatibility.h"
#include "LKObjects.h"

#ifndef ALTAIRSYNC
#include "externs.h" 

extern int DisplayTimeOut;
#ifndef GNAV
#if (WINDOWSPC<1)
#ifndef __MINGW32__
#include <projects.h>
#endif
#endif
#endif
#endif

#include "utils/heapcheck.h"

#ifdef ALTAIRSYNC
#define ISCALE 1
void SetSourceRectangle(RECT fromRect) {};
RECT WINAPI DrawWireRects(LPRECT lprcTo, UINT nMilliSecSpeed) {
  return *lprcTo;
}

#else
#define ISCALE InfoBoxLayout::scale
#endif

#define DEFAULTBORDERPENWIDTH 1*ISCALE
#define SELECTORWIDTH         4*ISCALE

#if defined(LKAIRSPACE) || defined(NEW_OLC)
using std::min;
using std::max;
#endif

#if FIXDC
HDC sHdc;
HDC  GetTempDeviceContext(void){return(sHdc);};
#endif

// utility functions

void DrawLine(const HDC&hdc, int x1, int y1, int x2, int y2) {
#ifndef NOLINETO
  MoveToEx(hdc, x1, y1, NULL);
  LineTo(hdc, x2, y2);
#else
  POINT p[2];
  p[0].x = x1;
  p[0].y = y1;
  p[1].x = x2;
  p[1].y = y2;
  Polyline(hdc, p, 2);
#endif

}


void DrawLine2(const HDC&hdc, int x1, int y1, int x2, int y2, int x3, int y3) {
#ifndef NOLINETO
  MoveToEx(hdc, x1, y1, NULL);
  LineTo(hdc, x2, y2);
  LineTo(hdc, x3, y3);
#else
  POINT p[3];
  p[0].x = x1;
  p[0].y = y1;
  p[1].x = x2;
  p[1].y = y2;
  p[2].x = x3;
  p[2].y = y3;
  Polyline(hdc, p, 3);
#endif
}

extern int dlgComboPicker(WndProperty* theProperty);
#ifdef GNAV
#define ENABLECOMBO false // master on/off for combo popup
#else
#define ENABLECOMBO true // master on/off for combo popup
// Must be off if no touchscreen
#endif

// returns true if it is a long press,
// otherwise returns false
static bool KeyTimer(bool isdown, DWORD thekey) {
  static DWORD fpsTimeDown= 0;
  static DWORD savedKey=0;

  unsigned int dT = ::GetTickCount()-fpsTimeDown;
  if ((dT>2000)&&(thekey==savedKey)) {
    fpsTimeDown = ::GetTickCount();
    savedKey = 0;
    return true;
  }

  if (!isdown) {
    // key is released
  } else {
    // key is lowered
    if (thekey != savedKey) {
      fpsTimeDown = ::GetTickCount();
      savedKey = thekey;
    }
  }
  return false;
}



BOOL IsDots(const TCHAR* str) {
  if(_tcscmp(str,TEXT(".")) && _tcscmp(str,TEXT(".."))) return FALSE;
  return TRUE;
}


int DataFieldFileReader::GetAsInteger(void){
  return mValue;
}


int DataFieldFileReader::SetAsInteger(int Value){
  Set(Value);
  return mValue;
}


void DataFieldFileReader::ScanDirectoryTop(const TCHAR* subdir, const TCHAR* filter) { // 091101
  
  TCHAR buffer[MAX_PATH] = TEXT("\0");
  LocalPath(buffer);
  if (_tcslen(subdir)>0) {
	_tcscat(buffer,_T("\\"));
	_tcscat(buffer,subdir);
  }
  ScanDirectories(buffer,filter);
  Sort();

}


BOOL DataFieldFileReader::ScanDirectories(const TCHAR* sPath, 
					  const TCHAR* filter) {

  HANDLE hFind;  // file handle
  WIN32_FIND_DATA FindFileData;

  TCHAR DirPath[MAX_PATH];
  TCHAR FileName[MAX_PATH];

  if (sPath) {
    _tcscpy(DirPath,sPath);
    _tcscpy(FileName,sPath);
  } else {
    DirPath[0]= 0;
    FileName[0]= 0;
  }

  ScanFiles(FileName, filter);

  _tcscat(DirPath,TEXT("\\"));
  _tcscat(FileName,TEXT("\\*"));

  hFind = FindFirstFile(FileName,&FindFileData); // find the first file
  if(hFind == INVALID_HANDLE_VALUE) {
    return FALSE;
  }
  _tcscpy(FileName,DirPath);

  if(!IsDots(FindFileData.cFileName)) {
    _tcscat(FileName,FindFileData.cFileName);

    if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      // we have found a directory, recurse
      //      if (!IsSystemDirectory(FileName)) {
	if(!ScanDirectories(FileName,filter)) { 
	  // none deeper
	}
	//      }
    }
  }
  _tcscpy(FileName,DirPath);

  bool bSearch = true;
  while(bSearch) { // until we finds an entry
    if(FindNextFile(hFind,&FindFileData)) {
      if(IsDots(FindFileData.cFileName)) continue;
      if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
	// we have found a directory, recurse
	_tcscat(FileName,FindFileData.cFileName);
	//	if (!IsSystemDirectory(FileName)) {
	  if(!ScanDirectories(FileName,filter)) { 
	    // none deeper
	  }
	  //	}
      }
      _tcscpy(FileName,DirPath);
    }
    else {
      if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
	bSearch = false;
      else {
	// some error occured, close the handle and return FALSE
	FindClose(hFind); 
	return FALSE;
      }
    }
  }
  FindClose(hFind);  // closing file handle
 
  return TRUE;
}


BOOL DataFieldFileReader::ScanFiles(const TCHAR* sPath, 
				    const TCHAR* filter) {
  HANDLE hFind;  // file handle
  WIN32_FIND_DATA FindFileData;

  TCHAR DirPath[MAX_PATH];
  TCHAR FileName[MAX_PATH];

  if (sPath) {
    _tcscpy(DirPath,sPath);
  } else {
    DirPath[0]= 0;
  }
  _tcscat(DirPath,TEXT("\\"));
  _tcscat(DirPath,filter);    
  if (sPath) {
    _tcscpy(FileName,sPath);
  } else {
    FileName[0]= 0;
  }
  _tcscat(FileName,TEXT("\\"));

  hFind = FindFirstFile(DirPath,&FindFileData); // find the first file
  if(hFind == INVALID_HANDLE_VALUE) return FALSE;
  _tcscpy(DirPath,FileName);


  // found first one
  if(!IsDots(FindFileData.cFileName)) {
    _tcscat(FileName,FindFileData.cFileName);
      
    if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      // do nothing
    }
    else {
      // DO SOMETHING WITH FileName
      if (checkFilter(FindFileData.cFileName, filter)) {
	addFile(FindFileData.cFileName, FileName);
      }
    }
    _tcscpy(FileName,DirPath);
  }

  bool bSearch = true;
  while(bSearch) { // until we finds an entry
    if(FindNextFile(hFind,&FindFileData)) {
      if(IsDots(FindFileData.cFileName)) continue;
      _tcscat(FileName,FindFileData.cFileName);

      if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
	// do nothing
      }
      else {
	// DO SOMETHING WITH FileName
	if (checkFilter(FindFileData.cFileName, filter)) {
	  addFile(FindFileData.cFileName, FileName);
	}
      }
      _tcscpy(FileName,DirPath);
    }
    else {
      if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
	bSearch = false;
      else {
	// some error occured, close the handle and return FALSE
	FindClose(hFind); 
	return FALSE;
      }

    }

  }
  FindClose(hFind);  // closing file handle
 
  return TRUE;
}

void DataFieldFileReader::Lookup(const TCHAR *Text) {
  int i=0;
  mValue = 0;
  for (i=1; i<(int)nFiles; i++) {    
    // if (_tcscmp(Text,fields[i].mTextPathFile)==0) { 091126
    if (_tcsicmp(Text,fields[i].mTextPathFile)==0) {
      mValue = i;
    }
  }
}

int DataFieldFileReader::GetNumFiles(void) {
  return nFiles;
}

TCHAR* DataFieldFileReader::GetPathFile(void) {
  if ((mValue<=nFiles)&&(mValue)) {
    return fields[mValue].mTextPathFile;
  }
  return (TCHAR*)TEXT("\0");
}


bool DataFieldFileReader::checkFilter(const TCHAR *filename,
				      const TCHAR *filter) {
  TCHAR *ptr;
  TCHAR upfilter[MAX_PATH];
  // checks if the filename matches the filter exactly

  if (!filter || (_tcslen(filter+1)==0)) {
    // invalid or short filter, pass
    return true;
  }

  _tcscpy(upfilter,filter+1);

  // check if trailing part of filter (*.exe => .exe) matches end
  ptr = _tcsstr((TCHAR*)filename, upfilter);
  if (ptr) {
    if (_tcslen(ptr)==_tcslen(upfilter)) {
      return true;
    }
  }

  _tcsupr(upfilter);
  ptr = _tcsstr((TCHAR*)filename, upfilter);
  if (ptr) {
    if (_tcslen(ptr)==_tcslen(upfilter)) {
      return true;
    }
  }

  return false;
}


void DataFieldFileReader::addFile(const TCHAR *Text, 
				  const TCHAR *PText) {
  // TODO enhancement: remove duplicates?
  if (nFiles<DFE_MAX_FILES) {
    fields[nFiles].mTextFile = (TCHAR*)malloc((_tcslen(Text)+1)*sizeof(TCHAR));
    _tcscpy(fields[nFiles].mTextFile, Text);

    fields[nFiles].mTextPathFile = (TCHAR*)malloc((_tcslen(PText)+1)*sizeof(TCHAR));
    _tcscpy(fields[nFiles].mTextPathFile, PText);

    nFiles++;
  }
}


TCHAR *DataFieldFileReader::GetAsString(void){
  if (mValue<nFiles) {
    return(fields[mValue].mTextFile);
  } else {
    return NULL;
  }
}


TCHAR *DataFieldFileReader::GetAsDisplayString(void){
  if (mValue<nFiles) {
    return(fields[mValue].mTextFile);
  } else {
    return NULL;
  }
}


void DataFieldFileReader::Set(int Value){
  if (Value<=(int)nFiles) {
    mValue = Value;
  }
  if (Value<0) {
    mValue = 0;
  }
}


void DataFieldFileReader::Inc(void){
  if (mValue<nFiles-1) {
    mValue++;
    // (mOnDataAccess)(this, daChange); 091126
    if (!GetDetachGUI()) (mOnDataAccess)(this, daChange);
  }
}


void DataFieldFileReader::Dec(void){
  if (mValue>0) {
    mValue--;
    // (mOnDataAccess)(this, daChange); 091126
    if (!GetDetachGUI()) (mOnDataAccess)(this, daChange);
  }
}


static int _cdecl DataFieldFileReaderCompare(const void *elem1, 
                                             const void *elem2 ){
  return _tcscmp(((DataFieldFileReaderEntry*)elem1)->mTextFile,
                 ((DataFieldFileReaderEntry*)elem2)->mTextFile);
}


void DataFieldFileReader::Sort(void){
  qsort(fields+1, nFiles-1, sizeof(DataFieldFileReaderEntry), 
        DataFieldFileReaderCompare);
}

int DataFieldFileReader::CreateComboList(void) {
  unsigned int i=0;
  for (i=0; i < nFiles; i++){
    mComboList.ComboPopupItemList[i] = mComboList.CreateItem(
                                          i, 
                                          i,
                                          fields[i].mTextFile,
                                          fields[i].mTextFile);
    if (i == mValue) {
      mComboList.ComboPopupItemSavedIndex=i;
    }
  }
  mComboList.ComboPopupItemCount=i;
  return mComboList.ComboPopupItemCount;
}


void DataField::Special(void){
  // (mOnDataAccess)(this, daSpecial); 091126
  if (!GetDetachGUI()) (mOnDataAccess)(this, daSpecial); 
}

void DataField::Inc(void){
  // (mOnDataAccess)(this, daInc); 091126
  if (!GetDetachGUI()) (mOnDataAccess)(this, daInc); 
}

void DataField::Dec(void){
  // (mOnDataAccess)(this, daDec); 091126
  if (!GetDetachGUI()) (mOnDataAccess)(this, daDec); 
}

void DataField::GetData(void){
  // (mOnDataAccess)(this, daGet); 091126
  if (!GetDetachGUI()) (mOnDataAccess)(this, daGet); 
}

void DataField::SetData(void){
  // (mOnDataAccess)(this, daPut); 091126
  if (!GetDetachGUI()) (mOnDataAccess)(this, daPut); 
}

  void __Dummy(DataField *Sender, DataField::DataAccessKind_t Mode){
    (void) Sender;
    (void) Mode;
  }

DataField::DataField(const TCHAR *EditFormat, const TCHAR *DisplayFormat, 
		     void(*OnDataAccess)(DataField *Sender, DataAccessKind_t Mode)){
  mUsageCounter=0;
  mOnDataAccess = OnDataAccess;
  _tcscpy(mEditFormat, EditFormat);
  _tcscpy(mDisplayFormat, DisplayFormat);
  SetDisableSpeedUp(false);
  SetDetachGUI(false); // disable dispaly of inc/dec/change values

  if (mOnDataAccess == NULL){
    mOnDataAccess = __Dummy;
  }

  // blank units
  mUnits[0]= 0;
}

void DataField::SetDisplayFormat(TCHAR *Value){
  _tcscpy(mDisplayFormat, Value);
}

void DataField::CopyString(TCHAR * szbuffOut, bool bFormatted) {
  int iLen=0;
  if (!bFormatted) 
  {
    if (GetAsString() != NULL) // null leaves iLen=0
    {
      iLen = _tcslen(GetAsString());
      _tcsncpy(szbuffOut, GetAsString(), min(iLen, ComboPopupITEMMAX-1));
    }
  }
  else 
  {
    if (GetAsDisplayString() != NULL) 
    {
      iLen = _tcslen(GetAsDisplayString());
      _tcsncpy(szbuffOut, GetAsDisplayString(), min(iLen, ComboPopupITEMMAX-1));
    }
  }
  szbuffOut[min(iLen, ComboPopupITEMMAX-1)] = '\0';
}



//----------------------------------------------------------
// DataField boolean
//----------------------------------------------------------
int DataFieldBoolean::CreateComboList(void) {
  int i=0;
  mComboList.ComboPopupItemList[i] = mComboList.CreateItem(i, 
                                                  i,
                                                  mTextFalse,
                                                  mTextFalse);

  i=1;
  mComboList.ComboPopupItemList[i] = mComboList.CreateItem(i, 
                                                  i,
                                                  mTextTrue,
                                                  mTextTrue);
  mComboList.ComboPopupItemCount=2;
  mComboList.ComboPopupItemSavedIndex=GetAsInteger();
  return mComboList.ComboPopupItemCount;
}

bool DataFieldBoolean::GetAsBoolean(void){
  return(mValue);
}

int DataFieldBoolean::GetAsInteger(void){
  if (mValue)
    return(1);
  else
    return(0);
}

double DataFieldBoolean::GetAsFloat(void){
  if (mValue)
    return(1.0);
  else
    return(0.0);
}

TCHAR *DataFieldBoolean::GetAsString(void){
  if (mValue)
    return(mTextTrue);
  else
    return(mTextFalse);
}


void DataFieldBoolean::Set(bool Value){
  mValue = Value;
}

bool DataFieldBoolean::SetAsBoolean(bool Value){
  bool res = mValue;
  if (mValue != Value){
    mValue = Value;
    if (!GetDetachGUI())(mOnDataAccess)(this,daChange); // fix rev 1.85
  }
  return(res);
}

int DataFieldBoolean::SetAsInteger(int Value){
  int res = GetAsInteger();
  if (GetAsInteger() != Value){
    SetAsBoolean(!(Value==0));
  }
  return(res);
}

double DataFieldBoolean::SetAsFloat(double Value){
  double res = GetAsFloat();
  if (GetAsFloat() != Value){
    SetAsBoolean(!(Value==0.0));
  }
  return(res);
}

TCHAR *DataFieldBoolean::SetAsString(const TCHAR *Value){
  TCHAR *res = GetAsString();
  if (_tcscmp(res, Value) != 0){
    SetAsBoolean(_tcscmp(Value, mTextTrue) == 0);
  }
  return(res);
}

void DataFieldBoolean::Inc(void){
  SetAsBoolean(!GetAsBoolean());
}

void DataFieldBoolean::Dec(void){
  SetAsBoolean(!GetAsBoolean());
}

//----------------------------------------------------------
// DataField enum
//----------------------------------------------------------

DataFieldEnum::~DataFieldEnum()
{
  for (unsigned int i=0; i<nEnums; i++) {
    if (mEntries[i].mText) {
      free(mEntries[i].mText);
      mEntries[i].mText= NULL;
    }
  }
  nEnums = 0;      
}

int DataFieldEnum::GetAsInteger(void){
  if ((mValue>=0) && (mValue<nEnums)) {
    return mEntries[mValue].index;
  } else {
    return 0; // JMW shouldn't get here
  }
}

void DataFieldEnum::addEnumText(const TCHAR *Text) {
  if (nEnums<DFE_MAX_ENUMS-1) {
    mEntries[nEnums].mText = (TCHAR*)malloc((_tcslen(Text)+1)*sizeof(TCHAR));
    _tcscpy(mEntries[nEnums].mText, Text);
    mEntries[nEnums].index = nEnums;
    nEnums++;
  }
}


TCHAR *DataFieldEnum::GetAsString(void){
  if ((mValue>=0) && (mValue<nEnums)) {
    return(mEntries[mValue].mText);
  } else {
    return NULL;
  }
}


void DataFieldEnum::Set(int Value){
  // first look it up
  if (Value<0) {
    Value = 0;
  }
  for (unsigned int i=0; i<nEnums; i++) {
    if (mEntries[i].index == (unsigned int) Value) {
      int lastValue = mValue;
      mValue = i;
      if (mValue != (unsigned int) lastValue){
        if (!GetDetachGUI())(mOnDataAccess)(this, daChange); 
      }
      return;
    }
  }
  mValue = 0; // fallback
}

int DataFieldEnum::SetAsInteger(int Value){
  Set(Value);
  return mEntries[mValue].index;  // this returns incorrect value RLD
  // JMW fixed (was Value, should be mValue)
}

void DataFieldEnum::Inc(void){
  if (mValue<nEnums-1) {
    mValue++;
    if (!GetDetachGUI())(mOnDataAccess)(this, daChange); // rev 1.85
  }
}

void DataFieldEnum::Dec(void){
  if (mValue>0) {
    mValue--;
    if (!GetDetachGUI())(mOnDataAccess)(this, daChange);
  }
}

static int _cdecl DataFieldEnumCompare(const void *elem1, 
                                             const void *elem2 ){
  return _tcscmp(((DataFieldEnumEntry*)elem1)->mText,
                 ((DataFieldEnumEntry*)elem2)->mText);
}

void DataFieldEnum::Sort(int startindex){
  qsort(mEntries+startindex, nEnums-startindex, sizeof(DataFieldEnumEntry), 
        DataFieldEnumCompare);
}
int DataFieldEnum::CreateComboList(void) {
  unsigned int i=0;
  for (i=0; i < nEnums; i++){
    mComboList.ComboPopupItemList[i] = mComboList.CreateItem(
                                          i, 
                                          mEntries[i].index,
                                          mEntries[i].mText,
                                          mEntries[i].mText);
//    if (mEntries[i].index == mValue) {
//      mComboList.ComboPopupItemSavedIndex=i;
//    }
  }
  mComboList.ComboPopupItemSavedIndex=mValue;
  mComboList.ComboPopupItemCount=i;
  return mComboList.ComboPopupItemCount;
}

//----------------------------------------------------------
// DataField Integer
//----------------------------------------------------------
int DataField::CreateComboListStepping(void) { // for DataFieldInteger and DataFieldFloat
// builds ComboPopupItemList[] by calling CreateItem for each item in list
// sets ComboPopupItemSavedIndex (global)
// returns ComboPopupItemCount
#define ComboListInitValue -99999
#define ComboFloatPrec 0.0000001 //rounds float errors to this precision

  double fNext=ComboListInitValue;
  double fCurrent=ComboListInitValue;
  double fLast=ComboListInitValue;
  TCHAR sTemp[ComboPopupITEMMAX];

  //int iCurrentDataFieldIndex=-1;
  mComboList.ComboPopupItemIndex =-1;

  int iListCount=0;
  //int iDataReaderIndex = -1;
  int iSelectedIndex = -1;
  //int i = 0, iLen=0;
  //bool bValid=true;
  int iStepDirection = 1; // for integer & float step may be negative
  double fBeforeDec=0.0, fAfterDec=0.0, fSavedValue=0.0;

  fNext=ComboListInitValue;
  fCurrent=ComboListInitValue;
  fLast=ComboListInitValue;

  SetDisableSpeedUp(true);
  SetDetachGUI(true); // disable display of inc/dec/change values

  // get step direction for int & float so we can detect if we skipped the value while iterating later
  CopyString(mComboList.PropertyValueSaved,false);
  CopyString(mComboList.PropertyValueSavedFormatted,true);

  fSavedValue=GetAsFloat();
  Inc();
  fBeforeDec = GetAsFloat();
  Dec();
  fAfterDec = GetAsFloat();

  if (fAfterDec < fBeforeDec) {
    iStepDirection = 1;
  } else {
    iStepDirection = -1;
  }

  // reset datafield to top of list (or for large floats, away from selected item so it will be in the middle)
  for ( iListCount = 0; iListCount < ComboPopupLISTMAX /2  ; iListCount++) // for floats, go half way down only 100222
  { 
    Dec();
    fNext=GetAsFloat();

    if (fNext == fCurrent) // we're at start of the list
      break;
    if (fNext == fLast)  // don't repeat Yes/No/etc  (is this needed w/out Bool?)
      break;      

    fLast = fCurrent;
    fCurrent = fNext;
  } // loop

  fNext=ComboListInitValue;
  fCurrent=ComboListInitValue;
  fLast=ComboListInitValue;

  fCurrent=GetAsFloat();
  mComboList.ComboPopupItemCount=0; 

  // if we stopped before hitting start of list create <<Less>> value at top of list
  if ( iListCount == ComboPopupLISTMAX /2 ) // 100222
  { // this data index item is checked on close of dialog
    mComboList.ComboPopupItemList[mComboList.ComboPopupItemCount] = mComboList.CreateItem(
                                                  mComboList.ComboPopupItemCount, 
                                                  (int)ComboPopupReopenLESSDataIndex,
                                                  TEXT("<<More Items>>"),
                                                  TEXT("<<More Items>>"));
    mComboList.ComboPopupItemCount += 1;
  }

  // now we're at the beginning of the list, so load forward until end
  for (iListCount = 0; iListCount < ComboPopupLISTMAX-3; iListCount++)
  { // stop at LISTMAX-3 b/c it may make an additional item if it's "off step", and 
    // potentially two more items for <<More>> and << Less>>

    // test if we've stepped over the selected value which was not a multiple of the "step"
    if (iSelectedIndex == -1) // not found yet
    {
      if ( ((double)iStepDirection) * GetAsFloat() > (fSavedValue + ComboFloatPrec * iStepDirection))
      { // step was too large, we skipped the selected value, so add it now
        mComboList.ComboPopupItemList[mComboList.ComboPopupItemCount] = mComboList.CreateItem(
                                                      mComboList.ComboPopupItemCount, 
                                                      ComboPopupNULL,
                                                      mComboList.PropertyValueSaved,
                                                      mComboList.PropertyValueSavedFormatted);
        iSelectedIndex = mComboList.ComboPopupItemCount;
        mComboList.ComboPopupItemCount += 1;
      }

    } // endif iSelectedIndex == -1

    if (iSelectedIndex == -1 && fabs(fCurrent-fSavedValue) < ComboFloatPrec) {// selected item index
      iSelectedIndex = mComboList.ComboPopupItemCount;
    }

    CopyString(sTemp,true); // can't call GetAsString & GetAsStringFormatted together (same output buffer)
    mComboList.ComboPopupItemList[mComboList.ComboPopupItemCount] = mComboList.CreateItem(
                                                  mComboList.ComboPopupItemCount, 
                                                  ComboPopupNULL,
                                                  GetAsString(),
                                                  sTemp);
    mComboList.ComboPopupItemCount += 1;

    Inc();
    fNext = GetAsFloat();
 
    if (fNext == fCurrent) {// we're at start of the list
      break;
    }

    if (fNext == fLast && mComboList.ComboPopupItemCount > 0) { //we're at the end of the range
      break;      
    }

    fLast = fCurrent;
    fCurrent = fNext;
  } 

  // if we stopped before hitting end of list create <<More>> value at end of list
  if ( iListCount == ComboPopupLISTMAX-3 )
  { // this data index item is checked on close of dialog
    mComboList.ComboPopupItemList[mComboList.ComboPopupItemCount] = mComboList.CreateItem(
                                                  mComboList.ComboPopupItemCount, 
                                                  (int)ComboPopupReopenMOREDataIndex,
                                                  TEXT("<<More Items>>"),
                                                  TEXT("<<More Items>>"));
    mComboList.ComboPopupItemCount += 1;
  }

  SetDisableSpeedUp(false);
  SetDetachGUI(false); // disable dispaly of inc/dec/change values

  if (iSelectedIndex >=0) {
    SetAsFloat(fSavedValue);
  }
  mComboList.ComboPopupItemSavedIndex = iSelectedIndex;
 
  return mComboList.ComboPopupItemCount;
}

bool DataFieldInteger::GetAsBoolean(void){
  return(mValue != 0);
}

int DataFieldInteger::GetAsInteger(void){
  return(mValue);
}

double DataFieldInteger::GetAsFloat(void){
  return(mValue);
}

TCHAR *DataFieldInteger::GetAsString(void){
  _stprintf(mOutBuf, mEditFormat, mValue);
  return(mOutBuf);
}

TCHAR *DataFieldInteger::GetAsDisplayString(void){
  _stprintf(mOutBuf, mDisplayFormat, mValue, mUnits);
  return(mOutBuf);
}


void DataFieldInteger::Set(int Value){
  mValue = Value;
}

bool DataFieldInteger::SetAsBoolean(bool Value){
  bool res = GetAsBoolean();
  if (Value)
    SetAsInteger(1);
  else
    SetAsInteger(0);
  return(res);
}

int DataFieldInteger::SetAsInteger(int Value){
  int res = mValue;
  if (Value < mMin)
    Value = mMin;
  if (Value > mMax)
    Value = mMax;
  if (mValue != Value){
    mValue = Value;
    if (!GetDetachGUI()) (mOnDataAccess)(this, daChange);
  }
  return(res);
}

double DataFieldInteger::SetAsFloat(double Value){
  double res = GetAsFloat();
  SetAsInteger(iround(Value));
  return(res);
}

TCHAR *DataFieldInteger::SetAsString(const TCHAR *Value){
  TCHAR *res = GetAsString();
  SetAsInteger(_ttoi(Value));
  return(res);
}

static bool DataFieldKeyUp = false;

void DataFieldInteger::Inc(void){
  SetAsInteger(mValue + mStep*SpeedUp(true));
}

void DataFieldInteger::Dec(void){
  SetAsInteger(mValue - mStep*SpeedUp(false));
}

int DataFieldInteger::SpeedUp(bool keyup){
  int res=1;  

#ifdef GNAV
  return res;
#endif

  if (GetDisableSpeedUp() == true) 
    return 1;

  if (keyup != DataFieldKeyUp) {
    mSpeedup = 0;
    DataFieldKeyUp = keyup;
    mTmLastStep = GetTickCount();
    return 1;
  }

  if ((unsigned long)(GetTickCount()-mTmLastStep) < 200){
    mSpeedup++;

    if (mSpeedup > 5){
      res = 10;

      mTmLastStep = GetTickCount()+350;
      return(res);

    }
  } else
    mSpeedup = 0;

  mTmLastStep = GetTickCount();

  return(res);
}
int DataFieldInteger::CreateComboList(void) {
  return CreateComboListStepping();
}

//----------------------------------------------------------
// DataField Float
//----------------------------------------------------------

bool DataFieldFloat::GetAsBoolean(void){
  return(mValue != 0.0);
}

int DataFieldFloat::GetAsInteger(void){
  return iround(mValue);
}

double DataFieldFloat::GetAsFloat(void){
  return(mValue);
}

TCHAR *DataFieldFloat::GetAsString(void){
  _stprintf(mOutBuf, mEditFormat, mValue);
  return(mOutBuf);
}

TCHAR *DataFieldFloat::GetAsDisplayString(void){
  _stprintf(mOutBuf, mDisplayFormat, mValue, mUnits);
  return(mOutBuf);
}

void DataFieldFloat::Set(double Value){
  mValue = Value;
}

double DataFieldFloat::SetMin(double Value){
  double res = mMin;
  mMin = Value;
  return(res);
};
double DataFieldFloat::SetMax(double Value){
  double res = mMax;
  mMax = Value;
  return(res);
};

bool DataFieldFloat::SetAsBoolean(bool Value){
  bool res = GetAsBoolean();
  if (res != Value){
    if (Value)
      SetAsFloat(1.0);
    else
      SetAsFloat(0.0);
  }
  return(res);
}

int DataFieldFloat::SetAsInteger(int Value){
  int res = GetAsInteger();
  SetAsFloat(Value);
  return(res);
}

double DataFieldFloat::SetAsFloat(double Value){
  double res = mValue;
  if (Value < mMin)
    Value = mMin;
  if (Value > mMax)
    Value = mMax;
  if (res != Value){
    mValue = Value;
    if (!GetDetachGUI()) (mOnDataAccess)(this, daChange);
  }
  return(res);
}

TCHAR *DataFieldFloat::SetAsString(const TCHAR *Value){
  TCHAR *res = GetAsString();
  SetAsFloat(_tcstod(Value, NULL));
  return(res);
}

void DataFieldFloat::Inc(void){
  // no keypad, allow user to scroll small values
  if(mFine && (mValue < 0.95) && (mStep>=0.5) && (mMin>=0.0))
    {
      SetAsFloat(mValue + 0.1);
    }
  else
    SetAsFloat(mValue + mStep*SpeedUp(true));
}

void DataFieldFloat::Dec(void){
  // no keypad, allow user to scroll small values
  if(mFine && (mValue <= 1.0) && (mStep>=0.5) && (mMin>=0.0))
    {
      SetAsFloat(mValue - 0.1);
    }
  else
    SetAsFloat(mValue - mStep*SpeedUp(false));
}

double DataFieldFloat::SpeedUp(bool keyup){
  double res=1.0;

#ifdef GNAV
  return res;
#endif

  if (GetDisableSpeedUp() == true) 
    return 1;

  if (keyup != DataFieldKeyUp) {
    mSpeedup = 0;
    DataFieldKeyUp = keyup;
    mTmLastStep = GetTickCount();
    return 1.0;
  }

  if ((unsigned long)(GetTickCount()-mTmLastStep) < 200){
    mSpeedup++;

    if (mSpeedup > 5){
      res = 10;

      mTmLastStep = GetTickCount()+350;
      return(res);

    }
  } else
    mSpeedup = 0;

  mTmLastStep = GetTickCount();

  return(res);
}

int DataFieldFloat::SetFromCombo(int iDataFieldIndex, TCHAR *sValue) {
  SetAsString(sValue);
  return 0;
}

int DataFieldFloat::CreateComboList(void) {
  return CreateComboListStepping();
}


//----------------------------------------------------------
// DataField String
//----------------------------------------------------------


TCHAR *DataFieldString::SetAsString(const TCHAR *Value){
  _tcscpy(mValue, Value);
  return(mValue);
}

void DataFieldString::Set(const TCHAR *Value){
  _tcscpy(mValue, Value);
}

TCHAR *DataFieldString::GetAsString(void){
  return(mValue);
}

TCHAR *DataFieldString::GetAsDisplayString(void){
  return(mValue);
}


//----------------------------------------------------------
// ComboList Class
//----------------------------------------------------------
ComboListEntry_t * ComboList::CreateItem(int ItemIndex, 
                                        int DataFieldIndex,
                                        TCHAR *StringValue,
                                        TCHAR *StringValueFormatted)
{
  int iLen = -1;
  ComboListEntry_t * theItem;

  // Copy current strings into structure
  theItem = (ComboListEntry_t*) malloc(sizeof(ComboListEntry_t));
  theItem->DataFieldIndex=0;  // NULL is same as 0, so it fails to set it if index value is 0
  theItem->ItemIndex=0;
  
  ASSERT(theItem!= NULL);

  theItem->ItemIndex=ItemIndex;

  if (DataFieldIndex != ComboPopupNULL) { // optional
    theItem->DataFieldIndex=DataFieldIndex;
  }

  if (StringValue == NULL)
  {
    theItem->StringValue = (TCHAR*)malloc((1) * sizeof(TCHAR));
    ASSERT(theItem->StringValue != NULL);
    theItem->StringValue[0]='\0';
  }
  else
  {
    iLen = _tcslen(StringValue);
    theItem->StringValue = (TCHAR*)malloc((iLen + 1) * sizeof(TCHAR));
    ASSERT(theItem->StringValue != NULL);
    _tcscpy(theItem->StringValue, StringValue);
  }


  // copy formatted display string
  if (StringValueFormatted == NULL) 
  {
    theItem->StringValueFormatted = (TCHAR*)malloc((1) * sizeof(TCHAR));
    ASSERT(theItem->StringValueFormatted != NULL);
    theItem->StringValueFormatted[0]='\0';
  }
  else
  {
    iLen = _tcslen(StringValueFormatted);
    theItem->StringValueFormatted = (TCHAR*)malloc((iLen + 1) * sizeof(TCHAR));
    ASSERT(theItem->StringValueFormatted != NULL);
    _tcscpy(theItem->StringValueFormatted, StringValueFormatted);
  }

  return theItem;
}
void ComboList::FreeComboPopupItemList(void)
{
  for (int i = 0; i < ComboPopupItemCount && i < ComboPopupLISTMAX; i++)
  {
    if (ComboPopupItemList[i] != NULL)
    {
      free (ComboPopupItemList[i]->StringValue);
      ComboPopupItemList[i]->StringValue=NULL;

      free (ComboPopupItemList[i]->StringValueFormatted);
      ComboPopupItemList[i]->StringValueFormatted=NULL;

      free (ComboPopupItemList[i]);
      ComboPopupItemList[i]=NULL;

    }
  }
}

//----------------------------------------------------------
// WindowControl Classes
//----------------------------------------------------------


extern HFONT  MapWindowFont;
extern HINSTANCE hInst;
WindowControl *ActiveControl = NULL;
WindowControl *LastFocusControl = NULL;


void InitWindowControlModule(void);
LRESULT CALLBACK WindowControlWndProc(HWND hwnd, UINT uMsg, 
                                      WPARAM wParam, LPARAM lParam);

static COLORREF bkColor = RGB_WINBACKGROUND; // PETROL
static COLORREF fgColor = RGB_WINFOREGROUND; // WHITE
int WindowControl::InstCount=0;
HBRUSH WindowControl::hBrushDefaultBk=NULL;
#if FIXGDI
HBRUSH WindowControl::mhBrushBk=NULL;
#endif
HPEN WindowControl::hPenDefaultBorder=NULL;
HPEN WindowControl::hPenDefaultSelector=NULL;

WindowControl::WindowControl(WindowControl *Owner, 
			     HWND Parent, 
			     const TCHAR *Name, 
			     int X, int Y, 
			     int Width, int Height, 
			     bool Visible){

  mHelpText = NULL;

  mHasFocus = false;
  mCanFocus = false;

  mReadOnly = false;

  mClientCount = 0;

  mOnHelpCallback = NULL;

  // todo

  DWORD Style = 0;

  mX = X;
  mY = Y;
  mWidth = Width;
  mHeight = Height;
  mParent = Parent;
  mOwner = Owner;
  // setup Master Window (the owner of all)
  mTopOwner = Owner;
  while (Owner != NULL && mTopOwner->GetOwner() != NULL)
    mTopOwner = mTopOwner->GetOwner();
    
  // todo
  mhFont = MapWindowFont;
  mVisible = Visible;
  mCaption[0] = '\0';
  mDontPaintSelector = false;

  if ((mParent == NULL) && (mOwner != NULL))
    mParent = mOwner->GetClientAreaHandle();

  if (Name != NULL)
    _tcscpy(mName, Name);  // todo size check
  else
    mName[0] = '\0';

  InitWindowControlModule();

  mColorBack = bkColor; // PETROL
  mColorFore = fgColor; // WHITE

  if (InstCount == 0){
	#if FIXDC
	hBrushDefaultBk = LKBrush_Petrol;
	hPenDefaultBorder = LKPen_White_N1;
	hPenDefaultSelector = LKPen_Petrol_C2;
	#else
	hBrushDefaultBk = (HBRUSH)CreateSolidBrush(mColorBack);
	hPenDefaultBorder = (HPEN)CreatePen(PS_SOLID, DEFAULTBORDERPENWIDTH, mColorFore); // NIBLSCCALE 1 White
	hPenDefaultSelector = (HPEN)CreatePen(PS_SOLID, DEFAULTBORDERPENWIDTH+2, RGB_LISTHIGHLIGHTCORNER); // NIBS(1)+2, PETROL
	#endif
  }
  InstCount++;

  Style = WS_CHILD | ES_MULTILINE | ES_CENTER
    | ES_READONLY | WS_CLIPCHILDREN
    | WS_CLIPSIBLINGS;

  if (mParent == NULL)
    Style |= WS_POPUP;

  mHWnd = CreateWindow(TEXT("STATIC"), TEXT("\0"),
		     Style,
		     mX, mY,
		     mWidth, mHeight,
		     mParent, NULL, hInst, NULL);

  SetWindowPos(mHWnd, HWND_TOP,
		     mX, mY,
		     mWidth, mHeight,
	       SWP_HIDEWINDOW);

  if (mOwner != NULL)
    mOwner->AddClient(this);

  mBoundRect.top = 0;
  mBoundRect.left = 0;
  mBoundRect.right = GetWidth();
  mBoundRect.bottom = GetHeight();

  mSavWndProcedure = GetWindowLong(mHWnd, GWL_WNDPROC);
  SetWindowLong(mHWnd, GWL_USERDATA, (long)this);
  SetWindowLong(mHWnd, GWL_WNDPROC, (LONG) WindowControlWndProc);

  mHdc = GetDC(mHWnd);
  #if FIXDC
  // mHdcTemp = CreateCompatibleDC(mHdc);  // 101205 made public, initialized by drawscrollbar
  #else
  mHdcTemp = CreateCompatibleDC(mHdc); 
  #endif

  #if FIXGDI
  if (mhBrushBk != hBrushDefaultBk) { //@ 101117
	DeleteObject(mhBrushBk);
  }
  #endif
  mhBrushBk = hBrushDefaultBk;
  mhPenBorder = hPenDefaultBorder;
  mhPenSelector = hPenDefaultSelector;
  mBorderSize = 1;

  mBorderKind = 0; //BORDERRIGHT | BORDERBOTTOM;

  SetBkMode(mHdc, TRANSPARENT);

  if (mVisible)
    ShowWindow(GetHandle(), SW_SHOW);

}

WindowControl::~WindowControl(void){
  if (mHelpText) {
    free(mHelpText);
    mHelpText = NULL;
  }
}

void WindowControl::Destroy(void){
  int i;
  for (i=mClientCount-1; i>=0; i--){
    mClients[i]->Destroy();
    delete mClients[i];
  }

  if (LastFocusControl == this)
    LastFocusControl = NULL;

  if (ActiveControl == this)
    ActiveControl = NULL;

  if (mhBrushBk != hBrushDefaultBk){
    DeleteObject(mhBrushBk);
  }
  if (mhPenBorder != hPenDefaultBorder){
    DeleteObject(mhPenBorder);
  }
  if (mhPenSelector != hPenDefaultSelector){
    DeleteObject(mhPenSelector);
  }

  ReleaseDC(mHWnd, mHdc);
  #if FIXDC
  if (sHdc!=NULL) {
	DeleteDC(sHdc);
	sHdc=NULL;
  }
  #else
  DeleteDC(mHdcTemp); 
  #endif
  SetWindowLong(mHWnd, GWL_WNDPROC, (LONG) mSavWndProcedure);
  SetWindowLong(mHWnd, GWL_USERDATA, (long)0);

  // SetWindowLong(mHWnd, GWL_WNDPROC, (LONG) WindowControlWndProc);
  // ShowWindow(GetHandle(), SW_SHOW);
  DestroyWindow(mHWnd);

  InstCount--;
  if (InstCount==0){
	#if FIXDC		// 101205
	#else
	DeleteObject(hBrushDefaultBk);
	DeleteObject(hPenDefaultBorder);
	DeleteObject(hPenDefaultSelector);
	#endif
  }

}

void WindowControl::UpdatePosSize(void){

  mBoundRect.top = 0;
  mBoundRect.left = 0;
  mBoundRect.right = GetWidth();
  mBoundRect.bottom = GetHeight();

  SetWindowPos(GetHandle(),0,
     mX, mY,
     mWidth, mHeight,
     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void WindowControl::SetTop(int Value){
  if (mY != Value){
    mY = Value;
    UpdatePosSize();
  }
}

void WindowControl::SetLeft(int Value){
  if (mX != Value){
    mX = Value;
    UpdatePosSize();
  }
}

void WindowControl::SetHeight(int Value){
  if (mHeight != Value){
    mHeight = Value;
    UpdatePosSize();
  }
}

void WindowControl::SetWidth(int Value){
  if (mWidth != Value){
    mWidth = Value;
    UpdatePosSize();
  }
}

WindowControl *WindowControl::GetCanFocus(void){
  if (mVisible && mCanFocus && !mReadOnly)
    return(this);

  if (!mVisible)
    return(NULL);

  for (int idx=0; idx<mClientCount; idx++){
    WindowControl *w;
    if ((w = mClients[idx]->GetCanFocus()) != NULL){
      return(w);
    }
  }
  return(NULL);
};

void WindowControl::AddClient(WindowControl *Client){
  mClients[mClientCount] = Client;
  mClientCount++;

  Client->SetOwner(this);
  // dont work propertly
//  Client->SetParentHandle(GetHandle());
  Client->SetFont(GetFont());

  // TODO unify these checks once consolidated LKWINCONTROL
  // use negative value to space down items
  // -999 to stay on the same line
  // -998 to advance one line with more space
  // -997 to advance one line with twice more space
  
  #if 100927
  if (Client->mY <0){
  	#if !101112
	if (Client->mY == -888) Client->mY=ScreenSizeY;
	#endif
	if (mClientCount > 1){
		if (Client->mY==-999) //@ 101008 
			Client->mY=mClients[mClientCount-2]->mY;
		else 
			if (Client->mY==-998) //@ 101115
				Client->mY = mClients[mClientCount-2]->mY + mClients[mClientCount-2]->mHeight +NIBLSCALE(3);
			else 
				if (Client->mY==-997) //@ 101115
					Client->mY = mClients[mClientCount-2]->mY + mClients[mClientCount-2]->mHeight +NIBLSCALE(6);
				else
					Client->mY = mClients[mClientCount-2]->mY - ((mClients[mClientCount-2]->mHeight)*Client->mY);
  #else
  if (Client->mY == -1){
	if (mClientCount > 1){
		Client->mY = mClients[mClientCount-2]->mY + mClients[mClientCount-2]->mHeight;
  #endif
		SetWindowPos(Client->GetHandle(), 0,
			Client->mX, Client->mY,
			0, 0,
			SWP_NOSIZE | SWP_NOZORDER 
			| SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	}
  }

  #if !101112
  if (Client->mX == -888) Client->mX=ScreenSizeX;
  if (Client->mWidth == -888) Client->mWidth=ScreenSizeX;
  if (Client->mHeight == -888) Client->mHeight=ScreenSizeY;
  #endif
  // Rescale mWidth
  if (Client->mWidth<-1) {
	int i=RescaleWidth(Client->mWidth);
	Client->mWidth=i;
	SetWindowPos(Client->GetHandle(), 0,
		Client->mX, Client->mY,
		Client->mWidth, Client->mHeight,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
  }
	
}

void WindowControl::FilterAdvanced(bool advanced){
  if (_tcsstr(mCaption, TEXT("*")) != NULL) {
    if (advanced) {
      SetVisible(true);
    } else {
      SetVisible(false);
    }
  } 
  for (int i=0; i<mClientCount; i++){
    mClients[i]->FilterAdvanced(advanced);
  }
}

WindowControl *WindowControl::FindByName(const TCHAR *Name){
  if (_tcscmp(mName, Name)==0)
    return(this);
  for (int i=0; i<mClientCount; i++){
    WindowControl *W = mClients[i]->FindByName(Name);
    if (W != NULL)
      return(W);
  }
  return(NULL);
}


void WindowControl::SetParentHandle(HWND hwnd){
  mParent = hwnd;
  SetParent(GetHandle(), hwnd);
}


WindowControl *WindowControl::SetOwner(WindowControl *Value){
  WindowControl *res = mOwner;
  if (mOwner != Value){
    mOwner = Value;
  }
  return(res);
}

void WindowControl::SetHelpText(const TCHAR *Value) {  
  if (mHelpText) {
	free(mHelpText);
	mHelpText = NULL;
  }
  if (Value == NULL) return;
  int len = _tcslen(Value);
  if (len==0) return;

  mHelpText= (TCHAR*)malloc((len+1)*sizeof(TCHAR));
  if (mHelpText != NULL) {
	_tcscpy(mHelpText, Value);
  }
}


void WindowControl::SetCaption(const TCHAR *Value){

  if (Value == NULL && mCaption[0] != '\0') {
	mCaption[0] ='\0';
	InvalidateRect(GetHandle(), GetBoundRect(), false);
	UpdateWindow(GetHandle());
	return;
  }

  if (_tcscmp(mCaption, Value) != 0) {
	_tcscpy(mCaption, Value);
	InvalidateRect(GetHandle(), GetBoundRect(), false);
	UpdateWindow(GetHandle());
  }

}

bool WindowControl::SetFocused(bool Value, HWND FromTo){
  (void)FromTo;
  bool res = mHasFocus;

  if (mHasFocus != Value){
    mHasFocus = Value;

    if (mCanFocus){
      RECT rc;
      rc.left = 0;
      rc.top = 0;
      rc.right = GetWidth();
      rc.bottom = GetHeight();
      InvalidateRect(GetHandle(), &rc, false);
      // todo, only paint the selector edges
#ifndef WINE
      UpdateWindow(GetHandle());
#endif /* WINE */
      // Paint(GetDeviceContext());
    }

  }

  if (Value){
    if (mCanFocus){
      ActiveControl = this;
      LastFocusControl = this;
    }
  } else {
    ActiveControl = NULL;
    /*
    if (FromTo == NULL){
      SetFocus(GetParent());
    }
    */
  }

  return(res);

}

bool WindowControl::SetCanFocus(bool Value){
  bool res = mCanFocus;
  mCanFocus = Value;
  return(res);
}

bool WindowControl::GetFocused(void){
  return(mHasFocus);
}

bool WindowControl::SetVisible(bool Value){
  bool res = mVisible;
  if (mVisible != Value){

    mVisible = Value;

    /*
    for (int i=0; i<mClientCount; i++){
      mClients[i]->SetVisible(mVisible);
    }
    */

    if (mVisible){
      InvalidateRect(GetHandle(), GetBoundRect(), false);
      UpdateWindow(GetHandle());
      ShowWindow(GetHandle(), SW_SHOW);
    } else {
      ShowWindow(GetHandle(), SW_HIDE);
    }

  }
  return(res);
}

bool WindowControl::GetVisible(void){
  return(mVisible);
}

int WindowControl::GetBorderKind(void){
  return(mBorderKind);
}

int WindowControl::SetBorderKind(int Value){
  int res = mBorderKind;
  if (mBorderKind != Value){
    mBorderKind = Value;
    InvalidateRect(GetHandle(), GetBoundRect(), false);
    UpdateWindow(GetHandle());
  }
  return(res);
}

HFONT WindowControl::SetFont(HFONT Value){
  HFONT res = mhFont;
  if (mhFont != Value){
    // todo
    mhFont = Value;
  }
  return(res);
}

bool WindowControl::SetReadOnly(bool Value){
  bool res = mReadOnly;
  if (mReadOnly != Value){
    mReadOnly = Value;
    Paint(GetDeviceContext());
  }
  return(res);
}

COLORREF WindowControl::SetForeColor(COLORREF Value){
  COLORREF res = mColorFore;
  if (mColorFore != Value){
    mColorFore = Value;
    if (mVisible)
      Paint(GetDeviceContext());
  }
  return(res);
}

COLORREF WindowControl::SetBackColor(COLORREF Value){
  COLORREF res = mColorBack;
  #if FIXGDI
  if (mColorBack != Value){
	mColorBack = Value;
	if (mhBrushBk != hBrushDefaultBk){
		// JMW possible memory leak if this brush is being used!
		DeleteObject(mhBrushBk);
	}
	mhBrushBk = (HBRUSH)CreateSolidBrush(mColorBack);
	if (mVisible)
		Paint(GetDeviceContext());
  }
  #else
  if (mColorBack != Value){
	mColorBack = Value;
	if (mhBrushBk != hBrushDefaultBk){
		// WE ARE NEVER GETTING HERE! 101117
		// JMW possible memory leak if this brush is being used! 
		DeleteObject(mhBrushBk);
	}
	mhBrushBk = (HBRUSH)CreateSolidBrush(mColorBack);
	if (mVisible)
		Paint(GetDeviceContext());
  }
  #endif
  return(res);
}


void WindowControl::PaintSelector(HDC hDC){

  if (!mDontPaintSelector && mCanFocus && mHasFocus){
    HPEN oldPen = (HPEN)SelectObject(hDC, hPenDefaultSelector);

    DrawLine2(hDC, 
	      mWidth-SELECTORWIDTH-1, 0,
	      mWidth-1, 0,
	      mWidth-1, SELECTORWIDTH+1);

    DrawLine2(hDC, 
	      mWidth-1, mHeight-SELECTORWIDTH-2,
	      mWidth-1, mHeight-1,
	      mWidth-SELECTORWIDTH-1, mHeight-1);

    DrawLine2(hDC, 
	      SELECTORWIDTH+1, mHeight-1, 
	      0, mHeight-1,
	      0, mHeight-SELECTORWIDTH-2);

    DrawLine2(hDC, 
	      0, SELECTORWIDTH+1,
	      0, 0,
	      SELECTORWIDTH+1, 0);

    SelectObject(hDC,oldPen);
  }

}

void WindowControl::Redraw(void){
  if (GetVisible()){
    InvalidateRect(GetHandle(), GetBoundRect(), false);
    UpdateWindow(GetHandle());
  }
}


#ifdef ALTAIRSYNC
#else
extern void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText);
#endif


int WindowControl::OnHelp() {
#ifdef ALTAIRSYNC
    return(0); // undefined. return 1 if defined
#else
    if (mHelpText) {
      dlgHelpShowModal(mCaption, mHelpText);
      return(1);
    } else {
      if (mOnHelpCallback) {
	(mOnHelpCallback)(this);
	return(1);
      } else {
	return(0);
      }
    }
#endif
};

void WindowControl::Paint(HDC hDC){

  RECT rc;

  rc.left = 0;
  rc.top = 0;
  rc.right = 0 + mWidth+2;
  rc.bottom = 0 + mHeight+2;

  if (!mVisible) return;

  FillRect(hDC, &rc, mhBrushBk);

  // JMW added highlighting, useful for lists
  if (!mDontPaintSelector && mCanFocus && mHasFocus){
    #if FIXDC
    HBRUSH hB = LKBrush_DarkYellow2;
    //HBRUSH hB = LKBrush_LcdDarkGreen;
    #else
    COLORREF ff = RGB_LISTHIGHLIGHTBG;
    HBRUSH hB = (HBRUSH)CreateSolidBrush(ff);
    #endif
    rc.left += 0;
    rc.right -= 2;
    rc.top += 0;
    rc.bottom -= 2;
    FillRect(hDC, &rc, hB);

      #if FIXDC
      #else
      DeleteObject(hB);
      #endif
  }

  if (mBorderKind != 0){

    HPEN oldPen = (HPEN)SelectObject(hDC, mhPenBorder);

    if (mBorderKind & BORDERTOP){
      DrawLine(hDC,0,0, mWidth, 0);
    }
    if (mBorderKind & BORDERRIGHT){
      DrawLine(hDC, mWidth-1, 0,
	       mWidth-1, mHeight);
    }
    if (mBorderKind & BORDERBOTTOM){
      DrawLine(hDC, 
	       mWidth-1, mHeight-1, 
	       -1, mHeight-1);
    }
    if (mBorderKind & BORDERLEFT){
      DrawLine(hDC, 
	       0, mHeight-1,
	       0, -1);
    }
    SelectObject(hDC,oldPen);
  }

  PaintSelector(hDC);

}

WindowControl *WindowControl::FocusNext(WindowControl *Sender){
  int idx;
  WindowControl *W;

  if (Sender != NULL){
    for (idx=0; idx<mClientCount; idx++)
      if (mClients[idx] == Sender) break;

    idx++;
  } else idx = 0;

  for (; idx<mClientCount; idx++){
    if ((W = mClients[idx]->GetCanFocus()) != NULL){
      SetFocus(W->GetHandle());
      return(W);
    }
  }

  if (GetOwner() != NULL){
    return(GetOwner()->FocusNext(this));
  }

  return(NULL);

}

WindowControl *WindowControl::FocusPrev(WindowControl *Sender){
  int idx;
  WindowControl *W;

  if (Sender != NULL){
    for (idx=0; idx<mClientCount; idx++)
      if (mClients[idx] == Sender) break;

    idx--;
  } else idx = mClientCount-1;

  for (; idx>=0; idx--)
    if ((W=mClients[idx]->GetCanFocus()) != NULL){
      SetFocus(W->GetHandle());
      return(W);
    }

  if (GetOwner() != NULL){
    return(GetOwner()->FocusPrev(this));
  }

  return(NULL);
}

LRESULT CALLBACK WindowControlWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	WindowControl *w = (WindowControl *) GetWindowLong(hwnd, GWL_USERDATA);

	if (w)
		return (w->WndProc(hwnd, uMsg, wParam, lParam));
	else
		return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}



int WindowControl::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

  PAINTSTRUCT ps;            // structure for paint info
  HDC hDC;                   // handle to graphics device context,

  switch (uMsg){


    case WM_ERASEBKGND:
      // we don't need one, we just paint over the top
    return TRUE;

    case WM_PAINT:
      hDC = BeginPaint(GetHandle(), &ps);
      Paint(hDC);

      DeleteDC(hDC);
      EndPaint(GetHandle(), &ps);
    return(0);

    case WM_WINDOWPOSCHANGED:
      //ib = (WindowControl *)GetWindowLong(hwnd, GWL_USERDATA);
      //ib->Paint(ib->GetDeviceContext());
    return 0;

    case WM_CREATE:
    break;

    case WM_DESTROY:
    break;

    case WM_COMMAND:
      if (OnCommand(wParam, lParam)) return(0);
    break;

    case WM_LBUTTONDBLCLK:
      InterfaceTimeoutReset();
      if (!OnLButtonDoubleClick(wParam, lParam)) {
        DisplayTimeOut = 0;
        return(0);
      }
    break;

    case WM_LBUTTONDOWN:
      InterfaceTimeoutReset();
      if (!OnLButtonDown(wParam, lParam)) {
        DisplayTimeOut = 0;
        return(0);
      }
      // TODO enhancement: need to be able to focus list items here...
    break;

    case WM_LBUTTONUP:
      InterfaceTimeoutReset();
      if (!OnLButtonUp(wParam, lParam)) {
        DisplayTimeOut = 0;
        return(0);
      }
    break;

    case WM_KEYDOWN:
      InterfaceTimeoutReset();
      // JMW: HELP
      KeyTimer(true, wParam & 0xffff);

      // return(OnKeyDown(wParam, lParam));
      // experimental 20060516:sgi
      if (!OnKeyDown(wParam, lParam)) {
        DisplayTimeOut = 0;
        return(0);
      }
      break;

    case WM_KEYUP:
      DisplayTimeOut = 0;
      InterfaceTimeoutReset();
      // JMW: detect long enter release
      // VENTA4: PNAs don't have Enter, so it should be better to find an alternate solution
	if (KeyTimer(false, wParam & 0xffff)) {
	  // activate tool tips if hit return for long time
	  if ((wParam & 0xffff) == VK_RETURN) {
	    if (OnHelp()) return (0);
	  }
	} 
      // return(OnKeyUp(wParam, lParam));
      // experimental 20060516:sgi
        if (!OnKeyUp(wParam, lParam)) {
          DisplayTimeOut = 0;
          return(0);
        }
      break;

    case WM_MOUSEMOVE:
      OnMouseMove(wParam, lParam);
      return (0);
      break;

    case WM_SETFOCUS:
      SetFocused(true, (HWND) wParam);
    return(0);

    case WM_KILLFOCUS:
      SetFocused(false, (HWND) wParam);
    return(0);

    case WM_ACTIVATE:
      /*
      if (wParam == WA_ACTIVE){
        if (LastFocusControl != NULL)
          SetFocus(LastFocusControl->GetHandle());
      }
      return(0);
      */
    break;

    case WM_QUIT:
    case WM_CLOSE:
      Close();
    return(0);

  }

  if (mTopOwner != NULL){
    if (!mTopOwner->OnUnhandledMessage(hwnd, uMsg, wParam, lParam))
     return(0);
  } else {
    if (OnUnhandledMessage(hwnd, uMsg, wParam, lParam))
     return(0);
  }

  return (DefWindowProc (hwnd, uMsg, wParam, lParam));
}


void InitWindowControlModule(void){

  static bool InitDone = false;

  if (InitDone)
    return;

  ActiveControl = NULL;

  InitDone = true;

}

DWORD WndForm::timeAnyOpenClose=0;
ACCEL  WndForm::mAccel[] = {
  {0, VK_ESCAPE,  VK_ESCAPE},
  {0, VK_RETURN,  VK_RETURN},
};

WndForm::WndForm(HWND Parent, const TCHAR *Name, const TCHAR *Caption, 
                 int X, int Y, int Width, int Height):
  WindowControl(NULL, Parent, Name, X, Y, Width, Height, false) {

  mClientWindow = NULL;
  mOnKeyDownNotify = NULL;
  mOnKeyUpNotify = NULL;
  mOnLButtonUpNotify = NULL;
  mOnTimerNotify = NULL;
  bLButtonDown= false; 
  mhAccelTable = CreateAcceleratorTable(mAccel, sizeof(mAccel)/sizeof(mAccel[0]));

  mColorTitle = RGB_MENUTITLEBG;

  mhTitleFont = GetFont();

  #if FIXDC
  mhBrushTitle = LKBrush_Black; // 101204
  #else
  mhBrushTitle = (HBRUSH)CreateSolidBrush(mColorTitle);
  #endif

  mClientWindow = new WindowControl(this, GetHandle(), TEXT(""), 20, 20, Width, Height);
  mClientWindow->SetBackColor(RGB_WINBACKGROUND);
  mClientWindow->SetCanFocus(false);

  mClientRect.top=0;
  mClientRect.left=0;
  mClientRect.bottom=Width;
  mClientRect.right=Height;

  cbTimerID = SetTimer(GetHandle(),1001,500,NULL);

  mModalResult = 0;
  if (Caption != NULL)
    _tcscpy(mCaption, Caption);

};

WndForm::~WndForm(void){
  Destroy();
}



void WndForm::Destroy(void){

  // animation

  if (mClientWindow)
    mClientWindow->SetVisible(false);

  KillTimer(GetHandle(),cbTimerID);

  DestroyAcceleratorTable(mhAccelTable);
  #if FIXDC
  #else
  DeleteObject(mhBrushTitle);
  #endif

  WindowControl::Destroy();  // delete all childs

}


HWND WndForm::GetClientAreaHandle(void){

  if (mClientWindow != NULL)

    return(mClientWindow->GetHandle());

  else

    return(GetHandle());

};


void WndForm::AddClient(WindowControl *Client){      // add client window
  if (mClientWindow != NULL){
    mClientWindow->AddClient(Client); // add it to the clientarea window
  } else
    WindowControl::AddClient(Client);
}


int WndForm::OnCommand(WPARAM wParam, LPARAM lParam){
   (void)lParam;

   // VENTA- DEBUG HARDWARE KEY PRESSED   
#ifdef VENTA_DEBUG_KEY
	TCHAR ventabuffer[80];
	wsprintf(ventabuffer,TEXT("ONCKEY WPARAM %d"), wParam);
	DoStatusMessage(ventabuffer);
#endif
   if ((wParam & 0xffff) == VK_ESCAPE){
     mModalResult = mrCancle;
     return(0);
   }
   return(1);

};

HFONT WndForm::SetTitleFont(HFONT Value){
  HFONT res = mhTitleFont;

  if (mhTitleFont != Value){
    // todo
    mhTitleFont = Value;



  }

  return(res);

}

void WndForm::SetToForeground(void) 
{
  BringWindowToTop(GetHandle());
  SetActiveWindow(GetHandle());
}


extern HWND hWndMapWindow;  // MapWindow

int WndForm::ShowModal(void){
  return ShowModal(false);
}

int WndForm::ShowModal(bool bEnableMap) {
#define OPENCLOSESUPPRESSTIME 500
  MSG msg;
  HWND oldFocusHwnd;

  enterTime = ::GetTickCount();

#ifndef ALTAIRSYNC
  Message::BlockRender(true);
#endif

  RECT mRc;
  GetWindowRect(GetHandle(), &mRc);
  DrawWireRects(&mRc, 5);

  SetVisible(true);

  SetToForeground();

  mModalResult = 0;

  oldFocusHwnd = SetFocus(GetHandle());

  FocusNext(NULL);

  bool hastimed = false;
  WndForm::timeAnyOpenClose = GetTickCount(); // when current dlg opens or child closes

  while ((mModalResult == 0) && GetMessage(&msg, NULL, 0, 0)) {
    DWORD timeMsg = GetTickCount();

//hack!
    
    // JMW update display timeout so we don't get blanking
    /*
    if (msg.message == WM_KEYDOWN) {
      if (!Debounce()) {
	continue;
      }
    }
    */

    if (msg.message == WM_KEYDOWN) {
      InterfaceTimeoutReset();
    }

    if ((msg.message == WM_KEYDOWN) && ((msg.wParam & 0xffff) == VK_ESCAPE))
      mModalResult = mrCancle;

    if (
        (msg.message == WM_KEYDOWN
          || msg.message == WM_KEYUP
	    || msg.message == WM_LBUTTONDOWN
          || msg.message == WM_LBUTTONUP
          || msg.message == WM_LBUTTONDBLCLK
        )  // screen event
        && msg.hwnd != GetHandle() && !IsChild(GetHandle(), msg.hwnd)  // not current window or child
#ifndef GNAV
        &&  !( // exception
              bEnableMap
              && msg.hwnd == hWndMapWindow
              && (
                msg.message == WM_LBUTTONDOWN
                || msg.message == WM_LBUTTONUP
                || msg.message == WM_MOUSEMOVE
              )
         )
#endif
    )
      continue;   // make it modal

    if (!TranslateAccelerator(GetHandle(), mhAccelTable, &msg)){

      if (msg.message == WM_KEYUP){
	/*
	if (KeyTimer(false,msg.wParam & 0xffff)) {
	  // activate tool tips
	  1;
	} else {
	  // behave as if it was a key down event
	  if (mOnKeyDownNotify != NULL)
	    if (!(mOnKeyDownNotify)(this, msg.wParam, msg.lParam))
	      continue;
	}
	*/
      }

      if (msg.message == WM_KEYDOWN){
	//	KeyTimer(true,msg.wParam & 0xffff);

/*
        if (ActiveControl != NULL){
          switch(msg.wParam & 0xffff){
            case VK_UP:
              if (ActiveControl->GetOwner() != NULL)
                ActiveControl->GetOwner()->FocusPrev(ActiveControl);
            continue;
            case VK_DOWN:
              if (ActiveControl->GetOwner() != NULL)
                ActiveControl->GetOwner()->FocusNext(ActiveControl);
            continue;
          }
        }
*/
        if (mOnKeyDownNotify != NULL)
          if (!(mOnKeyDownNotify)(this, msg.wParam, msg.lParam))
            continue;

      }
      if (msg.message == WM_KEYUP){
        if (mOnKeyUpNotify != NULL)
          if (!(mOnKeyUpNotify)(this, msg.wParam, msg.lParam))
            continue;
      }
      if (msg.message == WM_LBUTTONUP){
        if (mOnLButtonUpNotify != NULL)
          if (!(mOnLButtonUpNotify)(this, msg.wParam, msg.lParam))
            continue;

      }
      if (msg.message == WM_TIMER) {
        if (msg.hwnd == GetHandle()) {
          if (mOnTimerNotify) {
            mOnTimerNotify(this);
          }
          continue;
        }
      }

      TranslateMessage(&msg);
      if (msg.message != WM_LBUTTONUP || ((timeMsg - WndForm::timeAnyOpenClose) > OPENCLOSESUPPRESSTIME) ) // prevents child click from being repeat-handled by parent if buttons overlap
      {
        if (DispatchMessage(&msg)){

          /*
          // navigation messages are moved to unhandled messages, duto nav events handling changes in event loop
          if (msg.message == WM_KEYDOWN){
            if (ActiveControl != NULL){
              switch(msg.wParam & 0xffff){
                case VK_UP:
                  if (ActiveControl->GetOwner() != NULL)
                    ActiveControl->GetOwner()->FocusPrev(ActiveControl);
                continue;
                case VK_DOWN:
                  if (ActiveControl->GetOwner() != NULL)
                    ActiveControl->GetOwner()->FocusNext(ActiveControl);
                continue;
              }
            }
          } */

        } else {

          /*
          if (msg.message == WM_KEYDOWN){
            if (ActiveControl != NULL){
              switch(msg.wParam & 0xffff){
                case VK_UP:
                  if (ActiveControl->GetOwner() != NULL)
                    ActiveControl->GetOwner()->FocusPrev(ActiveControl);
                continue;
                case VK_DOWN:
                  if (ActiveControl->GetOwner() != NULL)
                    ActiveControl->GetOwner()->FocusNext(ActiveControl);
                continue;
              }
            }
          }
          */
        } // DispatchMessage
      } // timeMsg
  }

    // hack to stop exiting immediately
    // TODO code: maybe this should block all key handlers to avoid 
    // accidental key presses
    if (!hastimed) {
#if !defined(GNAV) && !defined(NOKEYDEBONCE)
	#if defined(PNA) || (WINDOWSPC>0) 
	if (::GetTickCount()-enterTime<400) { // 091217
	#else
	if (::GetTickCount()-enterTime<1000) {
	#endif
		mModalResult = 0;
	} else {
		hastimed = true;
	}
#endif
    }
  } // End Modal Loop
  WndForm::timeAnyOpenClose = GetTickCount(); // static.  this is current open/close or child open/close

  //  SetSourceRectangle(mRc);
  //  DrawWireRects(&aniRect, 5);

  /*
  // reset to center?
  aniRect.top = (mRc.top+mRc.bottom)/2;;
  aniRect.left = (mRc.left+mRc.right)/2;
  aniRect.right = (mRc.left+mRc.right)/2;
  aniRect.bottom = (mRc.top+mRc.bottom)/2;
  SetSourceRectangle(aniRect);
  */

  SetFocus(oldFocusHwnd);

#ifndef ALTAIRSYNC
  // JMW added to make sure screen is redrawn
  MapWindow::RequestFastRefresh();

  Message::BlockRender(false);
#endif

  return(mModalResult);

}

void WndForm::Paint(HDC hDC){

  RECT rcClient;
  SIZE tsize;
  HPEN oldPen;
  HBRUSH oldBrush;

  if (!GetVisible()) return;


  CopyRect(&rcClient, GetBoundRect());

  oldPen = (HPEN)SelectObject(hDC, GetBorderPen());
  oldBrush = (HBRUSH) SelectObject(hDC, GetBackBrush());

  DrawEdge(hDC, &rcClient, EDGE_RAISED, BF_ADJUST | BF_FLAT | BF_RECT);
  SetTextColor(hDC, RGB_MENUTITLEFG);
  SetBkColor(hDC, mColorTitle);
  SetBkMode(hDC, TRANSPARENT);

  SelectObject(hDC, mhTitleFont);
  GetTextExtentPoint(hDC, mCaption, _tcslen(mCaption), &tsize);
  if (_tcslen(mCaption)==0) tsize.cy=0; //@ 101115 BUGFIX

  // JMW todo add here icons?

  CopyRect(&mTitleRect, &rcClient);
  mTitleRect.bottom = mTitleRect.top + tsize.cy;

  POINT p1, p2;
  p1.x=0; p1.y=mTitleRect.bottom;
  p2.x=mTitleRect.right; p2.y=mTitleRect.bottom;
  MapWindow::_DrawLine(hDC, PS_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, mTitleRect);
  //rcClient.top += tsize.cy+NIBLSCALE(1)-1;
  if (ScreenLandscape && (ScreenSize!=ss800x480))
		rcClient.top = mTitleRect.bottom+NIBLSCALE(1);
  else
		rcClient.top = mTitleRect.bottom+NIBLSCALE(1)-1;

  if (mClientWindow && !EqualRect(&mClientRect, &rcClient)){

    SetWindowPos(mClientWindow->GetHandle(), HWND_TOP,
      rcClient.left, rcClient.top, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
      0);

    CopyRect(&mClientRect, &rcClient);

  }

  ExtTextOut(hDC, mTitleRect.left+1, mTitleRect.top-2,
             ETO_OPAQUE, &mTitleRect, mCaption, _tcslen(mCaption), NULL);

  SelectObject(hDC, oldBrush);
  SelectObject(hDC, oldPen);

}

void WndForm::SetCaption(const TCHAR *Value) {

  if (Value == NULL && mCaption[0] != '\0') {
	mCaption[0] ='\0';
	InvalidateRect(GetHandle(), GetBoundRect(), false);
	UpdateWindow(GetHandle());
	return;
  }

  if (_tcscmp(mCaption, Value) != 0) {
	_tcscpy(mCaption, Value);
	InvalidateRect(GetHandle(), &mTitleRect, false);
	UpdateWindow(GetHandle());
  }
}


COLORREF WndForm::SetForeColor(COLORREF Value){
  if (mClientWindow)
    mClientWindow->SetForeColor(Value);
  return(WindowControl::SetForeColor(Value));
}

COLORREF WndForm::SetBackColor(COLORREF Value){
  if (mClientWindow)
  mClientWindow->SetBackColor(Value);
  return(WindowControl::SetBackColor(Value));
}

HFONT WndForm::SetFont(HFONT Value){
  if (mClientWindow)
    mClientWindow->SetFont(Value);
  return(WindowControl::SetFont(Value));
}


void WndForm::SetKeyDownNotify(int (*KeyDownNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam)){
  mOnKeyDownNotify = KeyDownNotify;
}

void WndForm::SetKeyUpNotify(int (*KeyUpNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam)){
  mOnKeyUpNotify = KeyUpNotify;
}

void WndForm::SetLButtonUpNotify( int (*LButtonUpNotify)(WindowControl * Sender, WPARAM wParam, LPARAM lParam)){
  mOnLButtonUpNotify = LButtonUpNotify;
}

void WndForm::SetTimerNotify(int (*OnTimerNotify)(WindowControl * Sender)) {
  mOnTimerNotify = OnTimerNotify;
}

void WndForm::SetUserMsgNotify(int (*OnUserMsgNotify)(WindowControl * Sender, MSG *msg)){
  mOnUserMsgNotify = OnUserMsgNotify;
}

// normal form stuff (nonmodal)

bool WndForm::SetFocused(bool Value, HWND FromTo){

  bool res = WindowControl::SetFocused(Value, FromTo);

  return(res);

}


int WndForm::OnUnhandledMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){ 

  MSG msg;
  msg.hwnd = hwnd;
  msg.message = uMsg;
  msg.wParam = wParam;
  msg.lParam = lParam;
  msg.time = 0;
  msg.pt.x = 0;
  msg.pt.y = 0;

  /*if (msg.message == WM_ACTIVATE){
    msg.wParam = WA_ACTIVE;
  }*/

  if (msg.message >= WM_USER && msg.message < WM_USER+100){
    if (mOnUserMsgNotify != NULL)
      if (!(mOnUserMsgNotify)(this, &msg))
        return(0);
  }
  if (msg.message == WM_KEYUP){
  }
  if (msg.message == WM_KEYDOWN){
    InterfaceTimeoutReset();
    if (mOnKeyDownNotify != NULL)
      if (!(mOnKeyDownNotify)(this, msg.wParam, msg.lParam))
        return(0);

  }
  if (msg.message == WM_KEYUP){
    if (mOnKeyUpNotify != NULL)
      if (!(mOnKeyUpNotify)(this, msg.wParam, msg.lParam))
        return(0);
  }
  if (msg.message == WM_LBUTTONUP){
    bLButtonDown=false;
    InterfaceTimeoutReset();
    if (mOnLButtonUpNotify != NULL)
      if (!(mOnLButtonUpNotify)(this, msg.wParam, msg.lParam))
        return(0);

  }
  if (msg.message == WM_TIMER) {
    if (msg.hwnd == GetHandle()) {
      if (mOnTimerNotify) {
        mOnTimerNotify(this);
      }
      return(1);
    }
  }

  if (uMsg == WM_KEYDOWN){
    InterfaceTimeoutReset();
    if (ActiveControl != NULL){
      switch(wParam & 0xffff){
        case VK_UP:
          if (ActiveControl->GetOwner() != NULL)
            ActiveControl->GetOwner()->FocusPrev(ActiveControl);
        return(0);
        case VK_DOWN:
          if (ActiveControl->GetOwner() != NULL)
            ActiveControl->GetOwner()->FocusNext(ActiveControl);
        return(0);
      }
    }
  }
  else if (uMsg == WM_LBUTTONDOWN){
    bLButtonDown=true; 

    /*

    SetActiveWindow(hwnd);
    SetFocus(hwnd);

    if (!IsChild(GetHandle(), GetTopWindow(GetHandle()))){
      Show();
    }

    */
  }

  return(1);

}

void WndForm::Show(void){

  WindowControl::Show();

  SetToForeground();

//  SetFocus(GetTopWindow(GetHandle()));

//  SetActiveWindow(GetHandle());

}

//-----------------------------------------------------------
// WndButton
//-----------------------------------------------------------

WndButton::WndButton(WindowControl *Parent, const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height, void(*Function)(WindowControl * Sender)):
      WindowControl(Parent, NULL /*Parent->GetHandle()*/, Name, X, Y, Width, Height){

  mOnClickNotify = Function;
  mDown = false;
  mDefault = false;
  mCanFocus = true;

  SetForeColor(RGB_BUTTONFG);
  SetBackColor(GetOwner()->GetBackColor());

  _tcscpy(mCaption, Caption);

  mLastDrawTextHeight = -1;

};

void WndButton::Destroy(void){

  WindowControl::Destroy();

}


int WndButton::OnLButtonUp(WPARAM wParam, LPARAM lParam){
  POINT Pos;
  (void)wParam;

  mDown = false;
  Paint(GetDeviceContext());
  ReleaseCapture();

  Pos.x = lParam & 0x0000ffff; 
  Pos.y = (lParam >> 16)& 0x0000ffff;

  //POINTSTOPOINT(Pos, MAKEPOINTS(lParam));

  if (PtInRect(GetBoundRect(), Pos)){
    if (mOnClickNotify != NULL) {
      RECT mRc;
      GetWindowRect(GetHandle(), &mRc);
      SetSourceRectangle(mRc);
      (mOnClickNotify)(this);
    }
  }

  return(1);
};


int WndButton::OnKeyDown(WPARAM wParam, LPARAM lParam){
	(void)lParam;
#ifdef VENTA_DEBUG_EVENT  
	TCHAR ventabuffer[80];
	wsprintf(ventabuffer,TEXT("ONKEYDOWN WPARAM %d"), wParam); // VENTA-
	DoStatusMessage(ventabuffer);
#endif
  switch (wParam){
#ifdef GNAV
    // JMW added this to make data entry easier
    case VK_F4:
#endif
    case VK_RETURN:
    case VK_SPACE:
      if (!mDown){
        mDown = true;
        Paint(GetDeviceContext());
      }
    return(0);
  }
  return(1);
}

int WndButton::OnKeyUp(WPARAM wParam, LPARAM lParam){
	(void)lParam;
  switch (wParam){
#ifdef GNAV
    // JMW added this to make data entry easier
    case VK_F4:
#endif
    case VK_RETURN:
    case VK_SPACE:
      if (!Debounce()) return(1); // prevent false trigger
      if (mDown){
        mDown = false;
        Paint(GetDeviceContext());
        if (mOnClickNotify != NULL) {
          RECT mRc;
          GetWindowRect(GetHandle(), &mRc);
          SetSourceRectangle(mRc);
          (mOnClickNotify)(this);
        }
      }
    return(0);
  }
  return(1);
}

int WndButton::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	(void)lParam; (void)wParam;
  mDown = true;
  if (!GetFocused())
    SetFocus(GetHandle());
  else {
    InvalidateRect(GetHandle(), GetBoundRect(), false);
    UpdateWindow(GetHandle());
  }
  SetCapture(GetHandle());
  return(1);
};

int WndButton::OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam){
	(void)lParam; (void)wParam;
  mDown = true;
  InvalidateRect(GetHandle(), GetBoundRect(), false);
  UpdateWindow(GetHandle());
  SetCapture(GetHandle());
  return(1);
};


void WndButton::Paint(HDC hDC){

  RECT rc;

  if (!GetVisible()) return;

  WindowControl::Paint(hDC);

  CopyRect(&rc, GetBoundRect());
  InflateRect(&rc, -2, -2); // todo border width

  // JMW todo: add icons?

  if (mDown){
    DrawFrameControl(hDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
  }else{
    DrawFrameControl(hDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH);
  }

  if (mCaption != NULL && mCaption[0] != '\0'){

    SetTextColor(hDC, GetForeColor());

    SetBkColor(hDC, GetBackColor());
    SetBkMode(hDC, TRANSPARENT);

    HFONT oldFont = (HFONT)SelectObject(hDC, GetFont());

    CopyRect(&rc, GetBoundRect());
    InflateRect(&rc, -2, -2); // todo border width

    if (mDown)
      OffsetRect(&rc, 2, 2);

    if (mLastDrawTextHeight < 0){

      DrawText(hDC, mCaption, _tcslen(mCaption), &rc,
          DT_CALCRECT
        | DT_EXPANDTABS
        | DT_CENTER
        | DT_NOCLIP
        | DT_WORDBREAK // mCaptionStyle // | DT_CALCRECT
      );

      mLastDrawTextHeight = rc.bottom - rc.top;
      // DoTo optimize
      CopyRect(&rc, GetBoundRect());
      InflateRect(&rc, -2, -2); // todo border width
      if (mDown)
        OffsetRect(&rc, 2, 2);

    }

    rc.top += ((GetHeight()-4-mLastDrawTextHeight)/2);

    DrawText(hDC, mCaption, _tcslen(mCaption), &rc,
        DT_EXPANDTABS
      | DT_CENTER
      | DT_NOCLIP
      | DT_WORDBREAK // mCaptionStyle // | DT_CALCRECT
    );

    SelectObject(hDC, oldFont);

//    mLastDrawTextHeight = rc.bottom - rc.top;

  }

//  UINT lastAlign = SetTextAlign(hDC, TA_CENTER /*| VTA_CENTER*/);
//  ExtTextOut(hDC, GetWidth()/2, GetHeight()/2,
//    /*ETO_OPAQUE | */ETO_CLIPPED, &r, mCaption, _tcslen(mCaption), NULL);
//  if (lastAlign != GDI_ERROR){
//    SetTextAlign(hDC, lastAlign);
//  }


// 20060518:sgi old version
//  ExtTextOut(hDC, org.x, org.y,
//    /*ETO_OPAQUE | */ETO_CLIPPED, &r, mCaption, _tcslen(mCaption), NULL);

}



HBITMAP WndProperty::hBmpLeft32=NULL;
HBITMAP WndProperty::hBmpRight32=NULL;

int     WndProperty::InstCount=0;


LRESULT CALLBACK WndPropertyEditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


WndProperty::WndProperty(WindowControl *Parent, 
			 TCHAR *Name, 
			 TCHAR *Caption, 
			 int X, int Y, 
			 int Width, int Height, 
			 int CaptionWidth, 
			 int (*DataChangeNotify)(WindowControl * Sender, 
						 int Mode, int Value), 
			 int MultiLine):
  WindowControl(Parent, 
		NULL /*Parent->GetHandle()*/, 
		Name, X, Y, Width, Height){

  mOnClickUpNotify = NULL;
  mOnClickDownNotify = NULL;
  mOnDataChangeNotify = DataChangeNotify;
  _tcscpy(mCaption, Caption);
  mhEdit = NULL;
  mDataField = NULL;
  mDialogStyle=false; // this is set by ::SetDataField()

  mhValueFont = GetFont();
  mCaptionWidth = CaptionWidth;

  if (mCaptionWidth != 0){
    mBitmapSize = DLGSCALE(32)/2;
  } else {
    mBitmapSize = DLGSCALE(32)/2;
  }
  if (mDialogStyle)
    mBitmapSize = 0;

  UpdateButtonData(mBitmapSize);
  if (MultiLine) {
// VENTA3 better borders on PNA HP31X
#ifdef PNA // VENTA3 FIX
    if (GlobalModelType == MODELTYPE_PNA_HP31X )
    mhEdit = CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"), TEXT("\0"),
			  WS_BORDER | WS_VISIBLE | WS_CHILD 
			  | ES_LEFT // | ES_AUTOHSCROLL
			  | WS_CLIPCHILDREN
			  | WS_CLIPSIBLINGS
			  | WS_VSCROLL // RLD Added HSSCROLL
			  | ES_MULTILINE, // JMW added MULTILINE
        mEditPos.x, mEditPos.y,
			  mEditSize.x, mEditSize.y,
			  GetHandle(), NULL, hInst, NULL);
   else
    mhEdit = CreateWindow(TEXT("EDIT"), TEXT("\0"),
			  WS_BORDER | WS_VISIBLE | WS_CHILD 
			  | ES_LEFT // | ES_AUTOHSCROLL
			  | WS_CLIPCHILDREN
			  | WS_CLIPSIBLINGS
			  | WS_VSCROLL // RLD Added HSSCROLL
			  | ES_MULTILINE, // JMW added MULTILINE
			  mEditPos.x, mEditPos.y,
			  mEditSize.x, mEditSize.y,
			  GetHandle(), NULL, hInst, NULL);

#else
    mhEdit = CreateWindow(TEXT("EDIT"), TEXT("\0"),
			  WS_BORDER | WS_VISIBLE | WS_CHILD 
			  | ES_LEFT // | ES_AUTOHSCROLL
			  | WS_CLIPCHILDREN
			  | WS_CLIPSIBLINGS
			  | WS_VSCROLL // RLD Added HSSCROLL
			  | ES_MULTILINE, // JMW added MULTILINE
			  mEditPos.x, mEditPos.y,
			  mEditSize.x, mEditSize.y,
			  GetHandle(), NULL, hInst, NULL);
#endif
  } else {
#ifdef PNA // VENTA3 FIX
    if (GlobalModelType == MODELTYPE_PNA_HP31X )
    mhEdit = CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"), TEXT("\0"),
			  WS_BORDER | WS_VISIBLE | WS_CHILD 
			  | ES_LEFT | ES_AUTOHSCROLL
			  | WS_CLIPCHILDREN
			  | WS_CLIPSIBLINGS,
			  mEditPos.x, mEditPos.y,
			  mEditSize.x, mEditSize.y,
			  GetHandle(), NULL, hInst, NULL);
    else
    mhEdit = CreateWindow(TEXT("EDIT"), TEXT("\0"),
			  WS_BORDER | WS_VISIBLE | WS_CHILD 
			  | ES_LEFT | ES_AUTOHSCROLL
			  | WS_CLIPCHILDREN
			  | WS_CLIPSIBLINGS,
			  mEditPos.x, mEditPos.y,
			  mEditSize.x, mEditSize.y,
			  GetHandle(), NULL, hInst, NULL);
#else
    mhEdit = CreateWindow(TEXT("EDIT"), TEXT("\0"),
			  WS_BORDER | WS_VISIBLE | WS_CHILD 
			  | ES_LEFT | ES_AUTOHSCROLL
			  | WS_CLIPCHILDREN
			  | WS_CLIPSIBLINGS,
			  mEditPos.x, mEditPos.y,
			  mEditSize.x, mEditSize.y,
			  GetHandle(), NULL, hInst, NULL);
#endif
  }

  SetWindowLong(mhEdit, GWL_USERDATA, (long)this);
  mEditWindowProcedure = (WNDPROC)SetWindowLong(mhEdit, GWL_WNDPROC, (LONG) WndPropertyEditWndProc);

  SendMessage(mhEdit, WM_SETFONT,
		     (WPARAM)mhValueFont, MAKELPARAM(TRUE,0));


  mCanFocus = true;

  SetForeColor(GetOwner()->GetForeColor());
  SetBackColor(GetOwner()->GetBackColor());

  if (InstCount == 0){
    hBmpLeft32 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DLGBUTTONLEFT32));
    hBmpRight32 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DLGBUTTONRIGHT32));
  }
  InstCount++;

  mDownDown = false;
  mUpDown = false;

};


WndProperty::~WndProperty(void){
}

void WndProperty::Destroy(void){

  InstCount--;
  if (InstCount == 0){
    DeleteObject(hBmpLeft32);
    DeleteObject(hBmpRight32);
  }

  if (mDataField != NULL){
    if (!mDataField->Unuse()) {
      delete mDataField;
      mDataField = NULL;
    } else {
      ASSERT(0);
    }
  }

  SetWindowLong(mhEdit, GWL_WNDPROC, (LONG) mEditWindowProcedure);
  SetWindowLong(mhEdit, GWL_USERDATA, (long)0);

  DestroyWindow(mhEdit);

  WindowControl::Destroy();

}



void WndProperty::SetText(const TCHAR *Value){
  SetWindowText(mhEdit, Value);
}


LRESULT CALLBACK WndPropertyEditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	WndProperty *w = (WndProperty *) GetWindowLong(hwnd, GWL_USERDATA);
	if (w)
		return (w->WndProcEditControl(hwnd, uMsg, wParam, lParam));
	else
		return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}

HFONT WndProperty::SetFont(HFONT Value){
  HFONT res = GetFont();

  WindowControl::SetFont(Value);

  // todo, support value font

  if (res != Value){
    mhValueFont = Value;
    SendMessage(mhEdit, WM_SETFONT,
		     (WPARAM)mhValueFont, MAKELPARAM(TRUE,0));
  }
  return(res);
}

void WndProperty::UpdateButtonData(int Value){

  if (Value == 0) // if combo is enabled
    mBitmapSize = 0;
  else
    mBitmapSize = DLGSCALE(32)/2;  

  if (mCaptionWidth != 0){
    mEditSize.x = GetWidth()- mCaptionWidth - (DEFAULTBORDERPENWIDTH+1) - mBitmapSize;
    mEditSize.y = GetHeight()-2*(DEFAULTBORDERPENWIDTH+1);
    mEditPos.x = mCaptionWidth;
    mEditPos.y = (DEFAULTBORDERPENWIDTH+1);
  } else {
    mEditSize.x = GetWidth()- 2*((DEFAULTBORDERPENWIDTH+1)+mBitmapSize);
    mEditSize.y = (GetHeight()/2);
    mEditPos.x = mBitmapSize + (DEFAULTBORDERPENWIDTH+2);
    mEditPos.y = (GetHeight()/2)-2*(DEFAULTBORDERPENWIDTH+1);
  }

  mHitRectDown.left = mEditPos.x-mBitmapSize;
  mHitRectDown.top = mEditPos.y + (mEditSize.y)/2 - (mBitmapSize/2);
  mHitRectDown.right = mHitRectDown.left + mBitmapSize;
  mHitRectDown.bottom = mHitRectDown.top + mBitmapSize;

  mHitRectUp.left = GetWidth()-(mBitmapSize+2);
  mHitRectUp.top = mHitRectDown.top;
  mHitRectUp.right = mHitRectUp.left + mBitmapSize;
  mHitRectUp.bottom = mHitRectUp.top + mBitmapSize;

}

int WndProperty::SetButtonSize(int Value){
  int res = mBitmapSize;

  if (mBitmapSize != Value){

    UpdateButtonData(Value);

    SetWindowPos(mhEdit, 0, mEditPos.x, mEditPos.y,
      mEditSize.x, mEditSize.y,
      /*SWP_NOMOVE |*/  SWP_NOREPOSITION | SWP_NOACTIVATE // need to MOVE to enlarge/shift left for combopicker (no arrows)
                 | SWP_NOOWNERZORDER | SWP_NOZORDER
    );

    if (GetVisible()){
      InvalidateRect(GetHandle(), GetBoundRect(), false);
      UpdateWindow(GetHandle());
    }
  }
  return(res);
};


int WndProperty::WndProcEditControl(HWND hwnd, UINT uMsg, 
                                    WPARAM wParam, LPARAM lParam) {

  switch (uMsg){

    case WM_KEYDOWN:
      if ((wParam & 0xffff) == VK_RETURN || (wParam & 0xffff) == VK_F23) { // Compaq uses VKF23
        if (this->mDialogStyle) {
          InterfaceTimeoutReset();
          if (!OnLButtonDown(wParam, lParam)) {
            DisplayTimeOut = 0;
            return(0);
          }
        } //end combopicker
      }
      // tmep hack, do not process nav keys
      if (KeyTimer(true, wParam & 0xffff)) {
	// activate tool tips if hit return for long time
        if ((wParam & 0xffff) == VK_RETURN || (wParam & 0xffff) == VK_F23) { // Compaq uses VKF23
	  if (OnHelp()) return (0);
	}
      } 

      if (wParam == VK_UP || wParam == VK_DOWN){
        PostMessage(GetParent(), uMsg, wParam, lParam);   
	// pass the message to the parent window;
        return(0);
        // return(1);
      }
      if (!OnEditKeyDown(wParam, lParam))
        return(1);
    break;

    case WM_KEYUP:
	if (KeyTimer(false, wParam & 0xffff)) {
	  // activate tool tips if hit return for long time
    if ((wParam & 0xffff) == VK_RETURN || (wParam & 0xffff) == VK_F23) { // Compaq uses VKF23
	    if (OnHelp()) return (0);
	  }
	} else if ((wParam & 0xffff) == VK_RETURN) {
	  if (CallSpecial()) return (0);
	}
    break;

    case WM_LBUTTONDOWN:
      // if it's an Combopicker field, then call the combopicker routine
      if (this->mDialogStyle) {
        InterfaceTimeoutReset();
        if (!OnLButtonDown(wParam, lParam)) {
          DisplayTimeOut = 0;
          return(0);
        }
      } //end combopicker
      break;

    case WM_SETFOCUS:
      KeyTimer(true, 0);
      if (GetReadOnly()){
        SetFocus((HWND)wParam);
        return(0);
      } else {
        if ((HWND)wParam != GetHandle()){
          SetFocused(true, (HWND) wParam);
        }
      }
    break;

    case WM_KILLFOCUS:
      KeyTimer(true, 0);
      if ((HWND)wParam != GetHandle()){
        SetFocused(false, (HWND) wParam);
      }
    break;
  }

  return(CallWindowProc(mEditWindowProcedure, hwnd, uMsg, wParam, lParam));

}

bool WndProperty::SetReadOnly(bool Value){

  bool res = GetReadOnly();

  if (GetReadOnly() != Value){
    WindowControl::SetReadOnly(Value);

    SendMessage(mhEdit, EM_SETREADONLY, (WPARAM)(BOOL)Value, 0L);

  }

  return(res);
}

bool WndProperty::SetFocused(bool Value, HWND FromTo){

  TCHAR sTmp[STRINGVALUESIZE];

  if (Value && GetReadOnly()){  // keep focus on last control
    if (FromTo != mhEdit)
      SetFocus(FromTo);
    return(false);
  }

  if (!Value && (FromTo == mhEdit))
    Value = true;

    if (Value != GetFocused()){
      if (Value){
        if (mDataField != NULL){
          mDataField->GetData();
          SetWindowText(mhEdit, mDataField->GetAsString());
        }
      } else {
        if (mDataField != NULL){
          GetWindowText(mhEdit, sTmp, (sizeof(sTmp)/sizeof(TCHAR))-1);
          mDataField->SetAsString(sTmp);
          mDataField->SetData();
          SetWindowText(mhEdit, mDataField->GetAsDisplayString());
      }
    }
  }

  if (FromTo != mhEdit)
    WindowControl::SetFocused(Value, FromTo);
  if (Value){
    SetFocus(mhEdit);
    PostMessage(mhEdit, EM_SETSEL, 0, -1);
  }
  return(0);
}

int WndProperty::OnEditKeyDown(WPARAM wParam, LPARAM lParam){
  (void)lParam; 
  switch (wParam){
    case VK_RIGHT:
      IncValue();
    return(0);
    case VK_LEFT:
      DecValue();
    return(0);
  }

  return(1);
}

int WndProperty::OnKeyDown(WPARAM wParam, LPARAM lParam){
  (void)lParam;
  switch (wParam){
    case VK_RIGHT:
      IncValue();
    return(0);
    case VK_LEFT:
      DecValue();
    return(0);
  }

  return(1);
};

int WndProperty::OnLButtonDown(WPARAM wParam, LPARAM lParam){
  (void)wParam;
  POINT Pos;

  if (mDialogStyle)
  {
    if (!GetReadOnly())  // when they click on the label
    {
      dlgComboPicker(this);
    }
    else 
    {
      OnHelp(); // this would display xml file help on a read-only wndproperty if it exists
    }
  }
  else
  {

    if (!GetFocused()){
      SetFocus(GetHandle());
      return(0);
    }

    Pos.x = lParam & 0x0000ffff; 
    Pos.y = (lParam >> 16)& 0x0000ffff;
    //POINTSTOPOINT(Pos, MAKEPOINTS(lParam));

    mDownDown = (PtInRect(&mHitRectDown, Pos) != 0);

    if (mDownDown) {
      DecValue();
      InvalidateRect(GetHandle(), &mHitRectDown, false);
      UpdateWindow(GetHandle());
    }

    mUpDown = (PtInRect(&mHitRectUp, Pos) != 0);

    if (mUpDown) {
      IncValue();
      InvalidateRect(GetHandle(), &mHitRectUp, false);
      UpdateWindow(GetHandle());
    }
    SetCapture(GetHandle()); 
  }
  return(0);
};

int WndProperty::OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam){

  return(OnLButtonDown(wParam, lParam));

}

int WndProperty::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	(void)lParam;
	(void)wParam;

  if (mDialogStyle)
  {
  }
  else
  {
  
    if (mDownDown){
      mDownDown = false;
      InvalidateRect(GetHandle(), &mHitRectDown, false);
      UpdateWindow(GetHandle());
    }
    if (mUpDown){
      mUpDown = false;
      InvalidateRect(GetHandle(), &mHitRectUp, false);
      UpdateWindow(GetHandle());
    }

  }
  ReleaseCapture(); 
  return(0);
}


int WndProperty::CallSpecial(void){
  if (mDataField != NULL){
    mDataField->Special();
    SetWindowText(mhEdit, mDataField->GetAsString());
  }
  return(0);
}

int WndProperty::IncValue(void){
  if (mDataField != NULL){
    mDataField->Inc();
    SetWindowText(mhEdit, mDataField->GetAsString());
  }
  return(0);
}

int WndProperty::DecValue(void){
  if (mDataField != NULL){
    mDataField->Dec();
    SetWindowText(mhEdit, mDataField->GetAsString());
  }
  return(0);
}


void WndProperty::Paint(HDC hDC){

  RECT r;
  SIZE tsize;
  POINT org;
  HBITMAP oldBmp;
  

  if (!GetVisible()) return;

  WindowControl::Paint(hDC);

  r.left = 0;
  r.top = 0;
  r.right = GetWidth();
  r.bottom = GetHeight();

  SetTextColor(hDC, GetForeColor());
#ifdef WINE
  // JMW make it look nice on wine
  if (!GetFocused())
    SetBkColor(hDC, GetBackColor());
#endif /* WINE */
  SetBkMode(hDC, TRANSPARENT);
  HFONT oldFont = (HFONT)SelectObject(hDC, GetFont());

  GetTextExtentPoint(hDC, mCaption, _tcslen(mCaption), &tsize);
  if (_tcslen(mCaption)==0) tsize.cy=0; //@ 101115 BUGFIX

  if (mCaptionWidth==0){
	org.x = mEditPos.x;
	org.y = mEditPos.y - tsize.cy;
  } else {
	org.x = mCaptionWidth - mBitmapSize - (tsize.cx + 1);
	org.y = (GetHeight() - tsize.cy)/2;
  }

  if (org.x < 1)
	org.x = 1;

  ExtTextOut(hDC, org.x, org.y, ETO_OPAQUE, NULL, mCaption, _tcslen(mCaption), NULL);

  // these are button left and right icons for waypoint select, for example
  if (mDialogStyle) // can't but dlgComboPicker here b/c it calls paint when combopicker closes too
  {     // so it calls dlgCombopicker on the click/focus handlers for the wndproperty & label
	// opening a window, each subwindow goes here once
  }
  else 
  {
	if (GetFocused() && !GetReadOnly()) {

		oldBmp = (HBITMAP)SelectObject(GetTempDeviceContext(), hBmpLeft32);

		if (mDownDown)
			StretchBlt(hDC, mHitRectDown.left, mHitRectDown.top, mBitmapSize, mBitmapSize,
		        GetTempDeviceContext(),
		        32, 0,
		        32,32, 
		        SRCCOPY);
		else
			StretchBlt(hDC, mHitRectDown.left, mHitRectDown.top, mBitmapSize, mBitmapSize,
		        GetTempDeviceContext(),
		        0, 0,
		        32,32, 
		        SRCCOPY);

		SelectObject(GetTempDeviceContext(), hBmpRight32);

		if (mUpDown)
			StretchBlt(hDC, mHitRectUp.left, mHitRectUp.top, mBitmapSize, mBitmapSize,
		        GetTempDeviceContext(),
		        32, 0,
		        32,32, 
		        SRCCOPY);

		else
			StretchBlt(hDC, mHitRectUp.left, mHitRectUp.top, mBitmapSize, mBitmapSize,
		        GetTempDeviceContext(),
		        0, 0,
		        32,32, 
		        SRCCOPY);

		SelectObject(GetTempDeviceContext(), oldBmp);
	}
  }
  SelectObject(hDC, oldFont);
}


void WndProperty::RefreshDisplay() {
  if (!mDataField) return;
  if (GetFocused())
    SetWindowText(mhEdit, mDataField->GetAsString());
  else
    SetWindowText(mhEdit, mDataField->GetAsDisplayString());
}


DataField *WndProperty::SetDataField(DataField *Value){
  DataField *res = mDataField;

  if (mDataField != Value){

    if (mDataField!=NULL){

      if (!mDataField->Unuse()){

        delete(mDataField);

        res = NULL;

      }

    }

    Value->Use();

    mDataField = Value;

    mDataField->GetData();


    
    mDialogStyle=ENABLECOMBO;

    if (mDataField->SupportCombo == false )
      mDialogStyle=false;


    if (mDialogStyle)
    {
      this->SetButtonSize(0);
      this->SetCanFocus(true);
    }
    else
    {
      this->SetButtonSize(16);
    }

    RefreshDisplay();

  }

  return(res);

}


void WndOwnerDrawFrame::Paint(HDC hDC){

  if (!GetVisible()) return;

  WndFrame::Paint(hDC);

  HFONT oldFont = (HFONT)SelectObject(hDC, GetFont());

  if (mOnPaintCallback != NULL)
    (mOnPaintCallback)(this, hDC);

  SelectObject(hDC, oldFont);

}

void WndOwnerDrawFrame::Destroy(void){

  WndFrame::Destroy();

}


void WndFrame::Destroy(void){

  WindowControl::Destroy();

}


int WndFrame::OnKeyDown(WPARAM wParam, LPARAM lParam){
  if (mIsListItem && GetOwner()!=NULL){
    RECT mRc;
    GetWindowRect(GetHandle(), &mRc);
    SetSourceRectangle(mRc);
    return(((WndListFrame*)GetOwner())->OnItemKeyDown(this, wParam, lParam));
  }
  return(1);
}

void WndFrame::Paint(HDC hDC){

  if (!GetVisible()) return;

  if (mIsListItem && GetOwner()!=NULL) {
    ((WndListFrame*)GetOwner())->PrepareItemDraw();
  }

  WindowControl::Paint(hDC);

  if (mCaption != 0){

    RECT rc;

    SetTextColor(hDC, GetForeColor());
    SetBkColor(hDC, GetBackColor());
    SetBkMode(hDC, TRANSPARENT);

    HFONT oldFont = (HFONT)SelectObject(hDC, GetFont());

    CopyRect(&rc, GetBoundRect());
    InflateRect(&rc, -2, -2); // todo border width

//    h = rc.bottom - rc.top;

    DrawText(hDC, mCaption, _tcslen(mCaption), &rc,
      mCaptionStyle // | DT_CALCRECT
    );

    mLastDrawTextHeight = rc.bottom - rc.top;

    SelectObject(hDC, oldFont);
  }

}

void WndFrame::SetCaption(const TCHAR *Value){

  if (Value == NULL && mCaption[0] != '\0'){
    mCaption[0] ='\0';
    InvalidateRect(GetHandle(), GetBoundRect(), false);
    UpdateWindow(GetHandle());

    return;

  }

  if (_tcscmp(mCaption, Value) != 0){
    _tcscpy(mCaption, Value);  // todo size check
    InvalidateRect(GetHandle(), GetBoundRect(), false);
    UpdateWindow(GetHandle());

  }
}

UINT WndFrame::SetCaptionStyle(UINT Value){
  UINT res = mCaptionStyle;
  if (res != Value){
    mCaptionStyle = Value;

    InvalidateRect(GetHandle(), GetBoundRect(), false);
    UpdateWindow(GetHandle());

  }
  return(res);
}


WndListFrame::WndListFrame(WindowControl *Owner, TCHAR *Name, int X, int Y, 
                           int Width, int Height, 
                           void (*OnListCallback)(WindowControl * Sender, 
                                                  ListInfo_t *ListInfo)):
  WndFrame(Owner, Name, X, Y, Width, Height)
{

  mListInfo.ItemIndex = 0;
  mListInfo.DrawIndex = 0;
  mListInfo.ItemInPageCount = 0;
  mListInfo.TopIndex = 0;
  mListInfo.BottomIndex = 0;
//  mListInfo.SelectedIndex = 0;
  mListInfo.ItemCount = 0;
  mListInfo.ItemInViewCount = 0;

  mCaption[0] = '\0';
  mOnListCallback = OnListCallback;
  mOnListEnterCallback = NULL;
  SetForeColor(RGB_LISTFG);
  SetBackColor(RGB_LISTBG);
  mMouseDown = false;
  LastMouseMoveTime=0;
  ScrollbarWidth=-1;
  ScrollbarTop=-1;

  rcScrollBarButton.top=0; // make sure this rect is initialized so we don't "loose" random lbuttondown events if scrollbar not drawn
  rcScrollBarButton.bottom=0;
  rcScrollBarButton.left=0;
  rcScrollBarButton.right=0;

  rcScrollBar.left=0;  // don't need to initialize this rect, but it's good practice
  rcScrollBar.right=0;
  rcScrollBar.top=0;
  rcScrollBar.bottom=0;
  
};


void WndListFrame::Destroy(void){

  WndFrame::Destroy();

}


void WndListFrame::Paint(HDC hDC){
  int i;

  if (mClientCount > 0){
    ((WndFrame *)mClients[0])->SetIsListItem(true);
//    ShowWindow(mClients[0]->GetHandle(), SW_HIDE);
/*
    if (mOnListCallback != NULL){
      mListInfo.DrawIndex = mListInfo.ItemIndex;
      mOnListCallback(this, &mListInfo);
      mClients[0]->SetTop(mClients[0]->GetHeight() * (mListInfo.ItemIndex-mListInfo.TopIndex));
    }
*/
  }

  WndFrame::Paint(hDC);

  if (mClientCount > 0){

	HDC HdcTemp = CreateCompatibleDC(hDC);

    HBITMAP BmpMem = CreateCompatibleBitmap(hDC,
               mClients[0]->GetWidth(),
               mClients[0]->GetHeight());

    HBITMAP oldBmp = (HBITMAP)SelectObject(HdcTemp, BmpMem);

    for (i=0; i<mListInfo.ItemInViewCount; i++){
 
      HFONT oldFont = (HFONT)SelectObject(HdcTemp, mClients[0]->GetFont());

      if (mOnListCallback != NULL){
        mListInfo.DrawIndex = mListInfo.TopIndex + i;
        if (mListInfo.DrawIndex == mListInfo.ItemIndex)
          continue;
        mOnListCallback(this, &mListInfo);
      }

      mClients[0]->PaintSelector(true);
      mClients[0]->Paint(HdcTemp);
      mClients[0]->PaintSelector(false);

      BitBlt(hDC,
          mClients[0]->GetLeft(), i*mClients[0]->GetHeight(),
          mClients[0]->GetWidth(), mClients[0]->GetHeight(),
          HdcTemp,
          0,0,
          SRCCOPY
        );

      SelectObject(HdcTemp, oldFont);

    }

    mListInfo.DrawIndex = mListInfo.ItemIndex;

    SelectObject(HdcTemp, oldBmp);
    DeleteObject(BmpMem);
    DeleteDC(HdcTemp);

    DrawScrollBar(hDC);
  }
}

void WndListFrame::Redraw(void){
  WindowControl::Redraw();  // redraw all but not the current
  mClients[0]->Redraw();    // redraw the current                                      
}


void WndListFrame::DrawScrollBar(HDC hDC) {

  static HBITMAP hScrollBarBitmapTop = NULL;
  static HBITMAP hScrollBarBitmapMid = NULL;
  static HBITMAP hScrollBarBitmapBot = NULL;
  #ifndef FIXDC
  static HBITMAP hScrollBarBitmapFill = NULL; //101205
  #endif
  RECT rc;
  HPEN hP, hP3;
  HBITMAP oldBmp;


  if ( ScrollbarWidth == -1) {  // resize height for each dialog so top button is below 1st item (to avoid initial highlighted overlap)

	// shrink width factor.  Range .1 to 1 where 1 is very "fat"
	#if defined (PNA)
	#define SHRINKSBFACTOR 1.0 
	#else
	#define SHRINKSBFACTOR 0.75 
	#endif

	ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * InfoBoxLayout::dscale * SHRINKSBFACTOR);  
	if (mClientCount > 0) {
		ScrollbarTop = mClients[0]->GetHeight() + 2;
	} else {
		ScrollbarTop = (int)(18.0 * InfoBoxLayout::dscale + 2);
	}
  }


  int w = GetWidth()- (ScrollbarWidth);
  int h = GetHeight() - ScrollbarTop;

  #if FIXDC
  if ( (sHdc==NULL) || (hScrollBarBitmapTop == NULL)) {
	//StartupStore(_T("... sHdc Create\n")); // REMOVE
	sHdc = CreateCompatibleDC(hDC); 
  }
  if (hScrollBarBitmapTop == NULL) {
	hScrollBarBitmapTop=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARTOP));
  }
  #else
  if (hScrollBarBitmapTop == NULL) 
	hScrollBarBitmapTop=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARTOP));
  #endif
  if (hScrollBarBitmapMid == NULL)
	hScrollBarBitmapMid=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARMID));
  if (hScrollBarBitmapBot == NULL)
	hScrollBarBitmapBot=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARBOT));
  #ifndef FIXDC // 101205
  if (hScrollBarBitmapFill == NULL)
	hScrollBarBitmapFill=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARFILL));
  #endif

  #if FIXDC	// 101205
  hP = LKPen_Black_N1;

  #else	// ------  REMOVABLE 1/1/2011
  hP = (HPEN)CreatePen(PS_SOLID, DEFAULTBORDERPENWIDTH, RGB_SCROLLBARBORDER);
  #endif // -------- END REMOVABLE

  SelectObject(hDC, hP);

  // ENTIRE SCROLLBAR AREA
  rc.left = w;
  rc.top = ScrollbarTop;
  rc.right = w + (ScrollbarWidth) - 1;
  rc.bottom = h + ScrollbarTop;

  // save scrollbar size for mouse events
  rcScrollBar.left=rc.left;
  rcScrollBar.right=rc.right;
  rcScrollBar.top=rc.top;
  rcScrollBar.bottom=rc.bottom;

  if (mListInfo.BottomIndex == mListInfo.ItemCount) { // don't need scroll bar if one page only
	#if FIXDC
        #else
	DeleteObject(hP);
	#endif
	return;
  }

  // draw rectangle around entire scrollbar area
  DrawLine2(hDC, rc.left, rc.top, rc.left, rc.bottom, rc.right, rc.bottom); 
  DrawLine2(hDC, rc.right, rc.bottom, rc.right, rc.top, rc.left, rc.top); 

  // Just Scroll Bar Slider button
  rc.left = w;
  rc.top = GetScrollBarTopFromScrollIndex()-1;
  rc.right = w + (ScrollbarWidth) - 1; // -2 if use 3x pen.  -1 if 2x pen
  rc.bottom = rc.top + GetScrollBarHeight()+2;  // +2 for 3x pen, +1 for 2x pen

  if (rc.bottom >= GetHeight() - ScrollbarWidth){
	int d;
	d= (GetHeight() - ScrollbarWidth - rc.bottom) - 1;
	rc.bottom += d;
	rc.top += d;
  }

  unsigned long ctUpDown =0;
  unsigned long ctScroll =0;

  bool bTransparentUpDown = true;
  bool bTransparentScroll = true;

  if (bTransparentUpDown)
	ctUpDown=SRCAND;  //Combines the colors of the source and destination rectangles by using the Boolean AND operator.
  else
	ctUpDown=SRCCOPY;  //Copies the source rectangle directly to the destination rectangle.

  if (bTransparentScroll)
	ctScroll=SRCAND;  //Combines the colors of the source and destination rectangles by using the Boolean AND operator.
  else
	ctScroll=SRCCOPY;  //Copies the source rectangle directly to the destination rectangle.


  // TOP Dn Button 32x32
  // BOT Up Button 32x32
  if (ScrollbarWidth == SCROLLBARWIDTH_INITIAL) {
	oldBmp = (HBITMAP)SelectObject(GetTempDeviceContext(), hScrollBarBitmapTop);
	BitBlt(hDC, w, ScrollbarTop, SCROLLBARWIDTH_INITIAL, SCROLLBARWIDTH_INITIAL, GetTempDeviceContext(), 0, 0, ctUpDown); 
    
	SelectObject(GetTempDeviceContext(), hScrollBarBitmapBot);
	BitBlt(hDC, w, h-SCROLLBARWIDTH_INITIAL+ScrollbarTop, SCROLLBARWIDTH_INITIAL, SCROLLBARWIDTH_INITIAL,
	     GetTempDeviceContext(), 0, 0, ctUpDown);  
  } else {
	oldBmp = (HBITMAP)SelectObject(GetTempDeviceContext(), hScrollBarBitmapTop);

	StretchBlt(hDC, w, ScrollbarTop, 
		 (ScrollbarWidth), 
		 (ScrollbarWidth),
		 GetTempDeviceContext(),
		 0, 0, SCROLLBARWIDTH_INITIAL, SCROLLBARWIDTH_INITIAL,
		 ctUpDown);

    
	// BOT Up Button 32x32
	SelectObject(GetTempDeviceContext(), hScrollBarBitmapBot);

	StretchBlt(hDC, w, h-(ScrollbarWidth)+ScrollbarTop, 
		 (ScrollbarWidth), 
		 (ScrollbarWidth),
		 GetTempDeviceContext(),
		 0, 0, SCROLLBARWIDTH_INITIAL, SCROLLBARWIDTH_INITIAL,
		 ctUpDown);
  }

  // Middle Slider Button 30x28
  if (mListInfo.ItemCount > mListInfo.ItemInViewCount) {
    
	// handle on slider
	SelectObject(GetTempDeviceContext(), hScrollBarBitmapMid); 
	if (ScrollbarWidth == SCROLLBARWIDTH_INITIAL) {
		BitBlt(hDC, w+1, rc.top + GetScrollBarHeight()/2 - 14, 30, 28,
			GetTempDeviceContext(), 0, 0, SRCAND); // always SRCAND b/c on top of scrollbutton texture
	} else {
		static int SCButtonW = -1;
		static int SCButtonH = -1;
		static int SCButtonY = -1;
		if (SCButtonW == -1) {
			SCButtonW = (int) (30.0 * (float)ScrollbarWidth / (float)SCROLLBARWIDTH_INITIAL);
			SCButtonH = (int) (28.0 * (float)ScrollbarWidth / (float)SCROLLBARWIDTH_INITIAL);
			SCButtonY = (int) (14.0 * (float)ScrollbarWidth / (float)SCROLLBARWIDTH_INITIAL);
		}
		StretchBlt(hDC, w+1, rc.top + GetScrollBarHeight()/2 - SCButtonY, SCButtonW, SCButtonH,
			   GetTempDeviceContext(), 0, 0, 
			   30, 28,
			   SRCAND); // always SRCAND b/c on top of scrollbutton texture
	}

	// box around slider rect
	#if FIXDC
	hP3=LKPen_Black_N2;

	#else // ------- REMOVABLE
	hP3 = (HPEN)CreatePen(PS_SOLID, DEFAULTBORDERPENWIDTH * 2, RGB_SCROLLBARBOX); 
	#endif // -------- REMOVABLE

	int iBorderOffset = 1;  // set to 1 if BORDERWIDTH >2, else 0
	SelectObject(hDC, hP3);
	// just left line of scrollbar
	DrawLine2(hDC, rc.left+iBorderOffset, rc.top, rc.left+iBorderOffset, rc.bottom, rc.right, rc.bottom); 
	DrawLine2(hDC, rc.right, rc.bottom, rc.right, rc.top, rc.left+iBorderOffset, rc.top); // just left line of scrollbar
	#if FIXDC  
	#else
	DeleteObject(hP3);
	#endif

  } // more items than fit on screen

  SelectObject(GetTempDeviceContext(), oldBmp);
  
  rcScrollBarButton.left=rc.left;
  rcScrollBarButton.right=rc.right;
  rcScrollBarButton.top=rc.top;
  rcScrollBarButton.bottom=rc.bottom;

  #if FIXDC
  #else
  DeleteObject(hP);
  #endif
}


void WndListFrame::SetEnterCallback(void 
                                    (*OnListCallback)(WindowControl *Sender, 
                                                      ListInfo_t *ListInfo)) 
{
  mOnListEnterCallback = OnListCallback;
}


void WndListFrame::RedrawScrolled(bool all) {

  int newTop;

  /*       -> inefficient and flickering draws the list twice
  if (all) {
    int i;
    for (i=0; i<= mListInfo.ItemInViewCount; i++) {
      mListInfo.DrawIndex = mListInfo.TopIndex+i;
      mOnListCallback(this, &mListInfo);
      mClients[0]->SetTop(mClients[0]->GetHeight() * (i));
      mClients[0]->Redraw();
    }
  }
  */

  mListInfo.DrawIndex = mListInfo.ItemIndex;
  mOnListCallback(this, &mListInfo);
  newTop = mClients[0]->GetHeight() * (mListInfo.ItemIndex - mListInfo.TopIndex);
  if (newTop == mClients[0]->GetTop()){
    Redraw();                     // non moving the helper window force redraw
  } else {
    mClients[0]->SetTop(newTop);  // moving the helper window invalidate the list window
    mClients[0]->Redraw();

    // to be optimized: after SetTop Paint redraw all list items

  }

}


int WndListFrame::RecalculateIndices(bool bigscroll) {

// scroll to smaller of current scroll or to last page
  mListInfo.ScrollIndex = max(0,min(mListInfo.ScrollIndex,
				    mListInfo.ItemCount-mListInfo.ItemInPageCount));

// if we're off end of list, move scroll to last page and select 1st item in last page, return
  if (mListInfo.ItemIndex+mListInfo.ScrollIndex >= mListInfo.ItemCount) {
    mListInfo.ItemIndex = max(0,mListInfo.ItemCount-mListInfo.ScrollIndex-1);
    mListInfo.ScrollIndex = max(0,
			      min(mListInfo.ScrollIndex,
				  mListInfo.ItemCount-mListInfo.ItemIndex-1));
    return(1);
  }

// again, check to see if we're too far off end of list
  mListInfo.ScrollIndex = max(0,
			      min(mListInfo.ScrollIndex,
				  mListInfo.ItemCount-mListInfo.ItemIndex-1));

  if (mListInfo.ItemIndex >= mListInfo.BottomIndex){
    if ((mListInfo.ItemCount>mListInfo.ItemInPageCount) 
	&& (mListInfo.ItemIndex+mListInfo.ScrollIndex < mListInfo.ItemCount)) {
      mListInfo.ScrollIndex++;
      mListInfo.ItemIndex = mListInfo.BottomIndex-1;
      // JMW scroll
      RedrawScrolled(true);
      return(0);
    } else {
      mListInfo.ItemIndex = mListInfo.BottomIndex-1;
      return(1);
    }
  }
  if (mListInfo.ItemIndex < 0){

    mListInfo.ItemIndex = 0;
    // JMW scroll
    if (mListInfo.ScrollIndex>0) {
      mListInfo.ScrollIndex--;
      RedrawScrolled(true);
      return(0);
    } else {
      // only return if no more scrolling left to do
      return(1);
    }
  }
  RedrawScrolled(bigscroll);
  return (0);
}


int WndListFrame::OnItemKeyDown(WindowControl *Sender, WPARAM wParam, LPARAM lParam){
	(void)Sender;
	(void)lParam;
  switch (wParam){
#ifdef GNAV
    // JMW added this to make data entry easier
    case VK_F4:
#endif
  case VK_RETURN:
    if (mOnListEnterCallback) {
      mOnListEnterCallback(this, &mListInfo);
      RedrawScrolled(false);
      return(0);
    } else 
      return(1);
    //#ifndef GNAV
  case VK_LEFT:
    if ((mListInfo.ScrollIndex>0)
	&&(mListInfo.ItemCount>mListInfo.ItemInPageCount)) {
      mListInfo.ScrollIndex -= mListInfo.ItemInPageCount;
    }
    return RecalculateIndices(true);
  case VK_RIGHT:
    if ((mListInfo.ItemIndex+mListInfo.ScrollIndex<
	 mListInfo.ItemCount)
	&&(mListInfo.ItemCount>mListInfo.ItemInPageCount)) {
      mListInfo.ScrollIndex += mListInfo.ItemInPageCount;
    }
    return RecalculateIndices(true);
    //#endif
  case VK_DOWN:

	  
    mListInfo.ItemIndex++;
    return RecalculateIndices(false);
  case VK_UP:
    mListInfo.ItemIndex--;
    return RecalculateIndices(false);
  }
  mMouseDown=false;
  return(1);

}

void WndListFrame::ResetList(void){

  mListInfo.ScrollIndex = 0;
  mListInfo.ItemIndex = 0;
  mListInfo.DrawIndex = 0;
  mListInfo.ItemInPageCount = ((GetHeight()+mClients[0]->GetHeight()-1)
			       /mClients[0]->GetHeight())-1;
  mListInfo.TopIndex = 0;
  mListInfo.BottomIndex = 0;
//  mListInfo.SelectedIndex = 0;
  mListInfo.ItemCount = 0;
  mListInfo.ItemInViewCount = (GetHeight()+mClients[0]->GetHeight()-1)
    /mClients[0]->GetHeight()-1;

  if (mOnListCallback != NULL){
    mListInfo.DrawIndex = -1;                               // -1 -> initialize data
    mOnListCallback(this, &mListInfo);
    mListInfo.DrawIndex = 0;                                // setup data for first item,
    mOnListCallback(this, &mListInfo);
  }

  if (mListInfo.BottomIndex  == 0){                         // calc bounds
    mListInfo.BottomIndex  = mListInfo.ItemCount;
    if (mListInfo.BottomIndex > mListInfo.ItemInViewCount){
      mListInfo.BottomIndex = mListInfo.ItemInViewCount;
    }
  }

  mClients[0]->SetTop(0);     // move item window to the top
  mClients[0]->Redraw();
}

int WndListFrame::PrepareItemDraw(void){
  if (mOnListCallback)
    mOnListCallback(this, &mListInfo);
  return(1);
}

int WndListFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam) {
    mMouseDown=false;
    return 1;
}

static bool isselect = false;

int WndFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam) {
  return 1;
}

// JMW needed to support mouse/touchscreen
int WndFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam) {
	(void)wParam;

  if (mIsListItem && GetOwner()!=NULL) {
 
    if (!GetFocused()) {
      SetFocus(GetHandle());  
      //return(1);
    } 
    //else {  // always doing this allows selected item in list to remain selected.
      InvalidateRect(GetHandle(), GetBoundRect(), false);
      UpdateWindow(GetHandle());
    //}

    int xPos = LOWORD(lParam);  // horizontal position of cursor 
    int yPos = HIWORD(lParam);  // vertical position of cursor 
    WndListFrame* wlf = ((WndListFrame*)GetOwner());
    RECT mRc;
    GetWindowRect(GetHandle(), &mRc);
    wlf->SelectItemFromScreen(xPos, yPos, &mRc);
  }
  isselect = false;
  return(1);
}


void WndListFrame::SetItemIndex(int iValue){
  

  mListInfo.ItemIndex=0;  // usually leaves selected item as first in screen
  mListInfo.ScrollIndex=iValue; 

  int iTail = mListInfo.ItemCount - iValue; // if within 1 page of end
  if ( iTail < mListInfo.ItemInPageCount)  
  {
    int iDiff = mListInfo.ItemInPageCount - iTail;
    int iShrinkBy = min(iValue, iDiff); // don't reduce by 

    mListInfo.ItemIndex += iShrinkBy;
    mListInfo.ScrollIndex -= iShrinkBy;
  }

  RecalculateIndices(false);
}

void WndListFrame::SelectItemFromScreen(int xPos, int yPos,
                                        RECT *rect) {
  (void)xPos;
/*  int w = GetWidth()- 4*SELECTORWIDTH;
  int h = GetHeight()- SELECTORWIDTH;

  if ((xPos>= w) && (mListInfo.ItemCount > mListInfo.ItemInViewCount)
      && (mListInfo.ItemCount>0)) {
    // TODO code: scroll!

    mListInfo.ScrollIndex = mListInfo.ItemCount*yPos/h;
    RecalculateIndices(true);

    return;
  }
*/
  int index;
  GetClientRect(GetHandle(), rect);
  index = yPos/mClients[0]->GetHeight(); // yPos is offset within ListEntry item!

  if ((index>=0)&&(index<mListInfo.BottomIndex)) {
    if (!isselect) {
      if (mOnListEnterCallback) {
        mOnListEnterCallback(this, &mListInfo);
      }
      RedrawScrolled(false);
    } else {
      mListInfo.ItemIndex = index;
      RecalculateIndices(false);
    }
  }
}


int WndListFrame::OnMouseMove(WPARAM wParam, LPARAM lParam) {  
  static bool bMoving = false;

  if ( (GetTickCount()) >= (unsigned int)LastMouseMoveTime )
  {
    bMoving=true;

    POINT Pos;
    Pos.x = LOWORD(lParam); 
    Pos.y = HIWORD(lParam);                     

    if (mMouseDown && PtInRect(&rcScrollBar, Pos))
    {
      int iScrollBarTop = max(1, (int)Pos.y - mMouseScrollBarYOffset);

      int iScrollIndex = GetScrollIndexFromScrollBarTop(iScrollBarTop);

      if(iScrollIndex !=mListInfo.ScrollIndex)
      {
        int iScrollAmount = iScrollIndex - mListInfo.ScrollIndex;
        mListInfo.ScrollIndex = mListInfo.ScrollIndex + iScrollAmount;
        Redraw();
      }
    }
    else //not in scrollbar
    {
      mMouseDown = false; // force re-click of scroll bar
    }
    LastMouseMoveTime = GetTickCount();
    bMoving=false;
  } // Tickcount
  return(1);
}

int WndListFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam) {  

  POINT Pos;
  Pos.x = LOWORD(lParam); 
  Pos.y = HIWORD(lParam);                     
  mMouseDown=false;
    
  if (PtInRect(&rcScrollBarButton, Pos))  // see if click is on scrollbar handle
  {
    mMouseScrollBarYOffset = max(0, (int)Pos.y - (int)rcScrollBarButton.top);  // start mouse drag
    mMouseDown=true;

  }
  else if (PtInRect(&rcScrollBar, Pos)) // clicked in scroll bar up/down/pgup/pgdn
  {
    if (Pos.y - rcScrollBar.top < (ScrollbarWidth)) // up arrow
      mListInfo.ScrollIndex = max(0, mListInfo.ScrollIndex- 1);
      
    else if (rcScrollBar.bottom -Pos.y < (ScrollbarWidth)  ) //down arrow
      mListInfo.ScrollIndex = max(0,min(mListInfo.ItemCount- mListInfo.ItemInViewCount, mListInfo.ScrollIndex+ 1));

    else if (Pos.y < rcScrollBarButton.top) // page up
      mListInfo.ScrollIndex = max(0, mListInfo.ScrollIndex- mListInfo.ItemInViewCount);

    else // page down
      if (mListInfo.ItemCount > mListInfo.ScrollIndex+ mListInfo.ItemInViewCount)
          mListInfo.ScrollIndex = min ( mListInfo.ItemCount- mListInfo.ItemInViewCount, mListInfo.ScrollIndex +mListInfo.ItemInViewCount);
    
    Redraw();
    
  }
  else
  if (mClientCount > 0)
  {
    isselect = true;
    ((WndFrame *)mClients[0])->OnLButtonDown(wParam, lParam);
  }

  return(1);
}

inline int WndListFrame::GetScrollBarHeight (void)
{
  int h = GetHeight() - ScrollbarTop;
  if(mListInfo.ItemCount ==0)
    return h-2*ScrollbarWidth;
  else
    return max(ScrollbarWidth,((h-2*ScrollbarWidth)*mListInfo.ItemInViewCount)/mListInfo.ItemCount);
}

inline int WndListFrame::GetScrollIndexFromScrollBarTop(int iScrollBarTop)
{
  int h = GetHeight() - ScrollbarTop;
  if (h-2*(ScrollbarWidth) - GetScrollBarHeight() == 0)
    return 0;
  else

    return max(0,
              min(mListInfo.ItemCount - mListInfo.ItemInPageCount,
              max(0, 
                  ( 0 + 
                    (mListInfo.ItemCount-mListInfo.ItemInViewCount) 
                    * (iScrollBarTop - (ScrollbarWidth)-ScrollbarTop) 
                  ) 
                    / ( h-2*(ScrollbarWidth) - GetScrollBarHeight() ) /*-ScrollbarTop(*/
              )
           ));
}

inline int WndListFrame::GetScrollBarTopFromScrollIndex()
{
  int iRetVal=0;
  int h = GetHeight() - ScrollbarTop;
  if (mListInfo.ItemCount - mListInfo.ItemInViewCount ==0) {
    iRetVal= h + (ScrollbarWidth);
  }
  else {
    iRetVal = 
      ( (ScrollbarWidth)+ScrollbarTop + 
        (mListInfo.ScrollIndex) *(h-2*(ScrollbarWidth)-GetScrollBarHeight() ) /*-ScrollbarTop*/
      /(mListInfo.ItemCount - mListInfo.ItemInViewCount)
      );
  }
  return iRetVal;
}


#ifndef ALTAIRSYNC
#include "InputEvents.h"

void WndEventButton_OnClickNotify(WindowControl *Sender) {
  WndEventButton *wb = (WndEventButton*)Sender;
  wb->CallEvent();
}

void WndEventButton::CallEvent() {
  if (inputEvent) {
    inputEvent(parameters);
  }
}

WndEventButton::~WndEventButton() {
  if (parameters) {
    free(parameters);
    parameters=NULL;
  }
}


WndEventButton::WndEventButton(WindowControl *Parent, const TCHAR *Name, 
			       const TCHAR *Caption, 
			       int X, int Y, int Width, int Height, 
			       const TCHAR* ename,
			       const TCHAR* theparameters):
  WndButton(Parent,Name,Caption,X,Y,Width,Height,
	    WndEventButton_OnClickNotify)
{
  inputEvent = InputEvents::findEvent(ename);
  if (theparameters) {
    parameters = _tcsdup(theparameters);
  } else {
    parameters = NULL;
  }

}


// 
// 
#endif
