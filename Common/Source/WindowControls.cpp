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
#include "Sideview.h"

#include "Screen/LKBitmapSurface.h"
#include "Screen/LKWindowSurface.h"

#include <functional>
#include <Form/Form.hpp>
#include <OS/RotateScreen.h>

#include "Event/Event.h"
#include "Asset.hpp"
#include "ScreenGeometry.h"
#include "Util/Clamp.hpp"

#ifndef USE_GDI
#include "Screen/SubCanvas.hpp"
#endif

using std::placeholders::_1;

// Old selector style has corners smoothed badly looking
// #define USE_OLD_SELECTOR

#define DEFAULTBORDERPENWIDTH DLGSCALE(1)

// utility functions

//
// Led button color ramps
//
const LEDCOLORRAMP LedRamp[MAXLEDCOLORS]= {
     {   0,   0,   0,     140,  140,  140,   10 },  // black
     { 160,   0,   0,     255,  130,  130,   10 },  // red ok
     {   0, 123,   0,      80,  255,   80,   10 },  // green ok
     {   0,   0, 160,     120,  120,  255,   10 },  // blue ok
     { 130, 130,   0,     255,  255,    0,   10 },  // yellow ok
     { 255, 128,   0,     255,  185,  140,   10 },  // orange ok
     { 164, 255, 164,     255,  255,  255,    5 },  // light green for dithering
     {   0, 0,   0,        70,  140,   70,   10 },  // very dark green 
     {   0,  90,  90,       0,  255,  255,   10 }   // cyan
};

//
// Distance from button borders for drawing the led rectangle
//
#define LEDBUTTONBORDER 3


#define ENABLECOMBO true // master on/off for combo popup
// Must be off if no touchscreen

// returns true if it is a long press,
// otherwise returns false
static bool KeyTimer(bool isdown, unsigned thekey) {
  static PeriodClock fpsTimeDown;
  static unsigned savedKey=0;

  if ((thekey==savedKey) && fpsTimeDown.CheckUpdate(2000)) {
    savedKey = 0;
    return true;
  }

  if (!isdown) {
    // key is released
  } else {
    // key is lowered
    if (thekey != savedKey) {
      fpsTimeDown.Update();
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
  LocalPath(buffer, subdir);
  ScanDirectories(buffer,_T(""), filter);
  Sort();

}

void DataFieldFileReader::ScanSystemDirectoryTop(const TCHAR* subdir, const TCHAR* filter) { // 091101
  
  TCHAR buffer[MAX_PATH] = TEXT("\0");
  SystemPath(buffer, subdir);
  ScanDirectories(buffer,_T(""), filter);
  Sort();

}


#ifdef ANDROID
void DataFieldFileReader::ScanZipDirectory(const TCHAR* subdir, const TCHAR* filter) { // 091101

  static zzip_strings_t ext [] = {".zip", ".ZIP", "", 0};
  zzip_error_t zzipError;

  tstring sRootPath = LKGetSystemPath();
  if(sRootPath.empty()) {
    return;
  }
  sRootPath.pop_back(); // remove trailing directory separator

  size_t subdir_size = _tcslen(subdir);
  // Open the archive root directory.
  ZZIP_DIR* dir = zzip_dir_open_ext_io(sRootPath.c_str(), &zzipError, ext, nullptr);
  if (dir) {
    ZZIP_DIRENT dirent;
    // Loop through the files in the archive.
    while(zzip_dir_read(dir, &dirent)) {

      if( _tcsnicmp(subdir, dirent.d_name, subdir_size) == 0) {
        if(checkFilter(dirent.d_name, filter)) {

          TCHAR* szFileName = _tcsrchr(dirent.d_name, _T('/'))+1;
          if(GetLabelIndex(szFileName) <= 0) {
            TCHAR * szFilePath = dirent.d_name + _tcslen(subdir);
            while( (*szFilePath) == _T('/') || (*szFilePath) == _T('\\') ) {
                ++szFilePath;
            }
            addFile(szFileName, szFilePath);
          }
        }
      }
    }
    zzip_dir_close(dir);
  }
}
#endif


BOOL DataFieldFileReader::ScanDirectories(const TCHAR* sPath, const TCHAR* subdir, const TCHAR* filter) {

    assert(sPath);
    assert(subdir);
    assert(filter);

    TCHAR FileName[MAX_PATH];

    _tcscpy(FileName, sPath);
    _tcscat(FileName, TEXT(DIRSEP));
    if(_tcslen(subdir) > 0) {
        _tcscat(FileName, subdir);
        _tcscat(FileName, TEXT(DIRSEP));
    }

    _tcscat(FileName, TEXT("*"));
    lk::filesystem::fixPath(FileName);

    for (lk::filesystem::directory_iterator It(FileName); It; ++It) {
        if (It.isDirectory()) {

            _tcscpy(FileName, subdir);
            if(_tcslen(FileName) > 0) {
                _tcscat(FileName, TEXT(DIRSEP));
            }
            _tcscat(FileName, It.getName());

            ScanDirectories(sPath, FileName, filter);
        } else if(checkFilter(It.getName(), filter)) {

            _tcscpy(FileName, subdir);
            if(_tcslen(FileName) > 0) {
              _tcscat(FileName, TEXT(DIRSEP));
            }
            _tcscat(FileName, It.getName());
            if(GetLabelIndex(FileName) <= 0) { // do not add same file twice...
              addFile(It.getName(), FileName);
            }
        }
    }

    return TRUE;
}

int DataFieldFileReader::GetLabelIndex(const TCHAR* label) {
  for (unsigned i=1; i<nFiles; i++) {
    // if (_tcscmp(Text,fields[i].mTextPathFile)==0) { 091126
    if (_tcsicmp(label,fields[i].mTextFile)==0) {
      return i;
    }
  }
  return -1;
}

bool DataFieldFileReader::Lookup(const TCHAR *Text) {
  mValue = 0;
  for (unsigned i=1; i<nFiles; i++) {    
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
    fields[nFiles].mTextFile = _tcsdup(Text);
    fields[nFiles].mTextPathFile = _tcsdup(PText);

    nFiles++;
  }
}


const TCHAR *DataFieldFileReader::GetAsString(void) {
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

struct DataFieldFileReaderCompare {
  bool operator()( const DataFieldFileReaderEntry &a,
                   const DataFieldFileReaderEntry &b ) {
    if(a.mTextFile && b.mTextFile) {
      return (_tcscmp(a.mTextFile, b.mTextFile) < 0);
    }
    return !a.mTextFile && b.mTextFile;
  }
};

void DataFieldFileReader::Sort(int startindex){
  auto begin = std::next(fields, startindex);
  auto end = std::next(fields, nFiles);

  std::sort(begin, end, DataFieldFileReaderCompare());
}

int DataFieldFileReader::CreateComboList(void) {
  unsigned int i=0;
  for (i=0; i < nFiles; i++){
    mComboList.ComboPopupItemList[i] = mComboList.CreateItem(
                                          i, 
                                          i,
                                          fields[i].mTextFile,
                                          fields[i].mTextFile);
  }
  mComboList.ComboPopupItemCount=i;
  mComboList.PropertyDataFieldIndexSaved = (mValue < i) ? mValue : 0;
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

DataField::DataField(const TCHAR *EditFormat, const TCHAR *DisplayFormat, DataAccessCallback_t OnDataAccess){
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
  unsigned iLen=0;
  const TCHAR *str = bFormatted ? GetAsDisplayString() : GetAsString();
  if (str) { // null leaves iLen=0
    iLen = std::min<unsigned>(_tcslen(str), ComboPopupITEMMAX-1);
    LK_tcsncpy(szbuffOut, str, iLen);
  }
  szbuffOut[iLen] = '\0';
}



//----------------------------------------------------------
// DataField boolean
//----------------------------------------------------------
int DataFieldBoolean::CreateComboList(void) {
  mComboList.ComboPopupItemList[0] = mComboList.CreateItem(0, 0, mTextFalse, mTextFalse);
  mComboList.ComboPopupItemList[1] = mComboList.CreateItem(1, 1, mTextTrue, mTextTrue);
  mComboList.ComboPopupItemCount=2;
  mComboList.PropertyDataFieldIndexSaved = (GetAsInteger() < 2) ? GetAsInteger() : 0;
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

const TCHAR *DataFieldBoolean::GetAsString(void) {
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

const TCHAR *DataFieldBoolean::SetAsString(const TCHAR *Value){
  const TCHAR *res = GetAsString();
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
  mEntries.clear();
  mValue = 0;
}

int DataFieldEnum::GetAsInteger(void){
  if (mValue<mEntries.size()) {
    return mEntries[mValue].index;
  } else {
    return 0; // JMW shouldn't get here
  }
}

bool DataFieldEnum::GetAsBoolean() {
  LKASSERT(mEntries.size() == 2);
  if (mValue<mEntries.size()) {
    return mEntries[mValue].index;
  }
  return false;
}

void DataFieldEnum::addEnumText(const TCHAR *Text, const TCHAR *Label) {
  mEntries.push_back({ (unsigned)mEntries.size(), Text, Label ? Label : _T("") });
}

void DataFieldEnum::addEnumTextNoLF(const TCHAR *Text) {
  tstring szTmp(Text);
  std::replace(szTmp.begin(), szTmp.end(), _T('\n'), _T(' '));

  const unsigned int idx = mEntries.size();
  mEntries.push_back({idx, std::move(szTmp), _T("")});
}

int DataFieldEnum::Find(const TCHAR *Text) {
  const auto it = std::find_if(mEntries.begin(), mEntries.end(), [Text](const DataFieldEnumEntry &item) {
    return (item.mText.compare(Text) == 0);
  });

  if(it != mEntries.end()) {
    return it->index;
  }
  return -1;
}

void DataFieldEnum::removeLastEnum() {
  mEntries.pop_back();
}

const TCHAR *DataFieldEnum::GetAsString(void) {
  if (mValue<mEntries.size()) {
    return(mEntries[mValue].mText.c_str());
  } else {
    return NULL;
  }
}

const TCHAR *DataFieldEnum::GetAsDisplayString(void) {
  if (mValue<mEntries.size()) {
    const DataFieldEnumEntry& entry = mEntries[mValue];
    return entry.mLabel.empty()
               ? entry.mText.c_str()
               : entry.mLabel.c_str();
  } else {
    return NULL;
  }
}


void DataFieldEnum::Set(unsigned Value){
  // first look it up
  if (Value >= mEntries.size()) {
    Value = 0;
  }
  for (unsigned int i=0; i<mEntries.size(); i++) {
    if (mEntries[i].index == Value) {
      unsigned lastValue = mValue;
      mValue = i;
      if (mValue != lastValue){
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
  if (mValue<mEntries.size()-1) {
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

struct DataFieldEnumEntry_sorter {

  bool operator()( const DataFieldEnumEntry& a, const DataFieldEnumEntry& b ) const {
    return ( a.mText < b.mText ); 
  }
};

void DataFieldEnum::Sort(int startindex){
    std::sort(std::next(mEntries.begin(), startindex), mEntries.end(), DataFieldEnumEntry_sorter());
}
int DataFieldEnum::CreateComboList(void) {
  unsigned int i=0;
  for (i=0; i < mEntries.size(); i++){
    const DataFieldEnumEntry& entry = mEntries[i];
    mComboList.ComboPopupItemList[i] = mComboList.CreateItem(
                                          i, 
                                          entry.index,
                                          entry.mText.c_str(),
                                          entry.mLabel.empty() ? entry.mText.c_str() : entry.mLabel.c_str() );
  }
  mComboList.ComboPopupItemCount=i;
  mComboList.PropertyDataFieldIndexSaved = (mValue < i) ? mValue : 0;
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
  mComboList.PropertyDataFieldIndexSaved = iSelectedIndex;
 
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

const TCHAR *DataFieldInteger::GetAsString(void){
  _stprintf(mOutBuf, mEditFormat, mValue);
  return(mOutBuf);
}

const TCHAR *DataFieldInteger::GetAsDisplayString(void){
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

const TCHAR *DataFieldInteger::SetAsString(const TCHAR *Value){
  const TCHAR *res = GetAsString();
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
    mTmLastStep.Update();
    return 1;
  }

  if (!mTmLastStep.Check(200)) {
    mSpeedup++;

    if (mSpeedup > 5){
      res = 10;

      mTmLastStep.UpdateWithOffset(350); // now + 350ms
      return(res);

    }
  } else
    mSpeedup = 0;

  mTmLastStep.Update();

  return(res);
}
int DataFieldInteger::CreateComboList(void) {
  return CreateComboListStepping();
}

bool DataFieldInteger::CreateKeyboard(void){
	TCHAR szText[20];
	_tcscpy(szText, GetAsString());
	dlgNumEntryShowModal(szText,20);

	SetAsFloat((int)floor((StrToDouble(szText, nullptr)/mStep)+0.5)*mStep);

	return true;
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

const TCHAR *DataFieldFloat::GetAsString(void){
  _stprintf(mOutBuf, mEditFormat, mValue);
  return(mOutBuf);
}

const TCHAR *DataFieldFloat::GetAsDisplayString(void) {
  _stprintf(mOutBuf, mDisplayFormat, mValue, mUnits);
  return(mOutBuf);
}

void DataFieldFloat::Set(double Value){
  mValue = Clamp(Value, mMin, mMax);
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
  Value = Clamp(Value, mMin, mMax);
  if (res != Value){
    mValue = Value;
    if (!GetDetachGUI()) (mOnDataAccess)(this, daChange);
  }
  return(res);
}

const TCHAR *DataFieldFloat::SetAsString(const TCHAR *Value){
  const TCHAR *res = GetAsString();
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
    mTmLastStep.Update();
    return 1.0;
  }

  if (!mTmLastStep.Check(200)){
    mSpeedup++;

    if (mSpeedup > 5){
      res = 10;

      mTmLastStep.UpdateWithOffset(350); // now + 350ms
      return(res);

    }
  } else
    mSpeedup = 0;

  mTmLastStep.Update();

  return(res);
}

int DataFieldFloat::SetFromCombo(int iDataFieldIndex, TCHAR *sValue) {
  SetAsString(sValue);
  return 0;
}

bool DataFieldFloat::CreateKeyboard(void){
	TCHAR szText[20];
	_tcscpy(szText, GetAsString());
	dlgNumEntryShowModal(szText,20);

	SetAsFloat(floor((StrToDouble(szText, nullptr)/mStep)+0.5)*mStep);

	return true;
}


int DataFieldFloat::CreateComboList(void) {
	return CreateComboListStepping();
}

const TCHAR *DataFieldTime::GetAsDisplayString(void) {
  double hours = 0;
  double minutes = modf(mValue, &hours);
  _stprintf(mOutBuf, mDisplayFormat, (int)hours, (int)(std::abs(minutes)*60.));
  return(mOutBuf);
}



//----------------------------------------------------------
// DataField String
//----------------------------------------------------------


const TCHAR *DataFieldString::SetAsString(const TCHAR *Value){
  Set(Value);
  return(mValue);
}

void DataFieldString::Set(const TCHAR *Value){
  _tcscpy(mValue, Value);
}

const TCHAR *DataFieldString::GetAsString(void){
  return(mValue);
}

bool DataFieldString::CreateKeyboard(void){
	dlgTextEntryShowModal(mValue,std::size(mValue));
	return true;
}

//----------------------------------------------------------
// ComboList Class
//----------------------------------------------------------
ComboListEntry_t * ComboList::CreateItem(int ItemIndex, 
                                        int DataFieldIndex,
                                        const TCHAR *StringValue,
                                        const TCHAR *StringValueFormatted)
{
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

  if (StringValue) {
    theItem->StringValue = _tcsdup(StringValue);
  }
  else { 
    theItem->StringValue = _tcsdup(_T(""));
  }

  // copy formatted display string
  if (StringValueFormatted) {
    theItem->StringValueFormatted = _tcsdup(StringValueFormatted);
  }
  else {
    theItem->StringValueFormatted = _tcsdup(_T(""));
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

int WindowControl::InstCount=0;

WindowControl::WindowControl(WindowControl *Owner, const TCHAR *Name, 
			     int X, int Y, int Width, int Height, bool Visible)
                : WndCtrlBase(Name)
{

  mHelpText = nullptr;

  mCanFocus = false;

  mReadOnly = false;

  mOnHelpCallback = NULL;

  mOwner = Owner?Owner->GetClientArea():NULL;
  // setup Master Window (the owner of all)
  mParentWndForm = NULL;
  if(Owner) {
    mParentWndForm = Owner->GetParentWndForm();
  }
    
  mDontPaintSelector = false;

  InitWindowControlModule();

  mColorBack = RGB_WINBACKGROUND; // PETROL
  mColorFore = RGB_WINFOREGROUND; // WHITE

  InstCount++;

  // if Owner is Not provided, use main_window
  ContainerWindow* WndOnwer = Owner
          ?static_cast<ContainerWindow*>(Owner->GetClientArea())
          :static_cast<ContainerWindow*>(main_window.get());
  
  if(mOwner) {
    mOwner->CalcChildRect(X, Y, Width, Height);
  }
  LKASSERT(X+Width>0);
  
  Create(WndOnwer,(RECT){X, Y, X+Width, Y+Height});
  SetTopWnd();
  SetFont(MapWindowFont);
  if (mOwner != NULL)
    mOwner->AddClient(this);  

  SetBorderColor(RGB_BLACK);
  mBorderKind = 0; //BORDERRIGHT | BORDERBOTTOM;

  if (Visible) {
    SetVisible(true);
  }
}

WindowControl::~WindowControl(void){
  if (mHelpText) {
    free(mHelpText);
    mHelpText = nullptr;
  }
}

void WindowControl::Destroy(){
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

bool WindowControl::OnPaint(LKSurface& Surface, const RECT& Rect) {
#ifndef USE_GDI
    Paint(Surface);
#else
    const int win_width = GetWidth();
    const int win_height = GetHeight();

    LKBitmapSurface MemSurface(Surface, win_width, win_height);

    HWND hChildWnd = NULL;
    if ((hChildWnd = GetWindow(_hWnd, GW_CHILD)) != NULL) {
        RECT Client_Rect;
        while ((hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT)) != NULL) {
            if (::IsWindowVisible(hChildWnd)) {
                ::GetWindowRect(hChildWnd, &Client_Rect);
                
                ::ScreenToClient(_hWnd, (LPPOINT)&Client_Rect.left);
                ::ScreenToClient(_hWnd, (LPPOINT)&Client_Rect.right);                

                ::ExcludeClipRect(MemSurface, Client_Rect.left, Client_Rect.top, Client_Rect.right, Client_Rect.bottom );
                ::ExcludeClipRect(Surface, Client_Rect.left, Client_Rect.top, Client_Rect.right, Client_Rect.bottom );
            }
        }
    }
    Paint(MemSurface);

    Surface.Copy(0, 0, win_width, win_height, MemSurface, 0, 0);
#endif
    return true;
}

void WindowControl::SetTop(int Value){
    if(GetTop() != Value) {
        Move(GetLeft(), Value);
    }
}

void WindowControl::SetLeft(int Value){
    if(GetLeft() != Value) {
        Move(Value, GetTop());
    }
}

void WindowControl::SetHeight(unsigned int Value){
    if(GetHeight() != Value) {
        Resize(GetWidth(), Value);
    }
}

void WindowControl::SetWidth(unsigned int Value){
    if(GetWidth() != Value) {
        Resize(Value, GetHeight());
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
    
    // use negative value to space down or right items
    // -999 to stay on the same line or column
    // -998 to advance one line or column with spacing IBLSCALE 3
    // -997 to advance one line or column with spacing IBLSCALE 6
    // -991 to advance one or column line with spacing IBLSCALE 1
    // -992 to advance one or column line with spacing IBLSCALE 2
    // other -n   advance exactly ((height or width) * n). 

    if (!mClients.empty()) {
      if (y < 0) {
        WindowControl * pPrev = mClients.back();
        switch (y) {
            case -999: //@ 101008
                y = pPrev->GetTop();
                break;
            case -998: //@ 101115
                y = (pPrev->GetBottom() + DLGSCALE(3));
                break;
            case -997: //@ 101115
                y = (pPrev->GetBottom() + DLGSCALE(6));
                break;
            case -992: 
                y = (pPrev->GetBottom() + DLGSCALE(2));
                break;
            case -991: 
                y = (pPrev->GetBottom() + DLGSCALE(1));
                break;
            default:
                y = (pPrev->GetTop() - ((pPrev->GetHeight()) * y));
                break;
          }
      }
      if (x < 0) {
        WindowControl * pPrev = mClients.back();
        switch (x) {
            case -999: //@ 101008
                x = pPrev->GetRight();
                break;
            case -998: //@ 101115
                x = (pPrev->GetRight() + DLGSCALE(3));
                break;
            case -997: //@ 101115
                x = (pPrev->GetRight() + DLGSCALE(6));
                break;
            case -992: 
                x = (pPrev->GetRight() + DLGSCALE(2));
                break;
            case -991: 
                x = (pPrev->GetRight() + DLGSCALE(1));
                break;
            default:
                x = (pPrev->GetRight() - ((pPrev->GetWidth()) * x));
                break;
        }
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
    if(Name) {
        if (_tcscmp(GetWndName(), Name) == 0) {
            return (this);
        }

        for (WindowControl* w : mClients) {
            WindowControl* res = w->FindByName(Name);
            if (res) {
                return (res);
            }
        }
    }
    return (NULL);
}

void WindowControl::SetHelpText(const TCHAR *Value) {  
  if (mHelpText) {
    free(mHelpText);
    mHelpText = nullptr;
  }
  if (Value && Value[0]) {
    mHelpText = _tcsdup(Value);
  }
}

void WindowControl::SetCaption(const TCHAR *Value) {
    const TCHAR* szCaption = GetWndText();
    if (Value == NULL) {
        if(szCaption[0] != _T('\0')) {
            SetWndText(_T(""));
            Redraw();
        }
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

    if (mBorderKind != 0) {

        const RECT rcClient = GetClientRect();

        if (mBorderKind & BORDERTOP) {
            const RECT rcLine = { rcClient.left, rcClient.top, rcClient.right, rcClient.top + DLGSCALE(1) };
            Surface.FillRect(&rcLine, mBrushBorder);
        }
        if (mBorderKind & BORDERRIGHT) {
            const RECT rcLine = { rcClient.right - DLGSCALE(1), rcClient.top, rcClient.right, rcClient.bottom };
            Surface.FillRect(&rcLine, mBrushBorder);
        }
        if (mBorderKind & BORDERBOTTOM) {
            const RECT rcLine = { rcClient.left, rcClient.bottom - DLGSCALE(1), rcClient.right, rcClient.bottom };
            Surface.FillRect(&rcLine, mBrushBorder);
        }
        if (mBorderKind & BORDERLEFT) {
            const RECT rcLine = { rcClient.left, rcClient.top, rcClient.left + DLGSCALE(1), rcClient.bottom };
            Surface.FillRect(&rcLine, mBrushBorder);
        }
    }
}

void WindowControl::PaintSelector(LKSurface& Surface){
#ifdef USE_OLD_SELECTOR
#define SELECTORWIDTH         DLGSCALE(4)
  if (!mDontPaintSelector && mCanFocus && HasFocus()){
    const auto oldPen = Surface.SelectObject(LKPen_Petrol_C2);

    const int Width = GetWidth();
    const int Height = GetHeight();

    Surface.DrawLine(
	      Width-SELECTORWIDTH-1, 0,
	      Width-1, 0,
	      Width-1, SELECTORWIDTH+1);

    Surface.DrawLine(
	      Width-1, Height-SELECTORWIDTH-2,
	      Width-1, Height-1,
	      Width-SELECTORWIDTH-1, Height-1);

    Surface.DrawLine(
	      SELECTORWIDTH+1, Height-1, 
	      0, Height-1,
	      0, Height-SELECTORWIDTH-2);

    Surface.DrawLine(
	      0, SELECTORWIDTH+1,
	      0, 0,
	      SELECTORWIDTH+1, 0);

    Surface.SelectObject(oldPen);
  }
#endif
}

int WindowControl::OnHelp() {
    if (mHelpText) {
      dlgHelpShowModal(GetWndText(), mHelpText);
      return(1);
    }
    if (mOnHelpCallback) {
      (mOnHelpCallback)(this);
      return(1);
    }
    return(0);
}

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
    #ifdef USE_OLD_SELECTOR
    PaintSelector(Surface);
    #endif
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
                 int X, int Y, int Width, int Height, bool Modal)
        : WindowControl(NULL, Name, X, Y, Width, Height, false)
        , mModal(Modal) 
{

  mClientWindow = NULL;
  
  mOnTimerNotify = NULL;
  mOnKeyDownNotify = NULL;
  mOnKeyUpNotify = NULL;
  mOnUser = NULL;

  mColorTitle = RGB_MENUTITLEBG;

  mhTitleFont = GetFont();

  mhBrushTitle = LKBrush_Black; // 101204

  mClientRect.top = (mBorderKind & BORDERTOP) ? DLGSCALE(1) : 0;
  mClientRect.left = (mBorderKind & BORDERLEFT) ? DLGSCALE(1) : 0;
  mClientRect.bottom = Height - ((mBorderKind & BORDERBOTTOM) ? DLGSCALE(1) : 0);
  mClientRect.right = Width - ((mBorderKind & BORDERRIGHT) ? DLGSCALE(1) : 0);

  mTitleRect.top = 0;
  mTitleRect.left = 0;
  mTitleRect.bottom = 0;
  mTitleRect.right = Width;

  mClientWindow = new WindowControl(this, TEXT(""), mClientRect.top, mClientRect.left, mClientRect.right - mClientRect.left, mClientRect.bottom - mClientRect.top);
  mClientWindow->SetBackColor(RGB_WINBACKGROUND);
  mClientWindow->SetCanFocus(false);

  SetCaption(Caption);
  
  mModalResult = 0;
};

WndForm::~WndForm(void){
  Destroy();
}

void WndForm::Destroy(void) {
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

int WndForm::ShowModal(void) {

    SHOWTHREAD(_T("ShowModal"));

    enterTime.Update();

    Message::ScopeBlockRender BlockRender;
    ScopeLockScreen LockScreen;

    SetVisible(true);

    SetToForeground();

    mModalResult = 0;

    Window* oldFocus = main_window->GetFocusedWindow();
    FocusNext(NULL);

    Redraw();

    LKASSERT(event_queue);
#if defined(ANDROID) || defined(USE_POLL_EVENT) || defined(ENABLE_SDL) || defined(NON_INTERACTIVE)
    EventLoop loop(*event_queue, *main_window);
#else
    EventLoop loop(*event_queue);
#endif
    Event event;
    while (mModalResult == 0 && loop.Get(event)) {
        if (mModal && !main_window->FilterEvent(event, this)) {
            continue;
        }
        
        if (event.IsKeyDown()) {
#ifndef WIN32
            Window* w = GetFocusedWindow();
            if (w == nullptr) {
                w = this;
            }
            if(w->OnKeyDown(event.GetKeyCode())) {
                continue;
            }

            if (OnKeyDownNotify(GetFocusedWindow(), event.GetKeyCode())) {
                continue;
            }
            
            /* The Windows CE dialog manager does not handle KEY_ESCAPE and
               so we have to do it by ourself */

            if (event.GetKeyCode() == KEY_ESCAPE) {
                mModalResult = mrCancel;
                continue;
            }

#ifdef USE_LINUX_INPUT
            if (event.GetKeyCode() == KEY_POWER) {
                /* the Kobo power button closes the modal dialog */
                mModalResult = mrCancel;
                continue;
            }
#endif
#endif
        }
        loop.Dispatch(event);
    } // End Modal Loop

    if(oldFocus) {
        oldFocus->SetFocus();
    }
    main_window->UnGhost();

#ifdef USE_GDI    
    MapWindow::RequestFastRefresh();
#endif

    return (mModalResult);
}

void WndForm::Paint(LKSurface& Surface){

    if (!IsVisible()) return;

    const TCHAR * szCaption = GetWndText();
    size_t nChar = _tcslen(szCaption);
    if(nChar > 0) {

        Surface.SetTextColor(RGB_MENUTITLEFG);
        Surface.SetBkColor(mColorTitle);
        Surface.SetBackgroundTransparent();

        const auto oldFont = Surface.SelectObject(mhTitleFont);

        Surface.FillRect(&mTitleRect, mhBrushTitle);

        RECT rcLine = GetClientRect();
        rcLine.top = mTitleRect.bottom;
        rcLine.bottom = mClientRect.top;
        
        Surface.FillRect(&rcLine, LKBrush_Green);

        Surface.DrawText(mTitleRect.left+DLGSCALE(2), mTitleRect.top, szCaption);

        Surface.SelectObject(oldFont);
    } 
    PaintBorder(Surface);
}

void WndForm::SetCaption(const TCHAR *Value) {
    const TCHAR* szCaption = GetWndText();
    bool bRedraw = false;
    if (Value == NULL) {
        if(szCaption[0] != _T('\0')) {
            SetWndText(_T(""));
            bRedraw = true;
        }
    } else if (_tcscmp(szCaption, Value) != 0) {
        SetWndText(Value);
        bRedraw = true;
    }

    RECT rcClient = mClientRect;
    
    size_t nChar = Value?_tcslen(Value):0;
    if(nChar > 0) {
        SIZE tsize = {0,0};
        LKWindowSurface Surface(*this);
        const auto oldFont = Surface.SelectObject(mhTitleFont);
        Surface.GetTextSize(Value, &tsize);
        Surface.SelectObject(oldFont);
        
        mTitleRect.bottom = mTitleRect.top + tsize.cy;
        rcClient.top = mTitleRect.bottom+DLGSCALE(1);
    } else {
        mTitleRect.bottom = 0;
        rcClient.top = (mBorderKind & BORDERTOP) ? DLGSCALE(1) : 0;
    }

    if (!EqualRect(&mClientRect, &rcClient)){
        mClientRect = rcClient;
        if(mClientWindow) {
            mClientWindow->FastMove(mClientRect);
            mClientWindow->SetTopWnd();
        }
        bRedraw = true;
    }    
    if(bRedraw) {
        Redraw();
    }
}

int  WndForm::SetBorderKind(int Value) {
    
    RECT rcClient = GetClientRect();

    if(mTitleRect.bottom > 0) {
        rcClient.top = mTitleRect.bottom+DLGSCALE(1);
    } else {
        rcClient.top += (Value & BORDERTOP) ? DLGSCALE(1) : 0;
    }    
    rcClient.left += (Value & BORDERLEFT) ? DLGSCALE(1) : 0;
    rcClient.bottom -= (Value & BORDERBOTTOM) ? DLGSCALE(1) : 0;
    rcClient.right -= (Value & BORDERRIGHT) ? DLGSCALE(1) : 0;
    
    if (!EqualRect(&mClientRect, &rcClient)){
        mClientRect = rcClient;
        if(mClientWindow) {
            mClientWindow->FastMove(mClientRect);
            mClientWindow->SetTopWnd();
        }
    }
    return WindowControl::SetBorderKind(Value);
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
    ScopeLockScreen LockScreen;
    main_window->UnGhost();
    
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
            switch (KeyCode) {
                case KEY_UP:
                    pCtrl->FocusPrev(ActiveControl);
                    return true;
                case KEY_DOWN:
                    pCtrl->FocusNext(ActiveControl);
                    return true;
            }
        }
    }
    if(KeyCode == KEY_ESCAPE)  {
        mModalResult = mrCancel;
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

  mLedMode = LEDMODE_DISABLED;
  mLedOnOff=false;
  mLedSize=5; // the default size, including pen borders
  mLedColor= LEDCOLOR_BLACK;

  SetForeColor(RGB_BUTTONFG);
  SetBackColor(GetParent()->GetBackColor());

  if(Caption) {
      SetCaption(Caption);
  }
  mLastDrawTextHeight = -1;
}

void WndButton::LedSetMode(unsigned short ledmode) {
   mLedMode=ledmode;
}
void WndButton::LedSetSize(unsigned short ledsize) {
   LKASSERT(ledsize>0 && ledsize<200);
   mLedSize=ledsize;
}
void WndButton::LedSetOnOff(bool ledonoff) {
   mLedOnOff=ledonoff;
}
void WndButton::LedSetColor(unsigned short ledcolor) {
   LKASSERT(ledcolor<MAXLEDCOLORS);
   mLedColor=ledcolor;
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

  Surface.DrawPushButton(rc, mDown);


  if (mLedMode) {
     RECT lrc={rc.left+(LEDBUTTONBORDER-1),rc.bottom-(LEDBUTTONBORDER+DLGSCALE(mLedSize)),
               rc.right-LEDBUTTONBORDER,rc.bottom-LEDBUTTONBORDER};
     unsigned short lcol=0;
     switch(mLedMode) {
        case LEDMODE_REDGREEN:
           if (IsDithered())
              lcol=(mLedOnOff?LEDCOLOR_LGREEN:LEDCOLOR_ORANGE);
           else {
              if (HasFocus())
                 lcol=LEDCOLOR_YELLOW;
              else
                 lcol=(mLedOnOff?LEDCOLOR_GREEN:LEDCOLOR_RED);
           }
           break;
        case LEDMODE_OFFGREEN:
           if (IsDithered())
              lcol=(mLedOnOff?LEDCOLOR_LGREEN:LEDCOLOR_BLUE);
           else {
              if (HasFocus())
                 lcol=LEDCOLOR_YELLOW;
              else
                 lcol=(mLedOnOff?LEDCOLOR_GREEN:LEDCOLOR_DGREEN);
           }
           break;
        case LEDMODE_MANUAL:
           lcol=mLedColor;
           break;
        default:
           LKASSERT(0);
           break;
     }
     RenderSky(Surface, lrc,
        LKColor( LedRamp[lcol].r1, LedRamp[lcol].g1, LedRamp[lcol].b1 ), 
        LKColor( LedRamp[lcol].r2, LedRamp[lcol].g2, LedRamp[lcol].b2 ), 
        LedRamp[lcol].l 
     );
                           

  }

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

      Surface.DrawText(szCaption, &rc,
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

    Surface.DrawText(szCaption, &rc,
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

  UpdateButtonData();

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

void WndProperty::UpdateButtonData(){

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
    mBitmapSize = Value;
    UpdateButtonData();

    if (IsVisible()){
        Redraw();
    }
  }
  return(res);
};

bool WndProperty::SetReadOnly(bool Value) {
    bool Res = WindowControl::SetReadOnly(Value);
    if(Value) {
        SetButtonSize(0);
        SetCanFocus(true);
    } else if (mDataField && !mDialogStyle ) {
        SetButtonSize(DLGSCALE(32)/2);
    }
    if(Res != Value) {
       Redraw();
    }
    return Res;
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
                if (KeyTimer(true, KeyCode) && OnHelp()) {
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

  //  RECT r;
  SIZE tsize;
  POINT org;

  if (!IsVisible()) return;

  PixelRect rc(GetClientRect());
  rc.right += 2;
  rc.bottom += 2;

  Surface.FillRect(&rc, GetBackBrush());
  


  if(HasFocus() && mCanFocus) {
    auto oldBrush = Surface.SelectObject(LK_HOLLOW_BRUSH);
    auto oldPen = Surface.SelectObject(LKPen_Higlighted);
    Surface.Rectangle(rc.left, rc.top, rc.right-3, rc.bottom-3);
    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
  }

  Surface.SetTextColor(GetForeColor());
  Surface.SetBkColor(GetBackColor());
  Surface.SetBackgroundTransparent();
  const auto oldFont = Surface.SelectObject(GetFont());

  const TCHAR * szCaption = GetWndText();
  const size_t nSize = _tcslen(szCaption);
  Surface.GetTextSize(szCaption, &tsize);
  if (nSize==0) {
      tsize.cy=0; //@ 101115 BUGFIX
  }

  if (mCaptionWidth==0){
	org.x = mEditRect.left;
	org.y = mEditRect.top - tsize.cy;
  } else {
	org.x = mCaptionWidth - mBitmapSize - (tsize.cx + DLGSCALE(2));
	org.y = (GetHeight() - tsize.cy)/2;
  }

  if (org.x < 1) {
	org.x = 1;
  }

  Surface.DrawText(org.x, org.y, szCaption);

  // these are button left and right icons for waypoint select, for example
  if (mDialogStyle) // can't but dlgComboPicker here b/c it calls paint when combopicker closes too
  {     // so it calls dlgCombopicker on the click/focus handlers for the wndproperty & label
	// opening a window, each subwindow goes here once
  } else {
	if (HasFocus() && !GetReadOnly()) {
        hBmpLeft32.Draw(Surface, mHitRectDown.left, mHitRectDown.top, mBitmapSize, mBitmapSize);
        hBmpRight32.Draw(Surface, mHitRectUp.left, mHitRectUp.top, mBitmapSize, mBitmapSize);
	}
  }

  if((mEditRect.right - mEditRect.left) > mBitmapSize) {
    auto oldBrush = Surface.SelectObject(GetReadOnly()?LKBrush_LightGrey:LKBrush_White);
    auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
    // Draw Text Bakground & Border
    Surface.Rectangle(mEditRect.left, mEditRect.top, mEditRect.right, mEditRect.bottom);
    // Draw Text Value

    RECT rcText = mEditRect;
    InflateRect(&rcText, -DLGSCALE(3), -1);
    
    Surface.SelectObject(mhValueFont);
    Surface.SetTextColor(RGB_BLACK);

    Surface.DrawText(mValue.c_str(), &rcText, DT_EXPANDTABS|(mMultiLine?DT_WORDBREAK:DT_SINGLELINE|DT_VCENTER));

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
        mDialogStyle = mDataField->SupportCombo||mUseKeyboard;


        if(GetReadOnly() || mDialogStyle) {
            SetButtonSize(0);
            this->SetCanFocus(true);
        } else if (mDataField && !mDialogStyle ) {
            SetButtonSize(DLGSCALE(32)/2);
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

    Surface.DrawText(szCaption,&rc,mCaptionStyle);

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
  mListInfo.ScrollIndex = 0;
  mListInfo.ItemIndex = 0;
  mListInfo.DrawIndex = 0;
  mListInfo.ItemInPageCount = 0;
  mListInfo.TopIndex = 0;
  mListInfo.BottomIndex = 0;
  mListInfo.ItemCount = 0;

  mOnListCallback = OnListCallback;
  mOnListEnterCallback = NULL;
  SetForeColor(RGB_LISTFG);
  SetBackColor(RGB_LISTBG);
  mMouseDown = false;
  mCaptureScrollButton = false;
  ScrollbarWidth=-1;

  rcScrollBarButton.top=0; // make sure this rect is initialized so we don't "loose" random lbuttondown events if scrollbar not drawn
  rcScrollBarButton.bottom=0;
  rcScrollBarButton.left=0;
  rcScrollBarButton.right=0;

  rcScrollBar.left=0;  // don't need to initialize this rect, but it's good practice
  rcScrollBar.right=0;
  rcScrollBar.top=0;
  rcScrollBar.bottom=0;
  
};

void WndListFrame::CalcChildRect(int& x, int& y, int& cx, int& cy) const {
    if(cx < 0) {
        cx -= const_cast<WndListFrame*>(this)->GetScrollBarWidth();
    }
    WndFrame::CalcChildRect(x, y, cx, cy);
}

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

        for (int i = 0; i < mListInfo.ItemInPageCount; i++) {
            if (mOnListCallback != NULL) {
                mListInfo.DrawIndex = mListInfo.TopIndex + i;
                if (mListInfo.DrawIndex == mListInfo.ItemIndex)
                    continue;
                mOnListCallback(this, &mListInfo);
            }

            const RasterPoint offset(pChildFrame->GetLeft(),  i * pChildFrame->GetHeight() + ((GetBorderKind()&BORDERTOP) ? DLGSCALE(1) : 0));
            const PixelSize size(pChildFrame->GetWidth(), pChildFrame->GetHeight());

#ifndef USE_GDI
            
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
                    offset.x, offset.y,
                    size.cx, size.cy,
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

int WndListFrame::GetScrollBarWidth() {
    if ( ScrollbarWidth == -1) {
        // use fat Scroll on Embedded platform for allow usage on touch screen with fat finger ;-)
        constexpr double scale = HasTouchScreen() ? 1.0 : 0.75; 
        ScrollbarWidth = DLGSCALE(ScrollbarWidthInitial * scale);
    }
    return ScrollbarWidth;
}

void WndListFrame::DrawScrollBar(LKSurface& Surface) {
    if (mListInfo.BottomIndex == mListInfo.ItemCount) { // don't need scroll bar if one page only
        return;
    }

    // ENTIRE SCROLLBAR AREA
    // also used for mouse events
    rcScrollBar = { (int)(GetWidth()- GetScrollBarWidth()), 0, (int)GetWidth(), (int)GetHeight() };

    const auto oldPen = Surface.SelectObject(LKPen_Black_N1);
    const auto oldBrush = Surface.SelectObject(LK_HOLLOW_BRUSH);

    // draw rectangle around entire scrollbar area
    Surface.Rectangle(rcScrollBar.left, rcScrollBar.top, rcScrollBar.right, rcScrollBar.bottom);

    // Just Scroll Bar Slider button
    rcScrollBarButton = {
        rcScrollBar.left,
        GetScrollBarTopFromScrollIndex(),
        rcScrollBar.right,
        GetScrollBarTopFromScrollIndex() + GetScrollBarHeight() + 2, // +2 for 3x pen, +1 for 2x pen
    };

    // TOP Dn Button 32x32
    // BOT Up Button 32x32
    hScrollBarBitmapTop.Draw(Surface, rcScrollBar.left, rcScrollBar.top, ScrollbarWidth, ScrollbarWidth);
    hScrollBarBitmapBot.Draw(Surface, rcScrollBar.left, rcScrollBar.bottom - ScrollbarWidth, ScrollbarWidth, ScrollbarWidth);

    if (mListInfo.ItemCount > mListInfo.ItemInPageCount) {

        // Middle Slider Button 30x28
        // handle on slider
        const int SCButtonW = (int) (30.0 * (float) ScrollbarWidth / (float) ScrollbarWidthInitial);
        const int SCButtonH = (int) (28.0 * (float) ScrollbarWidth / (float) ScrollbarWidthInitial);
        const int SCButtonY = (int) (14.0 * (float) ScrollbarWidth / (float) ScrollbarWidthInitial);
        hScrollBarBitmapMid.Draw(Surface, rcScrollBar.left, rcScrollBarButton.top + GetScrollBarHeight() / 2 - SCButtonY, SCButtonW, SCButtonH);

        // box around slider rect
        Surface.SelectObject(LKPen_Black_N2);
        // just left line of scrollbar
        Surface.Rectangle(rcScrollBarButton.left, rcScrollBarButton.top, rcScrollBarButton.right, rcScrollBarButton.bottom);

    } // more items than fit on screen

    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
}

void WndListFrame::RedrawScrolled(bool all) {

  mListInfo.DrawIndex = mListInfo.ItemIndex;
  mOnListCallback(this, &mListInfo);
  WindowControl * pChild = mClients.front();
  if(pChild) {
    const int newTop = pChild->GetHeight() * (mListInfo.ItemIndex - mListInfo.TopIndex) + ((GetBorderKind()&BORDERTOP) ? DLGSCALE(1) : 0);
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


  mListInfo.ItemInPageCount = (pChild ? (((GetHeight() + pChild->GetHeight() - 1) / pChild->GetHeight()) - 1) : 0);
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
    if (mListInfo.BottomIndex > mListInfo.ItemInPageCount){
      mListInfo.BottomIndex = mListInfo.ItemInPageCount;
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



bool WndFrame::OnLButtonDown(const POINT& Pos) {
  mLButtonDown = true;
  if (!HasFocus()) {
    SetFocus();
    Redraw();
  }
  return false;
}

void WndListFrame::CenterScrollCursor(void) {
  int Total    = mListInfo.ItemCount;
  int halfPage = mListInfo.ItemInPageCount/2;

  if( Total > mListInfo.ItemInPageCount) {
    if( mListInfo.ItemIndex > (halfPage)) {
      if(( mListInfo.ScrollIndex + mListInfo.ItemInPageCount) < Total) {
        mListInfo.ScrollIndex  += halfPage;
        mListInfo.ItemIndex    -= halfPage;

        RecalculateIndices(false);
      }
    }
  }
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

void WndListFrame::SelectItemFromScreen(int xPos, int yPos, RECT *rect, bool select) {
   if (PtInRect(&rcScrollBar, {xPos, yPos})) {
     // ignore if click inside scrollbar
     return;
   }
  
  WindowControl * pChild = NULL;
  if(!mClients.empty()) {
      pChild = mClients.front();

    *rect = GetClientRect();
    int index = yPos / pChild->GetHeight(); // yPos is offset within ListEntry item!

    if ((index>=0)&&(index<mListInfo.BottomIndex)) {
      if(mListInfo.ItemIndex != index) {
        mListInfo.ItemIndex = index;
        RecalculateIndices(false);
      } else {
        if(!select) {
          if (mOnListEnterCallback) {
            mOnListEnterCallback(this, &mListInfo);
          }
          RedrawScrolled(false);
        }
      }
    }
  }
}


bool WndListFrame::OnMouseMove(const POINT& Pos) {

  if (mMouseDown) {
    SetCapture();

    if(mCaptureScrollButton) {
      const int iScrollBarTop = max(1, (int)Pos.y - mMouseScrollBarYOffset);
      const int iScrollIndex = GetScrollIndexFromScrollBarTop(iScrollBarTop);

      if(iScrollIndex !=mListInfo.ScrollIndex) {
        const int iScrollAmount = iScrollIndex - mListInfo.ScrollIndex;
        mListInfo.ScrollIndex = mListInfo.ScrollIndex + iScrollAmount;
        Redraw();
      }
    } else if (mListInfo.ItemCount > mListInfo.ItemInPageCount) {
      const int ScrollOffset =  Pos.y - mScrollStart.y;
      const int ScrollStep = GetHeight() / mListInfo.ItemInPageCount;
      const int newIndex = Clamp(mListInfo.ScrollIndex - (ScrollOffset / ScrollStep), 0, mListInfo.ItemCount- mListInfo.ItemInPageCount) ;
      if(newIndex != mListInfo.ScrollIndex) {
        mListInfo.ScrollIndex = newIndex;
        mScrollStart = Pos;
        mCaptureScroll = true;
      }
      Redraw();
    }
  }
  return false;
}

#ifdef WIN32
bool WndListFrame::OnLButtonDownNotify(Window* pWnd, const POINT& Pos) { 
  POINT NewPos(Pos);
  pWnd->ToScreen(NewPos);
  ToClient(NewPos);

  return OnLButtonDown(NewPos); 
}
#endif

bool WndListFrame::OnLButtonDown(const POINT& Pos) {
  mMouseDown=true;
  mCaptureScroll = false;
  SetCapture();
  if (PtInRect(&rcScrollBarButton, Pos))  // see if click is on scrollbar handle
  {
    mMouseScrollBarYOffset = max(0, (int)Pos.y - (int)rcScrollBarButton.top);  // start mouse drag
    mCaptureScrollButton=true;

  } else if (PtInRect(&rcScrollBar, Pos)) { // clicked in scroll bar up/down/pgup/pgdn

    if (Pos.y - rcScrollBar.top < (ScrollbarWidth)) { // up arrow
      mListInfo.ScrollIndex = max(0, mListInfo.ScrollIndex- 1);
    } else if (rcScrollBar.bottom -Pos.y < (ScrollbarWidth)  ) { //down arrow
      mListInfo.ScrollIndex = max(0,min(mListInfo.ItemCount- mListInfo.ItemInPageCount, mListInfo.ScrollIndex+ 1));
    } else if (Pos.y < rcScrollBarButton.top) { // page up
      mListInfo.ScrollIndex = max(0, mListInfo.ScrollIndex- mListInfo.ItemInPageCount);
    } else if (mListInfo.ItemCount > mListInfo.ScrollIndex+ mListInfo.ItemInPageCount) {  // page down
        mListInfo.ScrollIndex = min ( mListInfo.ItemCount- mListInfo.ItemInPageCount, mListInfo.ScrollIndex +mListInfo.ItemInPageCount);
    }
    Redraw();
  } else {
      mScrollStart = Pos;
  }
  return true;
}

bool WndListFrame::OnLButtonUp(const POINT& Pos) {
  ReleaseCapture();

  if(mMouseDown) {
    mMouseDown=false;
    if(!mCaptureScrollButton) {

      if (!mClients.empty()) {
        RECT Rc = {};
        SelectItemFromScreen(Pos.x, Pos.y, &Rc, mCaptureScroll);
        mClients.front()->SetFocus();
      }
    }
  }
  mCaptureScroll = false;
  mCaptureScrollButton = false;
  return true;
}

inline int WndListFrame::GetScrollBarHeight (void)
{
  const int h = GetHeight() - 2 * GetScrollBarTop();
  if(mListInfo.ItemCount ==0) {
    return h;
  } else {
    return max(ScrollbarWidth, _MulDiv(h, mListInfo.ItemInPageCount, mListInfo.ItemCount));
  }
}

inline int WndListFrame::GetScrollIndexFromScrollBarTop(int iScrollBarTop) {
    const int h = GetHeight() - 2 * GetScrollBarTop();
    if ((h - GetScrollBarHeight()) == 0) {
        return 0;
    }

    // Number of item excluding first Page
    const int NbHidden = mListInfo.ItemCount - mListInfo.ItemInPageCount;



    return max(0,
            min(NbHidden,
                max(0, ((NbHidden * (iScrollBarTop - GetScrollBarTop())) / (h - GetScrollBarHeight())))
            )
        );
}

inline int WndListFrame::GetScrollBarTopFromScrollIndex()
{
  int iRetVal=0;
  int h = GetHeight() - 2 * GetScrollBarTop();
  if (mListInfo.ItemCount - mListInfo.ItemInPageCount ==0) {
    iRetVal = h;
  }
  else {
    iRetVal = 
      ( GetScrollBarTop() + 
        (mListInfo.ScrollIndex) * ( h-GetScrollBarHeight() )
      /(mListInfo.ItemCount - mListInfo.ItemInPageCount)
      );
  }
  return iRetVal;
}

