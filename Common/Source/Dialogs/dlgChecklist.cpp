/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgChecklist.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "utils/fileext.h"


#define MAXNOTETITLE 200	// max number of characters in a title note
#define MAXNOTEDETAILS 5000	// max size of each note details
#define MAXNOTELINES 300	// max number of lines in a note, enough for maxnotedetails size!
#define MAXNOTES 300		// max number of notes

#define MAXNOTELIMITER 50	// character reserved for last line comment on note oversized
#define NOTECONTINUED		"\r\n          >>>>>\r\n"

int page=0;
WndForm *wf=NULL;
WndListFrame *wDetails=NULL;
WndOwnerDrawFrame *wDetailsEntry = NULL;
int LineOffsets[MAXNOTELINES+1];
int DrawListIndex=0;
int nTextLines=0;
int nLists=0;
TCHAR *ChecklistText[MAXNOTES+1];
TCHAR *ChecklistTitle[MAXNOTES+1];
TCHAR NoteModeTitle[50];

static void InitNotepad(void) {
  page=0;
  wf=(WndForm *)NULL;
  wDetails=(WndListFrame *)NULL;
  wDetailsEntry = (WndOwnerDrawFrame *)NULL;
  DrawListIndex=0;
  nTextLines=0;
  nLists=0;
  for (int i=0; i<MAXNOTES; i++) {
	ChecklistText[i]=(TCHAR *)NULL;
	ChecklistTitle[i]=(TCHAR *)NULL;
  }
  for (int i=0; i<MAXNOTELINES; i++) {
	LineOffsets[i]=0;
  }
}


static void DeinitNotepad(void) {
  for (int i=0; i<MAXNOTES; i++) {
    if (ChecklistText[i]) {
      free(ChecklistText[i]);
      ChecklistText[i]= NULL;
    }
    if (ChecklistTitle[i]) {
      free(ChecklistTitle[i]);
      ChecklistTitle[i]= NULL;
    }
  }
}


static void NextPage(int Step){
  TCHAR buffer[200];
  page += Step;
  if (page>=nLists) {
    page=0;
  }
  if (page<0) {
    page= nLists-1;
  }

  nTextLines = TextToLineOffsets(ChecklistText[page], LineOffsets, MAXNOTELINES);

  switch(nLists) {
	case 0:
		_stprintf(buffer, _T("%s %s"),NoteModeTitle,gettext(TEXT("_@M1750_"))); // empty
		break;
	case 1:
		_stprintf(buffer, _T("%s"),NoteModeTitle); 
		break;
	default:
		_stprintf(buffer, _T("%s %d/%d"),NoteModeTitle,page+1,nLists); 
		break;
  }

#if 0 // REMOVE
  if (nLists>0)
	_stprintf(buffer, _T("%s %d/%d"),NoteModeTitle,page+1,nLists); 
  else
	_stprintf(buffer, _T("%s %s"),NoteModeTitle,gettext(TEXT("_@M1750_"))); // empty
#endif

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


static void OnPaintDetailsListItem(WindowControl * Sender, LKSurface& Surface){
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
      Surface.DrawText(2*ScreenScale, 2*ScreenScale, text+nstart, nlen);
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

static void OnNextClicked(Window* pWnd) {
    (void) pWnd;
    NextPage(+1);
}

static void OnPrevClicked(Window* pWnd) {
    (void) pWnd;
    NextPage(-1);
}

static void OnCloseClicked(Window* pWnd) {
    (void) pWnd;
    wf->SetModalResult(mrOK);
}

static bool FormKeyDown(Window* pWnd, unsigned KeyCode) {
    Window * pBtn = NULL;

    switch (KeyCode & 0xffff) {
        case VK_LEFT:
        case '6':
            pBtn = wf->FindByName(TEXT("cmdPrev"));
            NextPage(-1);
            break;
        case VK_RIGHT:
        case '7':
            pBtn = wf->FindByName(TEXT("cmdNext"));
            NextPage(+1);
            break;;
    }
    if (pBtn) {
        pBtn->SetFocus();
        return true;
    }

    return false;
}

static CallBackTableEntry_t CallBackTable[]={
  ClickNotifyCallbackEntry(OnNextClicked),
  ClickNotifyCallbackEntry(OnPrevClicked),
  OnPaintCallbackEntry(OnPaintDetailsListItem),
  OnListCallbackEntry(OnDetailsListInfo),
  EndCallBackEntry()
};


void addChecklist(TCHAR* name, TCHAR* details) {
  if (nLists<MAXNOTES) {
    ChecklistTitle[nLists] = (TCHAR*)malloc((_tcslen(name)+1)*sizeof(TCHAR));
    ChecklistText[nLists] = (TCHAR*)malloc((_tcslen(details)+1)*sizeof(TCHAR));
    LKASSERT(ChecklistTitle[nLists]!=NULL);
    LKASSERT(ChecklistText[nLists]!=NULL);
    _tcscpy(ChecklistTitle[nLists], name);
    if (_tcslen(name)>=MAXNOTETITLE)
	  ChecklistTitle[nLists][MAXNOTETITLE-1]= 0;
    _tcscpy(ChecklistText[nLists], details);
    if (_tcslen(details)>=MAXNOTEDETAILS)
	  ChecklistText[nLists][MAXNOTEDETAILS-1]= 0;
    nLists++;
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Add a line pointed to by TempString into checklist.
static void AddChecklistLine(const TCHAR* TempString, TCHAR* Details, TCHAR* Name, bool& inDetails) {
  size_t len = _tcslen(TempString);
  
  // check that we are not over the limit of the note details size
  if ((_tcslen(Details) + len) > (MAXNOTEDETAILS-MAXNOTELIMITER-1)) {
    // unfortunately, yes. So we need to split the note right now.
    // And we keep the same Name also for next splitted note.
    _tcscat(Details, TEXT(NOTECONTINUED));
    
    if (_tcslen(Name)>0 && _tcslen(Details)>0) {
         addChecklist(Name, Details);
    } 
    Details[0]= 0;
    inDetails=true;
  }

  if (TempString[0]=='[') { // Look for start
    // we found the beginning of a new note, so we may save the last one, if not empty
    if (inDetails) {
      _tcscat(Details,TEXT("\r\n"));
      addChecklist(Name, Details);
      Details[0]= 0;
      Name[0]= 0;
    }

    // extract name
    size_t i;
    for (i=1; i<MAXNOTETITLE; i++) {
      if (TempString[i]==']') {
        break;
      }
      Name[i-1]= TempString[i];
    }
    Name[i-1]= 0;

    inDetails = true;

  } else {
    // append text to details string
    // we already know we have enough space
    _tcsncat(Details,TempString,MAXNOTEDETAILS-2);
    _tcscat(Details,TEXT("\r\n"));
  } // not a new start line
} // AddChecklistLine


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads checklist from file encoded in system code page.
///
/// @retval true  data loaded 
/// @retval false data load error
///
static bool LoadAsciiChecklist(const TCHAR* fileName) {
  FILE * stream = _tfopen(fileName, _T("rt"));
  if(!stream) {
    StartupStore(_T("... Not found notes <%s>%s"),fileName,NEWLINE);
    return(false);
  }

  #if TESTBENCH
  StartupStore(_T(". Loading notes <%s>%s"),fileName,NEWLINE);
  #endif

  TCHAR TempString[MAXNOTETITLE+1];
  TCHAR Details[MAXNOTEDETAILS+1];
  TCHAR Name[MAXNOTETITLE+1];
  bool inDetails = false;

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;

  while (ReadStringX(stream, MAXNOTETITLE, TempString)) {
    size_t len = _tcslen(TempString);
    if (len > 0) {
      if (TempString[len - 1] == '\r') {
        TempString[len - 1]= 0;
        len--;
      }
    }
    // On PNA we may have TWO trailing CR
    if (len > 0) {
      if (TempString[len - 1] == '\r') {
        TempString[len - 1]= 0;
      }
    }
    
    AddChecklistLine(TempString, Details, Name, inDetails);
  } // while
  
  if (inDetails) {
    _tcscat(Details,TEXT("\r\n"));
    addChecklist(Name, Details);
  }
  
  fclose(stream);
  
  return(true);
} // LoadAsciiChecklist 


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads checklist from file encoded in UTF-8.
///
/// @retval true  data loaded 
/// @retval false data load error
///
static bool LoadUtfChecklist(const TCHAR* fileName) {
  Utf8File file;
  if (!file.Open(fileName, Utf8File::io_read)) {
    StartupStore(_T("... Not found notes <%s>%s"),fileName,NEWLINE);
    return(false);
  }

  #if TESTBENCH
  StartupStore(_T(". Loading notes <%s>%s"),fileName,NEWLINE);
  #endif

  TCHAR TempString[MAXNOTETITLE];
  TCHAR Details[MAXNOTEDETAILS+1];
  TCHAR Name[MAXNOTETITLE+1];
  bool inDetails = false;

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;

  while (file.ReadLn(TempString, MAXNOTETITLE)) {
    // skip comment lines
    if (TempString[0] == _T('#'))
      continue;
    
    AddChecklistLine(TempString, Details, Name, inDetails);
  } // while
  
  if (inDetails) {
    _tcscat(Details,TEXT("\r\n"));
    addChecklist(Name, Details);
  }
 
  return(true);
} // LoadUtfChecklist 


// return true if loaded file, false if not loaded
bool LoadChecklist(short checklistmode) {
  TCHAR filename[MAX_PATH];

  switch(checklistmode) {
	// notepad
	case 0:
		LocalPath(filename, TEXT(LKD_CONF));
		_tcscat(filename,_T("\\"));
		_tcscat(filename,_T(LKF_CHECKLIST));
		_stprintf(NoteModeTitle,_T("%s"),gettext(_T("_@M878_")));  // notepad
    return LoadAsciiChecklist(filename);
	// logbook TXT
	case 1:
		LocalPath(filename, TEXT(LKD_LOGS));
		_tcscat(filename,_T("\\"));
		_tcscat(filename,_T(LKF_LOGBOOKTXT));
		_stprintf(NoteModeTitle,_T("%s"),gettext(_T("_@M1748_")));  // logbook
   		 return LoadUtfChecklist(filename);
	// logbook LST
	case 2:
		LocalPath(filename, TEXT(LKD_LOGS));
		_tcscat(filename,_T("\\"));
		_tcscat(filename,_T(LKF_LOGBOOKLST));
		_stprintf(NoteModeTitle,_T("%s"),gettext(_T("_@M1748_")));  // logbook
		return LoadUtfChecklist(filename);
		break;
  default:
    StartupStore(_T("... Invalid checklist mode (%d)%s"),checklistmode,NEWLINE);
    return false;
  }
}

// checklistmode: 0=notepad 1=logbook 2=...
void dlgChecklistShowModal(short checklistmode){

  InitNotepad();
  LoadChecklist(checklistmode); // check if loaded really something

  if (!ScreenLandscape) {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgChecklist_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        TEXT("IDR_XML_CHECKLIST_L"));
  } else {
    TCHAR filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgChecklist.xml"));
    wf = dlgLoadFromXML(CallBackTable,                        
                        filename, 
                        TEXT("IDR_XML_CHECKLIST"));
  }

  nTextLines = 0;

  if (!wf) goto deinit;

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wDetails = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));
  //ASSERT(wDetails!=NULL);
  if (wDetails==NULL) {
	StartupStore(_T("..... NOTEPAD ERROR NULL frmDetails!\n"));
	goto deinit;
  }
  wDetails->SetBorderKind(BORDERLEFT);
  wDetails->SetWidth(wf->GetWidth() - wDetails->GetLeft()-2);

  wDetailsEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  //ASSERT(wDetailsEntry!=NULL);
  if (wDetailsEntry==NULL) {
	StartupStore(_T("..... NOTEPAD ERROR NULL frmDetailsEntry!\n"));
	goto deinit;
  }
  wDetailsEntry->SetCanFocus(true);
   // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
   if ( wDetails->ScrollbarWidth == -1) {
   #if defined (PNA)
   #define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #else
   #define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
   #endif
   wDetails->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);

   }
  wDetailsEntry->SetWidth(wDetails->GetWidth() - wDetails->ScrollbarWidth - 5);

  page = 0;
  NextPage(0);

  wf->ShowModal();

  delete wf;
  wf = NULL;

deinit:

  DeinitNotepad();
  return;

}

