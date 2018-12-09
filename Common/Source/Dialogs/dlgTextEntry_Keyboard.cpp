/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTextEntry_Keyboard.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include <ctype.h>
#include "Event/Event.h"
#include "Sound/Sound.h"
#include "resource.h"


static bool first= false;
static WndForm *wf=NULL;
static WndProperty * wKeyboardPopupWndProperty = nullptr;

#define MAX_TEXTENTRY 40
static unsigned int cursor = 0;
static unsigned int max_width = MAX_TEXTENTRY;
static TCHAR edittext[MAX_TEXTENTRY];
#define MAXENTRYLETTERS (sizeof(EntryLetters)/sizeof(EntryLetters[0])-1)

void ReduceKeysByWaypointList(void);
void  ReduceKeysByAirspaceList(void);
void RemoveKeys(char *EnabledKeyString, unsigned char size);
#define KEYRED_NONE     0
#define KEYRED_WAYPOINT 1
#define KEYRED_AIRSPACE 2
#define MAX_SEL_LIST_SIZE       120
#define NO_LAYOUTS 2
#define UPPERCASE 0
#define LOWERCASE 1
uint8_t  KeyboardLayout =UPPERCASE;
uint8_t  WaypointKeyRed = KEYRED_NONE;

 int IdenticalIndex=-1;
 int IdenticalOffset = 0;


char ToUpper(char in)
{
  return in;
/*
	if(in == '\xD6') return '\xD6'; // Ö -> Ö
	if(in == '\xDC') return '\xDC'; // Ü -> Ü
	if(in == '\xC4') return '\xC4'; // Ä -> Ä
	if(in == '\xF6') return '\xD6'; // ö -> Ö
	if(in == '\xFC') return '\xDC'; // ü -> Ü
	if(in == '\xE4') return '\xC4'; // ä -> Ä*/
//	return toupper(in);
}

static void UpdateTextboxProp(void)
{

  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpText"));
  if (wp) {
    wp->SetText(edittext);

    if(WaypointKeyRed == KEYRED_WAYPOINT)
      wp->SetCaption(MsgToken(1226));
    else
      if(WaypointKeyRed == KEYRED_AIRSPACE)
        wp->SetCaption(MsgToken(68));
      else
        wp->SetCaption(MsgToken(711));

  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpUnit"));
  if(wp && wKeyboardPopupWndProperty) {
      DataField* pField = wKeyboardPopupWndProperty->GetDataField();
      if(pField) {
        wp->SetCaption(pField->GetUnits());
        wp->RefreshDisplay();
        wp->Redraw();
      }
  }


  {
    WndButton *wb;
    if(WaypointKeyRed == KEYRED_WAYPOINT)
    {
      ReduceKeysByWaypointList();

	  wb =  (WndButton*) wf->FindByName(TEXT("prpDate")); if(wb != NULL) wb->SetVisible(false);
	  wb =  (WndButton*) wf->FindByName(TEXT("prpTime")); if(wb != NULL) wb->SetVisible(false);
    }

    if(WaypointKeyRed == KEYRED_AIRSPACE)
    {
        ReduceKeysByAirspaceList();

          wb =  (WndButton*) wf->FindByName(TEXT("prpDate")); if(wb != NULL) wb->SetVisible(false);
          wb =  (WndButton*) wf->FindByName(TEXT("prpTime")); if(wb != NULL) wb->SetVisible(false);
    }
    
    wp = (WndProperty*)wf->FindByName(TEXT("prpMatch")); 
    if(WaypointKeyRed == KEYRED_NONE)
    {
   //   if(cursor < GC_SUB_STRING_THRESHOLD/*1*/)   /* enable all keys if no char entered */
      {

       char Charlist[MAX_SEL_LIST_SIZE]={"abcdefghijklmnopqrstuvwxyz?!+$%#/()=:*_ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890.@- \xD6\xDC\xC4"};

        RemoveKeys((char*)Charlist , sizeof(Charlist));
      }
      if(wp != NULL) wp->SetVisible(false);
    }
    else
    {
      if(wp != NULL) wp->SetVisible(true);
    }
/*
    CharUpper(szLanguageFile);
    BOOL bGerChar = false;
    if( _tcscmp(szLanguageFile,_T("GERMAN.LNG"))==0)
	  bGerChar = true;

    wb = (WndButton*) wf->FindByName(TEXT("prpAe")); if(wb != NULL) wb->SetVisible(bGerChar);
    wb = (WndButton*) wf->FindByName(TEXT("prpOe")); if(wb != NULL) wb->SetVisible(bGerChar);
    wb = (WndButton*) wf->FindByName(TEXT("prpUe")); if(wb != NULL) wb->SetVisible(bGerChar);
*/
  }
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
  switch(KeyCode & 0xffff){
    case KEY_LEFT:
      if (cursor<1)
        return true; // min width
      cursor--;
      edittext[cursor] = 0;
      UpdateTextboxProp();
      return true;
  }
  return false;
}


static void ClearText(void)
{
  cursor = 0;
  memset(edittext, 0, sizeof(TCHAR)*MAX_TEXTENTRY);

  UpdateTextboxProp();
}

static void OnKey(WndButton* pWnd) {
    LKASSERT(pWnd);
    if(!pWnd) return;

    if (first) {
        ClearText();
        first = false;
    }
    PlayResource(TEXT("IDR_WAV_CLICK"));
    const TCHAR *Caption = pWnd->GetWndText();
    if (cursor < max_width - 1) {
        edittext[cursor++] = ToUpper(Caption[0]);
    }
    UpdateTextboxProp();
}


static void OnShift(WndButton* pWnd) {
    LKASSERT(pWnd);
    if(!pWnd) return;

    KeyboardLayout++;
    KeyboardLayout %= NO_LAYOUTS;
    UpdateTextboxProp();
}




static void OnSpace(WndButton* pWnd) {
    LKASSERT(pWnd);
    if(!pWnd) return;

    if (first) {
        ClearText();
        first = false;
    }
    PlayResource(TEXT("IDR_WAV_CLICK"));
    TCHAR Caption = ' ';

    if(WaypointKeyRed == KEYRED_NONE) Caption = ' ';
    if (cursor < max_width - 1) {
        edittext[cursor++] = ToUpper(Caption);
    }
    UpdateTextboxProp();
}


static void OnDel(WndButton* pWnd)
{
  first = false;
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if (cursor >0) {
	edittext[--cursor] = '\0';
  }

  UpdateTextboxProp();
}

static void OnTime(WndButton* pWnd)
{
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if ( (cursor+6)<(max_width-1) ) {
	TCHAR ltime[10];
	_stprintf(ltime,_T("%02d%02d%02d"),GPS_INFO.Hour,GPS_INFO.Minute,GPS_INFO.Second);
	_tcscat(&edittext[cursor],ltime);
	edittext[cursor+6] = '\0';
	cursor+=6;
  }
  UpdateTextboxProp();
}

static void OnDate(WndButton* pWnd)
{
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if ( (cursor+6)<(max_width-1) ) {
	int nyear=GPS_INFO.Year;
	if (nyear>2000)
		// 2011 = 11
		nyear-=2000;
	else
		// 1998 = 98
		nyear-=1900;

	if (nyear<0||nyear>99) nyear=0;
	TCHAR ltime[10];
	_stprintf(ltime,_T("%02d%02d%02d"),nyear,GPS_INFO.Month,GPS_INFO.Day);
	_tcscat(&edittext[cursor],ltime);
	edittext[cursor+6] = '\0';
	cursor+=6;
  }
  UpdateTextboxProp();
}

static void OnOk(WndButton* pWnd)
{
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}



static void OnClear(WndButton* pWnd)
{
  PlayResource(TEXT("IDR_WAV_CLICK"));
  ClearText();
}

static void OnHelpClicked(WindowControl* Sender){
    if(wKeyboardPopupWndProperty) {
        wKeyboardPopupWndProperty->OnHelp();
    }
}

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnKey),
  ClickNotifyCallbackEntry(OnClear),
  ClickNotifyCallbackEntry(OnOk),
  ClickNotifyCallbackEntry(OnDel),
  ClickNotifyCallbackEntry(OnSpace),
  ClickNotifyCallbackEntry(OnShift),
  ClickNotifyCallbackEntry(OnDate),
  ClickNotifyCallbackEntry(OnTime),
  OnHelpCallbackEntry(OnHelpClicked),
  EndCallBackEntry()
};

void dlgTextEntryKeyboardShowModal(TCHAR *text, int width, unsigned ResID)
{
  wf = NULL;
  if (width==0) {
    width = MAX_TEXTENTRY;
  }
  max_width = min(MAX_TEXTENTRY, width);
  wf = dlgLoadFromXML(CallBackTable, ResID);
  if (!wf) return;

 // cursor = _tcslen(text);
  ClearText();

  if (_tcslen(text)>0) {
  //  CharUpper(text);
    LK_tcsncpy(edittext, text, max_width-1);
    // show previous test.
    // this text is replaced by first key down
    // but used if "OK" is clicked first for don't reset current value.
  }
  cursor = _tcslen(edittext);
  UpdateTextboxProp();

  WindowControl* pBtHelp = wf->FindByName(TEXT("cmdHelp"));
  if(pBtHelp) {
     pBtHelp->SetVisible(wKeyboardPopupWndProperty && wKeyboardPopupWndProperty->HasHelpText());
  }

  wf->SetKeyDownNotify(FormKeyDown);
  wf->ShowModal();
  LK_tcsncpy(text, edittext, max_width-1);
 // cursor = _tcslen(text);
  delete wf;
  wf=NULL;
}

int  dlgTextEntryShowModal(TCHAR *text, int width, bool WPKeyRed)
{
	first = false;
	WaypointKeyRed = WPKeyRed;
	dlgTextEntryKeyboardShowModal(text, width, ScreenLandscape ? IDR_XML_TEXTENTRY_KEYBOARD_L : IDR_XML_TEXTENTRY_KEYBOARD_P);
	return IdenticalIndex;
}

int  dlgTextEntryShowModalAirspace(TCHAR *text, int width)
{
        first = false;

        WaypointKeyRed = KEYRED_AIRSPACE;
        dlgTextEntryKeyboardShowModal(text, width, ScreenLandscape ? IDR_XML_TEXTENTRY_KEYBOARD_L : IDR_XML_TEXTENTRY_KEYBOARD_P);
        return IdenticalIndex;
}

void dlgNumEntryShowModal(TCHAR *text, int width, bool WPKeyRed)
{  
    first = true;
	WaypointKeyRed = WPKeyRed;
	dlgTextEntryKeyboardShowModal(text, width, ScreenLandscape ? IDR_XML_NUMENTRY_KEYBOARD_L : IDR_XML_NUMENTRY_KEYBOARD_P);
}

BOOL dlgKeyboard(WndProperty* theProperty){
    BOOL Ret = FALSE;

    wKeyboardPopupWndProperty = theProperty;
	DataField* pField = theProperty->GetDataField();
	if(pField) {
		if(pField->CreateKeyboard()){
			theProperty->RefreshDisplay();
			Ret = TRUE;
		}
	}
    wKeyboardPopupWndProperty = nullptr;

	return Ret;
}



void ReduceKeysByWaypointList(void)
{

char SelList[MAX_SEL_LIST_SIZE]={""};
unsigned int NumChar=0;
bool CharEqual = true;

char Charlist[MAX_SEL_LIST_SIZE]={"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890.@-_?!+$%#/()=:* \xD6\xDC\xC4"};


unsigned int i,j,EqCnt=WayPointList.size();


WndProperty *wp;
TCHAR Found[NAME_SIZE + 1];
SelList[0] = '\0';
unsigned int NameLen=0;
 int Offset=0;
unsigned int k =0;
IdenticalOffset =999;
IdenticalIndex = -1;
  if(cursor < GC_SUB_STRING_THRESHOLD/*1*/)   /* enable all keys if no char entered */
  {
    RemoveKeys((char*)Charlist , sizeof(Charlist));
  }
  else
  {
    EqCnt=0; /* reset number of found waypoints */
    NumChar =0;
    for (i=NUMRESWP; i< WayPointList.size(); i++)
    {
      TCHAR wname[EXT_NAMESIZE];
      _sntprintf(wname,EXT_NAMESIZE, _T("%s %s"), WayPointList[i].Name, WayPointList[i].Code);
      NameLen =  _tcslen(wname);
      Offset = 0;
      if(cursor > NameLen)
	CharEqual = false;
      else
      {
        do
        {
          k=0;
          CharEqual = true;
          while((k < (cursor)) && ((k+Offset) < NameLen) && CharEqual)
          {
            LKASSERT(k < MAX_TEXTENTRY);
            LKASSERT((k+Offset) < NameLen);
            TCHAR ac = (TCHAR)wname[k+Offset];
            TCHAR bc = (TCHAR)edittext[k];
            if(  toupper(ac) !=   toupper(bc) ) /* waypoint has string ?*/
            {
              CharEqual = false;
            }
            k++;
          }
          Offset++;
        }
        while(((Offset-1+cursor) < NameLen) && !CharEqual );
        Offset--;
      }


      if(CharEqual)
      {

	if(Offset < IdenticalOffset)
        {
          IdenticalIndex = i; /* remember first found equal name */
          IdenticalOffset = Offset; /* remember first found equal name */
		   // StartupStore(_T("Found Best Fit %i Idx %i %s\n"), i, IdenticalIndex, WayPointList[IdenticalIndex].Name);
        }
        EqCnt++;
        LKASSERT((cursor+Offset)<=EXT_NAMESIZE);
        LKASSERT(i<=WayPointList.size());
        TCHAR newChar = ToUpper(wname[cursor+Offset]);
        bool existing = false;
        j=0;
        while(( j < NumChar) && (!existing))  /* new character already in list? */
        {
     //     StartupStore(_T(". j=%i  MAX_SEL_LIST_SIZE= %i\n"),j,MAX_SEL_LIST_SIZE);
          LKASSERT(j<MAX_SEL_LIST_SIZE);
          if(SelList[j] == (unsigned char)newChar)
		  existing = true;
          j++;
        }

        if(!existing && (NumChar <MAX_SEL_LIST_SIZE))  /* add new character to key enable list */
        {
     //     StartupStore(_T(". j=%i  MAX_SEL_LIST_SIZE= %i\n"),j,MAX_SEL_LIST_SIZE);
          LKASSERT(NumChar<MAX_SEL_LIST_SIZE);
          SelList[NumChar++] = ToUpper(newChar);
     //     SelList[NumChar++] = ToUpper(newChar);
        }
      }
    }

    SelList[NumChar++] = '\0';
    RemoveKeys((char*)SelList, NumChar);
    wp = (WndProperty*)wf->FindByName(TEXT("prpText"));

    if (wp)
    {
      if(EqCnt ==1)
      {
	LKASSERT(IdenticalIndex<= (int)WayPointList.size());
	    wp->SetText(WayPointList[IdenticalIndex].Name);
      }
      else
      {
        if((cursor >0) &&  (EqCnt >0))
        {
          LKASSERT(cursor < NAME_SIZE);
          LKASSERT(IdenticalIndex<=(int)WayPointList.size());
          _stprintf(Found,_T("%s"),WayPointList[IdenticalIndex].Name);
	  for( i = 0; i < cursor; i++)
	     Found[i+IdenticalOffset] = ToUpper(WayPointList[IdenticalIndex].Name[i+IdenticalOffset]);
          wp->SetText(Found);
        }
      }
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMatch"));
  if (wp)
  {
    _stprintf(Found,_T("%s:%i"),MsgToken(948),EqCnt); /* _@M948_ Found */
    wp->SetCaption(Found);
  }
}


/*********************************************************

const CAirspaceList CAirspaceManager::GetAllAirspaces() const {
    ScopeLock guard(_csairspaces);
    return _airspaces;
}

**********************************************************************************/
void ReduceKeysByAirspaceList(void)
{
char SelList[MAX_SEL_LIST_SIZE]={""};
unsigned int NumChar=0;
bool CharEqual = true;
char Charlist[MAX_SEL_LIST_SIZE]={"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890.@-_?!+$%#/()=:* \xD6\xDC\xC4"};


unsigned int i,j,EqCnt=WayPointList.size();

CAirspaceList airspaclist =   CAirspaceManager::Instance().GetAllAirspaces();
WndProperty *wp;
TCHAR Found[NAME_SIZE + 1];
SelList[0] = '\0';
unsigned int NameLen=0;
 int Offset=0;
unsigned int k =0;
IdenticalOffset =999;
//IdenticalIndex = -1;
TCHAR IdenticalName[255]= _T("");
TCHAR AS_Name[255];


  if(cursor < GC_SUB_STRING_THRESHOLD/*1*/)   /* enable all keys if no char entered */
  {
  //  RemoveKeys((char*)Charlist, sizeof(Charlist));
    RemoveKeys((char*)Charlist , sizeof(Charlist));
  }
  else
  {
    EqCnt=0; /* reset number of found waypoints */
    NumChar =0;


    CAirspaceList::const_iterator it;

    for (it = airspaclist.begin(); it != airspaclist.end(); ++it)
    {

      _stprintf(AS_Name,_T("%s"),(*it)->Name());

      NameLen =  _tcslen(AS_Name);
      Offset = 0;
      if(cursor > NameLen)
        CharEqual = false;
      else
      {
        do
        {
          k=0;
          CharEqual = true;
          while((k < (cursor)) && ((k+Offset) < NameLen) && CharEqual)
          {
            LKASSERT(k < MAX_TEXTENTRY);
            LKASSERT((k+Offset) < NameLen);
            TCHAR ac = (TCHAR)AS_Name[k+Offset];
            TCHAR bc = (TCHAR)edittext[k];
            if(  toupper(ac) !=   toupper(bc) ) /* waypoint has string ?*/
            {
              CharEqual = false;
            }
            k++;
          }
          Offset++;
        }
        while(((Offset-1+cursor) < NameLen) && !CharEqual );
        Offset--;
      }


      if(CharEqual)
      {

        if(Offset < IdenticalOffset)
        {
          _stprintf(IdenticalName ,_T("%s"),AS_Name);
          IdenticalOffset = Offset; /* remember first found equal name */
                   // StartupStore(_T("Found Best Fit %i Idx %i %s\n"), i, IdenticalIndex, WayPointList[IdenticalIndex].Name);
        }
        EqCnt++;
        LKASSERT((cursor+Offset)<=NAME_SIZE);
    //    LKASSERT(i<=WayPointList.size());
        TCHAR newChar = ToUpper(AS_Name[cursor+Offset]);
        bool existing = false;
        j=0;
        while(( j < NumChar) && (!existing))  /* new character already in list? */
        {
     //     StartupStore(_T(". j=%i  MAX_SEL_LIST_SIZE= %i\n"),j,MAX_SEL_LIST_SIZE);
          LKASSERT(j<MAX_SEL_LIST_SIZE);
          if(SelList[j] == (unsigned char)newChar)
                existing = true;
          j++;
        }

        if(!existing && (NumChar <MAX_SEL_LIST_SIZE))  /* add new character to key enable list */
        {
     //     StartupStore(_T(". j=%i  MAX_SEL_LIST_SIZE= %i\n"),j,MAX_SEL_LIST_SIZE);
          LKASSERT(NumChar<MAX_SEL_LIST_SIZE);
          SelList[NumChar++] = toupper(newChar);
          SelList[NumChar++] = tolower(newChar);
        }
      }
    }

    SelList[NumChar++] = '\0';
    RemoveKeys((char*)SelList, NumChar);
    wp = (WndProperty*)wf->FindByName(TEXT("prpText"));

    if (wp)
    {
      if(EqCnt ==1)
      {
        LKASSERT(IdenticalIndex<= (int)airspaclist.size());
            wp->SetText(IdenticalName  );
      }
      else
      {
        if((cursor >0) &&  (EqCnt >0))
        {
          LKASSERT(cursor < NAME_SIZE);

          _stprintf(Found,_T("%s"),IdenticalName );
          for( i = 0; i < cursor; i++)
             Found[i+IdenticalOffset] = ToUpper(IdenticalName[i+IdenticalOffset]);
        }
      }
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMatch"));
  if (wp)
  {
    _stprintf(Found,_T("%s:%i"),MsgToken(948),EqCnt); /* _@M948_ Found */
    wp->SetCaption(Found);
  }
}




void RemoveKeys(char *EnabledKeyString, unsigned char size)
{

bool bA=false, bB=false, bC=false, bD=false, bE=false, bF=false, bG=false, bH=false, bI=false,
     bJ=false, bK=false, bL=false, bM=false, bN=false, bO=false, bP=false, bQ=false, bR=false,
     bS=false, bT=false, bU=false, bV=false, bW=false, bX=false, bY=false, bZ=false, b0=false,
     b1=false, b2=false, b3=false, b4=false, b5=false, b6=false, b7=false, b8=false, b9=false,

     b_A=false, b_B=false, b_C=false, b_D=false, b_E=false, b_F=false, b_G=false, b_H=false, b_I=false,
     b_J=false, b_K=false, b_L=false, b_M=false, b_N=false, b_O=false, b_P=false, b_Q=false, b_R=false,
     b_S=false, b_T=false, b_U=false, b_V=false, b_W=false, b_X=false, b_Y=false, b_Z=false, b_0=false,
     b_1=false, b_2=false, b_3=false, b_4=false, b_5=false, b_6=false, b_7=false, b_8=false, b_9=false,

     bUe=false, bOe=false, bAe=false, bDot=false, bMin=false, bAt=false,  bSpace=false ,
     b_Dot=false, b_Min=false, b_At=false ;

unsigned int i=0;


    if(KeyboardLayout == UPPERCASE)
    {
      for (i = 0; i < size; i++ )
      {
	switch  (EnabledKeyString[i])
	{
	  case 'A': bA = true; break;
	  case 'B': bB = true; break;
	  case 'C': bC = true; break;
	  case 'D': bD = true; break;
	  case 'E': bE = true; break;
	  case 'F': bF = true; break;
	  case 'G': bG = true; break;
	  case 'H': bH = true; break;
	  case 'I': bI = true; break;
	  case 'J': bJ = true; break;
	  case 'K': bK = true; break;
	  case 'L': bL = true; break;
	  case 'M': bM = true; break;
	  case 'N': bN = true; break;
	  case 'O': bO = true; break;
	  case 'P': bP = true; break;
	  case 'Q': bQ = true; break;
	  case 'R': bR = true; break;
	  case 'S': bS = true; break;
	  case 'T': bT = true; break;
	  case 'U': bU = true; break;
	  case 'V': bV = true; break;
	  case 'W': bW = true; break;
	  case 'X': bX = true; break;
	  case 'Y': bY = true; break;
	  case 'Z': bZ = true; break;
/*
	  case '\xF6': case '\xD6': bUe = true; break;    // ü Ü
	  case '\xFC': case '\xDC': bOe = true; break;  //ö Ö
	  case '\xE4': case '\xC4': bAe = true; break;  // ä Ä
*/
	  case '0':  b0 = true; break;
	  case '1':  b1 = true; break;
	  case '2':  b2 = true; break;
	  case '3':  b3 = true; break;
	  case '4':  b4 = true; break;
	  case '5':  b5 = true; break;
	  case '6':  b6 = true; break;
	  case '7':  b7 = true; break;
	  case '8':  b8 = true; break;
	  case '9':  b9 = true; break;

	  case '.':  bDot = true; break;
	  case '-':  bMin = true; break;
	  case '@':  bAt  = true; break;
	  case ' ':  bSpace = true; break;

	  default: break;
	}
      }
    }

    if(KeyboardLayout == LOWERCASE)
    {
      for (i = 0; i < size; i++ )
      {
        switch  (EnabledKeyString[i])
        {
          case 'a': b_A = true; break;
          case 'b': b_B = true; break;
          case 'c': b_C = true; break;
          case 'd': b_D = true; break;
          case 'e': b_E = true; break;
          case 'f': b_F = true; break;
          case 'g': b_G = true; break;
          case 'h': b_H = true; break;
          case 'i': b_I = true; break;
          case 'j': b_J = true; break;
          case 'k': b_K = true; break;
          case 'l': b_L = true; break;
          case 'm': b_M = true; break;
          case 'n': b_N = true; break;
          case 'o': b_O = true; break;
          case 'p': b_P = true; break;
          case 'q': b_Q = true; break;
          case 'r': b_R = true; break;
          case 's': b_S = true; break;
          case 't': b_T = true; break;
          case 'u': b_U = true; break;
          case 'v': b_V = true; break;
          case 'w': b_W = true; break;
          case 'x': b_X = true; break;
          case 'y': b_Y = true; break;
          case 'z': b_Z = true; break;
/*
          case '\xF6': case '\xD6': bUe = true; break;    // ü Ü
          case '\xFC': case '\xDC': bOe = true; break;  //ö Ö
          case '\xE4': case '\xC4': bAe = true; break;  // ä Ä
*/
          case '?':  b_0 = true; break;
          case '!':  b_1 = true; break;
          case '+':  b_2 = true; break;
          case '$':  b_3 = true; break;
          case '%':  b_4 = true; break;
          case '#':  b_5 = true; break;
          case '/':  b_6 = true; break;
          case '(':  b_7 = true; break;
          case ')':  b_8 = true; break;
          case '=':  b_9 = true; break;

          case ':':  b_Dot = true; break;
          case '_':  b_Min = true; break;
          case '*':  b_At  = true; break;
        /*  case '_':  bUn  = true; break;*/
          case ' ':  bSpace = true; break;

          default: break;
        }
      }
    }


  WndButton *wb;

	wb =  (WndButton*) wf->FindByName(TEXT("prpA")); if(wb != NULL) wb->SetVisible(bA);
	wb =  (WndButton*) wf->FindByName(TEXT("prpB")); if(wb != NULL) wb->SetVisible(bB);
	wb =  (WndButton*) wf->FindByName(TEXT("prpC")); if(wb != NULL) wb->SetVisible(bC);
	wb =  (WndButton*) wf->FindByName(TEXT("prpD")); if(wb != NULL) wb->SetVisible(bD);
	wb =  (WndButton*) wf->FindByName(TEXT("prpE")); if(wb != NULL) wb->SetVisible(bE);
	wb =  (WndButton*) wf->FindByName(TEXT("prpF")); if(wb != NULL) wb->SetVisible(bF);
	wb =  (WndButton*) wf->FindByName(TEXT("prpG")); if(wb != NULL) wb->SetVisible(bG);
	wb =  (WndButton*) wf->FindByName(TEXT("prpH")); if(wb != NULL) wb->SetVisible(bH);
	wb =  (WndButton*) wf->FindByName(TEXT("prpI")); if(wb != NULL) wb->SetVisible(bI);
	wb =  (WndButton*) wf->FindByName(TEXT("prpJ")); if(wb != NULL) wb->SetVisible(bJ);
	wb =  (WndButton*) wf->FindByName(TEXT("prpK")); if(wb != NULL) wb->SetVisible(bK);
	wb =  (WndButton*) wf->FindByName(TEXT("prpL")); if(wb != NULL) wb->SetVisible(bL);
	wb =  (WndButton*) wf->FindByName(TEXT("prpM")); if(wb != NULL) wb->SetVisible(bM);
	wb =  (WndButton*) wf->FindByName(TEXT("prpN")); if(wb != NULL) wb->SetVisible(bN);
	wb =  (WndButton*) wf->FindByName(TEXT("prpO")); if(wb != NULL) wb->SetVisible(bO);
	wb =  (WndButton*) wf->FindByName(TEXT("prpP")); if(wb != NULL) wb->SetVisible(bP);
	wb =  (WndButton*) wf->FindByName(TEXT("prpQ")); if(wb != NULL) wb->SetVisible(bQ);
	wb =  (WndButton*) wf->FindByName(TEXT("prpR")); if(wb != NULL) wb->SetVisible(bR);
	wb =  (WndButton*) wf->FindByName(TEXT("prpS")); if(wb != NULL) wb->SetVisible(bS);
	wb =  (WndButton*) wf->FindByName(TEXT("prpT")); if(wb != NULL) wb->SetVisible(bT);
	wb =  (WndButton*) wf->FindByName(TEXT("prpU")); if(wb != NULL) wb->SetVisible(bU);
	wb =  (WndButton*) wf->FindByName(TEXT("prpV")); if(wb != NULL) wb->SetVisible(bV);
	wb =  (WndButton*) wf->FindByName(TEXT("prpW")); if(wb != NULL) wb->SetVisible(bW);
	wb =  (WndButton*) wf->FindByName(TEXT("prpX")); if(wb != NULL) wb->SetVisible(bX);
	wb =  (WndButton*) wf->FindByName(TEXT("prpY")); if(wb != NULL) wb->SetVisible(bY);
	wb =  (WndButton*) wf->FindByName(TEXT("prpZ")); if(wb != NULL) wb->SetVisible(bZ);

	wb =  (WndButton*) wf->FindByName(TEXT("prp0")); if(wb != NULL) wb->SetVisible(b0);
	wb =  (WndButton*) wf->FindByName(TEXT("prp1")); if(wb != NULL) wb->SetVisible(b1);
	wb =  (WndButton*) wf->FindByName(TEXT("prp2")); if(wb != NULL) wb->SetVisible(b2);
	wb =  (WndButton*) wf->FindByName(TEXT("prp3")); if(wb != NULL) wb->SetVisible(b3);
	wb =  (WndButton*) wf->FindByName(TEXT("prp4")); if(wb != NULL) wb->SetVisible(b4);
	wb =  (WndButton*) wf->FindByName(TEXT("prp5")); if(wb != NULL) wb->SetVisible(b5);
	wb =  (WndButton*) wf->FindByName(TEXT("prp6")); if(wb != NULL) wb->SetVisible(b6);
	wb =  (WndButton*) wf->FindByName(TEXT("prp7")); if(wb != NULL) wb->SetVisible(b7);
	wb =  (WndButton*) wf->FindByName(TEXT("prp8")); if(wb != NULL) wb->SetVisible(b8);
	wb =  (WndButton*) wf->FindByName(TEXT("prp9")); if(wb != NULL) wb->SetVisible(b9);
        wb =  (WndButton*) wf->FindByName(TEXT("prpDot"))   ; if(wb != NULL) wb->SetVisible(bDot);
        wb =  (WndButton*) wf->FindByName(TEXT("prpMin"))   ; if(wb != NULL) wb->SetVisible(bMin);
        wb =  (WndButton*) wf->FindByName(TEXT("prpAt"))    ; if(wb != NULL) wb->SetVisible(bAt);

	/**************************** upercase **********************************************/
        wb =  (WndButton*) wf->FindByName(TEXT("prp_A")); if(wb != NULL) wb->SetVisible(b_A);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_B")); if(wb != NULL) wb->SetVisible(b_B);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_C")); if(wb != NULL) wb->SetVisible(b_C);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_D")); if(wb != NULL) wb->SetVisible(b_D);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_E")); if(wb != NULL) wb->SetVisible(b_E);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_F")); if(wb != NULL) wb->SetVisible(b_F);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_G")); if(wb != NULL) wb->SetVisible(b_G);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_H")); if(wb != NULL) wb->SetVisible(b_H);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_I")); if(wb != NULL) wb->SetVisible(b_I);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_J")); if(wb != NULL) wb->SetVisible(b_J);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_K")); if(wb != NULL) wb->SetVisible(b_K);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_L")); if(wb != NULL) wb->SetVisible(b_L);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_M")); if(wb != NULL) wb->SetVisible(b_M);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_N")); if(wb != NULL) wb->SetVisible(b_N);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_O")); if(wb != NULL) wb->SetVisible(b_O);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_P")); if(wb != NULL) wb->SetVisible(b_P);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_Q")); if(wb != NULL) wb->SetVisible(b_Q);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_R")); if(wb != NULL) wb->SetVisible(b_R);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_S")); if(wb != NULL) wb->SetVisible(b_S);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_T")); if(wb != NULL) wb->SetVisible(b_T);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_U")); if(wb != NULL) wb->SetVisible(b_U);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_V")); if(wb != NULL) wb->SetVisible(b_V);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_W")); if(wb != NULL) wb->SetVisible(b_W);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_X")); if(wb != NULL) wb->SetVisible(b_X);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_Y")); if(wb != NULL) wb->SetVisible(b_Y);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_Z")); if(wb != NULL) wb->SetVisible(b_Z);

        wb =  (WndButton*) wf->FindByName(TEXT("prp_0")); if(wb != NULL) wb->SetVisible(b_0);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_1")); if(wb != NULL) wb->SetVisible(b_1);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_2")); if(wb != NULL) wb->SetVisible(b_2);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_3")); if(wb != NULL) wb->SetVisible(b_3);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_4")); if(wb != NULL) wb->SetVisible(b_4);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_5")); if(wb != NULL) wb->SetVisible(b_5);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_6")); if(wb != NULL) wb->SetVisible(b_6);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_7")); if(wb != NULL) wb->SetVisible(b_7);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_8")); if(wb != NULL) wb->SetVisible(b_8);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_9")); if(wb != NULL) wb->SetVisible(b_9);

        wb =  (WndButton*) wf->FindByName(TEXT("prp_Dot"))   ; if(wb != NULL) wb->SetVisible(b_Dot);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_Min"))   ; if(wb != NULL) wb->SetVisible(b_Min);
        wb =  (WndButton*) wf->FindByName(TEXT("prp_At"))    ; if(wb != NULL) wb->SetVisible(b_At);
        /***********************************************************************************************/

        wb =  (WndButton*) wf->FindByName(TEXT("prpUe")); if(wb != NULL) wb->SetVisible(bUe);
        wb =  (WndButton*) wf->FindByName(TEXT("prpOe")); if(wb != NULL) wb->SetVisible(bOe);
        wb =  (WndButton*) wf->FindByName(TEXT("prpAe")); if(wb != NULL) wb->SetVisible(bAe);


//      wb =  (WndButton*) wf->FindByName(TEXT("prpUn"))    ; if(wb != NULL) wb->SetVisible(bUn);
        wb =  (WndButton*) wf->FindByName(TEXT("prpSpace")) ; if(wb != NULL) wb->SetVisible(bSpace);

}
