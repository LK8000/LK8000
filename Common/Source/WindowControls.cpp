/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: WindowControls.cpp,v 8.10 2010/12/13 01:17:08 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "Message.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "RGB.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "Modeltype.h"
#include "TraceThread.h"

#include "Screen/LKBitmapSurface.h"
#include "Screen/LKWindowSurface.h"

#include <functional>

#include "Event/Event.h"
#include "Asset.hpp"

#ifndef USE_GDI
#include "Screen/SubCanvas.hpp"
#endif

using std::placeholders::_1;


#define ISCALE ScreenScale

#define DEFAULTBORDERPENWIDTH 1*ISCALE
#define SELECTORWIDTH         4*ISCALE

// utility functions


#define ENABLECOMBO true // master on/off for combo popup
// Must be off if no touchscreen

// returns true if it is a long press,
// otherwise returns false
static bool KeyTimer(bool isdown, DWORD thekey) {
  static Poco::Timestamp fpsTimeDown = 0;
  static DWORD savedKey=0;

  Poco::Timespan dT = fpsTimeDown.elapsed();
  if ((dT.totalMilliseconds()>2000)&&(thekey==savedKey)) {
    fpsTimeDown.update();
    savedKey = 0;
    return true;
  }

  if (!isdown) {
    // key is released
  } else {
    // key is lowered
    if (thekey != savedKey) {
      fpsTimeDown.update();
      savedKey = thekey;
    }
  }
  return false;
}




void DataFieldFileReader::Clear() {
    for (unsigned int i=1; i<nFiles; i++) {
      if (fields[i].mTextFile) {
        free(fields[i].mTextFile);
        fields[i].mTextFile= NULL;
      }
      if (fields[i].mTextPathFile) {
        free(fields[i].mTextPathFile);
        fields[i].mTextPathFile= NULL;
      }
    }
    nFiles = 1;
    mValue = 0;
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
    const TCHAR* ptr = subdir;
    const TCHAR* ptr2 = buffer + _tcslen(buffer) -1;

    if( (*ptr2=='/')||(*ptr2=='\\') ) {
        if( ((*ptr=='/')||(*ptr=='\\')) ) {
            ++ptr;
        }
    } else {
        if( (*ptr!='/')||(*ptr!='\\') ) {
            _tcscat(buffer, _T(DIRSEP));
        }
    }
	_tcscat(buffer,ptr);
  }
  ScanDirectories(buffer,filter);
  Sort();

}


BOOL DataFieldFileReader::ScanDirectories(const TCHAR* sPath, const TCHAR* filter) {

    TCHAR DirPath[MAX_PATH];
    TCHAR FileName[MAX_PATH];

    if (sPath) {
        _tcscpy(DirPath, sPath);
        _tcscpy(FileName, sPath);
    } else {
        DirPath[0] = 0;
        FileName[0] = 0;
    }

    _tcscat(DirPath, TEXT("\\"));
    _tcscat(FileName, TEXT("\\*"));
    lk::filesystem::fixPath(DirPath);
    lk::filesystem::fixPath(FileName);
    
    for (lk::filesystem::directory_iterator It(FileName); It; ++It) {
        if (It.isDirectory()) {
            _tcscpy(FileName, DirPath);
            _tcscat(FileName, It.getName());
            ScanDirectories(FileName, filter);
        } else if(checkFilter(It.getName(), filter)) {
            _tcscpy(FileName, DirPath);
            _tcscat(FileName, It.getName());
            addFile(It.getName(), FileName);
        }
    }

    return TRUE;
}

bool DataFieldFileReader::Lookup(const TCHAR *Text) {
  int i=0;
  mValue = 0;
  for (i=1; i<(int)nFiles; i++) {    
    // if (_tcscmp(Text,fields[i].mTextPathFile)==0) { 091126
    if (_tcsicmp(Text,fields[i].mTextPathFile)==0) {
      mValue = i;
      return true;
    }
  }
  return false;
}

int DataFieldFileReader::GetNumFiles(void) {
  return nFiles;
}

const TCHAR* DataFieldFileReader::GetPathFile(void) const {
  if ((mValue<=nFiles)&&(mValue)) {
    return fields[mValue].mTextPathFile;
  }
  return TEXT("\0");
}


bool DataFieldFileReader::checkFilter(const TCHAR *filename,
				      const TCHAR *filter) {
  const TCHAR *ptr;
  TCHAR upfilter[MAX_PATH];
  // checks if the filename matches the filter exactly

  if (!filter || (_tcslen(filter+1)==0)) {
    // invalid or short filter, pass
    return true;
  }

  _tcscpy(upfilter,filter+1);

  // check if trailing part of filter (*.exe => .exe) matches end
  ptr = _tcsstr(filename, upfilter);
  if (ptr) {
    if (_tcslen(ptr)==_tcslen(upfilter)) {
      return true;
    }
  }

  CharUpper(upfilter);
  ptr = _tcsstr(filename, upfilter);
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
    if (fields[nFiles].mTextFile) _tcscpy(fields[nFiles].mTextFile, Text); // MALLOC ALERT

    fields[nFiles].mTextPathFile = (TCHAR*)malloc((_tcslen(PText)+1)*sizeof(TCHAR));
    if (fields[nFiles].mTextPathFile) _tcscpy(fields[nFiles].mTextPathFile, PText); // MALLOC ALERT

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


static int DataFieldFileReaderCompare(const void *elem1, 
                                             const void *elem2 ){
  return _tcscmp(((const DataFieldFileReaderEntry*)elem1)->mTextFile,
                 ((const DataFieldFileReaderEntry*)elem2)->mTextFile);
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

void DataField::Clear() { 
    // need to implement in derived class ...
    LKASSERT(false); 
};

void DataField::SetDisplayFormat(const TCHAR *Value){
  LKASSERT(_tcslen(Value)<=FORMATSIZE);
  _tcscpy(mDisplayFormat, Value);
}
void DataField::SetEditFormat(const TCHAR *Value){
  LKASSERT(_tcslen(Value)<=FORMATSIZE);
  _tcscpy(mEditFormat, Value);
}

void DataField::CopyString(TCHAR * szbuffOut, bool bFormatted) {
  int iLen=0;
  if (!bFormatted) 
  {
    if (GetAsString() != NULL) // null leaves iLen=0
    {
      iLen = _tcslen(GetAsString());
      LK_tcsncpy(szbuffOut, GetAsString(), min(iLen, ComboPopupITEMMAX-1));
    }
  }
  else 
  {
    if (GetAsDisplayString() != NULL) 
    {
      iLen = _tcslen(GetAsDisplayString());
      LK_tcsncpy(szbuffOut, GetAsDisplayString(), min(iLen, ComboPopupITEMMAX-1));
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
    Clear();
}

void DataFieldEnum::Clear() {
  for (unsigned int i=0; i<nEnums; i++) {
    if (mEntries[i].mText) {
      free(mEntries[i].mText);
      mEntries[i].mText= NULL;
    }
  }
  nEnums = 0;
  mValue = 0;
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
    if (mEntries[nEnums].mText == NULL) return; // MALLOC ALERT
    _tcscpy(mEntries[nEnums].mText, Text);
    mEntries[nEnums].index = nEnums;
    nEnums++;
  }
}

void DataFieldEnum::addEnumTextNoLF(const TCHAR *Text) {
  if (nEnums<DFE_MAX_ENUMS-1) {
    TCHAR *p;
    mEntries[nEnums].mText = (TCHAR*)malloc((_tcslen(Text)+1)*sizeof(TCHAR));
    if (mEntries[nEnums].mText == NULL) return;
    _tcscpy(mEntries[nEnums].mText, Text);
    p = _tcschr(mEntries[nEnums].mText, _T('\n'));
    if (p)
	*p = _T(' ');
    
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

static int DataFieldEnumCompare(const void *elem1, 
                                             const void *elem2 ){
  return _tcscmp(((const DataFieldEnumEntry*)elem1)->mText,
                 ((const DataFieldEnumEntry*)elem2)->mText);
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
  SetAsInteger(_tcstol(Value, NULL, 10));
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


  if (GetDisableSpeedUp() == true) 
    return 1;

  if (keyup != DataFieldKeyUp) {
    mSpeedup = 0;
    DataFieldKeyUp = keyup;
    mTmLastStep.update();
    return 1;
  }

  if (!mTmLastStep.isElapsed(Poco::Timespan(0,200*1000).totalMicroseconds())){
    mSpeedup++;

    if (mSpeedup > 5){
      res = 10;

      mTmLastStep.update(); // now + 350ms
      mTmLastStep += Poco::Timespan(0, 350*1000).totalMicroseconds();
      return(res);

    }
  } else
    mSpeedup = 0;

  mTmLastStep.update();

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
double DataFieldFloat::SetStep(double Value){
  double res = mStep;
  mStep = Value;
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


  if (GetDisableSpeedUp() == true) 
    return 1;

  if (keyup != DataFieldKeyUp) {
    mSpeedup = 0;
    DataFieldKeyUp = keyup;
    mTmLastStep.update();
    return 1.0;
  }

  if (!mTmLastStep.isElapsed(Poco::Timespan(0,200*1000).totalMicroseconds())){
    mSpeedup++;

    if (mSpeedup > 5){
      res = 10;

      mTmLastStep.update(); // now + 350ms
      mTmLastStep += Poco::Timespan(0, 350*1000).totalMicroseconds();
      return(res);

    }
  } else
    mSpeedup = 0;

  mTmLastStep.update();

  return(res);
}

int DataFieldFloat::SetFromCombo(int iDataFieldIndex, TCHAR *sValue) {
  SetAsString(sValue);
  return 0;
}

bool DataFieldFloat::CreateKeyboard(void){
	TCHAR szText[20];
	_tcscpy(szText, GetAsString());
	dlgNumEntryShowModal(szText,20, false);

	TCHAR*szStop;
	SetAsFloat(floor((StrToDouble(szText, &szStop)/mStep)+0.5)*mStep);

	return true;
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
                                        const TCHAR *StringValue,
                                        const TCHAR *StringValueFormatted)
{
  int iLen = -1;
  ComboListEntry_t * theItem;

  // Copy current strings into structure
  theItem = (ComboListEntry_t*) malloc(sizeof(ComboListEntry_t));
  LKASSERT(theItem!=NULL); 
  theItem->DataFieldIndex=0;  // NULL is same as 0, so it fails to set it if index value is 0
  theItem->ItemIndex=0;
  
  theItem->ItemIndex=ItemIndex;

  if (DataFieldIndex != ComboPopupNULL) { // optional
    theItem->DataFieldIndex=DataFieldIndex;
  }

  if (StringValue == NULL)
  {
    theItem->StringValue = (TCHAR*)malloc((1) * sizeof(TCHAR));
    LKASSERT(theItem->StringValue != NULL); 
    theItem->StringValue[0]='\0';
  }
  else
  {
    iLen = _tcslen(StringValue);
    theItem->StringValue = (TCHAR*)malloc((iLen + 1) * sizeof(TCHAR));
    LKASSERT(theItem->StringValue != NULL);
    _tcscpy(theItem->StringValue, StringValue);
  }


  // copy formatted display string
  if (StringValueFormatted == NULL) 
  {
    theItem->StringValueFormatted = (TCHAR*)malloc((1) * sizeof(TCHAR));
    LKASSERT(theItem->StringValueFormatted != NULL);
    theItem->StringValueFormatted[0]='\0';
  }
  else
  {
    iLen = _tcslen(StringValueFormatted);
    theItem->StringValueFormatted = (TCHAR*)malloc((iLen + 1) * sizeof(TCHAR));
    LKASSERT(theItem->StringValueFormatted != NULL);
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


WindowControl *ActiveControl = NULL;
WindowControl *LastFocusControl = NULL;


void InitWindowControlModule(void);

static LKColor bkColor = RGB_WINBACKGROUND; // PETROL
static LKColor fgColor = RGB_WINFOREGROUND; // WHITE
int WindowControl::InstCount=0;

PenReference WindowControl::hPenDefaultBorder;
PenReference WindowControl::hPenDefaultSelector;

WindowControl::WindowControl(WindowControl *Owner, const TCHAR *Name, 
			     int X, int Y, int Width, int Height, bool Visible)
                : WndCtrlBase(Name)
{

  mHelpText = NULL;

  mCanFocus = false;

  mReadOnly = false;

  mOnHelpCallback = NULL;

  mX = X;
  mY = Y;
  mWidth = Width;
  mHeight = Height;

  mOwner = Owner?Owner->GetClientArea():NULL;
  // setup Master Window (the owner of all)
  mTopOwner = NULL;
  if(Owner) {
    mTopOwner = Owner->GetTopOwner();
  }
  if(!mTopOwner) {
    mTopOwner = Owner;
  }
    
  // todo
  mhFont = MapWindowFont;
  mDontPaintSelector = false;

  InitWindowControlModule();

  mColorBack = bkColor; // PETROL
  mColorFore = fgColor; // WHITE

  if (InstCount == 0){
	hPenDefaultBorder = LKPen_White_N1;
	hPenDefaultSelector = LKPen_Petrol_C2;
  }
  InstCount++;

  // if Owner is Not provided, use MainWindow
  ContainerWindow* WndOnwer = Owner
          ?static_cast<ContainerWindow*>(Owner->GetClientArea())
          :static_cast<ContainerWindow*>(&MainWindow);
  
  if(mOwner) {
    mOwner->CalcChildRect(mX, mY, mWidth, mHeight);
  }
  LKASSERT(mX+mWidth>0);
  
  Create(WndOnwer,(RECT){mX, mY, mX+mWidth, mY+mHeight});
  SetTopWnd();

  if (mOwner != NULL)
    mOwner->AddClient(this);  

  mhPenBorder = hPenDefaultBorder;
  mhPenSelector = hPenDefaultSelector;
  mBorderSize = 1;

  mBorderKind = 0; //BORDERRIGHT | BORDERBOTTOM;

  if (Visible) {
    SetVisible(true);
  }
}

WindowControl::~WindowControl(void){
  if (mHelpText) {
    free(mHelpText);
    mHelpText = NULL;
  }
}

void WindowControl::Destroy(void){
    for(WindowControl* pCtrl : mClients) {
        pCtrl->Destroy();
        delete pCtrl;
    }
    mClients.clear();

  if (LastFocusControl == this)
    LastFocusControl = NULL;

  if (ActiveControl == this)
    ActiveControl = NULL;

  WndCtrlBase::Destroy();

  InstCount--;

}

void WindowControl::OnSetFocus() {
    WndCtrlBase::OnSetFocus();
    Redraw();
    ActiveControl = this;
    LastFocusControl = this;    
}

void WindowControl::OnKillFocus() {
    WndCtrlBase::OnKillFocus();
    Redraw();
    ActiveControl = NULL;
}

void WindowControl::UpdatePosSize(void){

  RECT WndRect = (RECT){0,0, GetWidth(),GetHeight()};
  OffsetRect(&WndRect, mX, mY);
  Move(WndRect, true);
}

bool WindowControl::OnPaint(LKSurface& Surface, const RECT& Rect) {
#ifndef USE_GDI
    Paint(Surface);
#else
    const RECT Client_Rect = GetClientRect();

    int win_width = Client_Rect.right - Client_Rect.left;
    int win_height = Client_Rect.bottom + Client_Rect.left;

    LKBitmapSurface MemSurface(Surface, win_width, win_height);
    Paint(MemSurface);

#warning "TODO : Exclude client rect"
/*
    ptOffset.x = Client_Rect.left;
    ptOffset.y = Client_Rect.top;
    ClientToScreen(hWnd, &ptOffset);
    if ((hChildWnd = GetWindow(hWnd, GW_CHILD)) != NULL) {
        if (IsWindowVisible(hChildWnd)) {
            GetWindowRect(hChildWnd, &Client_Rect);
            OffsetRect(&Client_Rect, -ptOffset.x, -ptOffset.y);
            PaintSurface.ExcludeClipRect(Client_Rect);
        }
    }
*/
    Surface.Copy(0, 0, win_width, win_height, MemSurface, 0, 0);
#endif
    return true;
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

WindowControl *WindowControl::GetCanFocus(void) {
    if (!IsVisible()) {
        return NULL;
    }
    if (mCanFocus && !mReadOnly) {
        return (this);
    }

    for (WindowControl *w : mClients) {
        if (w && w->GetCanFocus()) {
            return (w);
        }
    }
    return (NULL);
};

void WindowControl::CalcChildRect(int& x, int& y, int& cx, int& cy) const {
    
    // use negative value to space down items
    // -999 to stay on the same line
    // -998 to advance one line with more space
    // -997 to advance one line with twice more space

    if (y < 0 && !mClients.empty()) {
        WindowControl * pPrev = mClients.back();
        switch (y) {
            case -999: //@ 101008
                y = pPrev->GetTop();
                break;
            case -998: //@ 101115
                y = (pPrev->GetTop() + pPrev->GetHeight() + NIBLSCALE(3));
                break;
            case -997: //@ 101115
                y = (pPrev->GetTop() + pPrev->GetHeight() + NIBLSCALE(6));
                break;
            default:
                y = (pPrev->GetTop() - ((pPrev->GetHeight()) * y));
                break;
        }
    }    
    if(y<0) y = 0;
    if(x<0) x = 0;
    
    // negative value for cx is right margin relative to parent;
    if (cx<0) {
        cx = GetWidth() - x + cx;
    }
    // negative value for cy is bottom margin relative to parent;
    if (cy<0) {
        cy = GetHeight() - y + cy;
    }
    LKASSERT(cx>0);
    LKASSERT(cy>0);
}

void WindowControl::AddClient(WindowControl *Client) {

    Client->SetFont(GetFont());
    mClients.push_back(Client);
}

// 110411 This is always set true because we don't use UserLevel anymore

void WindowControl::FilterAdvanced(bool advanced) {
    if (_tcsstr(GetWndText(), TEXT("*")) != NULL) {
        SetVisible(advanced);
    }
    for (WindowControl* w : mClients) {
        w->FilterAdvanced(advanced);
    }
}

WindowControl *WindowControl::FindByName(const TCHAR *Name) {
    if (_tcscmp(GetWndName(), Name) == 0) {
        return (this);
    }

    for (WindowControl* w : mClients) {
        WindowControl* res = w->FindByName(Name);
        if (res) {
            return (res);
        }
    }
    return (NULL);
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

void WindowControl::SetCaption(const TCHAR *Value) {
    const TCHAR* szCaption = GetWndText();
    if (Value == NULL && szCaption[0] != _T('\0')) {
        SetWndText(_T(""));
        Redraw();
    } else if (_tcscmp(szCaption, Value) != 0) {
        SetWndText(Value);
        Redraw();
    }
}

bool WindowControl::SetCanFocus(bool Value){
  bool res = mCanFocus;
  mCanFocus = Value;
  return(res);
}

int WindowControl::GetBorderKind(void){
  return(mBorderKind);
}

int WindowControl::SetBorderKind(int Value){
  int res = mBorderKind;
  if (mBorderKind != Value){
    mBorderKind = Value;
    Redraw();
  }
  return(res);
}

void WindowControl::SetFont(FontReference Value){
  mhFont = Value;
}

bool WindowControl::SetReadOnly(bool Value){
  bool res = mReadOnly;
  if (mReadOnly != Value){
    mReadOnly = Value;

    Redraw();
  }
  return(res);
}

LKColor WindowControl::SetForeColor(const LKColor& Value) {
    LKColor res = mColorFore;
    if (mColorFore != Value) {
        mColorFore = Value;
        if (IsVisible()) {
            Redraw();
        }
    }
    return res;
}

LKColor WindowControl::SetBackColor(const LKColor& Value){
  LKColor res = mColorBack;
  if (mColorBack != Value){
	mColorBack = Value;
	mBrushBk.Create(mColorBack);
	if (IsVisible()){
        Redraw();
	}
  }
  return res;
}

void WindowControl::PaintBorder(LKSurface& Surface) {
    
  if (mBorderKind != 0){

    const auto oldPen = Surface.SelectObject(mhPenBorder);

    if (mBorderKind & BORDERTOP){
      Surface.DrawLine(0,0, mWidth, 0);
    }
    if (mBorderKind & BORDERRIGHT){
      Surface.DrawLine(mWidth-1, 0, mWidth-1, mHeight);
    }
    if (mBorderKind & BORDERBOTTOM){
      Surface.DrawLine(mWidth-1, mHeight-1, -1, mHeight-1);
    }
    if (mBorderKind & BORDERLEFT){
      Surface.DrawLine(0, mHeight-1, 0, -1);
    }
    
    Surface.SelectObject(oldPen);
  }
}

void WindowControl::PaintSelector(LKSurface& Surface){

  if (!mDontPaintSelector && mCanFocus && HasFocus()){
    const auto oldPen = Surface.SelectObject(hPenDefaultSelector);

    Surface.DrawLine(
	      mWidth-SELECTORWIDTH-1, 0,
	      mWidth-1, 0,
	      mWidth-1, SELECTORWIDTH+1);

    Surface.DrawLine(
	      mWidth-1, mHeight-SELECTORWIDTH-2,
	      mWidth-1, mHeight-1,
	      mWidth-SELECTORWIDTH-1, mHeight-1);

    Surface.DrawLine(
	      SELECTORWIDTH+1, mHeight-1, 
	      0, mHeight-1,
	      0, mHeight-SELECTORWIDTH-2);

    Surface.DrawLine(
	      0, SELECTORWIDTH+1,
	      0, 0,
	      SELECTORWIDTH+1, 0);

    Surface.SelectObject(oldPen);
  }

}

extern void dlgHelpShowModal(const TCHAR* Caption, const TCHAR* HelpText);


int WindowControl::OnHelp() {
    if (mHelpText) {
      dlgHelpShowModal(GetWndText(), mHelpText);
      return(1);
    } else {
      if (mOnHelpCallback) {
	(mOnHelpCallback)(this);
	return(1);
      } else {
	return(0);
      }
    }
};

void WindowControl::Paint(LKSurface& Surface) {

    if (!IsVisible()) return;

    Surface.SetBackgroundTransparent();
    RECT rc = GetClientRect();
    rc.right += 2;
    rc.bottom += 2;

    Surface.FillRect(&rc, GetBackBrush());

    // JMW added highlighting, useful for lists
    if (!mDontPaintSelector && mCanFocus && HasFocus()) {
        rc.right -= 2;
        rc.bottom -= 2;
        Surface.FillRect(&rc, LKBrush_Higlighted);
    }
    PaintBorder(Surface);
    PaintSelector(Surface);
}

WindowControl *WindowControl::FocusNext(WindowControl *Sender) {
    WindowControl *W = NULL;
    if (Sender != NULL) {
        auto It = std::find(mClients.begin(), mClients.end(), Sender);
        if(It != mClients.end()) {
            ++It;
        }
        for( ; !W && It != mClients.end();  ++It) {
            W = (*It)->GetCanFocus();
        }
    } else if(!mClients.empty()) {
        W = mClients.front()->GetCanFocus();
    }

    if (W) {
        W->SetFocus();
        return W;
    }

    if (GetParent() != NULL) {
        return (GetParent()->FocusNext(this));
    }

    return (NULL);
}

WindowControl *WindowControl::FocusPrev(WindowControl *Sender){
    WindowControl *W = NULL;
    if (Sender != NULL) {
        auto It = std::find(mClients.rbegin(), mClients.rend(), Sender);
        if (It != mClients.rend()) {
            ++It;
        }
        for( ; !W && It != mClients.rend();  ++It) {
            W = (*It)->GetCanFocus();
        }
    } else if(!mClients.empty()) {
        W = mClients.back()->GetCanFocus();
    }

    if (W) {
        W->SetFocus();
        return W;
    }

    if (GetParent() != NULL) {
        return (GetParent()->FocusPrev(this));
    }

    return (NULL);
}

void InitWindowControlModule(void){

  static bool InitDone = false;

  if (InitDone)
    return;

  ActiveControl = NULL;

  InitDone = true;

}

WndForm::WndForm(const TCHAR *Name, const TCHAR *Caption, 
                 int X, int Y, int Width, int Height):
  WindowControl(NULL, Name, X, Y, Width, Height, false) {

  mClientWindow = NULL;
  
  mOnTimerNotify = NULL;
  mOnKeyDownNotify = NULL;
  mOnKeyUpNotify = NULL;

  bLButtonDown= false; 

  mColorTitle = RGB_MENUTITLEBG;

  SetBorderPen(LKPen_Black_N1);
  mhTitleFont = GetFont();

  mhBrushTitle = LKBrush_Black; // 101204

  mClientRect.top=0;
  mClientRect.left=0;
  mClientRect.bottom=Width;
  mClientRect.right=Height;

  mTitleRect.top=0;
  mTitleRect.left=0;
  mTitleRect.bottom=0;
  mTitleRect.right=Height;
  
  if (Caption != NULL) {
    SetCaption(Caption);
    size_t nChar = _tcslen(Caption);
    if(nChar > 0) {
        SIZE tsize = {0,0};
        LKWindowSurface Surface(*this);
        Surface.SelectObject(mhTitleFont);
        Surface.GetTextSize(Caption, nChar, &tsize);

        mTitleRect.bottom = mTitleRect.top + tsize.cy;
        mClientRect.top = mTitleRect.bottom+NIBLSCALE(1)-1;;
    }
  }  
  
  mClientWindow = new WindowControl(this, TEXT(""), mClientRect.top, 0, Width, Height-mClientRect.top);
  mClientWindow->SetBackColor(RGB_WINBACKGROUND);
  mClientWindow->SetCanFocus(false);

  mModalResult = 0;
};

WndForm::~WndForm(void){
  Destroy();
}

void WndForm::Destroy(void) {
    if (mClientWindow) {
        mClientWindow->SetVisible(false);
    }
    WindowControl::Destroy(); // delete all childs
}

void WndForm::AddClient(WindowControl *Client){      // add client window
  if (mClientWindow != NULL){
    mClientWindow->AddClient(Client); // add it to the clientarea window
  } else
    WindowControl::AddClient(Client);
}

FontReference WndForm::SetTitleFont(FontReference Value){
  FontReference res = mhTitleFont;
  mhTitleFont = Value;
  return (res);
}

static bool
IsSpecialKey(unsigned key_code)
{
  return key_code == KEY_LEFT || key_code == KEY_RIGHT ||
    key_code == KEY_UP || key_code == KEY_DOWN ||
    key_code == KEY_TAB || key_code == KEY_RETURN || key_code == KEY_ESCAPE;
}

/**
 * Is this key handled by the focused control? (bypassing the dialog
 * manager)
 */
gcc_pure
static bool
CheckKey(Window *container, const Event &event)
{
#ifdef WIN32
  const MSG &msg = event.msg;
  LRESULT r = ::SendMessage(msg.hwnd, WM_GETDLGCODE, msg.wParam,
                            (LPARAM)&msg);
  return (r & DLGC_WANTMESSAGE) != 0;
#else
  Window *focused = container->GetFocusedWindow();
  if (focused == NULL)
    return false;

  return focused->OnKeyCheck(event.GetKeyCode());
#endif
}

/**
 * Is this "special" key handled by the focused control? (bypassing
 * the dialog manager)
 */
gcc_pure
static bool
CheckSpecialKey(Window *container, const Event &event)
{
  return IsSpecialKey(event.GetKeyCode()) && CheckKey(container, event);
}


int WndForm::ShowModal(void) {
    MainWindow.UnGhost();

#define OPENCLOSESUPPRESSTIME Poco::Timespan(0,500*1000).totalMicroseconds()
    SHOWTHREAD(_T("ShowModal"));

    enterTime.update();

    Message::BlockRender(true);

    SetVisible(true);

    SetToForeground();

    mModalResult = 0;

    Window* oldFocus = Window::GetFocusedWindow();
    FocusNext(NULL);

#ifndef USE_GDI
    MainWindow.Refresh();
#else
    Redraw();
#endif    

    LKASSERT(event_queue);
#if defined(ANDROID) || defined(USE_CONSOLE) || defined(ENABLE_SDL) || defined(NON_INTERACTIVE)
    EventLoop loop(*event_queue, MainWindow);
#else
    DialogEventLoop loop(*event_queue, Handle());
#endif
    Event event;
    while (mModalResult == 0 && loop.Get(event)) {
        if (!MainWindow.FilterEvent(event, this)) {
            continue;
        }

        if (event.IsKeyDown()) {
            if (
#ifdef WIN32
                    IdentifyDescendant(event.msg.hwnd) &&
#endif
                    !CheckSpecialKey(this, event))
                continue;

#ifdef ENABLE_SDL
            if (event.GetKeyCode() == SDLK_TAB) {
                /* the Tab key moves the keyboard focus */
#if SDL_MAJOR_VERSION >= 2
                const Uint8 *keystate = ::SDL_GetKeyboardState(NULL);
                event.event.key.keysym.sym =
                        keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT]
                        ? SDLK_UP : SDLK_DOWN;
#else
                const Uint8 *keystate = ::SDL_GetKeyState(NULL);
                event.event.key.keysym.sym =
                        keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT]
                        ? SDLK_UP : SDLK_DOWN;
#endif
            }
#endif

#ifdef _WIN32_WCE
            /* The Windows CE dialog manager does not handle KEY_ESCAPE and
               so we have to do it by ourself */

            // On Altair, the RemoteKey ("E" Button) shall also close the analyse-page
            if (IsAltair()) {
                if (event.GetKeyCode() == KEY_ESCAPE || event.GetKeyCode() == KEY_F15) {
                    mModalResult = mrOK;
                    continue;
                }
            } else 
#endif

            if (event.GetKeyCode() == KEY_ESCAPE) {
                mModalResult = mrCancel;
                continue;
            }

#ifdef USE_LINUX_INPUT
            if (event.GetKeyCode() == KEY_POWER) {
                /* the Kobo power button closes the modal dialog */
                OnAnyKeyDown = mrCancel;
                continue;
            }
#endif
        }

        loop.Dispatch(event);
    } // End Modal Loop

    if(oldFocus) {
        oldFocus->SetFocus();
    }
    MainWindow.UnGhost();

    MapWindow::RequestFastRefresh();
    Message::BlockRender(false);

    return (mModalResult);
}

void WndForm::Paint(LKSurface& Surface){

    if (!IsVisible()) return;

    RECT rcClient = GetClientRect();
    if(GetBorderKind()&BORDERLEFT) {
        rcClient.left += 1;
    }
    if(GetBorderKind()&BORDERRIGHT) {
        rcClient.right -= 1;
    }
    if(GetBorderKind()&BORDERBOTTOM) {
        rcClient.bottom -= 1;
    }

    const TCHAR * szCaption = GetWndText();
    size_t nChar = _tcslen(szCaption);
    if(nChar > 0) {
        SIZE tsize = {0,0};

        Surface.SetTextColor(RGB_MENUTITLEFG);
        Surface.SetBkColor(mColorTitle);
        Surface.SetBackgroundTransparent();
        
        const auto oldPen = Surface.SelectObject(GetBorderPen());
        const auto oldBrush = Surface.SelectObject(GetBackBrush());
        const auto oldFont = Surface.SelectObject(mhTitleFont);
        
        Surface.GetTextSize(szCaption, nChar, &tsize);

        mTitleRect = rcClient;
        mTitleRect.bottom = mTitleRect.top + tsize.cy;

        Surface.FillRect(&mTitleRect, mhBrushTitle);
        
        POINT p1, p2;
        p1.x=0; p1.y=mTitleRect.bottom;
        p2.x=mTitleRect.right; p2.y=mTitleRect.bottom;
        Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p1, p2, RGB_GREEN, mTitleRect);

        rcClient.top = mTitleRect.bottom+NIBLSCALE(1)-1;

        if (mClientWindow && !EqualRect(&mClientRect, &rcClient)){
            mClientWindow->Move(rcClient, true);
            mClientWindow->SetTopWnd();
            mClientRect = rcClient;
        }

        Surface.DrawText(mTitleRect.left+NIBLSCALE(2), mTitleRect.top, szCaption, nChar);

        Surface.SelectObject(oldBrush);
        Surface.SelectObject(oldPen);
        Surface.SelectObject(oldFont);
    } else {
        if(GetBorderKind()&BORDERTOP) {
            rcClient.top += 1;
        }
        if (mClientWindow && !EqualRect(&mClientRect, &rcClient)){
            mClientWindow->Move(rcClient, true);
            mClientWindow->SetTopWnd();
            mClientRect = rcClient;
        }
    }
    PaintBorder(Surface);
}

void WndForm::SetCaption(const TCHAR *Value) {
    const TCHAR* szCaption = GetWndText();
    if (Value == NULL && szCaption[0] != _T('\0')) {
        SetWndText(_T(""));
        Redraw();
    } else if (_tcscmp(szCaption, Value) != 0) {
        SetWndText(Value);
        Redraw(mTitleRect);
    }
}


LKColor WndForm::SetForeColor(const LKColor& Value){
  if (mClientWindow)
    mClientWindow->SetForeColor(Value);
  return(WindowControl::SetForeColor(Value));
}

LKColor WndForm::SetBackColor(const LKColor& Value){
  if (mClientWindow)
  mClientWindow->SetBackColor(Value);
  return(WindowControl::SetBackColor(Value));
}

void WndForm::SetFont(FontReference Value){
    if (mClientWindow) {
        mClientWindow->SetFont(Value);
    }
    WindowControl::SetFont(Value);
}

void WndForm::Show() {
    MainWindow.UnGhost();
    
    WindowControl::Show();
    SetToForeground();
}

bool WndForm::OnKeyDownNotify(Window* pWnd, unsigned KeyCode) {
    if (mOnKeyDownNotify && (mOnKeyDownNotify) (this, KeyCode)) {
        return true;
    }
    if (ActiveControl) {
        WindowControl * pCtrl = ActiveControl->GetParent();
        if (pCtrl) {
            switch (KeyCode & 0xffff) {
                case KEY_UP:
                    pCtrl->FocusPrev(ActiveControl);
                    break;
                case KEY_DOWN:
                    pCtrl->FocusNext(ActiveControl);
                    break;
            }
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------
// WndButton
//-----------------------------------------------------------

WndButton::WndButton(WindowControl *Parent, const TCHAR *Name, const TCHAR *Caption, int X, int Y, int Width, int Height, ClickNotifyCallback_t Function):
      WindowControl(Parent, Name, X, Y, Width, Height){

  SetOnClickNotify(Function);
  mDown = false;
  mDefault = false;
  mCanFocus = true;

  SetForeColor(RGB_BUTTONFG);
  SetBackColor(GetParent()->GetBackColor());

  if(Caption) {
      SetCaption(Caption);
  }
  mLastDrawTextHeight = -1;
}

bool WndButton::OnKeyDown(unsigned KeyCode) {
    switch (KeyCode) {
        case KEY_RETURN:
        case KEY_SPACE:
            if (!mDown) {
                mDown = true;
                Redraw();
                return true;
            }
    }
    return false;
}

bool WndButton::OnKeyUp(unsigned KeyCode) {
    switch (KeyCode) {
        case KEY_RETURN:
        case KEY_SPACE:
            if (!Debounce()) return (1); // prevent false trigger
            if (mDown) {
                mDown = false;
                Redraw();
                if (mOnClickNotify != NULL) {
                    (mOnClickNotify) (this);
                    return true;
                }
            }
    }
    return false;
}

bool WndButton::OnLButtonDown(const POINT& Pos) {
    mDown = true;
    SetCapture();
    if (!HasFocus()) {
        SetFocus();
    } else {
        Redraw();
    }
    return true;
}

bool WndButton::OnLButtonUp(const POINT& Pos) {
    bool bTmp = mDown;
    mDown = false;
    ReleaseCapture();
    Redraw();
    if (bTmp && mOnClickNotify != NULL) {    
        RECT rcClient = GetClientRect();
        if (PtInRect(&rcClient, Pos)) {
            (mOnClickNotify) (this);
        }
    }
    return true;
}

bool WndButton::OnLButtonDblClick(const POINT& Pos) {
    mDown = true;
    Redraw();
    return true;
}

void WndButton::Paint(LKSurface& Surface){

  if (!IsVisible()) return;

  WindowControl::Paint(Surface);

  RECT rc = GetClientRect();
  InflateRect(&rc, -2, -2); // todo border width

  // JMW todo: add icons?

  Surface.DrawPushButton(rc, mDown);

  const TCHAR * szCaption = GetWndText();
  const size_t nSize = _tcslen(szCaption);
  if (nSize > 0) {

    Surface.SetTextColor(IsDithered()?
            (mDown ? RGB_WHITE : RGB_BLACK) : 
            GetForeColor());

    Surface.SetBkColor(IsDithered()?
            (mDown ? RGB_BLACK : RGB_WHITE) : 
            GetBackColor());
    
    Surface.SetBackgroundTransparent();

    const auto oldFont = Surface.SelectObject(GetFont());
    const RECT rcClient = GetClientRect();
    rc = rcClient;
    InflateRect(&rc, -2, -2); // todo border width

    if (mDown)
      OffsetRect(&rc, 2, 2);

    if (mLastDrawTextHeight < 0){

      Surface.DrawText(szCaption, nSize, &rc,
          DT_CALCRECT
        | DT_EXPANDTABS
        | DT_CENTER
        | DT_NOCLIP
        | DT_WORDBREAK // mCaptionStyle // | DT_CALCRECT
      );

      mLastDrawTextHeight = rc.bottom - rc.top;
      // DoTo optimize
      rc = rcClient;
      InflateRect(&rc, -2, -2); // todo border width
      if (mDown)
        OffsetRect(&rc, 2, 2);

    }

    rc.top += ((GetHeight()-4-mLastDrawTextHeight)/2);

    Surface.DrawText(szCaption, nSize, &rc,
        DT_EXPANDTABS
      | DT_CENTER
      | DT_NOCLIP
      | DT_WORDBREAK // mCaptionStyle // | DT_CALCRECT
    );

    Surface.SelectObject(oldFont);
  }
}

WndProperty::WndProperty(WindowControl *Parent, 
			 TCHAR *Name, 
			 TCHAR *Caption, 
			 int X, int Y, 
			 int Width, int Height, 
			 int CaptionWidth, 
			 DataChangeCallback_t DataChangeNotify,
			 int MultiLine):
  WindowControl(Parent, Name, X, Y, Width, Height){

  mOnDataChangeNotify = DataChangeNotify;
  if(Caption) {
      SetCaption(Caption);
  }

  mDataField = NULL;
  mDialogStyle=false; // this is set by ::SetDataField()

  mUseKeyboard=false;
  mMultiLine = MultiLine;
          
  mhValueFont = GetFont();
  mCaptionWidth = CaptionWidth;

  mBitmapSize = DLGSCALE(32)/2;
  if (mDialogStyle)
    mBitmapSize = 0;

  UpdateButtonData(mBitmapSize);

    mCanFocus = true;

    SetForeColor(GetParent()->GetForeColor());
    SetBackColor(GetParent()->GetBackColor());

    mDownDown = false;
    mUpDown = false;

};


WndProperty::~WndProperty(void){
}

void WndProperty::Destroy(void){

  if (mDataField != NULL){
    if (!mDataField->Unuse()) {
      delete mDataField;
      mDataField = NULL;
    } else {
      //ASSERT(0);
    }
  }
  WindowControl::Destroy();
}

void WndProperty::SetText(const TCHAR *Value) {
    if ( !Value && mValue != _T("")) {
        mValue.clear();
        Redraw();
    } else if (Value && mValue != Value) {
        mValue = Value;
        Redraw();
    }
}

void WndProperty::SetFont(FontReference Value) {
    WindowControl::SetFont(Value);
    mhValueFont = Value;
}

void WndProperty::UpdateButtonData(int Value){

  if (Value == 0) // if combo is enabled
    mBitmapSize = 0;
  else
    mBitmapSize = DLGSCALE(32)/2;  

  if (mCaptionWidth != 0){
    mEditRect.left = mCaptionWidth;
    mEditRect.top = (DEFAULTBORDERPENWIDTH+1);

    mEditRect.right = mEditRect.left + GetWidth()- mCaptionWidth - (DEFAULTBORDERPENWIDTH+1) - mBitmapSize;
    mEditRect.bottom = mEditRect.top + GetHeight()-2*(DEFAULTBORDERPENWIDTH+1);
  } else {
    mEditRect.left = mBitmapSize + (DEFAULTBORDERPENWIDTH+2);
    mEditRect.top = (GetHeight()/2)-2*(DEFAULTBORDERPENWIDTH+1);
    mEditRect.right = mEditRect.left + GetWidth()- 2*((DEFAULTBORDERPENWIDTH+1)+mBitmapSize);
    mEditRect.bottom = mEditRect.top + (GetHeight()/2);
  }

  mHitRectDown.left = mEditRect.left-mBitmapSize;
  mHitRectDown.top = mEditRect.top + (mEditRect.top)/2 - (mBitmapSize/2);
  mHitRectDown.right = mHitRectDown.left + mBitmapSize;
  mHitRectDown.bottom = mHitRectDown.top + mBitmapSize;

  mHitRectUp.left = GetWidth()-(mBitmapSize+2);
  mHitRectUp.top = mHitRectDown.top;
  mHitRectUp.right = mHitRectUp.left + mBitmapSize;
  mHitRectUp.bottom = mHitRectUp.top + mBitmapSize;

}

bool WndProperty::SetUseKeyboard(bool Value) {
	return mUseKeyboard=Value;
}

int WndProperty::SetButtonSize(int Value){
  int res = mBitmapSize;

  if (mBitmapSize != Value){
    UpdateButtonData(Value);

    if (IsVisible()){
        Redraw();
    }
  }
  return(res);
};

bool WndProperty::SetReadOnly(bool Value){
  bool res = GetReadOnly();
  if (GetReadOnly() != Value){
    WindowControl::SetReadOnly(Value);
  }
  return(res);
}

bool WndProperty::OnKeyDown(unsigned KeyCode) {
    switch (KeyCode) {
        case KEY_RIGHT:
            IncValue();
            return true;
        case KEY_LEFT:
            DecValue();
            return true;
        case KEY_RETURN:
            if (this->mDialogStyle) {
                if (OnLButtonDown((POINT) {0, 0})) {
                    return true;
                }
            } else {
                if (KeyTimer(true, KeyCode & 0xffff) && OnHelp()) {
                    return true;
                }
            }
        default:
            break;
    }
    return false;
}

bool WndProperty::OnKeyUp(unsigned KeyCode) {
    if (KeyTimer(false, KeyCode&0xffff)) {
        // activate tool tips if hit return for long time
        if ((KeyCode&0xffff) == KEY_RETURN) {
            if (OnHelp()) return true;
        }
    } else if ((KeyCode&0xffff) == KEY_RETURN) {
        if (CallSpecial()) return true;
    }
    return false;
}

extern BOOL dlgKeyboard(WndProperty* theProperty);

bool WndProperty::OnLButtonDown(const POINT& Pos){
  if (mDialogStyle)
  {
    if (!GetReadOnly())  // when they click on the label
    {
    	if(!mUseKeyboard || !dlgKeyboard(this)) {
   	       dlgComboPicker(this);
    	}
    }
    else 
    {
      OnHelp(); // this would display xml file help on a read-only wndproperty if it exists
    }
  }
  else
  {

    if (!HasFocus()) {
      SetFocus();
    }  else {

        mDownDown = (PtInRect(&mHitRectDown, Pos) != 0);

        if (mDownDown) {
          DecValue();
          Redraw(mHitRectDown);
        }

        mUpDown = (PtInRect(&mHitRectUp, Pos) != 0);

        if (mUpDown) {
          IncValue();
          Redraw(mHitRectUp);
        }
    }
  }
  return true;
};

bool WndProperty::OnLButtonDblClick(const POINT& Pos){

  return (OnLButtonDown(Pos));

}

bool WndProperty::OnLButtonUp(const POINT& Pos) {

    if (!mDialogStyle) {
        if (mDownDown) {
            mDownDown = false;
            Redraw(mHitRectDown);
        }
        if (mUpDown) {
            mUpDown = false;
            Redraw(mHitRectUp);
        }
    }
    return true;
}

int WndProperty::CallSpecial(void) {
    if (mDataField != NULL) {
        mDataField->Special();
        SetText(mDataField->GetAsString());
    }
    return (0);
}

int WndProperty::IncValue(void){
  if (mDataField != NULL){
    mDataField->Inc();
    SetText(mDataField->GetAsString());
  }
  return(0);
}

int WndProperty::DecValue(void){
  if (mDataField != NULL){
    mDataField->Dec();
    SetText(mDataField->GetAsString());
  }
  return(0);
}


void WndProperty::Paint(LKSurface& Surface){

    if((mBitmapSize > 0) && GetReadOnly()) {
        SetButtonSize(0);
    } else if (mDataField && !mDialogStyle ) {
        SetButtonSize(16);
    }
    
  //  RECT r;
  SIZE tsize;
  POINT org;

  if (!IsVisible()) return;

  WindowControl::Paint(Surface);

  // r.left = 0;
  // r.top = 0;
  // r.right = GetWidth();
  // r.bottom = GetHeight();

  Surface.SetTextColor(GetForeColor());
  // JMW make it look
  if (!HasFocus()) {
    Surface.SetBkColor(GetBackColor());
  }

  Surface.SetBackgroundTransparent();
  const auto oldFont = Surface.SelectObject(GetFont());

  const TCHAR * szCaption = GetWndText();
  const size_t nSize = _tcslen(szCaption);
  Surface.GetTextSize(szCaption, nSize, &tsize);
  if (nSize==0) {
      tsize.cy=0; //@ 101115 BUGFIX
  }

  if (mCaptionWidth==0){
	org.x = mEditRect.left;
	org.y = mEditRect.top - tsize.cy;
  } else {
	org.x = mCaptionWidth - mBitmapSize - (tsize.cx + 1);
	org.y = (GetHeight() - tsize.cy)/2;
  }

  if (org.x < 1) {
	org.x = 1;
  }

  Surface.DrawText(org.x, org.y, szCaption, nSize);

  // these are button left and right icons for waypoint select, for example
  if (mDialogStyle) // can't but dlgComboPicker here b/c it calls paint when combopicker closes too
  {     // so it calls dlgCombopicker on the click/focus handlers for the wndproperty & label
	// opening a window, each subwindow goes here once
  } else {
	if (HasFocus() && !GetReadOnly()) {
        Surface.DrawMaskedBitmap(mHitRectDown.left, mHitRectDown.top, mBitmapSize, mBitmapSize, hBmpLeft32, 32, 32);
        Surface.DrawMaskedBitmap(mHitRectUp.left, mHitRectUp.top, mBitmapSize, mBitmapSize, hBmpRight32, 32, 32);
	}
  }

  if((mEditRect.right - mEditRect.left) > mBitmapSize) {
    auto oldBrush = Surface.SelectObject(GetReadOnly()?LKBrush_LightGrey:LKBrush_White);
    auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
    // Draw Text Bakground & Border
    Surface.Rectangle(mEditRect.left, mEditRect.top, mEditRect.right, mEditRect.bottom);
    // Draw Text Value

    RECT rcText = mEditRect;
    InflateRect(&rcText, -NIBLSCALE(3), -1);
    
#ifndef USE_GDI
    // SubCanvas is used for clipping.
    // TODO : OpenGL SubCanvas don't clip rect, need to be fixed...
    SubCanvas ClipCanvas(Surface, rcText.GetOrigin(), rcText.GetSize() );
    rcText.Offset(-rcText.left, -rcText.top);

    ClipCanvas.Select(*mhValueFont);
    ClipCanvas.SetTextColor(RGB_BLACK);
    ClipCanvas.SetBackgroundTransparent();

    ClipCanvas.DrawFormattedText(&rcText, mValue.c_str(), DT_EXPANDTABS|(mMultiLine?DT_WORDBREAK:DT_SINGLELINE|DT_VCENTER));
#else
    Surface.SelectObject(mhValueFont);
    Surface.SetTextColor(RGB_BLACK);

    Surface.DrawText(mValue.c_str(), mValue.size(), &rcText, DT_EXPANDTABS|(mMultiLine?DT_WORDBREAK:DT_SINGLELINE|DT_VCENTER));
#endif

    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
  }
  Surface.SelectObject(oldFont);
}

void WndProperty::RefreshDisplay() {
    if (!mDataField) return;
    if (HasFocus()) {
        SetText(mDataField->GetAsString());
    } else {
        SetText(mDataField->GetAsDisplayString());
    }
    Redraw();
}

DataField *WndProperty::SetDataField(DataField *Value) {
    DataField *res = mDataField;

    if (mDataField != Value) {
        if (mDataField && !mDataField->Unuse()) {
            delete(mDataField);
            res = NULL;
        }

        Value->Use();
        mDataField = Value;
        mDataField->GetData();
        mDialogStyle = mDataField->SupportCombo;

        if (mDialogStyle) {
            this->SetButtonSize(0);
            this->SetCanFocus(true);
        } else {
            this->SetButtonSize(16);
        }

        RefreshDisplay();
    }
    return (res);
}

void WndOwnerDrawFrame::Paint(LKSurface& Surface) {
    if (IsVisible()) {

        WndFrame::Paint(Surface);
        const auto oldFont = Surface.SelectObject(GetFont());
        if (mOnPaintCallback != NULL) {
            (mOnPaintCallback) (this, Surface);
        }
        Surface.SelectObject(oldFont);
    }
}

bool WndFrame::OnKeyDown(unsigned KeyCode) {
    if (mIsListItem && GetParent() != NULL) {
        return (((WndListFrame*) GetParent())->OnItemKeyDown(this, KeyCode));
    }
    return false;
}

void WndFrame::Paint(LKSurface& Surface){

  if (!IsVisible()) return;

  if (mIsListItem && GetParent()!=NULL) {
    ((WndListFrame*)GetParent())->PrepareItemDraw();
  }

  WindowControl::Paint(Surface);

  const TCHAR* szCaption = GetWndText();
  const size_t nSize = _tcslen(szCaption);
  if (nSize > 0){

    Surface.SetTextColor(GetForeColor());
    Surface.SetBkColor(GetBackColor());
    Surface.SetBackgroundTransparent();

    const auto oldFont = Surface.SelectObject(GetFont());

    RECT rc = GetClientRect();
    InflateRect(&rc, -2, -2); // todo border width

    Surface.DrawText(szCaption, nSize, &rc,mCaptionStyle);

    mLastDrawTextHeight = rc.bottom - rc.top;

    Surface.SelectObject(oldFont);
  }

}

UINT WndFrame::SetCaptionStyle(UINT Value) {
    UINT res = mCaptionStyle;
    if (res != Value) {
        mCaptionStyle = Value;
        Redraw();
    }
    return (res);
}


WndListFrame::WndListFrame(WindowControl *Owner, TCHAR *Name, int X, int Y, 
                           int Width, int Height, 
                           OnListCallback_t OnListCallback):
  WndFrame(Owner, Name, X, Y, Width, Height)
{

  mListInfo.ItemIndex = 0;
  mListInfo.DrawIndex = 0;
  mListInfo.ItemInPageCount = 0;
  mListInfo.TopIndex = 0;
  mListInfo.BottomIndex = 0;
  mListInfo.ItemCount = 0;
  mListInfo.ItemInViewCount = 0;

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

void WndListFrame::Paint(LKSurface& Surface) {

    WndFrame* pChildFrame = NULL;
    if (!mClients.empty()) {
        pChildFrame = (WndFrame*) mClients.front();
        pChildFrame->SetIsListItem(true);
    }

    WndFrame::Paint(Surface);

    if (pChildFrame) {

#ifdef USE_GDI
        LKBitmapSurface TmpSurface;
        TmpSurface.Create(Surface, pChildFrame->GetWidth(), pChildFrame->GetHeight());
        const auto oldFont = TmpSurface.SelectObject(pChildFrame->GetFont());
#endif

        for (int i = 0; i < mListInfo.ItemInViewCount; i++) {
            if (mOnListCallback != NULL) {
                mListInfo.DrawIndex = mListInfo.TopIndex + i;
                if (mListInfo.DrawIndex == mListInfo.ItemIndex)
                    continue;
                mOnListCallback(this, &mListInfo);
            }

#ifndef USE_GDI
            const RasterPoint offset(pChildFrame->GetLeft(),  i * pChildFrame->GetHeight());
            const PixelSize size(pChildFrame->GetWidth(), pChildFrame->GetHeight());
            
            SubCanvas TmpCanvas(Surface, offset, size);
            LKSurface TmpSurface;
            TmpSurface.Attach(&TmpCanvas);
            TmpSurface.SelectObject(pChildFrame->GetFont());
#endif            
            
            pChildFrame->PaintSelector(true);
            pChildFrame->Paint(TmpSurface);
            pChildFrame->PaintSelector(false);

#ifdef USE_GDI
            Surface.Copy(
                    pChildFrame->GetLeft(), i * pChildFrame->GetHeight(),
                    pChildFrame->GetWidth(), pChildFrame->GetHeight(),
                    TmpSurface, 0, 0);
        }
        TmpSurface.SelectObject(oldFont);
#else
        }
#endif

        mListInfo.DrawIndex = mListInfo.ItemIndex;

        DrawScrollBar(Surface);
    }
}

void WndListFrame::Redraw(void) {
    WindowControl::Redraw(); // redraw all but not the current
    if (!mClients.empty()) {
        mClients.front()->Redraw(); // redraw the current
    }
}


void WndListFrame::DrawScrollBar(LKSurface& Surface) {
  RECT rc;

  if ( ScrollbarWidth == -1) {  // resize height for each dialog so top button is below 1st item (to avoid initial highlighted overlap)

	// shrink width factor.  Range .1 to 1 where 1 is very "fat"
	#if defined (PNA)
	#define SHRINKSBFACTOR 1.0 
	#else
	#define SHRINKSBFACTOR 0.75 
	#endif

	ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);  
	if (!mClients.empty()) {
		ScrollbarTop = mClients.front()->GetHeight() + 2;
	} else {
		ScrollbarTop = (int)(18.0 * ScreenDScale + 2);
	}
  }


  int w = GetWidth()- (ScrollbarWidth);
  int h = GetHeight() - ScrollbarTop;

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
	return;
  }

  const PenReference hP = LKPen_Black_N1;
  const auto oldPen = Surface.SelectObject(hP);

  
  // draw rectangle around entire scrollbar area
  Surface.DrawLine(rc.left, rc.top, rc.left, rc.bottom, rc.right, rc.bottom); 
  Surface.DrawLine(rc.right, rc.bottom, rc.right, rc.top, rc.left, rc.top); 

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

  // TOP Dn Button 32x32
  // BOT Up Button 32x32
  Surface.DrawMaskedBitmap(w, ScrollbarTop, ScrollbarWidth, ScrollbarWidth, hScrollBarBitmapTop, SCROLLBARWIDTH_INITIAL, SCROLLBARWIDTH_INITIAL);
  Surface.DrawMaskedBitmap(w, h-(ScrollbarWidth)+ScrollbarTop, ScrollbarWidth, ScrollbarWidth, hScrollBarBitmapBot, SCROLLBARWIDTH_INITIAL, SCROLLBARWIDTH_INITIAL);

  // Middle Slider Button 30x28
  if (mListInfo.ItemCount > mListInfo.ItemInViewCount) {
    
	// handle on slider
	if (ScrollbarWidth == SCROLLBARWIDTH_INITIAL) {
		Surface.DrawMaskedBitmap(w+1, rc.top + GetScrollBarHeight()/2 - 14, 30, 28, hScrollBarBitmapMid, 30, 28);
	} else {
		static int SCButtonW = -1;
		static int SCButtonH = -1;
		static int SCButtonY = -1;
		if (SCButtonW == -1) {
			SCButtonW = (int) (30.0 * (float)ScrollbarWidth / (float)SCROLLBARWIDTH_INITIAL);
			SCButtonH = (int) (28.0 * (float)ScrollbarWidth / (float)SCROLLBARWIDTH_INITIAL);
			SCButtonY = (int) (14.0 * (float)ScrollbarWidth / (float)SCROLLBARWIDTH_INITIAL);
		}
		Surface.DrawMaskedBitmap(w+1, rc.top + GetScrollBarHeight()/2 - SCButtonY, SCButtonW, SCButtonH, hScrollBarBitmapMid, 30, 28);
	}

	// box around slider rect
	const PenReference hP3=LKPen_Black_N2;
	int iBorderOffset = 1;  // set to 1 if BORDERWIDTH >2, else 0
	Surface.SelectObject(hP3);
	// just left line of scrollbar
	Surface.DrawLine(rc.left+iBorderOffset, rc.top, rc.left+iBorderOffset, rc.bottom, rc.right, rc.bottom); 
	Surface.DrawLine(rc.right, rc.bottom, rc.right, rc.top, rc.left+iBorderOffset, rc.top); // just left line of scrollbar

  } // more items than fit on screen

  Surface.SelectObject(oldPen);
  
  rcScrollBarButton.left=rc.left;
  rcScrollBarButton.right=rc.right;
  rcScrollBarButton.top=rc.top;
  rcScrollBarButton.bottom=rc.bottom;

}

void WndListFrame::RedrawScrolled(bool all) {

  mListInfo.DrawIndex = mListInfo.ItemIndex;
  mOnListCallback(this, &mListInfo);
  WindowControl * pChild = mClients.front();
  if(pChild) {
    int newTop = pChild->GetHeight() * (mListInfo.ItemIndex - mListInfo.TopIndex);
    if (newTop == pChild->GetTop()){
      Redraw();                     // non moving the helper window force redraw
    } else {
      pChild->SetTop(newTop);  // moving the helper window invalidate the list window
      pChild->Redraw();
      // to be optimized: after SetTop Paint redraw all list items
    }
  }
}


bool WndListFrame::RecalculateIndices(bool bigscroll) {

// scroll to smaller of current scroll or to last page
  mListInfo.ScrollIndex = max(0,min(mListInfo.ScrollIndex,
				    mListInfo.ItemCount-mListInfo.ItemInPageCount));

// if we're off end of list, move scroll to last page and select 1st item in last page, return
  if (mListInfo.ItemIndex+mListInfo.ScrollIndex >= mListInfo.ItemCount) {
    mListInfo.ItemIndex = max(0,mListInfo.ItemCount-mListInfo.ScrollIndex-1);
    mListInfo.ScrollIndex = max(0,
			      min(mListInfo.ScrollIndex,
				  mListInfo.ItemCount-mListInfo.ItemIndex-1));
    return false;
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
      return true;
    } else {
      mListInfo.ItemIndex = mListInfo.BottomIndex-1;
      return false;
    }
  }
  if (mListInfo.ItemIndex < 0){

    mListInfo.ItemIndex = 0;
    // JMW scroll
    if (mListInfo.ScrollIndex>0) {
      mListInfo.ScrollIndex--;
      RedrawScrolled(true);
      return true;
    } else {
      // only return if no more scrolling left to do
      return false;
    }
  }
  RedrawScrolled(bigscroll);
  return true;
}

bool WndListFrame::OnItemKeyDown(WindowControl *Sender, unsigned KeyCode) {
    switch (KeyCode) {
        case KEY_RETURN:
            if (mOnListEnterCallback) {
                mOnListEnterCallback(this, &mListInfo);
                RedrawScrolled(false);
                return true;
            }
            return false;
        case KEY_LEFT:
            if ((mListInfo.ScrollIndex > 0)
                    &&(mListInfo.ItemCount > mListInfo.ItemInPageCount)) {
                mListInfo.ScrollIndex -= mListInfo.ItemInPageCount;
            }
            return RecalculateIndices(true);
        case KEY_RIGHT:
            if ((mListInfo.ItemIndex + mListInfo.ScrollIndex <
                    mListInfo.ItemCount)
                    &&(mListInfo.ItemCount > mListInfo.ItemInPageCount)) {
                mListInfo.ScrollIndex += mListInfo.ItemInPageCount;
            }
            return RecalculateIndices(true);
        case KEY_DOWN:
            mListInfo.ItemIndex++;
            return RecalculateIndices(false);
        case KEY_UP:
            mListInfo.ItemIndex--;
            return RecalculateIndices(false);
    }
    mMouseDown = false;
    return false;
}

void WndListFrame::ResetList(void){

  mListInfo.ScrollIndex = 0;
  mListInfo.ItemIndex = 0;
  mListInfo.DrawIndex = 0;

  WindowControl * pChild = mClients.front();


  mListInfo.ItemInViewCount = mListInfo.ItemInPageCount = (pChild ? (((GetHeight() + pChild->GetHeight() - 1) / pChild->GetHeight()) - 1) : 0);
  mListInfo.TopIndex = 0;
  mListInfo.BottomIndex = 0;
  mListInfo.ItemCount = 0;

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

  if(pChild) {
    pChild->SetTop(0);     // move item window to the top
    pChild->Redraw();
  }
}

int WndListFrame::PrepareItemDraw(void){
  if (mOnListCallback)
    mOnListCallback(this, &mListInfo);
  return(1);
}

bool WndListFrame::OnLButtonUp(const POINT& Pos) {
    mMouseDown=false;
    return true;
}

static bool isselect = false;

bool WndFrame::OnLButtonUp(const POINT& Pos) {
  return false;
}

// JMW needed to support mouse/touchscreen
bool WndFrame::OnLButtonDown(const POINT& Pos) {

  if (mIsListItem && GetParent()!=NULL) {
 
    if (!HasFocus()) {
      SetFocus();  
    }
    Redraw();
    WndListFrame* wlf = ((WndListFrame*)GetParent());
    if(wlf) {
        RECT Rc = {};
        wlf->SelectItemFromScreen(Pos.x, Pos.y, &Rc);
    }
  }
  isselect = false;

  return true;
}

void WndListFrame::SetItemIndexPos(int iValue)
{
int Total = mListInfo.ItemCount;
	mListInfo.ScrollIndex = 0;
	mListInfo.ItemIndex=iValue;

  if(Total > mListInfo.ItemInPageCount)
  {
	if(iValue >  (mListInfo.ItemInPageCount -1))
	{
	  mListInfo.ScrollIndex = iValue - mListInfo.ItemInPageCount ;
	  mListInfo.ItemIndex   = iValue - mListInfo.ScrollIndex;
	}

    if((Total -  iValue) < (mListInfo.ItemInPageCount))
	{
	  mListInfo.ScrollIndex = Total  - mListInfo.ItemInPageCount ;
  	  mListInfo.ItemIndex   = iValue - mListInfo.ScrollIndex;
	}
  }
	RecalculateIndices(false);
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

void WndListFrame::SelectItemFromScreen(int xPos, int yPos, RECT *rect) {
  (void)xPos;
  WindowControl * pChild = NULL;
  if(!mClients.empty()) {
      pChild = mClients.front();

    *rect = GetClientRect();
    int index = yPos / pChild->GetHeight(); // yPos is offset within ListEntry item!

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
}


bool WndListFrame::OnMouseMove(const POINT& Pos) {

  if ( Poco::Timestamp() >= LastMouseMoveTime )
  {
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
    LastMouseMoveTime.update();
  } // Tickcount
  return false;
}

bool WndListFrame::OnLButtonDown(const POINT& Pos) {

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
  else if (!mClients.empty())
  {
    isselect = true;
    ((WndFrame*)mClients.front())->OnLButtonDown(Pos);
  }

  return true;
}

inline int WndListFrame::GetScrollBarHeight (void)
{
  int h = GetHeight() - ScrollbarTop;
  if(mListInfo.ItemCount ==0)
    return h-2*ScrollbarWidth;
  else {
    LKASSERT(mListInfo.ItemCount>0);
    return max(ScrollbarWidth,((h-2*ScrollbarWidth)*mListInfo.ItemInViewCount)/mListInfo.ItemCount);
  }
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

