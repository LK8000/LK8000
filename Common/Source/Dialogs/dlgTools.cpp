/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTools.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <limits.h>
#include "dlgTools.h"
#include "WindowControls.h"
#include "RGB.h"
#include "Dialogs.h"
#include "utils/stringext.h"
#include "utils/stl_utils.h"
#include "LKInterface.h"
#include "Event/Event.h"
#include "Asset.hpp"
#include "Library/rapidxml/rapidxml.hpp"
#include "Library/rapidxml/rapidxml_iterators.hpp"

#include <stdio.h>

#ifndef WIN32_RESOURCE
#include "resource_data.h"
#else
extern HINSTANCE _hInstance;
#endif

#ifdef KOBO
  #include <linux/input.h>
#endif

static void OnButtonClick(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(pWnd->GetTag());
    }
  }
}

static bool FormKeyDown(WndForm* pWnd, unsigned KeyCode){

    WindowControl* pBt = nullptr;
    switch(KeyCode) {
#ifdef KOBO
        case KEY_POWER:
#endif
        case KEY_ESCAPE:
            pBt = pWnd->FindByName(_T("CANCEL"));
            if(pBt) {
                // if no "CANCEL" button find "OK"
                break;
            }
       case KEY_RETURN:
            pBt = pWnd->FindByName(_T("OK"));
            break;
    }
    if(pBt) {
        pWnd->SetModalResult(pBt->GetTag());
        return true;
    }
    return false;
}

MsgReturn_t MessageBoxX(LPCTSTR lpText, LPCTSTR lpCaption, MsgType_t uType, bool wfullscreen){

  WndForm *wf=NULL;
  WndFrame *wText=NULL;
  int X, Y, Width, Height;
  WndButton *wButtons[10];
  int ButtonCount = 0;
  int i,x,y,d,w,h,dY;
  MsgReturn_t res;
  RECT rc = main_window->GetClientRect();

  if (wfullscreen) {
	Width = rc.right;
	Height = rc.bottom;
  } else
  if (ScreenLandscape) {
	Width = DLGSCALE(280);
	Height = DLGSCALE(160);
  } else {
	Width = DLGSCALE(230);
	Height = DLGSCALE(160);
  }

  X = ((rc.right-rc.left) - Width)/2;
  Y = ((rc.bottom-rc.top) - Height)/2;

  if (wfullscreen) {
	dY=0;
	y = DLGSCALE(ScreenLandscape?160:200);
  } else {
	dY = DLGSCALE(-40);
	y = DLGSCALE(100);
  }
  w = DLGSCALE(60);
  h = DLGSCALE(32);

  Height += dY;

  wf = new WndForm(TEXT("frmXcSoarMessageDlg"),
                   lpCaption, X, Y, Width, Height);
  wf->SetFont(MapWindowBoldFont);
  wf->SetTitleFont(MapWindowBoldFont);
  wf->SetBackColor(RGB_WINBACKGROUND);
  wf->SetBorderKind(BORDERTOP|BORDERRIGHT|BORDERBOTTOM|BORDERLEFT);

  const PixelRect clientRect(wf->GetClientRect());
  wText = new WndFrame(wf,
                       TEXT("frmMessageDlgText"),
                       0,
                       DLGSCALE(5),
                       clientRect.GetSize().cx,
                       clientRect.GetSize().cy - y - dY - DLGSCALE(5));

  wText->SetCaption(lpText);
  wText->SetFont(MapWindowBoldFont);
  wText->SetCaptionStyle(
        DT_EXPANDTABS
      | DT_CENTER
      | DT_NOCLIP
      | DT_WORDBREAK
  );

  y += dY;

  if (uType == mbOk
      || uType == mbOkCancel)
  {
    wButtons[ButtonCount] = new WndButton(wf, TEXT("OK"), TEXT("OK"), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdOk);
    ButtonCount++;
  }

  if (uType == mbYesNo
      || uType == mbYesNoCancel)
  {
	// LKTOKEN  _@M827_ = "Yes"
    wButtons[ButtonCount] = new WndButton(wf, TEXT("OK"), MsgToken(827), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdYes);
    ButtonCount++;
    wButtons[ButtonCount] = new WndButton(wf, TEXT("CANCEL"), MsgToken(890), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdNo);
    ButtonCount++;
  }

  if (uType == mbAbortRetryIgnore
      || uType == mbRetryCancel)
  {
    wButtons[ButtonCount] = new WndButton(wf, TEXT("OK"), MsgToken(566), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdRetry);
    ButtonCount++;
  }

  if (uType == mbOkCancel
      || uType == mbRetryCancel
      || uType == mbYesNoCancel)
  {
    wButtons[ButtonCount] = new WndButton(wf, TEXT("CANCEL"), MsgToken(161), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdCancel);
    ButtonCount++;
  }

  if (uType == mbAbortRetryIgnore)
  {
    wButtons[ButtonCount] = new WndButton(wf, TEXT("CANCEL"), MsgToken(47), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdAbort);
    ButtonCount++;
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), MsgToken(349), 0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IdIgnore);
    ButtonCount++;
  }

  LKASSERT(ButtonCount!=0);
  d = Width / (ButtonCount);
  x = d/2-w/2;

  for (i=0; i<ButtonCount; i++){
    wButtons[i]->SetLeft(x);
    x += d;
  }

  wf->SetKeyDownNotify(FormKeyDown);

  res = static_cast<MsgReturn_t>(wf->ShowModal());

  delete wf;

  return(res);

}


namespace {

using xml_document = rapidxml::xml_document<char>;
using xml_node = rapidxml::xml_node<char>;
using xml_attribute = rapidxml::xml_attribute<char>;
using node_iterator = rapidxml::node_iterator<char>;


long AttributeToLong(const xml_node& node, const char* name, long default_value){
  const xml_attribute* attribute = node.first_attribute(name);
  if (attribute) {
    const char* string = attribute->value();
    if (string && *string) {
      return strtol(string, nullptr, 10);
    }
  }
  return default_value;
}

unsigned long AttributeToULong(const xml_node& node, const char* name, unsigned long default_value){
  const xml_attribute* attribute = node.first_attribute(name);
  if (attribute) {
    const char* string = attribute->value();
    if (string && *string) {
      return strtoul(string, nullptr, 10);
    }
  }
  return default_value;
}

double AttributeToDouble(const xml_node& node, const char* name, double default_value){
  const xml_attribute* attribute = node.first_attribute(name);
  if (attribute) {
    const char* string = attribute->value();
    if (string && *string) {
      return strtod(string, nullptr);
    }
  }
  return default_value;
}

const char *AttributeToString(const xml_node& node, const char* name, const char *default_value){
  const xml_attribute* attribute = node.first_attribute(name);
  if (attribute) {
    const char* string = attribute->value();
    if (*string) {
      return string;
    }
  }
  return default_value;
}


void GetDefaultWindowControlProps(const xml_node& Node, TCHAR (&Name)[64], int& X, int& Y, int& Width, int& Height, int& Popup, TCHAR (&Caption)[128]){

  Popup = AttributeToLong(Node, "Popup", 0); // don't make wndForm block entire screen

  X = AttributeToLong(Node, "X", 0);
  if (X>=0) { // negative value are "magic number" don't scale 
    X = DLGSCALE(X);
  }

  Y = AttributeToLong(Node, "Y", 0);
  if (Y>=0) { // negative value are "magic number" don't scale
    Y = DLGSCALE(Y);
  }

  Width = DLGSCALE(AttributeToLong(Node, "Width", 50));
  Height = DLGSCALE(AttributeToLong(Node, "Height", 50));

  from_utf8(AttributeToString(Node, "Name", ""), Name);

  from_utf8(AttributeToString(Node, "Caption", ""), Caption);
  const TCHAR* translated = LKGetText(Caption);
  if(translated != Caption) {
    // avoid to copy to itself if token is not found in language table.
    _tcscpy(Caption, LKGetText(Caption));
  }
}


template<typename function_t>
function_t CallBackLookup(const CallBackTableEntry_t *LookUpTable, const char* Name){
  if (LookUpTable && Name && Name[0]) {
    for (size_t i = 0; LookUpTable[i].Name; i++) {
      if (strcmp(LookUpTable[i].Name, Name) == 0) {
        try {
          return std::get<function_t>(LookUpTable[i].callback);
        } catch (std::bad_variant_access& e) {
          StartupStore(_T("CallBackLookup(\"%s\"): %s"), Name, to_tstring(e.what()).c_str());
          assert(false);
        }
      }
    }
  }
  return nullptr;
}


std::string xmlLoadFromResource(const TCHAR* lpName) {

#ifdef WIN32_RESOURCE

  // Find the xml resource.
  HRSRC hResInfo = FindResource(_hInstance, lpName, TEXT("XMLDialog"));

  if (hResInfo == NULL) {
    MessageBoxX(
      TEXT("Can't find resource"),
      TEXT("Dialog error"),
      mbOk);

    // unable to find the resource
    return "";
  }

  // Load the xml resource.
  HGLOBAL hRes = LoadResource(_hInstance, hResInfo);

  if (hRes == nullptr) {
    MessageBoxX(
      TEXT("Can't load resource"),
      TEXT("Dialog error"),
      mbOk);

    // unable to load the resource
    return "";
  }

  // Retrieves a pointer to the xml resource in memory
  auto szXML = static_cast<const char*>(LockResource(hRes));

  // win32 ressource are not null terminated...
  const size_t len = SizeofResource(_hInstance, hResInfo);
  return std::string(szXML, len);
#else
  return GetNamedResourceString(lpName); // always null terminated
#endif
}

// The Font=n in dialogs.  0-4, 4 unused kept for compat issue with very old code
FontReference GetFontRef(unsigned id) {
  switch (id) {
    case 0: 
      return TitleWindowFont;
    case 1:
      return MapWindowFont;
    case 2:
      return MapWindowBoldFont;
    case 3:
      return CDIWindowFont;
    case 4:
      return CDIWindowFont;
  }
  return FontReference();
}

void LoadChildsFromXML(WindowControl *Parent,
                       const CallBackTableEntry_t *LookUpTable,
                       const xml_node& node,
                       int ParentFont) {

  for (node_iterator child(&node); child != node_iterator(); ++child) {

    WindowControl *WC = nullptr;

    int X, Y, Width, Height, Popup;
    TCHAR Caption[128];
    TCHAR Name[64];

    GetDefaultWindowControlProps(*child, Name, X, Y,
                                 Width, Height, Popup, Caption);

    unsigned uBackColor = AttributeToULong(*child, "BackColor", 0xffffffff);
    unsigned uForeColor = AttributeToULong(*child, "ForeColor", 0xffffffff);

    bool Visible = AttributeToLong(*child, "Visible", 1) == 1;
    int Font = AttributeToLong(*child, "Font", ParentFont);
    int Border = AttributeToLong(*child, "Border", 0);

    if (strcmp(child->name(), "WndProperty") == 0) {

      WndProperty *W;

      int CaptionWidth = DLGSCALE(AttributeToLong(*child, "CaptionWidth", 0));
      int MultiLine = AttributeToLong(*child, "MultiLine", 0);
      int ReadOnly = AttributeToLong(*child, "ReadOnly", 0);

      const char* OnHelpCallback = AttributeToString(*child, "OnHelp", "");

      WC = W =
        new WndProperty(Parent, Name, Caption, X, Y,
                        Width, Height, CaptionWidth,
                        MultiLine);

      W->SetOnHelpCallback(CallBackLookup<WindowControl::OnHelpCallback_t>(LookUpTable, OnHelpCallback));

      W->SetHelpText(utf8_to_tstring(AttributeToString(*child, "Help", "")).c_str());

      W->SetReadOnly(ReadOnly != 0);

      W->SetUseKeyboard(AttributeToLong(*child, "Keyboard", 0));

      xml_node* dataFieldNode = child->first_node("DataField");
      if (dataFieldNode) {

        const char* DataType = AttributeToString(*dataFieldNode, "DataType", "");
        const char* DisplayFmt = AttributeToString(*dataFieldNode, "DisplayFormat", "");
        const char* EditFormat = AttributeToString(*dataFieldNode, "EditFormat", "");
        const char* OnDataAccess = AttributeToString(*dataFieldNode, "OnDataAccess", "");

        double Min = AttributeToDouble(*dataFieldNode, "Min", std::numeric_limits<double>::min());
        double Max = AttributeToDouble(*dataFieldNode, "Max", std::numeric_limits<double>::max());
        double Step = AttributeToDouble(*dataFieldNode, "Step", 1);
        int Fine = AttributeToLong(*dataFieldNode, "Fine", 0);

        if (strcasecmp(DataType, "enum")==0){
          W->SetDataField(
              new DataFieldEnum(EditFormat, DisplayFmt, false,
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess)));
        } else if (strcasecmp(DataType, "filereader")==0){
          W->SetDataField(
              new DataFieldFileReader(EditFormat, DisplayFmt,
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess))
          );
        } else if (strcasecmp(DataType, "boolean")==0){
          W->SetDataField(
              new DataFieldBoolean(EditFormat, DisplayFmt, false, MsgToken(958), MsgToken(959), // ON OFF
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess))
          );
        } else if (strcasecmp(DataType, "double")==0){
          W->SetDataField(
			        new DataFieldFloat(EditFormat, DisplayFmt, Min, Max, 0, Step, Fine,
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess))
          );
        } else if (strcasecmp(DataType, "integer")==0){
          W->SetDataField(
              new DataFieldInteger(EditFormat, DisplayFmt, (int)Min, (int)Max, (int)0, (int)Step,
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess))
          );
        } else if (strcasecmp(DataType, "string")==0){
          W->SetDataField(
              new DataFieldString(EditFormat, DisplayFmt, TEXT(""),
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess))
          );
        } else if (strcasecmp(DataType, "time")==0){
          W->SetDataField(
      			  new DataFieldTime(EditFormat, DisplayFmt, Min, Max, 0, Step, Fine,
                    CallBackLookup<DataField::DataAccessCallback_t>(LookUpTable, OnDataAccess))
          );
        } else {
          // Unknown DataType
          assert(false);
        }
      }

    } else if (strcmp(child->name(), "WndButton") == 0) {

      const char* ClickCallback = AttributeToString(*child, "OnClickNotify", "");

      WC = new WndButton(Parent, Name, Caption, X, Y, Width, Height,
                      CallBackLookup<WndButton::ClickNotifyCallback_t>(LookUpTable, ClickCallback));

    } else if (strcmp(child->name(), "WndOwnerDrawFrame") == 0) {

      const char* PaintCallback = AttributeToString(*child, "OnPaint", "");

      WC = new WndOwnerDrawFrame(Parent, Name, X, Y, Width, Height,
                      CallBackLookup<WndOwnerDrawFrame::OnPaintCallback_t>(LookUpTable, PaintCallback));

    } else if (strcmp(child->name(), "WndFrame") == 0) {

      WC = new WndFrame(Parent, Name, X, Y, Width, Height);

      LoadChildsFromXML(WC, LookUpTable, *child, ParentFont);  // recursivly create dialog

    } else if (strcmp(child->name(), "WndListFrame") == 0) {

      const char* ListCallback = AttributeToString(*child, "OnListInfo", "");

      WC = new WndListFrame(Parent, Name, X, Y, Width, Height,
                    CallBackLookup<WndListFrame::OnListCallback_t>(LookUpTable, ListCallback));

      LoadChildsFromXML(WC, LookUpTable, *child, ParentFont);  // recursivly create dialog

    } else {
        LKASSERT(false); // unknow control.
    }

    if (WC != nullptr) {

      if (Font != -1) {
        FontReference FontRef = GetFontRef(Font);
        if(FontRef) {
          WC->SetFont(FontRef);
        }
      }

      if (uBackColor != 0xffffffff) {
        WC->SetBackColor(LKColor((uBackColor>>16)&0xff, (uBackColor>>8)&0xff, (uBackColor>>0)&0xff));
      }

      if (uForeColor != 0xffffffff) {
        WC->SetForeColor(LKColor((uForeColor>>16)&0xff, (uForeColor>>8)&0xff, (uForeColor>>0)&0xff));
      }

      if (!Visible) {
        WC->SetVisible(Visible);
      }

      if (Caption[0] != '\0') {
        WC->SetCaption(LKGetText(Caption));
      }

      if (Border != 0) {
        WC->SetBorderKind(Border);
      }
    }
  }
}

} // namespace

WndForm *dlgLoadFromXML(const CallBackTableEntry_t *LookUpTable, unsigned resID) {

  WndForm* theForm = nullptr;
  std::string xml_string = xmlLoadFromResource(MAKEINTRESOURCE(resID));

  xml_document xmldoc;
  try {
    constexpr int Flags = rapidxml::parse_trim_whitespace | rapidxml::parse_normalize_whitespace;
    xmldoc.parse<Flags>(xml_string.data());
  } catch (rapidxml::parse_error& e) {
    StartupStore(TEXT(".. XML Dialog parse failed : %s"), to_tstring(e.what()).c_str());
    return nullptr;
  }

  xml_node* rootNode = xmldoc.first_node("PMML");
  // TODO code: put in error checking here and get rid of exits in xmlParser
  if (!rootNode) {

    MessageBoxX(
      TEXT("Error in loading XML dialog"),
      TEXT("Dialog error"),
      mbOk);

    return nullptr;
  }

  xml_node* node = rootNode->first_node("WndForm");
  if (node) {
    int X, Y, Width, Height, Popup;
    TCHAR Name[64];
    TCHAR sTmp[128];


    // fix screen width adjust but do not enlarge it if popup is selected
    GetDefaultWindowControlProps(*node, Name, X, Y,
                                 Width, Height, Popup, sTmp);
    if (!Popup) {
      const RECT rc = main_window->GetClientRect();

      Width=rc.right;
      Height=rc.bottom;
      X=0;
      Y=0;
    }

    bool bModal = AttributeToLong(*node, "Modal", 1);

    theForm = new WndForm(Name, sTmp, X, Y, Width, Height, bModal);

    int Font = AttributeToLong(*node, "Font", -1);
    if (Font != -1) {
      FontReference FontRef = GetFontRef(Font);
      if(FontRef) {
        theForm->SetTitleFont(FontRef);
        theForm->SetFont(FontRef);
      }
    }

    unsigned uBackColor = AttributeToULong(*node, "BackColor", 0xffffffff);
    if (uBackColor != 0xffffffff){
      theForm->SetBackColor(LKColor((uBackColor>>16)&0xff, (uBackColor>>8)&0xff, (uBackColor>>0)&0xff));
    }

    unsigned uForeColor = AttributeToULong(*node, "ForeColor", 0xffffffff);
    if (uForeColor != 0xffffffff){
      theForm->SetForeColor(LKColor((uForeColor>>16)&0xff, (uForeColor>>8)&0xff, (uForeColor>>0)&0xff));
    }

    LoadChildsFromXML(theForm, LookUpTable, *node, Font);
  }

  return theForm;
}
