/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: dlgTextEntry_Keyboard.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
 */

#include "externs.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include <ctype.h>
#include "Event/Event.h"
#include "Sound/Sound.h"
#include "resource.h"
#include "utils/printf.h"
#include <regex>
#include "Form/Clipboard.h"
#include <memory>

static bool first= false;
static WndProperty * wKeyboardPopupWndProperty = nullptr;

#define MAX_TEXTENTRY 40
static unsigned int cursor = 0;
static unsigned int max_width = MAX_TEXTENTRY;
static TCHAR edittext[MAX_TEXTENTRY];

#define MAX_SEL_LIST_SIZE       120
#define NO_LAYOUTS 2
#define UPPERCASE 0
#define LOWERCASE 1
uint8_t  KeyboardLayout = UPPERCASE;

key_filter_interface* key_filter = nullptr;

void HideKey(WindowControl* pWnd) {
  const TCHAR* Name = pWnd->GetWndName();
  if (_tcsstr(Name, _T("prp_lower_")) == Name) {
    pWnd->SetVisible(KeyboardLayout == LOWERCASE);
  }
  else if (_tcsstr(Name, _T("prp_upper_")) == Name) {
    pWnd->SetVisible(KeyboardLayout == UPPERCASE);
  }

  if (key_filter) {
    const TCHAR* Text = pWnd->GetWndText();
    if (_tcslen(Text) == 1) {
      if(key_filter->isHiddenKey(Text[0])) {
        pWnd->SetVisible(false);
      } 
    }
  }
}

void UpdateKeyLayout(WndForm* pForm) {
  if (!pForm) {
    return;
  }

  if (key_filter) {
    key_filter->Update(edittext);
  }

  auto wpPast = pForm->FindByName(_T("prp_past"));
  if (wpPast) {
    wpPast->SetVisible(ClipboardAvailable());
  }

  auto wpText = dynamic_cast<WndProperty*>(pForm->FindByName(_T("prpText")));
  if (wpText) {
    if(key_filter) {
      wpText->SetText(key_filter->GetMatchText());
      wpText->SetCaption(key_filter->GetLabel());
    } else {
      wpText->SetText(edittext);
      wpText->SetCaption(MsgToken<711>()); // _@M711_ "Text:"
    }
  }

  auto wpUnit = dynamic_cast<WndProperty*>(pForm->FindByName(TEXT("prpUnit")));
  if(wpUnit && wKeyboardPopupWndProperty) {
    DataField* pField = wKeyboardPopupWndProperty->GetDataField();
    if(pField) {
      wpUnit->SetCaption(pField->GetUnits());
      wpUnit->RefreshDisplay();
    }
  }

  TCHAR Entered[MAX_SEL_LIST_SIZE]=_T("");
  _sntprintf(Entered, MAX_SEL_LIST_SIZE, _T("%s: %s"), MsgToken<251>(), edittext); /* _@251_ Edit Text */
  pForm->SetCaption(Entered);

  auto wpMatch = pForm->FindByName(TEXT("prpMatch"));
  if (wpMatch) {
    wpMatch->SetVisible(key_filter);

    if(key_filter) {
      TCHAR Found[EXT_SEARCH_SIZE];
      _sntprintf(Found,EXT_SEARCH_SIZE,_T("%s:%u"),MsgToken<948>(), key_filter->GetMatchCount()); /* _@M948_ Found */
      wpMatch->SetCaption(Found);
    }
  }

  auto wbDate = pForm->FindByName(TEXT("prpDate"));
  if (wbDate) {
    wbDate->SetVisible(!key_filter); // todo : replace by KeyFilter ?
  }

  auto wbTime = pForm->FindByName(TEXT("prpTime"));
  if (wbTime) {
    wbTime->SetVisible(!key_filter); // todo : replace by KeyFilter ?
  }

  pForm->ForEachChild(HideKey);
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode) {
  switch(KeyCode & 0xffff){
    case KEY_LEFT:
      if (cursor<1)
        return true; // min width
      cursor--;
      edittext[cursor] = 0;
      UpdateKeyLayout(pWnd);
      return true;
  }
  return false;
}

static void ClearText(WndForm* pForm) {
  cursor = 0;
  memset(edittext, 0, sizeof(TCHAR)*MAX_TEXTENTRY);

  UpdateKeyLayout(pForm);
}

static void OnKey(WndButton* pWnd) {
  if(!pWnd) return;

  WndForm* pForm = pWnd->GetParentWndForm();

  if (first) {
    ClearText(pForm);
    first = false;
  }
  PlayResource(TEXT("IDR_WAV_CLICK"));
  const TCHAR *Caption = pWnd->GetWndText();
  if (cursor < max_width - 1) {
    edittext[cursor++] = Caption[0];
  }
  UpdateKeyLayout(pForm);
}

static void OnShift(WndButton* pWnd) {
  if(!pWnd) return;

  KeyboardLayout++;
  KeyboardLayout %= NO_LAYOUTS;
  UpdateKeyLayout(pWnd->GetParentWndForm());
}

static void OnSpace(WndButton* pWnd) {
  if(!pWnd) return;

  WndForm* pForm = pWnd->GetParentWndForm();

  if (first) {
    ClearText(pForm);
    first = false;
  }
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if (cursor < max_width - 1) {
    edittext[cursor++] = _T(' ');
  }
  UpdateKeyLayout(pForm);
}

static void OnDel(WndButton* pWnd) {
  first = false;
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if (cursor >0) {
    edittext[--cursor] = '\0';
  }

  UpdateKeyLayout(pWnd->GetParentWndForm());
}

static void OnTime(WndButton* pWnd) {
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if ( (cursor+6)<(max_width-1) ) {
    TCHAR ltime[10];
    _sntprintf(ltime,10,_T("%02d%02d%02d"),GPS_INFO.Hour,GPS_INFO.Minute,GPS_INFO.Second);
    _tcscat(&edittext[cursor],ltime);
    edittext[cursor+6] = '\0';
    cursor+=6;
  }
  UpdateKeyLayout(pWnd->GetParentWndForm());
}

static void OnDate(WndButton* pWnd) {
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
    _sntprintf(ltime,10,_T("%02d%02d%02d"),nyear,GPS_INFO.Month,GPS_INFO.Day);
    _tcscat(&edittext[cursor],ltime);
    edittext[cursor+6] = '\0';
    cursor+=6;
  }
  UpdateKeyLayout(pWnd->GetParentWndForm());
}

static void OnOk(WndButton* pWnd) {
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnClear(WndButton* pWnd) {
  PlayResource(TEXT("IDR_WAV_CLICK"));
  ClearText(pWnd->GetParentWndForm());
}

static void OnHelpClicked(WindowControl* Sender) {
  if(wKeyboardPopupWndProperty) {
    wKeyboardPopupWndProperty->OnHelp();
  }
}

static void OnPaste(WindowControl* Sender) {
  PlayResource(TEXT("IDR_WAV_CLICK"));
  if(!Sender) return;

  tstring clipboard_text = GetClipboardData();
  if (clipboard_text.empty()) {
    return;
  }

  WndForm* pForm = Sender->GetParentWndForm();
  if (first) {
    ClearText(pForm);
    first = false;
  }

  size_t available = std::size(edittext) - cursor;
  if (available > 0) {
    clipboard_text = std::regex_replace(clipboard_text, std::basic_regex<TCHAR>(_T("[\r\n]")), _T(" "));

    lk::snprintf(&edittext[cursor], available, _T("%s"), clipboard_text.c_str());
    cursor = _tcslen(edittext);
  }
  UpdateKeyLayout(pForm);
}

static CallBackTableEntry_t CallBackTable[] = {
  ClickNotifyCallbackEntry(OnKey),
  ClickNotifyCallbackEntry(OnClear),
  ClickNotifyCallbackEntry(OnOk),
  ClickNotifyCallbackEntry(OnDel),
  ClickNotifyCallbackEntry(OnSpace),
  ClickNotifyCallbackEntry(OnShift),
  ClickNotifyCallbackEntry(OnDate),
  ClickNotifyCallbackEntry(OnTime),
  ClickNotifyCallbackEntry(OnHelpClicked),
  ClickNotifyCallbackEntry(OnPaste),
  EndCallBackEntry()
};

static void dlgTextEntryKeyboardShowModal(TCHAR *text, int width, unsigned ResID) {

  max_width = min(MAX_TEXTENTRY, width);
  std::unique_ptr<WndForm> wf(dlgLoadFromXML(CallBackTable, ResID));
  if (!wf) {
    return;
  }

  ClearText(wf.get());

  if (_tcslen(text)>0) {
    LK_tcsncpy(edittext, text, max_width-1);
    // show previous test.
    // this text is replaced by first key down
    // but used if "OK" is clicked first for don't reset current value.
  }
  cursor = _tcslen(edittext);

  UpdateKeyLayout(wf.get());

  WindowControl* pBtHelp = wf->FindByName(TEXT("cmdHelp"));
  if(pBtHelp) {
     pBtHelp->SetVisible(wKeyboardPopupWndProperty && wKeyboardPopupWndProperty->HasHelpText());
  }

  wf->SetKeyDownNotify(FormKeyDown);
  wf->ShowModal();
  LK_tcsncpy(text, edittext, width-1);

  wf = nullptr;
}

void dlgTextEntryShowModal(TCHAR *text, int width, key_filter_interface* filter) {
  first = false;
  key_filter = filter;
  dlgTextEntryKeyboardShowModal(text, width, ScreenLandscape ? IDR_XML_TEXTENTRY_KEYBOARD_L : IDR_XML_TEXTENTRY_KEYBOARD_P);
  key_filter = nullptr;
}

void dlgNumEntryShowModal(TCHAR *text, int width) {
  first = true;
  dlgTextEntryKeyboardShowModal(text, width, ScreenLandscape ? IDR_XML_NUMENTRY_KEYBOARD_L : IDR_XML_NUMENTRY_KEYBOARD_P);
}

BOOL dlgKeyboard(WndProperty* theProperty) {
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
