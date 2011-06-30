/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWaypointEdit.cpp,v 8.2 2010/12/13 16:57:06 root Exp root $
*/

#include "StdAfx.h"
#include "externs.h"
#include "Units.h"
#include "Utils2.h"
#include "InputEvents.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "Waypointparser.h"

#include "utils/heapcheck.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;
static WAYPOINT *global_wpt=NULL;

static WndButton *buttonName = NULL;
static WndButton *buttonComment = NULL;

static void UpdateButtons(void) {
  TCHAR text[MAX_PATH];
  if (buttonName) {
	if (_tcslen(global_wpt->Name)<=0) {
		// LKTOKEN  _@M451_ = "Name" 
		_stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M451_")),
		// LKTOKEN  _@M7_ = "(blank)" 
		gettext(TEXT("_@M7_")));
	} else {
		// LKTOKEN  _@M451_ = "Name" 
		_stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M451_")),
		global_wpt->Name);
	}
	buttonName->SetCaption(text);
  }
  if (buttonComment) {
	if ((global_wpt->Comment==NULL) || (_tcslen(global_wpt->Comment)<=0) ) {
		// LKTOKEN  _@M190_ = "Comment" 
		_stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M190_")),
		// LKTOKEN  _@M7_ = "(blank)" 
		gettext(TEXT("_@M7_")));
	} else {
		// LKTOKEN  _@M190_ = "Comment" 
		_stprintf(text,TEXT("%s: %s"), gettext(TEXT("_@M190_")),
		global_wpt->Comment);
	}
	buttonComment->SetCaption(text);
  }
}


static void OnNameClicked(WindowControl *Sender) {
	(void)Sender;
  if (buttonName) {
    dlgTextEntryShowModal(global_wpt->Name, NAME_SIZE);
  }
  UpdateButtons();
}


static void OnCommentClicked(WindowControl *Sender) {
	(void)Sender;
  if (buttonComment) {
	//@ 101219
	TCHAR comment[COMMENT_SIZE*2];
	if (global_wpt->Comment != NULL)
		_tcscpy(comment,global_wpt->Comment);
	else
		_tcscpy(comment,_T(""));
	dlgTextEntryShowModal(comment, COMMENT_SIZE);

	// in any case free the space
	if (global_wpt->Comment != NULL) free(global_wpt->Comment);
	if (_tcslen(comment)>0) {
		// do we have a new comment?
		global_wpt->Comment = (TCHAR*)malloc((_tcslen(comment)+2)*sizeof(TCHAR));
		if (global_wpt->Comment == NULL) {
			StartupStore(_T("------ Wp Edit new comment malloc failed for comment <%s>! Memory problem!%s"),comment,NEWLINE);
		} else {
			_tcscpy(global_wpt->Comment,comment);
		}
	}
  }
  UpdateButtons();
}


static void SetUnits(void) {
  WndProperty* wp;
  switch (Units::CoordinateFormat) {
  case 0: // ("DDMMSS");
  case 1: // ("DDMMSS.ss");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeDDDD"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeDDDD"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudemmm"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudemmm"));
    if (wp) {
      wp->SetVisible(false);
    }
    break;
  case 2: // ("DDMM.mmm");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeDDDD"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeDDDD"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeS"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeS"));
    if (wp) {
      wp->SetVisible(false);
    }
    break;
  case 3: // ("DD.dddd");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeM"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeM"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeS"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeS"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudemmm")); 
    // hide this field for DD.dddd format
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudemmm"));
    if (wp) {
      wp->SetVisible(false);
    }
    break;
#ifdef NEWUTM
  case 4: // UTM (" 32T 123456 1234567 ")
    wp = (WndProperty*)wf->FindByName(TEXT("prpUTMgrid"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpUTMeast"));
    if (wp) {
      wp->SetVisible(false);
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpUTMnorth"));
    if (wp) {
      wp->SetVisible(false);
    }
#endif
  }
}

static void SetValues(void) {
  WndProperty* wp;
  bool sign;
  int dd,mm,ss;

  Units::LongitudeToDMS(global_wpt->Longitude,
			&dd, &mm, &ss, &sign);
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeSign"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText((TEXT("W")));
    dfe->addEnumText((TEXT("E")));
    dfe->Set(sign);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeD"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }

  switch (Units::CoordinateFormat) {
  case 0: // ("DDMMSS");
  case 1: // ("DDMMSS.ss");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeM"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(mm);
      wp->RefreshDisplay();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeS"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(ss);
      wp->RefreshDisplay();
    }
    break;
  case 2: // ("DDMM.mmm");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeM"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(mm);
      wp->RefreshDisplay();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudemmm"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(1000.0*ss/60.0);
      wp->RefreshDisplay();
    }
    break;
  case 3: // ("DD.dddd");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeDDDD"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(10000.0*(mm+ss/60.0)/60.0);
      wp->RefreshDisplay();
    }
    break;
#ifdef NEWUTM
  case 4:
    wp = (WndProperty*)wf->FindByName(TEXT("prpUTMeast"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(mm);
      wp->RefreshDisplay();
    }
    break;
#endif
  }
  
  Units::LatitudeToDMS(global_wpt->Latitude,
		       &dd, &mm, &ss, &sign);
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeSign"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->addEnumText((TEXT("S")));
    dfe->addEnumText((TEXT("N")));
    dfe->Set(sign);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeD"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(dd);
    wp->RefreshDisplay();
  }

  switch (Units::CoordinateFormat) {
  case 0: // ("DDMMSS");
  case 1: // ("DDMMSS.ss");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeM"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(mm);
      wp->RefreshDisplay();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeS"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(ss);
      wp->RefreshDisplay();
    }
    break;
  case 2: // ("DDMM.mmm");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeM"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(mm);
      wp->RefreshDisplay();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudemmm"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(1000.0*ss/60.0);
      wp->RefreshDisplay();
    }
    break;
  case 3: // ("DD.dddd");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeDDDD"));
    if (wp) {
      wp->GetDataField()->SetAsFloat(10000.0*(mm+ss/60.0)/60.0);
      wp->RefreshDisplay();
    }
    break;
  }
    
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
  if (wp) {
    wp->GetDataField()->SetAsFloat(
				   iround(global_wpt->Altitude
					  *ALTITUDEMODIFY));
    wp->GetDataField()->SetUnits(Units::GetAltitudeName());
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpFlags"));
  if (wp) {
    DataFieldEnum* dfe;
    dfe = (DataFieldEnum*)wp->GetDataField();
	// LKTOKEN _@M1226_ "Turnpoint"
    dfe->addEnumText(gettext(TEXT("_@M1226_")));
	// LKTOKEN _@M1224_ "Airport"
    dfe->addEnumText(gettext(TEXT("_@M1224_")));
	// LKTOKEN _@M1225_ "Landable"
    dfe->addEnumText(gettext(TEXT("_@M1225_")));
    dfe->Set(0);
    if ((global_wpt->Flags & LANDPOINT)==LANDPOINT) {
      dfe->Set(2);
    } 
    if ((global_wpt->Flags & AIRPORT)==AIRPORT) {
      dfe->Set(1);
    }
    
    wp->RefreshDisplay();
  }
}


static void GetValues(void) {
  WndProperty* wp;
  bool sign = false;
  int dd = 0;
  double num=0, mm = 0, ss = 0; // mm,ss are numerators (division) so don't want to lose decimals

  wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeSign"));
  if (wp) {
    sign = (wp->GetDataField()->GetAsInteger()==1);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeD"));
  if (wp) {
    dd = wp->GetDataField()->GetAsInteger();
  }

  switch (Units::CoordinateFormat) {
  case 0: // ("DDMMSS");
  case 1: // ("DDMMSS.ss");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeM"));
    if (wp) {
      mm = wp->GetDataField()->GetAsInteger();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeS"));
    if (wp) {
      ss = wp->GetDataField()->GetAsInteger();
    }
    num = dd+mm/60.0+ss/3600.0;
    break;
  case 2: // ("DDMM.mmm");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeM"));
    if (wp) {
      mm = wp->GetDataField()->GetAsInteger();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudemmm"));
    if (wp) {
      ss = wp->GetDataField()->GetAsInteger();
    }
    num = dd+(mm+ss/1000.0)/60.0;
    break;
  case 3: // ("DD.dddd");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLongitudeDDDD"));
    if (wp) {
      mm = wp->GetDataField()->GetAsInteger();
    }
    num = dd+mm/10000;
    break;
  }
  if (!sign) {
    num = -num;
  }
  
  global_wpt->Longitude = num;
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeSign"));
  if (wp) {
    sign = (wp->GetDataField()->GetAsInteger()==1);
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeD"));
  if (wp) {
    dd = wp->GetDataField()->GetAsInteger();
  }

  switch (Units::CoordinateFormat) {
  case 0: // ("DDMMSS");
  case 1: // ("DDMMSS.ss");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeM"));
    if (wp) {
      mm = wp->GetDataField()->GetAsInteger();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeS"));
    if (wp) {
      ss = wp->GetDataField()->GetAsInteger();
    }
    num = dd+mm/60.0+ss/3600.0;
    break;
  case 2: // ("DDMM.mmm");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeM"));
    if (wp) {
      mm = wp->GetDataField()->GetAsInteger();
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudemmm"));
    if (wp) {
      ss = wp->GetDataField()->GetAsInteger();
    }
    num = dd+(mm+ss/1000.0)/60.0;
    break;
  case 3: // ("DD.dddd");
    wp = (WndProperty*)wf->FindByName(TEXT("prpLatitudeDDDD"));
    if (wp) {
      mm = wp->GetDataField()->GetAsInteger();
    }
    num = dd+mm/10000;
    break;
  }
  if (!sign) {
    num = -num;
  }
  
  global_wpt->Latitude = num;
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
  if (wp) {
    ss = wp->GetDataField()->GetAsInteger();
    if (ss==0) {
      WaypointAltitudeFromTerrain(global_wpt);
    } else {
      global_wpt->Altitude = ss/ALTITUDEMODIFY;
    }
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpFlags"));
  if (wp) {
    int myflag = wp->GetDataField()->GetAsInteger();
    switch(myflag) {
    case 0:
      global_wpt->Flags = TURNPOINT;
	#if 100825
	if ( global_wpt->Format == LKW_CUP) {
		// set normal turnpoint style
		global_wpt->Style = 1;
	}
	#endif
      break;
    case 1:
      global_wpt->Flags = AIRPORT | TURNPOINT;
	#if 100825
	if ( global_wpt->Format == LKW_CUP) {
		// set airfield style
		global_wpt->Style = 5;
	}
	#endif
      break;
    case 2:
      global_wpt->Flags = LANDPOINT;
	#if 100825
	if ( global_wpt->Format == LKW_CUP) {
		// set outlanding style
		global_wpt->Style = 3;
	}
	#endif
      break;
    default:
      global_wpt->Flags = 0;
    };
  }
}


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(NULL)
};



void dlgWaypointEditShowModal(WAYPOINT *wpt) {
  if (!wpt) {
    return;
  }

  global_wpt = wpt;

  if (!InfoBoxLayout::landscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWaypointEdit_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename,
                        hWndMainWindow,
                        TEXT("IDR_XML_WAYPOINTEDIT_L"));
  } else {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgWaypointEdit.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_WAYPOINTEDIT"));
  }

  if (wf) {

    buttonName = ((WndButton *)wf->FindByName(TEXT("cmdName")));
    if (buttonName) {
      buttonName->SetOnClickNotify(OnNameClicked);
    }

    buttonComment = ((WndButton *)wf->FindByName(TEXT("cmdComment")));
    if (buttonComment) {
      buttonComment->SetOnClickNotify(OnCommentClicked);
    }

    UpdateButtons();

    SetUnits();

    SetValues();

    wf->SetModalResult(mrCancle);

    if (wf->ShowModal()==mrOK) {

      ////
      GetValues();

    }
    delete wf;
  }
  wf = NULL;

}


