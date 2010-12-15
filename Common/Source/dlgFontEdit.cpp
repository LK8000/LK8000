/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"


#include "externs.h"
#include "Utils.h"
#include "XCSoar.h"

extern HWND   hWndMainWindow;
extern void InitializeOneFont (HFONT * theFont, 
                               const TCHAR FontRegKey[] , 
                               LOGFONT autoLogFont, 
                               LOGFONT * LogFontUsed);
extern HFONT InfoWindowFont;
extern HFONT TitleWindowFont;
extern HFONT MapWindowFont;
extern HFONT TitleSmallWindowFont;
extern HFONT MapWindowBoldFont;
extern HFONT CDIWindowFont; // New
extern HFONT MapLabelFont;
extern HFONT StatisticsFont;

static WndForm *wf=NULL;
static LOGFONT OriginalLogFont;
static LOGFONT NewLogFont;
static LOGFONT resetLogFont;
static HFONT NewFont;
const static TCHAR * OriginalFontRegKey;
static bool IsInitialized=false;

void LoadGUI();

static void OnCloseClicked(WindowControl * Sender){
(void)Sender;
	wf->SetModalResult(mrOK);
}
static void OnCancelClicked(WindowControl * Sender){
(void)Sender;
	wf->SetModalResult(mrCancle);
}
static void OnResetClicked(WindowControl * Sender){
(void)Sender;

  NewLogFont=resetLogFont;
  LoadGUI();
}


static void RedrawSampleFont(void)
{
  if (!IsInitialized) {  
    return;
  }

  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontName"));
  if(wp) {
    _tcsncpy(NewLogFont.lfFaceName,wp->GetDataField()->GetAsString(), LF_FACESIZE-1); 
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontHeight"));
  if(wp) {
    NewLogFont.lfHeight = wp->GetDataField()->GetAsInteger();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontWeight"));
  if(wp) {
    NewLogFont.lfWeight= wp->GetDataField()->GetAsInteger();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontItalic"));
  if(wp) {
    if ( wp->GetDataField()->GetAsInteger() ) {
      NewLogFont.lfItalic=1;
    }
    else {
      NewLogFont.lfItalic=0;
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontPitchAndFamily"));
  if (wp) {
    NewLogFont.lfPitchAndFamily=wp->GetDataField()->GetAsInteger();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontTrueType"));
  if(wp) {
    if ( wp->GetDataField()->GetAsBoolean() ) {
      NewLogFont.lfQuality = ANTIALIASED_QUALITY;
    }
    else {
      NewLogFont.lfQuality = NONANTIALIASED_QUALITY;
    }
  }
  
  DeleteObject(NewFont);

  NewFont = CreateFontIndirect (&NewLogFont);

  if ( _tcscmp(OriginalFontRegKey, szRegistryFontMapWindowBoldFont) == 0 ) {
    wf->SetFont(NewFont);
    wf->SetTitleFont(NewFont);
    wf->SetVisible(false);
    wf->SetVisible(true);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontSample"));

  if(wp) {
    if (GetObjectType(NewFont) == OBJ_FONT) {
      wp->SetFont(NewFont);
      wp->SetCaption(TEXT("Sample Text 123"));
      wp->SetVisible(false);
      wp->SetVisible(true);
      wp->RefreshDisplay();
    }
    else {
      wp->SetCaption(TEXT("Error Creating Font!"));
      wp->RefreshDisplay();
    }
  }
}




static void OnFontNameData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RedrawSampleFont();

    break;
  }
}
static void OnFontWeightData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RedrawSampleFont();

    break;
  }
}
static void OnFontHeightData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RedrawSampleFont();

    break;
  }
}
static void OnFontItalicData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RedrawSampleFont();

    break;
  }
}

static void OnFontTrueTypeData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RedrawSampleFont();

    break;
  }
}
static void OnFontPitchAndFamilyData(DataField *Sender, DataField::DataAccessKind_t Mode){
  switch(Mode){
    case DataField::daGet:
    break;

    case DataField::daPut: 
    break;

    case DataField::daChange:
      RedrawSampleFont();

    break;
  }
}




static CallBackTableEntry_t CallBackTable[]={

  DeclareCallBackEntry(OnFontTrueTypeData),
  DeclareCallBackEntry(OnFontPitchAndFamilyData),
  DeclareCallBackEntry(OnFontItalicData),
  DeclareCallBackEntry(OnFontWeightData),
  DeclareCallBackEntry(OnFontHeightData),
  DeclareCallBackEntry(OnFontNameData),
  DeclareCallBackEntry(OnResetClicked),
  DeclareCallBackEntry(OnCancelClicked), 
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};


void SaveValues(const TCHAR * FontRegKey )
{
  // update reg key for font
  TCHAR sValue [256];
  wsprintf(sValue,TEXT("%d,%d,0,0,%d,%d,0,0,0,0,0,%d,%d,%s"),
                        NewLogFont.lfHeight,
                        NewLogFont.lfWidth,
                        NewLogFont.lfWeight,
                        NewLogFont.lfItalic,
                        NewLogFont.lfQuality,
                        NewLogFont.lfPitchAndFamily,
                        NewLogFont.lfFaceName);
  SetRegistryString(FontRegKey, sValue); 
}

void InitGUI(const TCHAR * FontDescription)
{ 
#define FONTEDIT_GUI_MAX_TITLE 128

  WndProperty* wp;

  TCHAR sTitle[FONTEDIT_GUI_MAX_TITLE];
  TCHAR sTitlePrefix[]=TEXT("Edit Font: ");

  _tcscpy(sTitle, sTitlePrefix);
  _tcsncpy(sTitle + _tcslen(sTitlePrefix), FontDescription,FONTEDIT_GUI_MAX_TITLE - _tcslen(sTitlePrefix) -1);

  wf->SetCaption(sTitle);

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontName"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText(TEXT("Tahoma"));
    dfe->addEnumText(TEXT("TahomaBD"));
    dfe->addEnumText(TEXT("DejaVu Sans Condensed"));
    // RLD ToDo code: add more font faces, and validate their availabiliy
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontPitchAndFamily"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN  _@M227_ = "Default" 
    dfe->addEnumText(gettext(TEXT("_@M227_")));
	// LKTOKEN  _@M304_ = "Fixed" 
    dfe->addEnumText(gettext(TEXT("_@M304_")));
	// LKTOKEN  _@M779_ = "Variable" 
    dfe->addEnumText(gettext(TEXT("_@M779_")));
  }
}


void LoadGUI() 
{
#define MAX_ENUM 10
  IsInitialized=false;
  int i=0;
  WndProperty* wp;

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontName"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    if (dfe) 
    {
      for (i=0 ;i < MAX_ENUM ; i++) { 
        dfe->Dec();
      } // rewind

      bool bFound=false;
      for (i=0 ;i < MAX_ENUM ; i++ ) {
        if (_tcsncmp(dfe->GetAsString(), NewLogFont.lfFaceName, LF_FACESIZE) == 0) {
          bFound=true;
          break;
        }
        dfe->Inc();
      }
      if (!bFound) {
        dfe->addEnumText(NewLogFont.lfFaceName);
        for (i=0 ;i < MAX_ENUM ; i++) { 
          dfe->Dec();
        } // rewind
        for (i=0 ;i < MAX_ENUM ; i++ ) {
          if (_tcsncmp(dfe->GetAsString(), NewLogFont.lfFaceName,LF_FACESIZE) == 0) {
            break;
          }
          dfe->Inc();
        }
      }
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontHeight"));
  if (wp) {
    DataFieldInteger * dfi;
    dfi = (DataFieldInteger*)wp->GetDataField();
    if (dfi)
    {
      dfi->Set(NewLogFont.lfHeight);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontWeight"));
  if (wp) {
    DataFieldInteger* dfi;
    dfi = (DataFieldInteger*)wp->GetDataField();
    if (dfi)
    {
      dfi->Set(NewLogFont.lfWeight);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontItalic"));
  if (wp) {
    DataFieldBoolean* dfb;
    dfb = (DataFieldBoolean*)wp->GetDataField();
    if (dfb)
    {
      dfb->Set(NewLogFont.lfItalic);
    }
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpFontPitchAndFamily"));
  if (wp) {
    DataFieldEnum * dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    if (dfe)
    {
      dfe->SetAsInteger(NewLogFont.lfPitchAndFamily);
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFontTrueType"));
  if (wp) {
    DataFieldBoolean* dfb;
    dfb = (DataFieldBoolean*)wp->GetDataField();
    if (dfb)
    {
      dfb->Set(NewLogFont.lfQuality == ANTIALIASED_QUALITY);
    }
    wp->RefreshDisplay();
  }

  IsInitialized=true;

  RedrawSampleFont();
}


bool dlgFontEditShowModal(const TCHAR * FontDescription, 
                          const TCHAR * FontRegKey, 
                          LOGFONT autoLogFont){

  bool bRetVal=false;
  char filename[MAX_PATH];
  IsInitialized=false;

  LocalPathS(filename, TEXT("dlgFontEdit.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_FONTEDIT"));


  int UseCustomFontsold = UseCustomFonts; 
  UseCustomFonts=1;// global var
  InitializeOneFont (&NewFont, 
                        FontRegKey, 
                        autoLogFont,
                        &OriginalLogFont);
  UseCustomFonts=UseCustomFontsold;


  OriginalFontRegKey=FontRegKey;
  NewLogFont=OriginalLogFont;
  resetLogFont = autoLogFont;


  if (wf) {

    InitGUI(FontDescription);
    LoadGUI();

    if (wf->ShowModal()==mrOK) {
      SaveValues(FontRegKey);  
      bRetVal=true;
    }
    delete wf;
  }
  wf = NULL;

  return bRetVal;
}

