/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgChecklist.cpp,v 8.2 2010/12/13 12:30:19 root Exp root $
*/


#include "StdAfx.h"
#include <aygshell.h>

#include "Defines.h"
#include "lk8000.h"
#include "externs.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"

#include "utils/heapcheck.h"

#define MAXTITLE 200
#define MAXDETAILS 5000

static int page=0;
static WndForm *wf=NULL;
static WndListFrame *wDetails=NULL;
static WndOwnerDrawFrame *wDetailsEntry = NULL;

#define MAXLINES 100
#define MAXLISTS 20
static int LineOffsets[MAXLINES];
static int DrawListIndex=0;
static int nTextLines=0;
static int nLists=0;
static TCHAR *ChecklistText[MAXLISTS];
static TCHAR *ChecklistTitle[MAXLISTS];

static void NextPage(int Step){
  TCHAR buffer[80];
  page += Step;
  if (page>=nLists) {
    page=0;
  }
  if (page<0) {
    page= nLists-1;
  }

  nTextLines = TextToLineOffsets(ChecklistText[page],
				 LineOffsets,
				 MAXLINES);

  _stprintf(buffer, gettext(TEXT("_@M878_"))); // Notepad

  if (ChecklistTitle[page] &&
      (_tcslen(ChecklistTitle[page])>0) 
      && (_tcslen(ChecklistTitle[page])<60)) {
    _tcscat(buffer, TEXT(": ")); 
    _tcscat(buffer, ChecklistTitle[page]); 
  }
  wf->SetCaption(buffer);

  wDetails->ResetList();
  wDetails->Redraw();

}


static void OnPaintDetailsListItem(WindowControl * Sender, HDC hDC){
  (void)Sender;
  if (DrawListIndex < nTextLines){
    TCHAR* text = ChecklistText[page];
    if (!text) return;
    int nstart = LineOffsets[DrawListIndex];
    int nlen;
    if (DrawListIndex<nTextLines-1) {
      nlen = LineOffsets[DrawListIndex+1]-LineOffsets[DrawListIndex]-1;
      nlen--;
    } else {
      nlen = _tcslen(text+nstart);
    }
    while (_tcscmp(text+nstart+nlen-1,TEXT("\r"))==0) {
      nlen--;
    }
    while (_tcscmp(text+nstart+nlen-1,TEXT("\n"))==0) {
      nlen--;
    }
    if (nlen>0) {
      ExtTextOut(hDC, 2*InfoBoxLayout::scale, 2*InfoBoxLayout::scale,
		 ETO_OPAQUE, NULL,
		 text+nstart,
		 nlen, 
		 NULL);
    }
  }
}


static void OnDetailsListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = nTextLines-1;
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}



static void OnNextClicked(WindowControl * Sender){
  (void)Sender;
  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
  (void)Sender;
  NextPage(-1);
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}

static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
	(void)lParam;
	(void)Sender;
  switch(wParam & 0xffff){
    case VK_LEFT:
    case '6':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case VK_RIGHT:
    case '7':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wf->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnPaintDetailsListItem),
  DeclareCallBackEntry(OnDetailsListInfo),
  DeclareCallBackEntry(NULL)
};

void addChecklist(TCHAR* name, TCHAR* details) {
  if (nLists<MAXLISTS) {
    ChecklistTitle[nLists] = (TCHAR*)malloc((_tcslen(name)+1)*sizeof(TCHAR));
    ChecklistText[nLists] = (TCHAR*)malloc((_tcslen(details)+1)*sizeof(TCHAR));
    _tcscpy(ChecklistTitle[nLists], name);
    if (_tcslen(name)>=MAXTITLE) // 100315 BUGFIX XCSOAR
	  ChecklistTitle[nLists][MAXTITLE-1]= 0;
    _tcscpy(ChecklistText[nLists], details);
    if (_tcslen(details)>=MAXDETAILS) // 100315 BUGFIX XCSOAR
	  ChecklistText[nLists][MAXDETAILS-1]= 0;
    nLists++;
  }
}

void LoadChecklist(void) {
  HANDLE hChecklist;
  nLists = 0;
  if (ChecklistText[0]) {
    free(ChecklistText[0]);
    ChecklistText[0]= NULL;
  }
  if (ChecklistTitle[0]) {
    free(ChecklistTitle[0]);
    ChecklistTitle[0]= NULL;
  }

  TCHAR filename[MAX_PATH];
  LocalPath(filename, TEXT(LKD_CONF));
  _tcscat(filename,_T("\\"));
  _tcscat(filename,_T(LKF_CHECKLIST));

  hChecklist = INVALID_HANDLE_VALUE;
  hChecklist = CreateFile(filename,
			  GENERIC_READ,0,NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,NULL);
  if( hChecklist == INVALID_HANDLE_VALUE)
    {
	StartupStore(_T(". No Notepad file <%s>%s"),filename,NEWLINE);
	return;
    }

  StartupStore(_T(". Found Notepad file <%s>%s"),filename,NEWLINE);

  TCHAR TempString[MAXTITLE];
  TCHAR Details[MAXDETAILS];
  TCHAR Name[100];
  BOOL inDetails = FALSE;
  int i;
//  int k=0;

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;

  while(ReadString(hChecklist,MAXTITLE,TempString))
    {
      int len = _tcslen(TempString);
      if (len>0) {
	// JMW strip extra \r if it exists
	if (TempString[len-1]=='\r') {
	  TempString[len-1]= 0;
	}
      }

      if(TempString[0]=='[') { // Look for start

	if (inDetails) {
	  wcscat(Details,TEXT("\r\n"));
	  addChecklist(Name, Details);
	  Details[0]= 0;
	  Name[0]= 0;
	}

	// extract name
	for (i=1; i<MAXTITLE; i++) {
	  if (TempString[i]==']') {
	    break;
	  }
	  Name[i-1]= TempString[i];
	}
	Name[i-1]= 0;

	inDetails = TRUE;

      } else {
	// append text to details string
	wcsncat(Details,TempString,MAXDETAILS-2);
	wcscat(Details,TEXT("\r\n"));
	// TODO code: check the string is not too long
      }
    }

  if (inDetails) {
    wcscat(Details,TEXT("\r\n"));
    addChecklist(Name, Details);
  }

  CloseHandle(hChecklist);
  hChecklist = NULL;

}


void dlgChecklistShowModal(void){
  static bool first=true;
  if (first) {
    LoadChecklist();
    first=false;
  }

  //  WndProperty *wp;

  if (!InfoBoxLayout::landscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgChecklist_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_CHECKLIST_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgChecklist.xml"));
    wf = dlgLoadFromXML(CallBackTable,                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_CHECKLIST"));
  }

  nTextLines = 0;

  if (!wf) return;

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wDetails = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));
  ASSERT(wDetails!=NULL);

  wDetailsEntry = 
    (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  ASSERT(wDetailsEntry!=NULL);
  wDetailsEntry->SetCanFocus(true);

  wDetails->SetBorderKind(BORDERLEFT);

  page = 0;

  NextPage(0); // JMW just to turn proper pages on/off

  wf->ShowModal();

  delete wf;

  wf = NULL;

}

