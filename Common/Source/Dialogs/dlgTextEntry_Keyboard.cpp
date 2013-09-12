/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTextEntry_Keyboard.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InfoBoxLayout.h"
#include "Dialogs.h"
#include <ctype.h>

static bool first= true;
static WndForm *wf=NULL;
static WndProperty * wKeyboardPopupWndProperty;

#define MAX_TEXTENTRY 40
static unsigned int cursor = 0;
static unsigned int max_width = MAX_TEXTENTRY;
static TCHAR edittext[MAX_TEXTENTRY];
#define MAXENTRYLETTERS (sizeof(EntryLetters)/sizeof(EntryLetters[0])-1)

void ReduceKeysByWaypointList(void);
void RemoveKeys(char *EnabledKeyString, unsigned char size);
bool WaypointKeyRed = false;

char ToUpper(char in)
{
	if(in == '�') return '�';
	if(in == '�') return '�';
	if(in == '�') return '�';
	if(in == '�') return '�';
	if(in == '�') return '�';
	if(in == '�') return '�';
	if(in == ' ') return '_';
	if(in == '_') return '_';
	return toupper(in);
}

static void UpdateTextboxProp(void)
{

  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpText"));
  if (wp) {
    wp->SetText(edittext);

    if(WaypointKeyRed)
      wp->SetCaption(gettext(TEXT("_@M949_")));
    else
      wp->SetCaption(TEXT("Text"));
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
    if(WaypointKeyRed)
    {
      ReduceKeysByWaypointList();

	  wb =  (WndButton*) wf->FindByName(TEXT("prpDate")); if(wb != NULL) wb->SetVisible(false);
	  wb =  (WndButton*) wf->FindByName(TEXT("prpTime")); if(wb != NULL) wb->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpMatch"));; if(wp != NULL) wp->SetVisible(WaypointKeyRed);

  }
}

static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam) {
  switch(wParam & 0xffff){
    case VK_LEFT:
      if (cursor<1)
        return(0); // min width
      cursor--;
      edittext[cursor] = 0;
      UpdateTextboxProp();
      return(0);
  }
  return(1);
}


static void ClearText(void)
{
  cursor = 0;
  memset(edittext, 0, sizeof(TCHAR)*MAX_TEXTENTRY);

  UpdateTextboxProp();
}

static void OnKey(WindowControl * Sender)
{
if(first)
{
    ClearText();
    first = false;
}


  TCHAR *Caption = Sender->GetCaption();
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  if (cursor < max_width-1)
    {
      edittext[cursor++] = toupper(Caption[0]);
    }



  UpdateTextboxProp();
}





static void OnDel(WindowControl * Sender)
{
  first = false;
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  if (cursor >0) {
	edittext[--cursor] = '\0';
  }

  UpdateTextboxProp();
}

static void OnTime(WindowControl * Sender)
{
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  if ( (cursor+6)<(max_width-1) ) {
	TCHAR ltime[10];
	_stprintf(ltime,_T("%02d%02d%02d"),GPS_INFO.Hour,GPS_INFO.Minute,GPS_INFO.Second);
	_tcscat(&edittext[cursor],ltime);
	edittext[cursor+6] = '\0';
	cursor+=6;
  }
  UpdateTextboxProp();
}

static void OnDate(WindowControl * Sender)
{
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
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

static void OnOk(WindowControl * Sender)
{
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  wf->SetModalResult(mrOK);
}



static void OnClear(WindowControl * Sender)
{
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  ClearText();
}

static void OnHelpClicked(WindowControl * Sender){
  (void)Sender;

  wKeyboardPopupWndProperty->OnHelp();
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnKey),
  DeclareCallBackEntry(OnClear),
  DeclareCallBackEntry(OnOk),
  DeclareCallBackEntry(OnDel),
  DeclareCallBackEntry(OnDate),
  DeclareCallBackEntry(OnTime),
  DeclareCallBackEntry(OnHelpClicked),
  DeclareCallBackEntry(NULL)
};

void dlgTextEntryKeyboardShowModal(TCHAR *text, int width, const TCHAR* szFile, const TCHAR* szResource)
{

  first = true;
  wf = NULL;
  if (width==0) {
    width = MAX_TEXTENTRY;
  }
  max_width = min(MAX_TEXTENTRY, width);
  char filename[MAX_PATH];
    LocalPathS(filename, szFile);
    wf = dlgLoadFromXML(CallBackTable,
			filename,
			hWndMainWindow,
			szResource);
  if (!wf) return;

  cursor = 0;
  ClearText();

  if (_tcslen(text)>0) {
    _tcsupr(text);
    LK_tcsncpy(edittext, text, max_width-1);
    // position cursor at the end of imported text
    cursor=_tcslen(text);
  }

  UpdateTextboxProp();
  wf->SetKeyDownNotify(FormKeyDown);
  wf->ShowModal();
  LK_tcsncpy(text, edittext, max_width-1);
  delete wf;
  wf=NULL;
}

void dlgTextEntryShowModal(TCHAR *text, int width, bool WPKeyRed)
{
	WaypointKeyRed = WPKeyRed;
	if (ScreenLandscape)  {
		dlgTextEntryKeyboardShowModal(text, width, TEXT("frmTextEntry_Keyboard_L.xml"), TEXT("IDR_XML_TEXTENTRY_KEYBOARD_L"));
	} else {
		dlgTextEntryKeyboardShowModal(text, width, TEXT("frmTextEntry_Keyboard.xml"), TEXT("IDR_XML_TEXTENTRY_KEYBOARD"));
	}
}

void dlgNumEntryShowModal(TCHAR *text, int width, bool WPKeyRed)
{
	WaypointKeyRed = WPKeyRed;
	if (ScreenLandscape)  {
		dlgTextEntryKeyboardShowModal(text, width, TEXT("frmNumEntry_Keyboard_L.xml"), TEXT("IDR_XML_NUMENTRY_KEYBOARD_L"));
	} else {
		dlgTextEntryKeyboardShowModal(text, width, TEXT("frmNumEntry_Keyboard.xml"), TEXT("IDR_XML_NUMENTRY_KEYBOARD"));
	}
}

BOOL dlgKeyboard(WndProperty* theProperty){
    wKeyboardPopupWndProperty = theProperty;

	DataField* pField = theProperty->GetDataField();
	if(pField) {
		if(pField->CreateKeyboard()){
			theProperty->RefreshDisplay();
			return TRUE;
		}
	}
	return FALSE;
}



void ReduceKeysByWaypointList(void)
{
#define MAX_SEL_LIST_SIZE	60
char SelList[MAX_SEL_LIST_SIZE]={""};
unsigned int NumChar=0;
bool CharEqual = true;
char Charlist[MAX_SEL_LIST_SIZE]={"ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890.@-_ ���"};

unsigned int i,j,EqCnt=NumberOfWayPoints;
unsigned int IdenticalIndex=-1;
unsigned int IdenticalOffset = 0;

WndProperty *wp;
TCHAR Found[NAME_SIZE + 1];
SelList[0] = '\0';
unsigned int NameLen=0;
unsigned int Offset=0;
unsigned int k =0;

  if(cursor < GC_SUB_STRING_THRESHOLD/*1*/)   /* enable all keys if no char entered */
  {
    RemoveKeys((char*)Charlist, sizeof(Charlist));
  }
  else
  {
    EqCnt=0; /* reset number of found waypoints */
    NumChar =0;
    for (i=0; i< NumberOfWayPoints; i++)
    {
      NameLen =  _tcslen(WayPointList[i].Name);
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
            char ac = (char)WayPointList[i].Name[k+Offset];
            char bc = (char)edittext[k];
            if(  ToUpper(ac) !=   ToUpper(bc) ) /* waypoint has string ?*/
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
        EqCnt++;
        if(IdenticalIndex -1)
        {
          IdenticalIndex = i; /* remember first found equal name */
          IdenticalOffset = Offset; /* remember first found equal name */
        }
        TCHAR newChar = ToUpper(WayPointList[i].Name[cursor+Offset]);
        bool existing = false;
        j=0;
        while(( j < NumChar) && (!existing))  /* new character already in list? */
        {
     //     StartupStore(_T(". j=%i  MAX_SEL_LIST_SIZE= %i\n"),j,MAX_SEL_LIST_SIZE);
          LKASSERT(j<MAX_SEL_LIST_SIZE);
          if(SelList[j] == newChar)
        	existing = true;
          j++;
        }

        if(!existing && (NumChar <MAX_SEL_LIST_SIZE))  /* add new character to key enable list */
        {
     //     StartupStore(_T(". j=%i  MAX_SEL_LIST_SIZE= %i\n"),j,MAX_SEL_LIST_SIZE);
          LKASSERT(NumChar<MAX_SEL_LIST_SIZE);
          SelList[NumChar++] = newChar;
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
	    wp->SetText(WayPointList[IdenticalIndex].Name);
      }
      else
      {
        if((cursor >0) &&  (EqCnt >0))
        {
          LKASSERT(cursor < NAME_SIZE);
          _stprintf(Found,_T("%s"),WayPointList[IdenticalIndex].Name);
    	  for( i = 0; i < cursor; i++)
    	     Found[i+IdenticalOffset] = toupper(WayPointList[IdenticalIndex].Name[i+IdenticalOffset]);
          wp->SetText(Found);
        }
      }
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMatch"));
  if (wp)
  {
    _stprintf(Found,_T("%s:%i"),gettext(TEXT("_@M948_")),EqCnt); /* _@M948_ Found */
    wp->SetCaption(Found);
  }
}


void RemoveKeys(char *EnabledKeyString, unsigned char size)
{

bool bA=false, bB=false, bC=false, bD=false, bE=false, bF=false, bG=false, bH=false, bI=false,
	 bJ=false, bK=false, bL=false, bM=false, bN=false, bO=false, bP=false, bQ=false, bR=false,
	 bS=false, bT=false, bU=false, bV=false, bW=false, bX=false, bY=false, bZ=false, b0=false,
	 b1=false, b2=false, b3=false, b4=false, b5=false, b6=false, b7=false, b8=false, b9=false,
	 bDot=false, bMin=false, bAt=false,  bUn=false ;

unsigned int i=0;

  for (i = 0; i < size; i++ )
  {
	switch  (EnabledKeyString[i])
	{
	  case 'a': case 'A': bA = true; break;
	  case 'b': case 'B': bB = true; break;
	  case 'c': case 'C': bC = true; break;
	  case 'd': case 'D': bD = true; break;
	  case 'e': case 'E': bE = true; break;
	  case 'f': case 'F': bF = true; break;
	  case 'g': case 'G': bG = true; break;
	  case 'h': case 'H': bH = true; break;
	  case 'i': case 'I': bI = true; break;
	  case 'j': case 'J': bJ = true; break;
	  case 'k': case 'K': bK = true; break;
	  case 'l': case 'L': bL = true; break;
	  case 'm': case 'M': bM = true; break;
	  case 'n': case 'N': bN = true; break;
	  case 'o': case 'O': bO = true; break;
	  case 'p': case 'P': bP = true; break;
	  case 'q': case 'Q': bQ = true; break;
	  case 'r': case 'R': bR = true; break;
	  case 's': case 'S': bS = true; break;
	  case 't': case 'T': bT = true; break;
	  case 'u': case 'U': bU = true; break;
	  case 'v': case 'V': bV = true; break;
	  case 'w': case 'W': bW = true; break;
	  case 'x': case 'X': bX = true; break;
	  case 'y': case 'Y': bY = true; break;
	  case 'z': case 'Z': bZ = true; break;
/*
	  case '�': case '�': bUe = true; break;    // � �
	  case '�': case '�': bOe = true; break;  //� �
	  case '�': case '�': bAe = true; break;  // � �


	  case 'ü': case 'Ü': bUe = true; break;    // � �
	  case 'ö': case 'Ö': bOe = true; break;  //� �
	  case 'ä': case 'Ä': bAe = true; break;  // � �
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
	  case '_':  bUn  = true; break;
//	  case ' ':  bSpace = true; break;

	  default: break;
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
/*
	wb =  (WndButton*) wf->FindByName(TEXT("prpUe")); if(wb != NULL) wb->SetVisible(bUe);
	wb =  (WndButton*) wf->FindByName(TEXT("prpOe")); if(wb != NULL) wb->SetVisible(bOe);
	wb =  (WndButton*) wf->FindByName(TEXT("prpAe")); if(wb != NULL) wb->SetVisible(bAe);
*/
	wb =  (WndButton*) wf->FindByName(TEXT("prpDot"))   ; if(wb != NULL) wb->SetVisible(bDot);
	wb =  (WndButton*) wf->FindByName(TEXT("prpUn"))    ; if(wb != NULL) wb->SetVisible(bUn);
	wb =  (WndButton*) wf->FindByName(TEXT("prpSpace")) ; if(wb != NULL) wb->SetVisible(bUn);

	wb =  (WndButton*) wf->FindByName(TEXT("prpMin"))   ; if(wb != NULL) wb->SetVisible(bMin);
	wb =  (WndButton*) wf->FindByName(TEXT("prpAt"))    ; if(wb != NULL) wb->SetVisible(bAt);
}
