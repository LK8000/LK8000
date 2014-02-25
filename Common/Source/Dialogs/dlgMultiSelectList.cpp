/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceSelectList.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
 */

#include "externs.h"
#include "MapWindow.h"
#include "Dialogs.h"
#include "Multimap.h"


#define MAX_LIST_ITEMS 50
ListElement* pResult = NULL;

ListElement Elements[MAX_LIST_ITEMS];

int iNO_ELEMENTS = 0;
static WndForm *wf = NULL;
static WndListFrame *wMultiSelectListList = NULL;
static WndOwnerDrawFrame *wMultiSelectListListEntry = NULL;

static int ItemIndex = -1;

static int NoAirfields = 0;
static int NoOutlands = 0;
static int NoWaypoints = 0;
static int NoTaskPoints = 0;
static int NoAirspace = 0;


#define MAX_AIRFILEDS 3
#define MAX_OUTLAND   2
#define MAX_WAYPOINTS 3
#define MAX_TASK      3
#define MAX_AIRSPACES 10

static void UpdateList(void) {
    wMultiSelectListList->ResetList();
    wMultiSelectListList->Redraw();

    wMultiSelectListListEntry->SetFocused(true, NULL);
}

static int DrawListIndex = 0;


// Keep the dialog list updated every second

static int OnTimer(WindowControl * Sender) {
    (void) Sender;

    // Timer events comes at 500ms, we need every second
    static bool timer_divider = false;
    timer_divider = !timer_divider;
    if (timer_divider) return 0;

    wMultiSelectListList->Redraw();
    return 0;
}

static void OnUpClicked(WindowControl * Sender) {
    if (ItemIndex > 0) {
        ItemIndex--;
    } else {
        ItemIndex = (iNO_ELEMENTS - 1);
    }
    wMultiSelectListList->SetItemIndexPos(ItemIndex);
    wMultiSelectListList->Redraw();
    wMultiSelectListListEntry->SetFocused(true, NULL);
}

static void OnDownClicked(WindowControl * Sender) {
    (void) Sender;

    if (ItemIndex < (iNO_ELEMENTS - 1)) {
        ItemIndex++;
    } else {
        ItemIndex = 0;
    }
    wMultiSelectListList->SetItemIndexPos(ItemIndex);
    wMultiSelectListList->Redraw();
    wMultiSelectListListEntry->SetFocused(true, NULL);
}

void dlgAddMultiSelectListDetailsDialog(int Index) {
    int iLastTaskPoint = 0;
    while (ValidTaskPoint(iLastTaskPoint))
        iLastTaskPoint++;
    iLastTaskPoint--;
    if ((Index >= 0) && (Index < iNO_ELEMENTS)) {
        switch (Elements[Index].type) {
        case IM_AIRSPACE:
            LKASSERT(Elements[Index].ptr);
            dlgAirspaceDetails((CAirspace*) Elements[Index].ptr);
            break;

        case IM_WAYPOINT:
            LKASSERT(Elements[Index].iIdx < (int) NumberOfWayPoints);
            SelectedWaypoint = Elements[Index].iIdx;
            PopupWaypointDetails();
            break;

        case IM_TASK_PT:
            LKASSERT(Elements[Index].iIdx <= MAXTASKPOINTS);
            RealActiveWaypoint = -1;
            if (Elements[Index].iIdx == 0)
                dlgTaskWaypointShowModal(Elements[Index].iIdx, 0, false, true);
            else {
                if (Elements[Index].iIdx == iLastTaskPoint)
                    dlgTaskWaypointShowModal(Elements[Index].iIdx, 2, false, true);
                else {
                    if ((AATEnabled) && (CALCULATED_INFO.Flying) && (!IsMultiMapNoMain())) {
                        wf->SetModalResult(mrOK);
                        wf->Close();
                        dlgTarget(Elements[Index].iIdx);
                    } else {
                        dlgTaskWaypointShowModal(Elements[Index].iIdx, 1, false, true);
                    }
                }
            }
            break;
        } // switch
    } // if Index..
}

int dlgGetNoElements(void) {
    return iNO_ELEMENTS;
}

void dlgAddMultiSelectListItem(long* pNew, int Idx, char type, double Distance) {

    if (type == IM_TASK_PT) {
        int iLastTaskPoint = 0;
        while (ValidTaskPoint(iLastTaskPoint))
            iLastTaskPoint++;

        if (iLastTaskPoint < 2)
            return;

        if (ValidTaskPoint(PanTaskEdit))
            return;

        if (Task[Idx].Index == RESWP_PANPOS)
            return;
    }

    if (type == IM_WAYPOINT)
        if (Idx == RESWP_PANPOS)
            return;

    for (int i = 0; i < iNO_ELEMENTS; i++) {
        LKASSERT(i < MAX_LIST_ITEMS);
        if (pNew != NULL)
            if (Elements[i].ptr == pNew)
                return;

        if (type != IM_AIRSPACE)
            if (type == Elements[i].type)
                if (Idx == Elements[i].iIdx)
                    return;

    } // for

    bool full = false;
    if (iNO_ELEMENTS < MAX_LIST_ITEMS - 1) {
        switch (type) {
        case IM_AIRSPACE:
            if (NoAirspace < MAX_AIRSPACES)
                NoAirspace++;
            else
                full = true;

            break;

        case IM_TASK_PT:
            if (NoTaskPoints < MAX_TASK)
                NoTaskPoints++;
            else
                full = true;

            break;

        case IM_WAYPOINT:
            if (WayPointCalc[Idx].IsLandable) {
                if (WayPointCalc[Idx].IsAirport) {
                    if (NoAirfields < MAX_AIRFILEDS)
                        NoAirfields++;
                    else
                        full = true;
                } else {
                    if (NoOutlands < MAX_OUTLAND)
                        NoOutlands++;
                    else
                        full = true;
                }
            } else {
                if (NoWaypoints < MAX_WAYPOINTS)
                    NoWaypoints++;
                else
                    full = true;
            }

            break;
        } // switch

        if (full) {
            for (int i = 0; i < iNO_ELEMENTS; i++) {
                if (Elements[i].type == type)
                    if (Distance < Elements[i].Dist) {
                        Elements[i].ptr = pNew;
                        Elements[i].type = type;
                        Elements[i].iIdx = Idx;
                        Elements[i].Dist = Distance;
                        return;
                    }
            }
        } else {
            int Pos = 0;
            while ((Elements[Pos].Dist <= Distance) && (Pos < iNO_ELEMENTS))
                Pos++;
            LKASSERT(Pos < MAX_LIST_ITEMS);
            LKASSERT(iNO_ELEMENTS < MAX_LIST_ITEMS);
            for (int i = iNO_ELEMENTS; i > Pos; i--) {
                LKASSERT(i >= 0);
                Elements[i].ptr = Elements[i - 1].ptr;
                Elements[i].type = Elements[i - 1].type;
                Elements[i].iIdx = Elements[i - 1].iIdx;
                Elements[i].Dist = Elements[i - 1].Dist;
            }
            Elements[Pos].ptr = pNew;
            Elements[Pos].type = type;
            Elements[Pos].iIdx = Idx;
            Elements[Pos].Dist = Distance;
            iNO_ELEMENTS++;
        }

    } // if no elements..
}

static void OnMultiSelectListPaintListItem(WindowControl * Sender, HDC hDC) {
    (void) Sender;
#define PICTO_WIDTH 50
    if ((DrawListIndex < iNO_ELEMENTS) &&(DrawListIndex >= 0)) {
        int j;
        static CAirspace airspace_copy;
        int i = DrawListIndex;
        LKASSERT(i < MAX_LIST_ITEMS);
        RECT rc = {0 * ScreenScale, 0 * ScreenScale, PICTO_WIDTH*ScreenScale, 34 * ScreenScale};

        const CAirspace* pAS = NULL;
        int HorDist, Bearing, VertDist;
        double Distance;
        unsigned int idx = 0;
        TCHAR text1[180] = {TEXT("empty")};
        TCHAR text2[180] = {TEXT("empty")};
        TCHAR Comment[80] = {TEXT("")};
        TCHAR Comment1[80] = {TEXT("")};
        SetBkColor(hDC, RGB(0xFF, 0xFF, 0xFF));
        LKASSERT(i < MAX_LIST_ITEMS);

        switch (Elements[i].type) {
        case IM_AIRSPACE:
            pAS = (CAirspace*) Elements[i].ptr;
            if (pAS) {
                /***********************************************************************
                 * here we use a local copy of the airspace, only common property exists
                 ***********************************************************************/
                airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(pAS);

                // airspace type already in name?
                if (_tcsnicmp(airspace_copy.Name(), airspace_copy.TypeName(), _tcslen(airspace_copy.TypeName())) == 0) {
                    _stprintf(text1, TEXT("%s"), airspace_copy.Name()); // yes, take name only
                } else {
                    // fixed strings max. 20 NAME_SIZE 30 => max. 30 char
                    _stprintf(text1, TEXT("%s %s"), airspace_copy.TypeName(), airspace_copy.Name());
                }

                CAirspaceManager::Instance().GetSimpleAirspaceAltText(Comment, sizeof (Comment) / sizeof (Comment[0]), airspace_copy.Top());
                CAirspaceManager::Instance().GetSimpleAirspaceAltText(Comment1, sizeof (Comment1) / sizeof (Comment1[0]), airspace_copy.Base());

                CAirspaceManager::Instance().AirspaceCalculateDistance((CAirspace*) pAS, &HorDist, &Bearing, &VertDist);
                _stprintf(text2, TEXT("%3.1f%s (%s - %s)"), (double) HorDist*DISTANCEMODIFY, Units::GetDistanceName(), Comment1, Comment); //8 + 8+3   21

                /****************************************************************
                 * for drawing the airspace pictorial, we need the original data.
                 * copy contain only base class property, not geo data,
                 * original data are shared ressources !
                 * for that we need to grant all called methods are thread safe
                 ****************************************************************/
                pAS->DrawPicto(hDC, rc);
            }
            break;


        case IM_TASK_PT:
        case IM_WAYPOINT:
            idx = Elements[i].iIdx;
            if (Elements[i].type == IM_TASK_PT) {
                LKASSERT(Elements[i].iIdx <= MAXTASKPOINTS);
                idx = Task[Elements[i].iIdx].Index;
            }

            if (idx >= NumberOfWayPoints) idx = NumberOfWayPoints;
            LKASSERT(idx <= NumberOfWayPoints);

            if (WayPointList[idx].Comment != NULL) {
                LK_tcsncpy(Comment, WayPointList[idx].Comment, 30);
            } else {
                _stprintf(Comment, TEXT(""));
            }

            DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude, WayPointList[idx].Latitude,
                            WayPointList[idx].Longitude, &Distance, NULL);

            if (Elements[i].type != IM_TASK_PT) {
                if (WayPointCalc[idx].IsLandable) {
                    MapWindow::DrawRunway(hDC, &WayPointList[idx], rc, 4000, true);

                    if (WayPointCalc[idx].IsAirport) {
                        // remove spaces from frequency
                        for (j = 1; j < (CUPSIZE_FREQ); j++)
                            if (WayPointList[idx].Freq[CUPSIZE_FREQ - j] == ' ')
                                WayPointList[idx].Freq[CUPSIZE_FREQ - j] = '\0';

                        if (_tcslen(WayPointList[idx].Freq) > 2)
                            _stprintf(text1, TEXT("%s %s MHz"), WayPointList[idx].Name, WayPointList[idx].Freq);
                        else
                            _stprintf(text1, TEXT("%s"), WayPointList[idx].Name);
                    } else {
                        if (WayPointList[idx].Comment != NULL)
                            _stprintf(text1, TEXT("%s %s"), WayPointList[idx].Name, Comment);
                        else
                            _stprintf(text1, TEXT("%s"), WayPointList[idx].Name);
                    }

                    if ((WayPointList[idx].RunwayLen >= 10) || (WayPointList[idx].RunwayDir > 0)) {
                        _stprintf(text2, TEXT("%3.1f%s (%i%s  %02i/%02i  %i%s)"), Distance*DISTANCEMODIFY
                                  , Units::GetDistanceName(), (int) (WayPointList[idx].Altitude * ALTITUDEMODIFY)
                                  , Units::GetAltitudeName()
                                  , (int) (WayPointList[idx].RunwayDir / 10.0 + 0.5)
                                  , (int) (AngleLimit360(WayPointList[idx].RunwayDir + 180.0) / 10.0 + 0.5)
                                  , (int) ((double) WayPointList[idx].RunwayLen * ALTITUDEMODIFY)
                                  , Units::GetAltitudeName());
                    } else {
                        _stprintf(text2, TEXT("%3.1f%s (%i%s) "), Distance*DISTANCEMODIFY, Units::GetDistanceName()
                                  , (int) (WayPointList[idx].Altitude * ALTITUDEMODIFY)
                                  , Units::GetAltitudeName());
                    }

                }// waypoint isLandable
                else {
                    MapWindow::DrawWaypointPicto(hDC, rc, &WayPointList[idx]);
                    _stprintf(text1, TEXT("%s %s"), WayPointList[idx].Name, Comment);

                    _stprintf(text2, TEXT("%3.1f%s (%i%s)"), Distance*DISTANCEMODIFY
                              , Units::GetDistanceName()
                              , (int) (WayPointList[idx].Altitude * ALTITUDEMODIFY)
                              , Units::GetAltitudeName());
                }

            }// Elements IM_TASK
            else {
                LockTaskData(); // protect from external task changes
                int iTaskIdx = Elements[i].iIdx;
                MapWindow::DrawTaskPicto(hDC, iTaskIdx, rc, 3000);
                int iLastTaskPoint = 0;

                while (ValidTaskPoint(iLastTaskPoint))
                    iLastTaskPoint++;

                iLastTaskPoint--;

                if (iTaskIdx == 0) {
                    // _@M2301_  "S"    # S = Start Task point
                    _stprintf(text1, TEXT("%s: (%s)"), gettext(_T("_@M2301_")), WayPointList[idx].Name);
                    _stprintf(text2, TEXT("Radius %3.1f%s (%i%s)"), StartRadius*DISTANCEMODIFY
                              , Units::GetDistanceName()
                              , (int) (WayPointList[idx].Altitude * ALTITUDEMODIFY)
                              , Units::GetAltitudeName());
                } else {
                    if (iTaskIdx == iLastTaskPoint) {
                        //	_@M2303_  "F"                 // max 30         30 => max 60 char
                        _stprintf(text1, TEXT("%s: (%s) "), gettext(_T("_@M2303_")), WayPointList[idx].Name);
                        _stprintf(text2, TEXT("Radius %3.1f%s (%i%s)"), FinishRadius*DISTANCEMODIFY
                                  , Units::GetDistanceName()
                                  , (int) (WayPointList[idx].Altitude * ALTITUDEMODIFY)
                                  , Units::GetAltitudeName());
                    } else {
                        //   _@M2302_  "T"    # F = Finish point            // max 30         30 => max 60 char
                        _stprintf(text1, TEXT("%s%i: (%s) "), gettext(_T("_@M2302_")), iTaskIdx, WayPointList[idx].Name);
                        double SecRadius = 0;

                        SecRadius = SectorRadius;
                        if (AATEnabled) {
                            if (Task[iTaskIdx].AATType == SECTOR)
                                SecRadius = Task[iTaskIdx].AATSectorRadius;
                            else
                                SecRadius = Task[iTaskIdx].AATCircleRadius;
                        }

                        _stprintf(text2, TEXT("Radius %3.1f%s (%i%s)"), SecRadius*DISTANCEMODIFY
                                  , Units::GetDistanceName()
                                  , (int) (WayPointList[idx].Altitude * ALTITUDEMODIFY)
                                  , Units::GetAltitudeName());
                    }
                }

                UnlockTaskData(); // protect from external task changes
            }
            break;
        }

        /********************
         * show text
         ********************/
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB_BLACK);
        int iLen = _tcslen(text1);
        if (iLen > 100)
            iLen = 100;
        ExtTextOut(hDC, (int) (PICTO_WIDTH * 1.1) * ScreenScale, 2 * ScreenScale, ETO_OPAQUE, NULL, text1, iLen, NULL);
        SetTextColor(hDC, RGB_DARKBLUE);
        iLen = _tcslen(text2);
        if (iLen > 100)
            iLen = 100;
        ExtTextOut(hDC, (int) (PICTO_WIDTH * 1.1) * ScreenScale, 15 * ScreenScale, ETO_OPAQUE, NULL, text2, iLen, NULL);

    }
}

static void OnMultiSelectListListEnter(WindowControl * Sender,
                                       WndListFrame::ListInfo_t *ListInfo) {
    (void) Sender;

    ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
    if (ItemIndex >= iNO_ELEMENTS) {
        ItemIndex = iNO_ELEMENTS - 1;
    }
    if (ItemIndex >= 0) {
        wf->SetModalResult(mrOK);
    }


    if ((ItemIndex >= 0) && (ItemIndex < iNO_ELEMENTS)) {
        dlgAddMultiSelectListDetailsDialog(ItemIndex);
    }

}

static void OnEnterClicked(WindowControl * Sender) {
    (void) Sender;

    if (ItemIndex >= iNO_ELEMENTS) {
        ItemIndex = iNO_ELEMENTS - 1;
    }
    if (ItemIndex >= 0) {
        wf->SetModalResult(mrOK);
    }

    if ((ItemIndex >= 0) && (ItemIndex < iNO_ELEMENTS)) {
        dlgAddMultiSelectListDetailsDialog(ItemIndex);
    }

}

static void OnMultiSelectListListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {

    (void) Sender;
    if (ListInfo->DrawIndex == -1) {
        ListInfo->ItemCount = iNO_ELEMENTS;
    } else {
        DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
        ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
    }

}

static void OnCloseClicked(WindowControl * Sender) {
    (void) Sender;
    ItemIndex = -1;

    wf->SetModalResult(mrCancle);

}




static CallBackTableEntry_t CallBackTable[] = {
    DeclareCallBackEntry(OnMultiSelectListPaintListItem),
    DeclareCallBackEntry(OnMultiSelectListListInfo),
    DeclareCallBackEntry(OnCloseClicked),
    DeclareCallBackEntry(OnUpClicked),
    DeclareCallBackEntry(OnEnterClicked),
    DeclareCallBackEntry(OnDownClicked),
    DeclareCallBackEntry(NULL)
};

ListElement* dlgMultiSelectListShowModal(void) {

    ItemIndex = -1;

    if (iNO_ELEMENTS == 0) {
        if (EnableSoundModes)
            PlayResource(TEXT("IDR_WAV_MM1"));

        return NULL;
    }

    if (EnableSoundModes)
        PlayResource(TEXT("IDR_WAV_MM4"));

    if (!ScreenLandscape) {
        char filename[MAX_PATH];
        LocalPathS(filename, TEXT("dlgMultiSelectList_L.xml"));
        wf = dlgLoadFromXML(CallBackTable,
                            filename,
                            hWndMainWindow,
                            TEXT("IDR_XML_MULTISELECTLIST_L"));
    } else {
        char filename[MAX_PATH];
        LocalPathS(filename, TEXT("dlgMultiSelectList.xml"));
        wf = dlgLoadFromXML(CallBackTable,
                            filename,
                            hWndMainWindow,
                            TEXT("IDR_XML_MULTISELECTLIST"));
    }

    if (!wf) return NULL;

    wf->SetTimerNotify(OnTimer);
    wMultiSelectListList = (WndListFrame*) wf->FindByName(TEXT("frmMultiSelectListList"));
    LKASSERT(wMultiSelectListList != NULL);
    wMultiSelectListList->SetBorderKind(BORDERLEFT);
    wMultiSelectListList->SetEnterCallback(OnMultiSelectListListEnter);
    wMultiSelectListList->SetWidth(wf->GetWidth() - wMultiSelectListList->GetLeft() - 2);

    wMultiSelectListListEntry = (WndOwnerDrawFrame*) wf->FindByName(TEXT("frmMultiSelectListListEntry"));
    LKASSERT(wMultiSelectListListEntry != NULL);
    wMultiSelectListListEntry->SetCanFocus(true);

    // ScrollbarWidth is initialised from DrawScrollBar in WindowControls, so it might not be ready here
    if (wMultiSelectListList->ScrollbarWidth == -1) {
#if defined (PNA)
#define SHRINKSBFACTOR 1.0 // shrink width factor.  Range .1 to 1 where 1 is very "fat"
#else
#define SHRINKSBFACTOR 0.75  // shrink width factor.  Range .1 to 1 where 1 is very "fat"
#endif
        wMultiSelectListList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale * SHRINKSBFACTOR);
    }

    wMultiSelectListListEntry->SetWidth(wMultiSelectListList->GetWidth() - wMultiSelectListList->ScrollbarWidth - 5);

    UpdateList();

    wf->ShowModal();

    delete wf;

    wf = NULL;

    iNO_ELEMENTS = 0;

    NoAirfields = 0;
    NoOutlands = 0;
    NoWaypoints = 0;
    NoAirspace = 0;
    NoTaskPoints = 0;

    return pResult;
}
