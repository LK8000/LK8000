/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTools.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include <limits.h>

#include "WindowControls.h"
#include "xmlParser.h"
#include "InfoBoxLayout.h"
#include "RGB.h"
#include "Dialogs.h"
#include "utils/stringext.h"


int DLGSCALE(int x) {
  int iRetVal = x;
  iRetVal = (int) ((x)*ScreenDScale);
  return iRetVal;
}


static void OnButtonClick(WindowControl * Sender){

  ((WndForm *)Sender->GetOwner()->GetOwner())->SetModalResult(Sender->GetTag());
}

int WINAPI MessageBoxX(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, bool wfullscreen){

  WndForm *wf=NULL;
  WndFrame *wText=NULL;
  int X, Y, Width, Height;
  WndButton *wButtons[10];
  int ButtonCount = 0;
  int i,x,y,d,w,h,res,dY;
  RECT rc;

  // todo
  
  hWnd = hWndMainWindow;
  //ASSERT(hWnd == hWndMainWindow);

  //ASSERT(lpText != NULL);
  //ASSERT(lpCaption != NULL);

  GetClientRect(hWnd, &rc);

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
	if (ScreenLandscape)
		y = DLGSCALE(160);
	else
		y = DLGSCALE(200);
  } else
	y = DLGSCALE(100);
  w = DLGSCALE(60);
  h = DLGSCALE(32);

  wf = new WndForm(hWnd, TEXT("frmXcSoarMessageDlg"), 
                   lpCaption, X, Y, Width, Height);
  wf->SetFont(MapWindowBoldFont);
  wf->SetTitleFont(MapWindowBoldFont);
  wf->SetBackColor(RGB_WINBACKGROUND);

  wText = new WndFrame(wf, 
                       TEXT("frmMessageDlgText"), 
                       0, 
                       DLGSCALE(5), 
                       Width, 
                       Height);
  wText->SetCaption(lpText);
  wText->SetFont(MapWindowBoldFont);
  wText->SetCaptionStyle(
        DT_EXPANDTABS
      | DT_CENTER
      | DT_NOCLIP
      | DT_WORDBREAK
        //      | DT_VCENTER
  );

  /* TODO code: this doesnt work to set font height 
  dY = wText->GetLastDrawTextHeight() - Height;
  */
  if (wfullscreen)
	dY=0;
  else
	dY = DLGSCALE(-40);
  // wText->SetHeight(wText->GetLastDrawTextHeight()+5);
  wf->SetHeight(wf->GetHeight() + dY);

  y += dY;

  uType = uType & 0x000f;

  if (uType == MB_OK
      || uType == MB_OKCANCEL

  ){
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), TEXT("OK"), 
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDOK);
    ButtonCount++;
  }

  if (uType == MB_YESNO
      || uType == MB_YESNOCANCEL
  ){
	// LKTOKEN  _@M827_ = "Yes" 
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), gettext(TEXT("_@M827_")), 
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDYES);
    ButtonCount++;
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), gettext(TEXT("_@M890_")),  // No
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDNO);
    ButtonCount++;
  }

  if (uType == MB_ABORTRETRYIGNORE
      || uType == MB_RETRYCANCEL
  ){
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), 
	// LKTOKEN  _@M566_ = "Retry" 
                                          gettext(TEXT("_@M566_")), 
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDRETRY);
    ButtonCount++;
  }

  if (uType == MB_OKCANCEL
      || uType == MB_RETRYCANCEL
      || uType == MB_YESNOCANCEL
  ){
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), 
	// LKTOKEN  _@M161_ = "Cancel" 
                                          gettext(TEXT("_@M161_")), 
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDCANCEL);
    ButtonCount++;
  }

  if (uType == MB_ABORTRETRYIGNORE
  ){
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), 
	// LKTOKEN  _@M47_ = "Abort" 
                                          gettext(TEXT("_@M47_")), 
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDABORT);
    ButtonCount++;
    wButtons[ButtonCount] = new WndButton(wf, TEXT(""), 
	// LKTOKEN  _@M349_ = "Ignore" 
                                          gettext(TEXT("_@M349_")), 
                                          0, y, w, h, OnButtonClick);
    wButtons[ButtonCount]->SetTag(IDIGNORE);
    ButtonCount++;
  }

  LKASSERT(ButtonCount!=0);
  d = Width / (ButtonCount);
  x = d/2-w/2;

  for (i=0; i<ButtonCount; i++){
    wButtons[i]->SetLeft(x);
    x += d;
  }

  res = wf->ShowModal();

  delete wf;

  return(res);

}




long StringToIntDflt(const TCHAR *String, long Default){
  if (String == NULL || String[0] == '\0')
    return(Default);
  return(_tcstol(String, NULL, 0));        
}

double StringToFloatDflt(const TCHAR *String, double Default){
  if (String == NULL || String[0] == '\0')
    return(Default);
  return(_tcstod(String, NULL));
}

const TCHAR *StringToStringDflt(const TCHAR *String, const TCHAR *Default){
  if (String == NULL || String[0] == '\0')
    return(Default);
  return(String);
}

void GetDefaultWindowControlProps(XMLNode *Node, TCHAR *Name, int *X, int *Y, int *Width, int *Height, int *Popup, int *Font, TCHAR *Caption){

  *Popup = StringToIntDflt(Node->getAttribute(TEXT("Popup")), 0); // don't make wndForm block entire screen
  *X = DLGSCALE(StringToIntDflt(Node->getAttribute(TEXT("X")), 0));
  *Y = StringToIntDflt(Node->getAttribute(TEXT("Y")), 0);
  if (*Y>=0) { // not -1
    (*Y) = DLGSCALE(*Y);
  }
  *Width = DLGSCALE(StringToIntDflt(Node->getAttribute(TEXT("Width")), 50));
  *Height = StringToIntDflt(Node->getAttribute(TEXT("Height")), 50);
  if (*Height>=0) {
    (*Height) = DLGSCALE(*Height);
  }
  *Font = StringToIntDflt(Node->getAttribute(TEXT("Font")), -1);
  _tcscpy(Name, StringToStringDflt(Node->getAttribute(TEXT("Name")), TEXT("")));
  _tcscpy(Caption, StringToStringDflt(Node->getAttribute(TEXT("Caption")), TEXT("")));
  _tcscpy(Caption,LKGetText(Caption));

}

void *CallBackLookup(CallBackTableEntry_t *LookUpTable, TCHAR *Name){

  int i;

  if (LookUpTable!=NULL && Name!=NULL && Name[0]!= '\0')
    for (i=0; LookUpTable[i].Ptr != NULL; i++){
      if (_tcscmp(LookUpTable[i].Name, Name) == 0){
        return(LookUpTable[i].Ptr);
      }
    }

  return(NULL);

}

void LoadChildsFromXML(WindowControl *Parent, CallBackTableEntry_t *LookUpTable, XMLNode *Node, int Font);

// The Font=n in dialogs.  0-4, 4 unused kept for compat issue with very old code
static HFONT FontMap[5] = {
    TitleWindowFont,
    MapWindowFont,
    MapWindowBoldFont,
    CDIWindowFont,
    CDIWindowFont, // was InfoWindow 
  };


#include <stdio.h>


XMLNode xmlLoadFromResource(const TCHAR* lpName,
                            LPCTSTR tag, 
                            XMLResults *pResults) {
  LPTSTR lpRes; 
  HRSRC hResInfo;
  HGLOBAL hRes; 
  int l;
  TCHAR * szXML;

  // Find the xml resource.
  hResInfo = FindResource (hInst, lpName, TEXT("XMLDialog")); 

  if (hResInfo == NULL) {
    MessageBoxX(hWndMainWindow,
      TEXT("Can't find resource"),
      TEXT("Dialog error"),
      MB_OK|MB_ICONEXCLAMATION);

    // unable to find the resource
    return XMLNode::emptyXMLNode;
  }

  // Load the wave resource. 
  hRes = LoadResource (hInst, hResInfo); 

  if (hRes == NULL) {
    MessageBoxX(hWndMainWindow,
      TEXT("Can't load resource"),
      TEXT("Dialog error"),
      MB_OK|MB_ICONEXCLAMATION);

    // unable to load the resource
    return XMLNode::emptyXMLNode;
  }

  // Lock the wave resource and do something with it. 
  lpRes = (LPTSTR)LockResource (hRes);
  
  if (lpRes) {
    l = SizeofResource(hInst,hResInfo);
    if (l>0) {
      char *buf= (char*)malloc(l+2);
      if (!buf) {
        //StartupStore(_T("------ LoadFromRes malloc error%s"),NEWLINE); // 100101
          goto _errmem;
      }
      strncpy(buf,(char*)lpRes,l);
      buf[l]=0; // need to explicitly null-terminate.
      buf[l+1]=0;
     
      szXML = (TCHAR*) calloc(l+2, sizeof (TCHAR));
      if (!szXML) {
        //StartupStore(_T("------ LoadFromRes malloc2 error%s"),NEWLINE); // 100101
          goto _errmem;
      }
      int nSize = utf2unicode(buf, szXML, l+1);
      free(buf);
      if(nSize <=0) {
          free(szXML);
          
          MessageBoxX(hWndMainWindow,
                        TEXT("Invalid dialog template"),
                        TEXT("Dialog error"),
                        MB_OK|MB_ICONEXCLAMATION);
          
          return XMLNode::emptyXMLNode;
      }
      XMLNode x=XMLNode::parseString(szXML,tag,pResults);

      free(szXML);
      return x;
    }
  }
  MessageBoxX(hWndMainWindow,
              TEXT("Can't lock resource"),
              TEXT("Dialog error"),
              MB_OK|MB_ICONEXCLAMATION);
  return XMLNode::emptyXMLNode;
  
_errmem:
    MessageBoxX(hWndMainWindow,
                TEXT("Can't allocate memory"),
                TEXT("Dialog error"),
                MB_OK|MB_ICONEXCLAMATION);
    // unable to allocate memory
    return XMLNode::emptyXMLNode;
}



static XMLNode xmlOpenResourceHelper(const TCHAR *lpszXML, LPCTSTR tag)
{
    XMLResults pResults;

    pResults.error = eXMLErrorNone;
    XMLNode::GlobalError = false;
    XMLNode xnode=xmlLoadFromResource(lpszXML, tag, &pResults);
    if (pResults.error != eXMLErrorNone)
    {
      XMLNode::GlobalError = true;
      TCHAR errortext[100];
      _stprintf(errortext,TEXT("%s %i %i"), XMLNode::getError(pResults.error),
                pResults.nLine, pResults.nColumn);
                
      MessageBoxX(hWndMainWindow,
                  errortext,
                  TEXT("Dialog error"),
                  MB_OK|MB_ICONEXCLAMATION);
        // was exit(255);

    }
    return xnode;
}



WndForm *dlgLoadFromXML(CallBackTableEntry_t *LookUpTable, const char *filename, HWND Parent,
                        const TCHAR* raw_resource) {

  WndForm *theForm = NULL;
  XMLNode xMainNode;

  TCHAR tfilename[MAX_PATH];
  _stprintf(tfilename,_T("%S"),filename);

  // StartupStore(_T("... xmlOpen <%s>\n"),tfilename);

  if (FileExists(tfilename))
	xMainNode=XMLNode::openFileHelper(tfilename ,TEXT("PMML"));

  //
  // If nothing available in filesystem, load from internal resources
  // This is going to be removed in future LK versions.
  //
  if (xMainNode.isEmpty()) {
    if (raw_resource) {
      xMainNode =xmlOpenResourceHelper(raw_resource, TEXT("PMML"));
    }
  }


  // TODO code: put in error checking here and get rid of exits in xmlParser
  if (xMainNode.isEmpty()) {

    MessageBoxX(hWndMainWindow,
      TEXT("Error in loading XML dialog"),
      TEXT("Dialog error"),
      MB_OK|MB_ICONEXCLAMATION);

    return NULL;
  }


  XMLNode xNode=xMainNode.getChildNode(TEXT("WndForm"));

  FontMap[0] = TitleWindowFont;
  FontMap[1] = MapWindowFont;
  FontMap[2] = MapWindowBoldFont;
  FontMap[3] = CDIWindowFont;
  FontMap[4] = CDIWindowFont;

  if (!xNode.isEmpty()){
    int X,Y,Width,Height,Popup,Font;
    TCHAR sTmp[128];
    TCHAR Name[64];

    COLORREF BackColor;
    COLORREF ForeColor;
    RECT rc;
    GetClientRect(Parent, &rc);

    // fix screen width adjust but do not enlarge it if popup is selected
    GetDefaultWindowControlProps(&xNode, Name, &X, &Y, &Width, &Height, &Popup,
                                 &Font, sTmp);
    if (!Popup) {
      Width=rc.right;
      Height=rc.bottom;
      X=0;
      Y=0;
    }

    BackColor = StringToIntDflt(xNode.getAttribute(TEXT("BackColor")), 
                                0xffffffff);
    ForeColor = StringToIntDflt(xNode.getAttribute(TEXT("ForeColor")), 
                                0xffffffff);

    theForm = new WndForm(Parent, Name, sTmp, X, Y, Width, Height);

    if (Font != -1)
      theForm->SetTitleFont(FontMap[Font]);

    if (Font != -1)
      theForm->SetFont(FontMap[Font]);
    if (BackColor != 0xffffffff){
      	BackColor = RGB((BackColor>>16)&0xff, (BackColor>>8)&0xff, (BackColor>>0)&0xff);
      theForm->SetBackColor(BackColor);
    }
    if (ForeColor != 0xffffffff){
	ForeColor = RGB((ForeColor>>16)&0xff, (ForeColor>>8)&0xff, (ForeColor>>0)&0xff);
      theForm->SetForeColor(ForeColor);
    }

    LoadChildsFromXML(theForm, LookUpTable, &xNode, Font);

    if (XMLNode::GlobalError) {
      MessageBoxX(hWndMainWindow,
                 TEXT("Error in loading XML dialog"),
                 TEXT("Dialog error"),
                 MB_OK|MB_ICONEXCLAMATION);

      delete theForm;
      return NULL;
    }

  } else {
    MessageBoxX(hWndMainWindow,
      TEXT("Error in loading XML dialog"),
      TEXT("Dialog error"),
      MB_OK|MB_ICONEXCLAMATION);

    return NULL;
  }

  return(theForm);

}



void LoadChildsFromXML(WindowControl *Parent, 
                       CallBackTableEntry_t *LookUpTable, 
                       XMLNode *Node, 
                       int ParentFont) {

  int X,Y,Width,Height,Popup,Font;
  TCHAR Caption[128];
  TCHAR Name[64];
  COLORREF BackColor;
  COLORREF ForeColor;
  bool Visible;
  int Border;

  int Count = Node->nChildNode();

  for (int i=0; i<Count; i++){

    WindowControl *WC=NULL;

    XMLNode childNode = Node->getChildNode(i);

    GetDefaultWindowControlProps(&childNode, 
                                 Name, 
                                 &X, &Y, 
                                 &Width, &Height, &Popup,
                                 &Font, Caption);

    BackColor = StringToIntDflt(childNode.getAttribute(TEXT("BackColor")), 
                                0xffffffff);
    ForeColor = StringToIntDflt(childNode.getAttribute(TEXT("ForeColor")), 
                                0xffffffff);
    Visible = StringToIntDflt(childNode.getAttribute(TEXT("Visible")), 1) == 1;
    if (BackColor != 0xffffffff){
      BackColor = RGB((BackColor>>16)&0xff,
                      (BackColor>>8)&0xff,
                      (BackColor>>0)&0xff);
    }
    if (ForeColor != 0xffffffff){
      ForeColor = RGB((ForeColor>>16)&0xff,
                      (ForeColor>>8)&0xff,
                      (ForeColor>>0)&0xff);
    }
    Font = StringToIntDflt(childNode.getAttribute(TEXT("Font")), ParentFont);

    Border = StringToIntDflt(childNode.getAttribute(TEXT("Border")), 0);

    if (_tcscmp(childNode.getName(), TEXT("WndProperty")) == 0){

      WndProperty *W;
      int CaptionWidth;
      TCHAR DataNotifyCallback[128];
      TCHAR OnHelpCallback[128];
      int ReadOnly;
      int MultiLine;

      CaptionWidth = 
        DLGSCALE(StringToIntDflt(childNode.getAttribute(TEXT("CaptionWidth")), 
                        0));
      MultiLine = 
        StringToIntDflt(childNode.getAttribute(TEXT("MultiLine")), 
                        0);
      ReadOnly = \
        StringToIntDflt(childNode.getAttribute(TEXT("ReadOnly")), 
                        0);


      _tcscpy(DataNotifyCallback, 
              StringToStringDflt(childNode.getAttribute(TEXT("OnDataNotify")),
                                 TEXT("")));

      _tcscpy(OnHelpCallback, 
              StringToStringDflt(childNode.getAttribute(TEXT("OnHelp")),
                                 TEXT("")));

      _tcscpy(
		  Caption, 
			StringToStringDflt(childNode.getAttribute(TEXT("Caption")), TEXT(""))
		);
	  _tcscpy(Caption, LKGetText(Caption));

      WC = W = 
        new WndProperty(Parent, Name, Caption, X, Y, 
                        Width, Height, CaptionWidth,
                        (WndProperty::DataChangeCallback_t) 
                        CallBackLookup(LookUpTable, DataNotifyCallback), 
                        MultiLine);

      W->SetOnHelpCallback((WindowControl::OnHelpCallback_t) 
                           CallBackLookup(LookUpTable, OnHelpCallback));

      W->SetHelpText(StringToStringDflt(
                     childNode.getAttribute(TEXT("Help")), 
                     TEXT("")));

      Caption[0] = '\0';
      W->SetReadOnly(ReadOnly != 0);

      W->SetUseKeyboard(StringToIntDflt(childNode.getAttribute(TEXT("keyboard")), 0));

      if (childNode.nChildNode(TEXT("DataField")) > 0){

        TCHAR DataType[32];
        TCHAR DisplayFmt[32];
        TCHAR EditFormat[32];
        TCHAR OnDataAccess[64];
        double Min, Max, Step;
	int Fine;

        XMLNode dataFieldNode = 
          childNode.getChildNode(TEXT("DataField"), 0);

        _tcscpy(DataType, 
                StringToStringDflt(dataFieldNode.
                                   getAttribute(TEXT("DataType")), 
                                   TEXT("")));
        _tcscpy(DisplayFmt, 
                StringToStringDflt(dataFieldNode.
                                   getAttribute(TEXT("DisplayFormat")), 
                                   TEXT("")));
        _tcscpy(EditFormat, 
                StringToStringDflt(dataFieldNode.
                                   getAttribute(TEXT("EditFormat")), 
                                   TEXT("")));
        _tcscpy(OnDataAccess, 
                StringToStringDflt(dataFieldNode.
                                   getAttribute(TEXT("OnDataAccess")), 
                                   TEXT("")));
        ReadOnly = StringToIntDflt(dataFieldNode.
                                   getAttribute(TEXT("ReadOnly")), 0);
        Min = StringToIntDflt(dataFieldNode.
                              getAttribute(TEXT("Min")), INT_MIN);
        Max = StringToIntDflt(dataFieldNode.
                              getAttribute(TEXT("Max")), INT_MAX);
        Step = StringToFloatDflt(dataFieldNode.
                                 getAttribute(TEXT("Step")), 1);

	Fine = StringToIntDflt(dataFieldNode.
			       getAttribute(TEXT("Fine")), 0);

        if (_tcsicmp(DataType, TEXT("enum"))==0){
          W->SetDataField(
                          new DataFieldEnum(EditFormat, DisplayFmt, false,
                                            (DataField::DataAccessCallback_t) 
                                            CallBackLookup(LookUpTable, 
                                                           OnDataAccess))
          );
        }
        if (_tcsicmp(DataType, TEXT("filereader"))==0){
          W->SetDataField(
                          new DataFieldFileReader(EditFormat, 
                                                  DisplayFmt,
                                                  (DataField::DataAccessCallback_t) 
                                                  CallBackLookup(LookUpTable, OnDataAccess))
          );
        }
        if (_tcsicmp(DataType, TEXT("boolean"))==0){
          W->SetDataField(
            new DataFieldBoolean(EditFormat, DisplayFmt, false, gettext(TEXT("_@M958_")), gettext(TEXT("_@M959_")), // ON OFF
              (DataField::DataAccessCallback_t) CallBackLookup(LookUpTable, OnDataAccess))
          );
        }
        if (_tcsicmp(DataType, TEXT("double"))==0){
          W->SetDataField(
			  new DataFieldFloat(EditFormat, DisplayFmt, Min, Max, 0, Step, Fine,
              (DataField::DataAccessCallback_t) CallBackLookup(LookUpTable, OnDataAccess))
          );
        }
        if (_tcsicmp(DataType, TEXT("integer"))==0){
          W->SetDataField(
                          new DataFieldInteger(EditFormat, DisplayFmt, (int)Min, (int)Max, (int)0, (int)Step,
              (DataField::DataAccessCallback_t) CallBackLookup(LookUpTable, OnDataAccess))
          );
        }
        if (_tcsicmp(DataType, TEXT("string"))==0){
          W->SetDataField(
            new DataFieldString(EditFormat, DisplayFmt, TEXT(""),
              (DataField::DataAccessCallback_t) CallBackLookup(LookUpTable, OnDataAccess))
          );
        }

      }

    }else

    if (_tcscmp(childNode.getName(), TEXT("WndButton")) == 0){

      TCHAR ClickCallback[128];

       _tcscpy(ClickCallback, StringToStringDflt(childNode.getAttribute(TEXT("OnClickNotify")), TEXT("")));

      WC = new WndButton(Parent, Name, Caption, X, Y, Width, Height,
               (WndButton::ClickNotifyCallback_t) 
                         CallBackLookup(LookUpTable, ClickCallback));

      Caption[0] = '\0';

    }else



    if (_tcscmp(childNode.getName(), TEXT("WndEventButton")) == 0){

      TCHAR iename[100];
      TCHAR ieparameters[100];
      _tcscpy(iename, 
              StringToStringDflt(childNode.
                                 getAttribute(TEXT("InputEvent")), 
                                 TEXT("")));
      _tcscpy(ieparameters, 
              StringToStringDflt(childNode.
                                 getAttribute(TEXT("Parameters")), 
                                 TEXT("")));

      WC = new WndEventButton(Parent, Name, Caption, X, Y, Width, Height,
                              iename, ieparameters);

      Caption[0] = '\0';

    }else



    if (_tcscmp(childNode.getName(), TEXT("WndOwnerDrawFrame")) == 0){

      TCHAR PaintCallback[128];

      _tcscpy(PaintCallback, StringToStringDflt(childNode.getAttribute(TEXT("OnPaint")), TEXT("")));

      WC = new WndOwnerDrawFrame(Parent, Name, X, Y, Width, Height,
               (WndOwnerDrawFrame::OnPaintCallback_t) CallBackLookup(LookUpTable, PaintCallback));

    }else

    if (_tcscmp(childNode.getName(), TEXT("WndFrame")) == 0){

      WndFrame *W;

      WC = W = new WndFrame(Parent, Name, X, Y, Width, Height);

      LoadChildsFromXML(W, LookUpTable, &childNode, ParentFont);  // recursivly create dialog

    }else

    if (_tcscmp(childNode.getName(), TEXT("WndListFrame")) == 0){

      TCHAR ListCallback[128];

      _tcscpy(ListCallback, StringToStringDflt(childNode.getAttribute(TEXT("OnListInfo")), TEXT("")));

      WC = new WndListFrame(Parent, Name, X, Y, Width, Height,
               (WndListFrame::OnListCallback_t) CallBackLookup(LookUpTable, ListCallback));

      LoadChildsFromXML(WC, LookUpTable, &childNode, ParentFont);  // recursivly create dialog

    }

    if (WC != NULL){

      if (Font != -1)
        WC->SetFont(FontMap[Font]);

      if (BackColor != 0xffffffff){
        WC->SetBackColor(BackColor);
      }

      if (ForeColor != 0xffffffff){
        WC->SetForeColor(ForeColor);
      }

      if (!Visible){
        WC->SetVisible(Visible);
      }

      if (Caption[0] != '\0'){
        WC->SetCaption(LKGetText(Caption));
      }

      if (Border != 0){
        WC->SetBorderKind(Border); 
      }

    }

  }

}

