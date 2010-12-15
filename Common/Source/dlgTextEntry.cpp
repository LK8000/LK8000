/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTextEntry.cpp,v 8.2 2010/12/13 17:43:49 root Exp root $
*/

#include "StdAfx.h"
#include "XCSoar.h"
#include "Utils.h"
#include "dlgTools.h"
#include "externs.h"

static WndForm *wf=NULL;
static WndOwnerDrawFrame *wGrid=NULL;

#define MAX_TEXTENTRY 40
static unsigned int cursor = 0;
static int lettercursor=0;
static int max_width = MAX_TEXTENTRY;

static TCHAR edittext[MAX_TEXTENTRY];

static TCHAR EntryLetters[] = TEXT(" ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.-");

#define MAXENTRYLETTERS (sizeof(EntryLetters)/sizeof(EntryLetters[0])-1)

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static void OnTextPaint(WindowControl *Sender, HDC hDC) {
  RECT  rcgfx;
  HFONT hfOld;

  CopyRect(&rcgfx, Sender->GetBoundRect());
  // background is painted in the base-class
  hfOld = (HFONT)SelectObject(hDC, Sender->GetFont());
  SetBkMode(hDC, TRANSPARENT);
  SetTextColor(hDC, Sender->GetForeColor());

  // Do the actual painting of the text

  SIZE tsize;
  GetTextExtentPoint(hDC, edittext, _tcslen(edittext), &tsize);
  SIZE tsizec;
  GetTextExtentPoint(hDC, edittext, cursor, &tsizec);
  SIZE tsizea;
  GetTextExtentPoint(hDC, edittext, cursor+1, &tsizea);
  
  POINT p[5];
  p[0].x = 10;
  p[0].y = 20;

  p[2].x = p[0].x + tsizec.cx;
  p[2].y = p[0].y + tsize.cy+5;

  p[3].x = p[0].x + tsizea.cx;
  p[3].y = p[0].y + tsize.cy+5;

  p[1].x = p[2].x;
  p[1].y = p[2].y-2;

  p[4].x = p[3].x;
  p[4].y = p[3].y-2;

  SelectObject(hDC, GetStockObject(WHITE_PEN));
  Polyline(hDC, p+1, 4);

  SetBkMode(hDC, OPAQUE);
  ExtTextOut(hDC, p[0].x, p[0].y, ETO_OPAQUE, NULL, 
             edittext, _tcslen(edittext), NULL);
  SetBkMode(hDC, TRANSPARENT);

  SelectObject(hDC, hfOld);
}



static void UpdateCursor(void) {
  if (lettercursor>=(signed) MAXENTRYLETTERS) // 100501 FIXED
    lettercursor = 0;
  if (lettercursor<0)
    lettercursor = MAXENTRYLETTERS-1;
  edittext[cursor] = EntryLetters[lettercursor];

  if (wGrid != NULL)
    wGrid->Redraw();

}


static void MoveCursor(void) {
  if (cursor>=_tcslen(edittext)) {
    edittext[cursor+1] = 0;
  }
  for (lettercursor=0; lettercursor< (signed)MAXENTRYLETTERS; lettercursor++) { // 100501 FIXED
    if (edittext[cursor]== EntryLetters[lettercursor])
      break;
  }
  if (lettercursor== MAXENTRYLETTERS) {
    lettercursor = 0;
    edittext[cursor] = EntryLetters[lettercursor];
  }
  if (edittext[cursor]== 0) {
    lettercursor= 0;
  }
  UpdateCursor();
}


static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam) {
	(void)lParam; (void)Sender;
  switch(wParam & 0xffff){
    case VK_LEFT:
      if (cursor<1)
        return(0); // min width
      cursor--;
      MoveCursor();
      return(0);
    case VK_RIGHT:
      if ((int)cursor>=(max_width-2))
        return(0); // max width
      cursor++;
      MoveCursor();
      return(0);
    case VK_UP:
      lettercursor--;
      UpdateCursor();
      return(0);
    case VK_DOWN:
      lettercursor++;
      UpdateCursor();
      return(0);
    case VK_RETURN:
      wf->SetModalResult(mrOK);
      return(0);
  }
  return(1);
}



static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnTextPaint),
  DeclareCallBackEntry(NULL)
};


static void OnLeftClicked(WindowControl * Sender){
  (void)Sender;
  FormKeyDown(Sender, VK_LEFT, 0);
}

static void OnRightClicked(WindowControl * Sender){
  (void)Sender;
  FormKeyDown(Sender, VK_RIGHT, 0);
}

static void OnUpClicked(WindowControl * Sender){
  (void)Sender;
  FormKeyDown(Sender, VK_UP, 0);
}

static void OnDownClicked(WindowControl * Sender){
  (void)Sender;
  FormKeyDown(Sender, VK_DOWN, 0);
}



void dlgTextEntryHighscoreType(TCHAR *text, int width)
{
  wf = NULL;
  wGrid = NULL;

  if (width==0) {
    width = MAX_TEXTENTRY;
  }
  max_width = min(MAX_TEXTENTRY, width);

  char filename[MAX_PATH];
#ifndef GNAV
  LocalPathS(filename, TEXT("dlgTextEntry_T.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_TEXTENTRY_T"));
#else
  LocalPathS(filename, TEXT("dlgTextEntry.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_TEXTENTRY"));
#endif
  if (!wf) return;

  wGrid = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmGrid"));

  WndButton* wb;
  wb = (WndButton *)(wf->FindByName(TEXT("cmdClose")));
  if (wb) {
    wb->SetOnClickNotify(OnCloseClicked);
  }

  wb = (WndButton *)(wf->FindByName(TEXT("cmdLeft")));
  if (wb) {
    wb->SetOnClickNotify(OnLeftClicked);
  }

  wb = (WndButton *)(wf->FindByName(TEXT("cmdRight")));
  if (wb) {
    wb->SetOnClickNotify(OnRightClicked);
  }

  wb = (WndButton *)(wf->FindByName(TEXT("cmdUp")));
  if (wb) {
    wb->SetOnClickNotify(OnUpClicked);
  }

  wb = (WndButton *)(wf->FindByName(TEXT("cmdDown")));
  if (wb) {
    wb->SetOnClickNotify(OnDownClicked);
  }


  cursor = 0;
  edittext[0]= 0;
  edittext[1]= 0;
  if (_tcslen(text)>0) {
    _tcsupr(text);
    _tcsncpy(edittext, text, max_width-1);
    edittext[max_width-1]= 0;
  }
  MoveCursor();

  wf->SetKeyDownNotify(FormKeyDown);

  wf->ShowModal();

  _tcsncpy(text, edittext, max_width);
  text[max_width-1]=0;

  // strip trailing spaces
  int len = _tcslen(text)-1;
  while ((len>0) && (text[len] == _T(' '))) {
    text[len] = 0;
    len--;
  }

  delete wf;
}


void dlgTextEntryShowModal(TCHAR *text, int width) 
{
  switch (Appearance.TextInputStyle)
    {
    case tiKeyboard:
      dlgTextEntryKeyboardShowModal(text, width);	
      break;
    case tiHighScore:
    default:
      dlgTextEntryHighscoreType(text, width);
      break;
    }
}
