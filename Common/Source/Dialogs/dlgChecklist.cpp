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
#include "Event/Event.h"
#include "utils/TextWrapArray.h"
#include "resource.h"

#define MAXNOTETITLE 200	// max number of characters in a title note
#define MAXNOTEDETAILS 5000	// max size of each note details
#define MAXNOTELINES 300	// max number of lines in a note, enough for maxnotedetails size!
#define MAXNOTES 300		// max number of notes

#define MAXNOTELIMITER 50	// character reserved for last line comment on note oversized
#define NOTECONTINUED		"\r\n          >>>>>\r\n"

#define LKF_CHECKLISTDEMO "NOTEDEMO.TXT"

#define ENDOFLINE  "\n"         // \r\n  in v5

int page=0;

int DrawListIndex=0;
static TextWrapArray aTextLine;

int nLists=0;

TCHAR *ChecklistText[MAXNOTES+1];
TCHAR *ChecklistTitle[MAXNOTES+1];
TCHAR NoteModeTitle[50];

static void InitNotepad(void) {
  page=0;
  DrawListIndex=0;
  aTextLine.clear();

  nLists=0;
  for (int i=0; i<MAXNOTES; i++) {
	ChecklistText[i]=(TCHAR *)NULL;
	ChecklistTitle[i]=(TCHAR *)NULL;
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


static void NextPage(WndForm* pForm, int Step){
  if(!pForm) {
    return;
  }

  TCHAR buffer[200];
  page += Step;
  if (page>=nLists) {
    page=0;
  }
  if (page<0) {
    page= nLists-1;
  }

  WndOwnerDrawFrame* wDetailsEntry = (WndOwnerDrawFrame*)pForm->FindByName(TEXT("frmDetailsEntry"));
  if(!wDetailsEntry) {
    return;
  }

  LKWindowSurface Surface(*wDetailsEntry);
  Surface.SelectObject(wDetailsEntry->GetFont());
  aTextLine.update(Surface, wDetailsEntry->GetWidth(), ChecklistText[page]);

  switch(nLists) {
	case 0:
		_stprintf(buffer, _T("%s %s"),NoteModeTitle,MsgToken(1750)); // empty
		break;
	case 1:
		_stprintf(buffer, _T("%s"),NoteModeTitle);
		break;
	default:
		_stprintf(buffer, _T("%s %d/%d"),NoteModeTitle,page+1,nLists);
		break;
  }

  if (ChecklistTitle[page] &&
      (_tcslen(ChecklistTitle[page])>0)
      && (_tcslen(ChecklistTitle[page])<60)) {
	_tcscat(buffer, TEXT(": "));
	_tcscat(buffer, ChecklistTitle[page]);
  }

  pForm->SetCaption(buffer);

  WndListFrame* wDetails = (WndListFrame*)pForm->FindByName(TEXT("frmDetails"));
  if(wDetails) {
    wDetails->ResetList();
    wDetails->Redraw();
  }
}


static void OnPaintDetailsListItem(WindowControl * Sender, LKSurface& Surface){
  (void)Sender;
  if (DrawListIndex < (int)aTextLine.size()){
      LKASSERT(DrawListIndex>=0);
      const TCHAR* szText = aTextLine[DrawListIndex];
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(2*ScreenScale, 2*ScreenScale, szText);
  }
}


static void OnDetailsListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo){
  (void)Sender;
  if (ListInfo->DrawIndex == -1){
    ListInfo->ItemCount = aTextLine.size();
  } else {
    DrawListIndex = ListInfo->DrawIndex+ListInfo->ScrollIndex;
  }
}

static void OnNextClicked(WndButton* pWnd) {
    if(pWnd) {
      NextPage(pWnd->GetParentWndForm(), +1);
    }
}

static void OnPrevClicked(WndButton* pWnd) {
    if(pWnd) {
      NextPage(pWnd->GetParentWndForm(), -1);
    }
}

static void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
    Window * pBtn = NULL;

    switch (KeyCode & 0xffff) {
        case KEY_LEFT:
        case '6':
            pBtn = pWnd->FindByName(TEXT("cmdPrev"));
            NextPage(pWnd, -1);
            break;
        case KEY_RIGHT:
        case '7':
            pBtn = pWnd->FindByName(TEXT("cmdNext"));
            NextPage(pWnd, +1);
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
      _tcscat(Details,TEXT(ENDOFLINE));
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
    _tcscat(Details,TEXT(ENDOFLINE));
  } // not a new start line
} // AddChecklistLine


#if 0
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
  StartupStore(_T(". Loading Ascii notes <%s>%s"),fileName,NEWLINE);
  #endif

  TCHAR TempString[MAXNOTEDETAILS+1];
  TCHAR Details[MAXNOTEDETAILS+1];
  TCHAR Name[MAXNOTETITLE+1];
  bool inDetails = false;

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;

  charset cs = charset::unknown;
  while (ReadStringX(stream, MAXNOTETITLE, TempString, cs)) {
/*
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
*/

    AddChecklistLine(TempString, Details, Name, inDetails);
  } // while

  if (inDetails) {
    _tcscat(Details,TEXT(ENDOFLINE));
    addChecklist(Name, Details);
  }

  fclose(stream);

  return(true);
} // LoadAsciiChecklist
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads checklist from file encoded in UTF-8.
///
/// @retval true  data loaded
/// @retval false data load error
///
static bool LoadUtfChecklist(const TCHAR* fileName, bool warn) {
  Utf8File file;
  if (!file.Open(fileName, Utf8File::io_read)) {
    if (warn) StartupStore(_T("... Not found notes <%s>%s"),fileName,NEWLINE);
    return(false);
  }

  #if TESTBENCH
  StartupStore(_T(". Loading UTF notes <%s>%s"),fileName,NEWLINE);
  #endif

  TCHAR TempString[MAXNOTETITLE];
  TCHAR Details[MAXNOTEDETAILS+1];
  TCHAR Name[MAXNOTETITLE+1];
  bool inDetails = false;

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;
  bool firstline=true;

  while (file.ReadLn(TempString, MAXNOTETITLE)) {
    // skip comment lines
    if (TempString[0] == _T('#')) {
      firstline=false;
      continue;
    }

    // Skip BOMs, if existing. Just to be sure, we only check the very first line of the file.
    // UTF8 BOM: 0xef 0xbb 0xbf
    // Unicode l.e. BOM: 0xff 0xfe (not supported)
    // Unicode b.e. BOM: 0xfe 0xff (not supported)

    if (firstline && (byte)TempString[0]==0xef && (byte)TempString[1]==0xbb && (byte)TempString[2]==0xbf) {
       #ifdef TESTBENCH
       StartupStore(_T("... LoadUtfChecklist, ignoring UTF8 BOM%s\n"),NEWLINE);
       #endif
       AddChecklistLine(&TempString[3], Details, Name, inDetails);
    } else {
       if (firstline && ((unsigned)TempString[0]==0xffef || (unsigned)TempString[0]==0xfeff)) {
          #ifdef TESTBENCH
          StartupStore(_T("... LoadUtfChecklist, ignoring UNICODE BOM%s\n"),NEWLINE);
          #endif
          AddChecklistLine(&TempString[1], Details, Name, inDetails);
       } else {
          AddChecklistLine(TempString, Details, Name, inDetails);
       }
    }
    firstline=false;
  } // while

  if (inDetails) {
    _tcscat(Details,TEXT(ENDOFLINE));
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
		_tcscat(filename,_T(DIRSEP));
		_tcscat(filename,_T(LKF_CHECKLIST));
		_stprintf(NoteModeTitle,_T("%s"),MsgToken(878));  // notepad

		if (LoadUtfChecklist(filename,false)) return true;
                // if no user file, look for demo file
		LocalPath(filename, TEXT(LKD_CONF));
		_tcscat(filename,_T(DIRSEP));
		_tcscat(filename,_T(LKF_CHECKLISTDEMO));
		return LoadUtfChecklist(filename,true);
	// logbook TXT
	case 1:
		LocalPath(filename, TEXT(LKD_LOGS));
		_tcscat(filename,_T(DIRSEP));
		_tcscat(filename,_T(LKF_LOGBOOKTXT));
		_stprintf(NoteModeTitle,_T("%s"),MsgToken(1748));  // logbook
		 return LoadUtfChecklist(filename,true);
	// logbook LST
	case 2:
		LocalPath(filename, TEXT(LKD_LOGS));
		_tcscat(filename,_T(DIRSEP));
		_tcscat(filename,_T(LKF_LOGBOOKLST));
		_stprintf(NoteModeTitle,_T("%s"),MsgToken(1748));  // logbook
		return LoadUtfChecklist(filename,true);
		break;
	case 3:
		SystemPath(filename, TEXT(LKD_SYSTEM));
		_tcscat(filename,_T(DIRSEP));
		_tcscat(filename,_T(LKF_CREDITS));
		_stprintf(NoteModeTitle,_T("%s"),gettext(_T("Credits")));
		return LoadUtfChecklist(filename,true);
		break;
  default:
    StartupStore(_T("... Invalid checklist mode (%d)%s"),checklistmode,NEWLINE);
    return false;
  }
}

// checklistmode: 0=notepad 1=logbook 2=...
void dlgChecklistShowModal(short checklistmode){

  WndListFrame* wDetails = NULL;
  WndOwnerDrawFrame* wDetailsEntry = NULL;

  InitNotepad();
  LoadChecklist(checklistmode); // check if loaded really something

  WndForm* wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_CHECKLIST_L : IDR_XML_CHECKLIST_P);

  aTextLine.clear();

  if (!wf) goto deinit;

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wDetails = (WndListFrame*)wf->FindByName(TEXT("frmDetails"));
  if (!wDetails) {
    StartupStore(_T("..... NOTEPAD ERROR NULL frmDetails!\n"));
    goto deinit;
  }
  wDetails->SetBorderKind(BORDERLEFT);

  wDetailsEntry = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmDetailsEntry"));
  if (!wDetailsEntry) {
    StartupStore(_T("..... NOTEPAD ERROR NULL frmDetailsEntry!\n"));
    goto deinit;
  }
  wDetailsEntry->SetCanFocus(true);

  page = 0;
  NextPage(wf, 0);

  wf->ShowModal();


deinit:

  delete wf;
  DeinitNotepad();
}
