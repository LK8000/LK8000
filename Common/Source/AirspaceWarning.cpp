/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: AirspaceWarning.cpp,v 8.3 2010/12/10 22:22:47 root Exp root $
*/

#include "StdAfx.h"
#include "Calculations.h"
#include "externs.h"
#include "Dialogs.h"
#include "Utils.h"
#ifdef LKAIRSPACE
#include "LKAirspace.h"
#undef max
#undef min
#include <deque>
#else
#include "Airspace.h"
#endif
#include "device.h"


#ifdef HAVEEXCEPTIONS
#define  __try     __try
#define  __finally __finally
#else
#define  __try
#define  __finally
#endif

#ifndef LKAIRSPACE
#include "simpleList.h"
#endif

#include "utils/heapcheck.h"

static bool NewAirspaceWarnings = false;
#ifdef LKAIRSPACE
#else
static CRITICAL_SECTION  csAirspaceWarnings;
static bool InitDone = false;
#endif
#define OUTSIDE_CHECK_INTERVAL 4

extern int AcknowledgementTime;

#ifdef LKAIRSPACE
#else
List<AirspaceWarningNotifier_t> AirspaceWarningNotifierList;
List<AirspaceInfo_c> AirspaceWarnings;

static void LockList(void){
  EnterCriticalSection(&csAirspaceWarnings);
}

static void UnLockList(void){
  LeaveCriticalSection(&csAirspaceWarnings);
}
#endif

#ifdef LKAIRSPACE
bool CAirspaceManager::UpdateAirspaceAckBrush(AirspaceInfo_c *Item, int Force)
{
  bool res=false;

  CCriticalSection::CGuard guard(_csairspaces);
  if (Force == 0) {
	if (Item->Airspace) {
	  res = Item->Airspace->NewWarnAckNoBrush();
	  Item->Airspace->NewWarnAckNoBrush(((Item->WarnLevel > 0) && (Item->WarnLevel <= Item->Acknowledge))
			|| (Item->Acknowledge==4));
	} else {
	  res = false;
	}
  } else {
	if (Item->Airspace) {
	  res = Item->Airspace->NewWarnAckNoBrush();
	  Item->Airspace->NewWarnAckNoBrush((Force == 1));		//Always 1 here...
	} else {
	  res = false;
	}
  }
  return(res);
}
#else
static bool UpdateAirspaceAckBrush(AirspaceInfo_c *Item, int Force){
  bool res=false;

  if (Force == 0){
    if (Item->IsCircle){
      if (AirspaceCircle) {
        res = AirspaceCircle[Item->AirspaceIndex]._NewWarnAckNoBrush;
        AirspaceCircle[Item->AirspaceIndex]._NewWarnAckNoBrush =
          ((Item->WarnLevel > 0) && (Item->WarnLevel <= Item->Acknowledge))
          || (Item->Acknowledge==4);
      } else {
        res = false;
      }
    } else {
      if (AirspaceArea) {
        res = AirspaceArea[Item->AirspaceIndex]._NewWarnAckNoBrush;
        AirspaceArea[Item->AirspaceIndex]._NewWarnAckNoBrush =
          ((Item->WarnLevel > 0) && (Item->WarnLevel <= Item->Acknowledge))
          || (Item->Acknowledge==4);
      } else {
        res = false;
      }
    }
  } else {
    if (Item->IsCircle){
      if (AirspaceCircle) {
        res = AirspaceCircle[Item->AirspaceIndex]._NewWarnAckNoBrush;
        AirspaceCircle[Item->AirspaceIndex]._NewWarnAckNoBrush = (Force == 1);
      } else {
        res = false;
      }
    } else {
      if (AirspaceArea) {
        res = AirspaceArea[Item->AirspaceIndex]._NewWarnAckNoBrush;
        AirspaceArea[Item->AirspaceIndex]._NewWarnAckNoBrush = (Force == 1);
      } else {
        res = false;
      }
    }
  }
  return(res);
}
#endif
#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListAddNotifier(AirspaceWarningNotifier_t Notifier)
{
  CCriticalSection::CGuard guard(_cswarnlist);
  _airspaceWarningNotifiers.push_front(Notifier);
}
#else
void AirspaceWarnListAddNotifier(AirspaceWarningNotifier_t Notifier){
  AirspaceWarningNotifierList.push_front(Notifier);
}
#endif
#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListRemoveNotifier(AirspaceWarningNotifier_t Notifier)
{
  CCriticalSection::CGuard guard(_cswarnlist);

  AirspaceWarningNotifiersList::iterator it = _airspaceWarningNotifiers.begin();
  while (it != _airspaceWarningNotifiers.end()) {
    if (*it == Notifier) {
      it = _airspaceWarningNotifiers.erase(it);
      continue;
    }
    ++it;
  }
}
#else
void AirspaceWarnListRemoveNotifier(AirspaceWarningNotifier_t Notifier){
  List<AirspaceWarningNotifier_t>::Node* it = AirspaceWarningNotifierList.begin();
  while(it){
    if (it->data == Notifier){
      it = AirspaceWarningNotifierList.erase(it);
      continue;
    }
    it = it->next;
  }
}
#endif
#ifdef LKAIRSPACE
bool CAirspaceManager::AirspaceWarnGetItem(unsigned int Index, AirspaceInfo_c &Item)
{
  CCriticalSection::CGuard guard(_cswarnlist);
 
  if (Index>=_airspaceWarnings.size()) return false;
  Item = _airspaceWarnings[Index];
  return true;
}
#else
bool AirspaceWarnGetItem(int Index, AirspaceInfo_c &Item){
  bool res = false;

  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); it; it = it->next, Index-- ){
      if (Index == 0){
	Item = it->data;  // make a copy!
	res = true;
	break;
      }
    }
  }__finally {
    UnLockList();
  }
  return(res);
}
#endif

#ifdef LKAIRSPACE
int CAirspaceManager::AirspaceWarnGetItemCount()
{
  CCriticalSection::CGuard guard(_cswarnlist);
  return _airspaceWarnings.size();
}
#else
int AirspaceWarnGetItemCount(void){
  int res=0;
  if (!InitDone) {
    return res;
  }
  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); it; it = it->next ){
      res++;
    }
  }__finally {
    UnLockList();
  }
  return(res);
}
#endif

#ifndef LKAIRSPACE
double RangeAirspaceCircle(const double &longitude, const double &latitude, int i);
double RangeAirspaceArea(const double &longitude, const double &latitude, int i, double *bearing);
#endif

#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListDoNotify(AirspaceWarningNotifyAction_t Action, AirspaceInfo_c *AirSpace)
{
  for (AirspaceWarningNotifiersList::iterator it = _airspaceWarningNotifiers.begin(); it != _airspaceWarningNotifiers.end(); ++it) {
    if (*it) (*it)(Action, AirSpace);
  }
}
#else
static void AirspaceWarnListDoNotify(AirspaceWarningNotifyAction_t Action, AirspaceInfo_c *AirSpace){
  for (List<AirspaceWarningNotifier_t>::Node* it = AirspaceWarningNotifierList.begin(); it; it = it->next ){
    (it->data)(Action, AirSpace);
  }
}
#endif

#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListCalcDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const CAirspace *airspace, int *hDistance, int *Bearing, int *vDistance)
{

  int vDistanceBase;
  int vDistanceTop;
  int alt;
  int agl;
  double fbearing;
  double distance;

  if (Basic->BaroAltitudeAvailable) {
    alt = (int)Basic->BaroAltitude;
  } else {
    alt = (int)Basic->Altitude;
  }
  agl = (int)Calculated->AltitudeAGL;
  if (1) {
	CCriticalSection::CGuard guard(_csairspaces);
	distance = airspace->Range(Basic->Longitude, Basic->Latitude, fbearing);
	if (distance < 0) distance = 0;
	if (airspace->Base()->Base != abAGL) {
		vDistanceBase = alt - (int)(airspace->Base()->Altitude);
	} else {
		vDistanceBase = agl - (int)(airspace->Base()->AGL);
	}
	if (airspace->Top()->Base != abAGL) {
		vDistanceTop  = alt - (int)(airspace->Top()->Altitude);
	} else {
		vDistanceTop  = agl - (int)(airspace->Top()->AGL);
	}
  }

  // EntryTime = ToDo
  if (Bearing) *Bearing = (int)fbearing;
  if (hDistance) *hDistance = (int)distance;
  if (vDistance) {
	if (vDistanceBase > 0 && vDistanceTop < 0) {
	  *vDistance  = 0;
	}
	else if (-vDistanceBase > vDistanceTop)
	  *vDistance = vDistanceBase;
	else
	  *vDistance = vDistanceTop;
  }
}
#else
static void AirspaceWarnListCalcDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated, bool IsCircle, int AsIdx, int *hDistance, int *Bearing, int *vDistance){

  int vDistanceBase;
  int vDistanceTop;
  int alt;
  int agl;

  if (Basic->BaroAltitudeAvailable) {
    alt = (int)Basic->BaroAltitude;
  } else {
    alt = (int)Basic->Altitude;
  }
  agl = (int)Calculated->AltitudeAGL;

  if (IsCircle){
    *hDistance = (int)RangeAirspaceCircle(Basic->Longitude, 
                                          Basic->Latitude, 
                                          AsIdx);
    if (*hDistance < 0)
      *hDistance = 0;
    if (AirspaceCircle[AsIdx].Base.Base != abAGL) {
      vDistanceBase = alt - (int)AirspaceCircle[AsIdx].Base.Altitude;
    } else {
      vDistanceBase = agl - (int)AirspaceCircle[AsIdx].Base.AGL;
    }
    if (AirspaceCircle[AsIdx].Top.Base != abAGL) {
      vDistanceTop  = alt - (int)AirspaceCircle[AsIdx].Top.Altitude;
    } else {
      vDistanceTop  = agl - (int)AirspaceCircle[AsIdx].Top.AGL;
    }
    // EntryTime = ToDo
  } else {
    if (!InsideAirspaceArea(Basic->Longitude, Basic->Latitude, AsIdx)){
      // WARNING: RangeAirspaceArea dont return negative values if
      // inside aera -> but RangeAirspaceCircle does!
      double fBearing;
      *hDistance = (int)RangeAirspaceArea(Basic->Longitude, Basic->Latitude, 
                                          AsIdx, &fBearing);
      *Bearing = (int)fBearing;
    } else {
      *hDistance = 0;
    }
    if (AirspaceArea[AsIdx].Base.Base != abAGL) {
      vDistanceBase = alt - (int)AirspaceArea[AsIdx].Base.Altitude;
    } else {
      vDistanceBase = agl - (int)AirspaceArea[AsIdx].Base.AGL;
    }
    if (AirspaceArea[AsIdx].Top.Base != abAGL) {
      vDistanceTop  = alt - (int)AirspaceArea[AsIdx].Top.Altitude;
    } else {
      vDistanceTop  = agl - (int)AirspaceArea[AsIdx].Top.AGL;
    }
    // EntryTime = ToDo
  }

  if (vDistanceBase > 0 && vDistanceTop < 0) {
    *vDistance  = 0;
  }
  else if (-vDistanceBase > vDistanceTop)
    *vDistance = vDistanceBase;
  else
    *vDistance = vDistanceTop;
}
#endif
#ifdef LKAIRSPACE
bool CAirspaceManager::calcWarnLevel(AirspaceInfo_c *asi)
{
  int LastWarnLevel;

  if (asi == NULL)
    return(false);

  int dh = asi->hDistance;
  int dv = abs(asi->vDistance);

  LastWarnLevel = asi->WarnLevel;

  if (!asi->Inside && asi->Predicted){
    if ((dh < 500) && (dv < 100)) { // JMW hardwired! 
      asi->WarnLevel = 2;
    } else
      asi->WarnLevel = 1;
  } else
  if (asi->Inside){
    asi->WarnLevel = 3;
  }

  asi->SortKey = dh + dv*20;

  UpdateAirspaceAckBrush(asi, 0);

  return((asi->WarnLevel > asi->Acknowledge) 
	 && (asi->WarnLevel > LastWarnLevel));

}
#else
static bool calcWarnLevel(AirspaceInfo_c *asi){

  int LastWarnLevel;

  if (asi == NULL)
    return(false);

  int dh = asi->hDistance;
  int dv = abs(asi->vDistance);

  LastWarnLevel = asi->WarnLevel;

  if (!asi->Inside && asi->Predicted){
    if ((dh < 500) && (dv < 100)) { // JMW hardwired! 
      asi->WarnLevel = 2;
    } else
      asi->WarnLevel = 1;
  } else
  if (asi->Inside){
    asi->WarnLevel = 3;
  }

  asi->SortKey = dh + dv*20;

  UpdateAirspaceAckBrush(asi, 0);

  return((asi->WarnLevel > asi->Acknowledge) 
	 && (asi->WarnLevel > LastWarnLevel));

}
#endif

#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListAdd(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                         bool Predicted, CAirspace *airspace,
                         bool ackDay)
{
  static int  Sequence = 0;

  if (!Predicted){
    if (++Sequence > 1000)
      Sequence = 1;
  }

  int   hDistance = 0;
  int   vDistance = 0;
  int   Bearing = 0;
  DWORD EntryTime = 0;
  bool  FoundInList = false;

  if (Predicted){  // ToDo calculate predicted data
	AirspaceWarnListCalcDistance(Basic, Calculated, airspace, &hDistance, 
				 &Bearing, &vDistance);
  }
  
  CCriticalSection::CGuard guard(_cswarnlist);
    for (AirspaceWarningsList::iterator it = _airspaceWarnings.begin(); it != _airspaceWarnings.end(); ++it) {
      if ( it->Airspace == airspace ) { // check if already in list
        if ((it->Sequence == Sequence) && (it->Sequence>=0) && !ackDay) {
			// still updated in real pos calculation
			// JMW: need to be able to override if ack day
        } else {
          it->Sequence = Sequence;
          it->TimeOut = 3;
          it->hDistance = hDistance;
          it->vDistance = vDistance;
          it->Bearing = Bearing;
          it->PredictedEntryTime = EntryTime;
          if (ackDay) {
			if (!Predicted) { 
			  it->Acknowledge = 4;
			} else {
			  // code for cancel daily ack
			  if (it->Acknowledge == 4) {
				it->Acknowledge = 0;
			  }
			}
            it->Inside = 0;
            it->Predicted = 0;
          } else {
			if (!Predicted) {
			  it->Inside = true;
			}
			it->Predicted = Predicted;
		  }
          if (calcWarnLevel(&(*it)))
            AirspaceWarnListDoNotify(asaWarnLevelIncreased, &(*it));
          else
            AirspaceWarnListDoNotify(asaItemChanged, &(*it));
        }
        
        it->InsideAckTimeOut = AcknowledgementTime / OUTSIDE_CHECK_INTERVAL;
        it->TimeOut = OUTSIDE_CHECK_INTERVAL;

        FoundInList = true;
        break;

      }
    }

    if (!FoundInList && !(Predicted && ackDay)) {
      // JMW Predicted & ackDay means cancel a daily ack
      AirspaceInfo_c asi; // not in list, add new
      asi.TimeOut = OUTSIDE_CHECK_INTERVAL;
      asi.InsideAckTimeOut = AcknowledgementTime / OUTSIDE_CHECK_INTERVAL;
      asi.Sequence = Sequence;

      asi.hDistance = hDistance;
      asi.vDistance = vDistance;
      asi.Bearing = Bearing;
      asi.PredictedEntryTime = EntryTime;  // ETE, ToDo

      if (ackDay) {
        asi.Acknowledge = 4;
        asi.Inside = 0;
        asi.Predicted = 0;
      } else {
        asi.Acknowledge = 0;
        asi.Inside = !Predicted;
        asi.Predicted = Predicted;
      }
      asi.Airspace = airspace;
      asi.SortKey = 0;
      asi.LastListIndex = 0; // JMW initialise
      asi.WarnLevel = 0; // JMW initialise

      calcWarnLevel(&asi);

      asi.ID = _static_unique++;			//TODO remove this hack

      _airspaceWarnings.push_front(asi);
      UpdateAirspaceAckBrush(&asi, 0);

      NewAirspaceWarnings = true;

      AirspaceWarnListDoNotify(asaItemAdded, &(*_airspaceWarnings.begin()));
    }
}
#else
void AirspaceWarnListAdd(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                         bool Predicted, bool IsCircle, int AsIdx,
                         bool ackDay){
  static int  Sequence = 0;

  if (!Predicted){
    if (++Sequence > 1000)
      Sequence = 1;
  }

  int   hDistance = 0;
  int   vDistance = 0;
  int   Bearing = 0;
  DWORD EntryTime = 0;
  bool  FoundInList = false;

  if (Predicted){  // ToDo calculate predicted data
    AirspaceWarnListCalcDistance(Basic, Calculated, IsCircle, AsIdx, &hDistance, 
				 &Bearing, &vDistance);
  }
  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); 
         it; it = it->next ){
      if ((it->data.IsCircle == IsCircle)
           && (it->data.AirspaceIndex == AsIdx)){ // check if already in list

        if ((it->data.Sequence == Sequence) && (it->data.Sequence>=0)
	    && !ackDay){
          // still updated in real pos calculation
	  // JMW: need to be able to override if ack day
        } else {

          it->data.Sequence = Sequence;
          it->data.TimeOut = 3;
          it->data.hDistance = hDistance;
          it->data.vDistance = vDistance;
          it->data.Bearing = Bearing;
          it->data.PredictedEntryTime = EntryTime;


          if (ackDay) {
	    if (!Predicted) { 
	      it->data.Acknowledge = 4;
	    } else {
	      // code for cancel daily ack
	      if (it->data.Acknowledge == 4) {
		it->data.Acknowledge = 0;
	      }
	    }
            it->data.Inside = 0;
            it->data.Predicted = 0;
          } else {
	    if (!Predicted) {
	      it->data.Inside = true;
	    }
	    it->data.Predicted = Predicted;
          }

          if (calcWarnLevel(&it->data))
            AirspaceWarnListDoNotify(asaWarnLevelIncreased, &it->data);
          else
            AirspaceWarnListDoNotify(asaItemChanged, &it->data);

        }
        
        it->data.InsideAckTimeOut = AcknowledgementTime / OUTSIDE_CHECK_INTERVAL;
        it->data.TimeOut = OUTSIDE_CHECK_INTERVAL;

        FoundInList = true;
        break;

      }
    }

    if (!FoundInList && !(Predicted && ackDay)) {
      // JMW Predicted & ackDay means cancel a daily ack

      AirspaceInfo_c asi; // not in list, add new

      asi.TimeOut = OUTSIDE_CHECK_INTERVAL;

      asi.InsideAckTimeOut = AcknowledgementTime / OUTSIDE_CHECK_INTERVAL;

      asi.Sequence = Sequence;

      asi.hDistance = hDistance;
      asi.vDistance = vDistance;
      asi.Bearing = Bearing;
      asi.PredictedEntryTime = EntryTime;  // ETE, ToDo

      if (ackDay) {
        asi.Acknowledge = 4;
        asi.Inside = 0;
        asi.Predicted = 0;
      } else {
        asi.Acknowledge = 0;
        asi.Inside = !Predicted;
        asi.Predicted = Predicted;
      }
      asi.IsCircle = IsCircle;
      asi.AirspaceIndex = AsIdx;
      asi.SortKey = 0;
      asi.LastListIndex = 0; // JMW initialise
      asi.WarnLevel = 0; // JMW initialise

      calcWarnLevel(&asi);

      asi.ID = AsIdx + (IsCircle ? 10000 : 0);

      AirspaceWarnings.push_front(asi);
      UpdateAirspaceAckBrush(&asi, 0);

      NewAirspaceWarnings = true;

      AirspaceWarnListDoNotify(asaItemAdded, &AirspaceWarnings.begin()->data);
    }
  }__finally {
    UnLockList();
  }
  
}
#endif

static int _cdecl cmp(const void *a, const void *b){

  int adAL, bdAL;

  adAL = ((AirspaceInfo_c *)a)->WarnLevel - ((AirspaceInfo_c *)a)->Acknowledge;
  bdAL = ((AirspaceInfo_c *)b)->WarnLevel - ((AirspaceInfo_c *)b)->Acknowledge;

  if (adAL < bdAL)
    return(1);
  if (adAL > bdAL)
    return(-1);
  if (((AirspaceInfo_c *)a)->LastListIndex > ((AirspaceInfo_c *)b)->LastListIndex)
    return(1);
  return(-1);

}

#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListSort()
{

  unsigned int i, idx=0;
  AirspaceInfo_c l[20];
  AirspaceWarningsList::iterator it;

  for (it = _airspaceWarnings.begin(); it != _airspaceWarnings.end(); ++it) {
    it->LastListIndex = idx;
    memcpy(&l[idx], &(*it), sizeof(AirspaceInfo_c));
    idx++;
    if (idx >= (sizeof(l)/(sizeof(l[0]))))
      return;
  }

  if (idx < 1)
    return;

  qsort(&l[0], idx, sizeof(l[0]), cmp);

  it = _airspaceWarnings.begin();

  for (i=0; i<idx; i++){
    memcpy(&(*it), &l[i], sizeof(AirspaceInfo_c));
    ++it;
  }

}
#else
void AirspaceWarnListSort(void){

  unsigned int i, idx=0;
  AirspaceInfo_c l[20];
  List<AirspaceInfo_c>::Node* it;

  for (it = AirspaceWarnings.begin(); it; it = it->next){
    it->data.LastListIndex = idx;
    l[idx] = it->data;
    idx++;
    if (idx >= (sizeof(l)/(sizeof(l[0]))))
      return;
  }

  if (idx < 1)
    return;

  qsort(&l[0], idx, sizeof(l[0]), cmp);

  it = AirspaceWarnings.begin();

  for (i=0; i<idx; i++){
    it->data = l[i];
    it = it->next;
  }

}
#endif

// This is called at the end of slow calculation AirspaceWarning function, after tables have been filled up
#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListProcess(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  CCriticalSection::CGuard guard(_cswarnlist);

  for (AirspaceWarningsList::iterator it = _airspaceWarnings.begin(); it != _airspaceWarnings.end(); ) {
      
      it->TimeOut--;                // age the entry
      
      if (it->TimeOut < 0){
	
        it->Predicted = false;
        it->Inside = false;
        it->WarnLevel = 0;

        int hDistance = 0;
        int vDistance = 0;
        int Bearing = 0;

        AirspaceWarnListCalcDistance(Basic, Calculated,
				     it->Airspace, 
				     &hDistance, &Bearing, &vDistance);

        it->hDistance = hDistance; // for all: update data
        it->vDistance = vDistance;
        it->Bearing = Bearing;
        it->TimeOut = OUTSIDE_CHECK_INTERVAL; // retrigger checktimeout

        if (calcWarnLevel(&(*it)))
          AirspaceWarnListDoNotify(asaWarnLevelIncreased, &(*it));

        UpdateAirspaceAckBrush(&(*it), 0);

        if (it->Acknowledge == 4){ // whole day achnowledged
          if (it->SortKey > 25000) {
            it->TimeOut = 60; // JMW hardwired why?
          }
          continue;
        }

        if ((hDistance > 2500) || (abs(vDistance) > 250)){  
	  // far away remove from warning list
          AirspaceInfo_c asi;
		  memcpy(&asi, &(*it), sizeof(AirspaceInfo_c));

          it = _airspaceWarnings.erase(it);
          UpdateAirspaceAckBrush(&asi, -1);
          AirspaceWarnListDoNotify(asaItemRemoved, &asi);

          continue;

        } else {
          if ((hDistance > 500) || (abs(vDistance) > 100)){   
	    // close clear inside ack and auto ACK til closer
            if (it->Acknowledge > 2)
              if (--(it->InsideAckTimeOut) < 0){      // timeout the
                it->Acknowledge = 1;
              }

          } else { // very close, just update ack timer
            it->InsideAckTimeOut = AcknowledgementTime / OUTSIDE_CHECK_INTERVAL;
	    // 20sec outside check interval prevent down ACK on circling
          }
        }
        AirspaceWarnListDoNotify(asaItemChanged, &(*it));
      }
      ++it;
    }
    AirspaceWarnListSort();

#ifdef DEBUG_AIRSPACE
    DebugStore("%d # airspace\n", _airspaceWarnings.size());
#endif

    AirspaceWarnListDoNotify(asaProcessEnd, NULL);
}
#else
void AirspaceWarnListProcess(NMEA_INFO *Basic, DERIVED_INFO *Calculated){

  if (!InitDone) return;

  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); it;){
      
      it->data.TimeOut--;                // age the entry
      
      if (it->data.TimeOut < 0){
	
        it->data.Predicted = false;
        it->data.Inside = false;
        it->data.WarnLevel = 0;

        int hDistance = 0;
        int vDistance = 0;
        int Bearing = 0;

        AirspaceWarnListCalcDistance(Basic, Calculated,
				     it->data.IsCircle, 
				     it->data.AirspaceIndex, 
				     &hDistance, &Bearing, &vDistance);

        it->data.hDistance = hDistance; // for all: update data
        it->data.vDistance = vDistance;
        it->data.Bearing = Bearing;
        it->data.TimeOut = OUTSIDE_CHECK_INTERVAL; // retrigger checktimeout


        if (calcWarnLevel(&it->data))
          AirspaceWarnListDoNotify(asaWarnLevelIncreased, &it->data);

        UpdateAirspaceAckBrush(&it->data, 0);

        if (it->data.Acknowledge == 4){ // whole day achnowledged
          if (it->data.SortKey > 25000) {
            it->data.TimeOut = 60; // JMW hardwired why?
          }
          continue;
        }

        if ((hDistance > 2500) || (abs(vDistance) > 250)){  
	  // far away remove from warning list
          AirspaceInfo_c asi = it->data;

          it = AirspaceWarnings.erase(it);
          UpdateAirspaceAckBrush(&asi, -1);
          AirspaceWarnListDoNotify(asaItemRemoved, &asi);

          continue;

        } else {
          if ((hDistance > 500) || (abs(vDistance) > 100)){   
	    // close clear inside ack and auto ACK til closer
            if (it->data.Acknowledge > 2)
              if (--(it->data.InsideAckTimeOut) < 0){      // timeout the
                it->data.Acknowledge = 1;
              }

          } else { // very close, just update ack timer
            it->data.InsideAckTimeOut = AcknowledgementTime / OUTSIDE_CHECK_INTERVAL;
	    // 20sec outside check interval prevent down ACK on circling
          }
        }

        AirspaceWarnListDoNotify(asaItemChanged, &it->data);

      }

      // SortTheList();

      /*

      // just for debugging

      TCHAR szMessageBuffer[1024];

      if (it->data.IsCircle){

        int i = it->data.AirspaceIndex;

        wsprintf(szMessageBuffer, TEXT("%20s %d %d %5d %5d"), AirspaceCircle[i].Name, it->data.Inside, it->data.Predicted, (int)it->data.hDistance, (int)it->data.vDistance);

      } else {

        int i = it->data.AirspaceIndex;

        wsprintf(szMessageBuffer, TEXT("%20s %d %d %5d %5d"), AirspaceArea[i].Name, it->data.Inside, it->data.Predicted, (int)it->data.hDistance, (int)it->data.vDistance);

      }

      devPutVoice(NULL, szMessageBuffer);
      */

      it = it->next;

    }


    AirspaceWarnListSort();

#ifdef DEBUG_AIRSPACE
    DebugStore("%d # airspace\n", AirspaceWarnGetItemCount());
#endif

    AirspaceWarnListDoNotify(asaProcessEnd, NULL);

  }__finally {
    UnLockList();
  }

}
#endif

#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnDoAck(int ID, int Ack)
{
  CCriticalSection::CGuard guard(_cswarnlist);
  for (AirspaceWarningsList::iterator it = _airspaceWarnings.begin(); it != _airspaceWarnings.end(); ++it ) {
	if (it->ID == ID) {
	  if (Ack < it->Acknowledge) it->TimeOut=0;  		// force data refresh on down ack
	  if (Ack==-1) it->Acknowledge = it->WarnLevel;		// ack current warnlevel
	  else it->Acknowledge = Ack;						// ack defined warnlevel

	  UpdateAirspaceAckBrush(&(*it), 0);
	  AirspaceWarnListDoNotify(asaItemChanged, &(*it));
	}
  }
}
#else
void AirspaceWarnDoAck(int ID, int Ack){
  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); it; it = it->next ){
      if (it->data.ID == ID){

        if (Ack < it->data.Acknowledge)   // force data refresh on down ack
          it->data.TimeOut=0;

        if (Ack==-1)                      // ack current warnlevel
          it->data.Acknowledge = it->data.WarnLevel;
        else                              // ack defined warnlevel
          it->data.Acknowledge = Ack;

        UpdateAirspaceAckBrush(&it->data, 0);

        AirspaceWarnListDoNotify(asaItemChanged, &it->data);

      }
    }
  }__finally{
    UnLockList();
  }
}
#endif

#ifdef LKAIRSPACE
void CAirspaceManager::AirspaceWarnListClear()
{
  CCriticalSection::CGuard guard(_cswarnlist);
  for (AirspaceWarningsList::iterator it = _airspaceWarnings.begin(); it != _airspaceWarnings.end(); ++it ) {
	UpdateAirspaceAckBrush(&(*it), -1);
  }
  _airspaceWarnings.clear();
  AirspaceWarnListDoNotify(asaClearAll ,NULL);
}
#else
void AirspaceWarnListClear(void){

  if (!InitDone)     // called by airspace parser during init, prevent
                     // working on unitialized data
    return;

  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); it; it = it->next ){
      UpdateAirspaceAckBrush(&it->data, -1);
    }
    AirspaceWarnings.clear();
    AirspaceWarnListDoNotify(asaClearAll ,NULL);
  }__finally {
    UnLockList();
  }
}
#endif

#ifndef LKAIRSPACE
void AirspaceWarnListInit(void){
  InitializeCriticalSection(&csAirspaceWarnings);
  InitDone = true;
}

void AirspaceWarnListDeInit(void){
  DeleteCriticalSection(&csAirspaceWarnings);
  InitDone = false;
}
#endif

#ifdef LKAIRSPACE
int CAirspaceManager::AirspaceWarnFindIndexByID(int ID)
{
  int idx=0;
  int res = -1;
  if ((ID<0)) return res;

  CCriticalSection::CGuard guard(_cswarnlist);
  for (AirspaceWarningsList::iterator it = _airspaceWarnings.begin(); it != _airspaceWarnings.end(); ++it ){
	if (it->ID == ID) {
	  res = idx;
	  break;
	}
	idx++;
  }
  return(res);
}
#else
int AirspaceWarnFindIndexByID(int ID){
  int idx=0;
  int res = -1;
  if (!InitDone || (ID<0)) {
    return res;
  }

  LockList();
  __try{
    for (List<AirspaceInfo_c>::Node* it = AirspaceWarnings.begin(); it; it = it->next ){
      if (it->data.ID == ID){
        res = idx;
        break;
      }
      idx++;
    }
  }__finally {
    UnLockList();
  }
  return(res);
}
#endif

#ifdef LKAIRSPACE
int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                         bool Predicted, CAirspace *airspace,
                         bool ackDay){

  int   hDistance = 0;
  int   vDistance = 0;
  int   Bearing = 0;

  CAirspaceManager::Instance().AirspaceWarnListCalcDistance(Basic, Calculated, airspace, &hDistance, &Bearing, &vDistance);

   //if (vDistance>100||vDistance<-100) return 99999;
   //	else
   return hDistance;
  
}
#else
int LKAirspaceDistance(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                         bool Predicted, bool IsCircle, int AsIdx,
                         bool ackDay){

  int   hDistance = 0;
  int   vDistance = 0;
  int   Bearing = 0;

    AirspaceWarnListCalcDistance(Basic, Calculated, IsCircle, AsIdx, &hDistance, &Bearing, &vDistance);

   //if (vDistance>100||vDistance<-100) return 99999;
   //	else
   return hDistance;
  
}
#endif
