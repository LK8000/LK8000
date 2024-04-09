/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgChecklist.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/


#include "externs.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "Event/Event.h"
#include "utils/TextWrapArray.h"
#include "resource.h"
#include "utils/zzip_stream.h"
#include <vector>


#define MAXNOTETITLE 200	// max number of characters in a title note
#define MAXNOTEDETAILS 5000	// max size of each note details

#define MAXNOTELIMITER 50	// character reserved for last line comment on note oversized
#define NOTECONTINUED		"\r\n          >>>>>\r\n"

#define LKF_CHECKLISTDEMO "NOTEDEMO.TXT"

#define ENDOFLINE  "\n"         // \r\n  in v5

namespace {

int DrawListIndex=0;
TextWrapArray aTextLine;

struct checklist_item {
  tstring title;
  tstring text;
};

std::vector<checklist_item> checklist_data;

int page = 0;

TCHAR NoteModeTitle[50];

} // namespace

static void InitNotepad(void) {
  page=0;
  DrawListIndex=0;
  aTextLine.clear();
  checklist_data = {};
}


static void DeinitNotepad(void) {
  aTextLine.clear();
  checklist_data = {}; // trick to force deallocate.
}


static void NextPage(WndForm* pForm, int Step){
  if(!pForm) {
    return;
  }

  TCHAR buffer[200];
  page += Step;
  if (page < 0) {
    page = checklist_data.size() - 1;
  }
  else if (static_cast<size_t>(page) >= checklist_data.size()) {
    page = 0;
  }

  WndOwnerDrawFrame* wDetailsEntry = pForm->FindByName<WndOwnerDrawFrame>(TEXT("frmDetailsEntry"));
  if(!wDetailsEntry) {
    return;
  }

  LKWindowSurface Surface(*wDetailsEntry);
  Surface.SelectObject(wDetailsEntry->GetFont());
  aTextLine.update(Surface, wDetailsEntry->GetWidth(), checklist_data[page].text.c_str());

  switch(checklist_data.size()) {
	case 0:
		_stprintf(buffer, _T("%s %s"),NoteModeTitle,MsgToken<1750>()); // empty
		break;
	case 1:
		_stprintf(buffer, _T("%s"),NoteModeTitle);
		break;
	default:
    _stprintf(buffer, _T("%s %d/%d"), NoteModeTitle, page + 1, static_cast<int>(checklist_data.size()));
    break;
  }

  const auto& title = checklist_data[page].title;
  if (!title.empty() && title.size() < 60) {
    _tcscat(buffer, TEXT(": "));
    _tcscat(buffer, title.c_str());
  }

  pForm->SetCaption(buffer);

  WndListFrame* wDetails = pForm->FindByName<WndListFrame>(TEXT("frmDetails"));
  if(wDetails) {
    wDetails->ResetList();
    wDetails->Redraw();
  }
}


static void OnPaintDetailsListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface){
  (void)Sender;
  if (DrawListIndex < (int)aTextLine.size()){
      LKASSERT(DrawListIndex>=0);
      const TCHAR* szText = aTextLine[DrawListIndex];
      Surface.SetTextColor(RGB_BLACK);
      Surface.DrawText(DLGSCALE(2), DLGSCALE(2), szText);
  }
}


static void OnDetailsListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t *ListInfo){

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
  checklist_data.push_back({name, details});
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Reads checklist from file encoded in UTF-8 or Latin-1.
///
/// @retval true  data loaded
/// @retval false data load error
///
static bool LoadChecklist(const TCHAR* fileName, bool warn) {

  zzip_stream stream(fileName, "rb");
  if(!stream) {
    if (warn) StartupStore(_T("... Not found notes <%s>%s"),fileName,NEWLINE);
    return(false);
  }

  #if TESTBENCH
  StartupStore(_T(". Loading UTF notes <%s>%s"),fileName,NEWLINE);
  #endif

  TCHAR TempString[MAXNOTETITLE+1];
  TCHAR Details[MAXNOTEDETAILS+1];
  TCHAR Name[MAXNOTETITLE+1];
  bool inDetails = false;

  Details[0]= 0;
  Name[0]= 0;
  TempString[0]=0;

  while(stream.read_line(TempString)) {
    // skip comment lines
    if (TempString[0] == _T('#')) {
      continue;
    }
    AddChecklistLine(TempString, Details, Name, inDetails);
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
		LocalPath(filename, TEXT(LKD_CONF), _T(LKF_CHECKLIST));
		_stprintf(NoteModeTitle,_T("%s"),MsgToken<878>());  // notepad

		if (LoadChecklist(filename,false)) return true;
                // if no user file, look for demo file
		LocalPath(filename, TEXT(LKD_CONF), _T(LKF_CHECKLISTDEMO));
		return LoadChecklist(filename,true);
	// logbook TXT
	case 1:
		LocalPath(filename, TEXT(LKD_LOGS), _T(LKF_LOGBOOKTXT));
		_stprintf(NoteModeTitle,_T("%s"),MsgToken<1748>());  // logbook
		 return LoadChecklist(filename,true);
	// logbook LST
	case 2:
		LocalPath(filename, TEXT(LKD_LOGS), _T(LKF_LOGBOOKLST));
		_stprintf(NoteModeTitle,_T("%s"),MsgToken<1748>());  // logbook
		return LoadChecklist(filename,true);
		break;
	case 3:
		SystemPath(filename, TEXT(LKD_SYSTEM));
		_tcscat(filename,_T(DIRSEP));
		_tcscat(filename,_T(LKF_CREDITS));
		_stprintf(NoteModeTitle,_T("%s"),LKGetText(_T("Info")));
		return LoadChecklist(filename,true);
		break;
  default:
    StartupStore(_T("... Invalid checklist mode (%d)%s"),checklistmode,NEWLINE);
    return false;
  }
}

// checklistmode: 0=notepad 1=logbook 2=...
void dlgChecklistShowModal(short checklistmode){

  std::unique_ptr<WndForm> wf(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_CHECKLIST_L : IDR_XML_CHECKLIST_P));
  if(!wf) {
    StartupStore(_T("..... NOTEPAD ERROR Failed To Load Dialog!\n"));
    return;
  }
  wf->SetKeyDownNotify(FormKeyDown);

  WndButton* wndClose =  wf->FindByName<WndButton>(TEXT("cmdClose"));
  if(!wndClose) {
    StartupStore(_T("..... NOTEPAD ERROR NULL cmdClose!\n"));
    return;
  }
  wndClose->SetOnClickNotify(OnCloseClicked);


  WndListFrame* wDetails = wf->FindByName<WndListFrame>(TEXT("frmDetails"));
  if (!wDetails) {
    StartupStore(_T("..... NOTEPAD ERROR NULL frmDetails!\n"));
    return;
  }
  wDetails->SetBorderKind(BORDERLEFT);


  WndOwnerDrawFrame* wDetailsEntry = wf->FindByName<WndOwnerDrawFrame>(TEXT("frmDetailsEntry"));
  if (!wDetailsEntry) {
    StartupStore(_T("..... NOTEPAD ERROR NULL frmDetailsEntry!\n"));
    return;
  }
  wDetailsEntry->SetCanFocus(true);

  // calculate text line height
  LKWindowSurface Surface(*wDetailsEntry);
  const auto oldFont = Surface.SelectObject(wDetailsEntry->GetFont());
  const int minHeight = Surface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
  Surface.SelectObject(oldFont);
  const int wHeight = wDetailsEntry->GetHeight();
  if(minHeight != wHeight) {
    wDetailsEntry->SetHeight(minHeight);
  }

  InitNotepad();
  LoadChecklist(checklistmode); // check if loaded really something
  aTextLine.clear();

  page = 0;
  NextPage(wf.get(), 0);

  wf->ShowModal();

  DeinitNotepad();
}
