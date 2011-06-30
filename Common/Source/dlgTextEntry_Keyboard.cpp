/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include "lk8000.h"
#include "Utils.h"
#include "dlgTools.h"
#include "externs.h"
#include "InfoBoxLayout.h"

#include "utils/heapcheck.h"
using std::min;
using std::max;

static WndForm *wf=NULL;
static WndOwnerDrawFrame *wGrid=NULL;

#define MAX_TEXTENTRY 40
static unsigned int cursor = 0;
static unsigned int max_width = MAX_TEXTENTRY;
static TCHAR edittext[MAX_TEXTENTRY];
#define MAXENTRYLETTERS (sizeof(EntryLetters)/sizeof(EntryLetters[0])-1)

static void UpdateTextboxProp(void)
{
  WndProperty *wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpText"));
  if (wp) {
    wp->SetText(edittext);
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
      /* JMW this prevents cursor buttons from being used to enter
    case VK_RETURN:
      wf->SetModalResult(mrOK);
      return(0);
      */
  }
  return(1);
}

static void OnKey(WindowControl * Sender)
{
  TCHAR *Caption = Sender->GetCaption();
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  if (cursor < max_width-1)
    {
      edittext[cursor++] = Caption[0];
    }
  UpdateTextboxProp();
}

static void OnDel(WindowControl * Sender)
{
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

static void ClearText(void)
{
  cursor = 0;
  memset(edittext, 0, sizeof(TCHAR)*MAX_TEXTENTRY); 
  UpdateTextboxProp();
}


static void OnClear(WindowControl * Sender)
{	
  #ifndef DISABLEAUDIO
  if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
  #endif
  ClearText();
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnKey),
  DeclareCallBackEntry(OnClear),
  DeclareCallBackEntry(OnOk),
  DeclareCallBackEntry(OnDel),
  DeclareCallBackEntry(OnDate),
  DeclareCallBackEntry(OnTime),
  DeclareCallBackEntry(NULL)
};

void dlgTextEntryKeyboardShowModal(TCHAR *text, int width) 
{
  wf = NULL;
  wGrid = NULL;
  if (width==0) {
    width = MAX_TEXTENTRY;
  }
  max_width = min(MAX_TEXTENTRY, width);
  char filename[MAX_PATH];
  if (InfoBoxLayout::landscape) 
  {
    LocalPathS(filename, TEXT("frmTextEntry_Keyboard_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
			filename, 
			hWndMainWindow,			  
			TEXT("IDR_XML_TEXTENTRY_KEYBOARD_L"));
    if (!wf) return;
  } else {
    LocalPathS(filename, TEXT("frmTextEntry_Keyboard.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
			filename, 
			hWndMainWindow,			  
			TEXT("IDR_XML_TEXTENTRY_KEYBOARD"));
    if (!wf) return;
  }

  wGrid = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmGrid"));

  cursor = 0;
  ClearText();

  /*edittext[0]= 0;
    edittext[1]= 0;*/

  if (_tcslen(text)>0) {
    _tcsupr(text);
    _tcsncpy(edittext, text, max_width-1);
    edittext[max_width-1]= 0;
    // position cursor at the end of imported text
    cursor=_tcslen(text); 
  }

  UpdateTextboxProp();
  wf->SetKeyDownNotify(FormKeyDown);
  wf->ShowModal();
  _tcsncpy(text, edittext, max_width);
  text[max_width-1]=0;
  delete wf;
  wf=NULL; //@ 101027
}

void dlgTextEntryShowModal(TCHAR *text, int width)
{
      dlgTextEntryKeyboardShowModal(text, width);
}

