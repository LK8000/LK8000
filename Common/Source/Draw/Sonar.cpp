/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"

#define GC_HORIZONTAL_TOLERANCE      100
#define GC_HORIZONTAL_THRESHOLD     2500
#define GC_VERTICAL_THRESHOLD        250
#define GC_VERTICAL_TOLERANCE         50
#define GC_HORIZONTAL_DELAY_FACT   250.0f
#define GC_VERTICAL_DELAY_FACT      25.0f


extern AirSpaceSonarLevelStruct sSonarLevel[];
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];
extern int Sideview_iNoHandeldSpaces;
extern bool Sonar_IsEnabled;

//#define DEBUG_SONAR	1

bool Sonar_IsEnabled = true; // must be made Global.cpp


AirSpaceSonarLevelStruct sSonarLevel[10] = {
    /* horizontal sonar levels */
    /* Dist , Delay *0.5s, V/H,      soundfile */
    {  150,     3,         true, TEXT("LK_SONAR_H1.WAV")},
    {  330,     3,         true, TEXT("LK_SONAR_H2.WAV")},
    {  500,     5,         true, TEXT("LK_SONAR_H3.WAV")},
    {  650,     5,         true, TEXT("LK_SONAR_H4.WAV")},
    {  850,     7,         true, TEXT("LK_SONAR_H5.WAV")},
    /* vertical sonar levels */
    {  30 ,     3,         false, TEXT("LK_SONAR_H1.WAV")},
    {  50 ,     3,         false, TEXT("LK_SONAR_H2.WAV")},
    {  70,      5,         false, TEXT("LK_SONAR_H3.WAV")},
    {  90,      5,         false, TEXT("LK_SONAR_H4.WAV")},
    {  110,     7,         false, TEXT("LK_SONAR_H5.WAV")}
   };




//
// calc the sonar delay time
// This function can manage more than 1 airspace as Input. 
// However currently we only use one, the closest.
//
int CalcSonarDelay (const int iNoAs, AirSpaceSideViewSTRUCT asAirspaces[], int iAltitudeAGL, int iAltitude)
{
  int iAS_HorDist;
  int iAS_VertDist;
  int iAS_Bearing;
  int i;
  bool bAS_Inside = false;
  bool bOK = false;

  int iTreadLevel;
//  CAirspace SelectedAS;
  CAirspace *sel_pasp = NULL;

  int	iH_Level = 1000;
  int	iV_Level = 1000;

  int divider=ISPARAGLIDER?2:1;

  for( i =  0 ; i < iNoAs ; i++)
  {

	sel_pasp =	asAirspaces[i].psAS;
	if(sel_pasp != NULL)
	{
		//SelectedAS = CAirspaceManager::Instance().GetAirspaceCopy( sel_pasp );
		//bOK = SelectedAS.GetDistanceInfo(bAS_Inside, iAS_HorDist, iAS_Bearing, iAS_VertDist);
		bOK = sel_pasp->GetDistanceInfo(bAS_Inside, iAS_HorDist, iAS_Bearing, iAS_VertDist);

		if(bOK)
		{
			int iTmpV_Level = -1;
			if((iAS_HorDist) < GC_HORIZONTAL_TOLERANCE)                          /* horizontal near or inside */
			{
				int iTmp =	abs(iAS_VertDist);
				if(iTmp < sSonarLevel[9].iDistantrance)  {
					iTmpV_Level = 9;
					if(iTmp < sSonarLevel[8].iDistantrance)  {
						iTmpV_Level = 8;
						if(iTmp < sSonarLevel[7].iDistantrance)  {
							iTmpV_Level = 7;
							if(iTmp < sSonarLevel[6].iDistantrance)  {
								iTmpV_Level = 6;
								if(iTmp < sSonarLevel[5].iDistantrance)  {
									iTmpV_Level = 5;
								}
							}
						}
					}
				}
			}

			if(iTmpV_Level != -1) {
				if(iTmpV_Level < iV_Level ) iV_Level = iTmpV_Level;
			}


			int iTmpH_Level = -1;
			//if(SelectedAS.IsAltitudeInside(iAltitude,iAltitudeAGL,GC_VERTICAL_TOLERANCE))  /* vertically near or inside ? */
			if(sel_pasp->IsAltitudeInside(iAltitude,iAltitudeAGL,GC_VERTICAL_TOLERANCE))  /* vertically near or inside ? */
			{
				int iTmp =	(iAS_HorDist);
				if(iTmp > 0) {
					LKASSERT(divider!=0);
            				if(iTmp < sSonarLevel[4].iDistantrance/divider)   {
						iTmpH_Level = 4;
						if(iTmp < sSonarLevel[3].iDistantrance/divider)   {
							iTmpH_Level = 3;
							if(iTmp < sSonarLevel[2].iDistantrance/divider)   {
								iTmpH_Level = 2;
								if(iTmp < sSonarLevel[1].iDistantrance/divider)   {
									iTmpH_Level = 1;
									if(iTmp < sSonarLevel[0].iDistantrance/divider)   {
										iTmpH_Level = 0;
									}
								}
							}
						}
					}
				}
			}

			if(iTmpH_Level != -1) {
				if(iTmpH_Level < iH_Level ) iH_Level = iTmpH_Level;
			}

			//if(SelectedAS.IsAltitudeInside(iAltitude,iAltitudeAGL,0))  /* vertically inside ? */
			if(sel_pasp->IsAltitudeInside(iAltitude,iAltitudeAGL,0))  /* vertically inside ? */
				if(iAS_HorDist < 0)                                             /* complete inside, do no beep! */
				{
					iV_Level = -1;   /* red allert */
					iH_Level = -1;   /* red allert */
				}
		} // bOk
	}  // sel_pasp != NULL
  } // for

  iTreadLevel = iV_Level;
  if(iH_Level > -1) {
	if((iH_Level+5) <= iV_Level) iTreadLevel = iH_Level;
  }

  // StartupStore(_T("... HDist=%d Vdist=%d SonarLevel=%d \n"), iAS_HorDist, iAS_VertDist,iTreadLevel);

  return iTreadLevel;
}



//
// We do Sonar from draw thread, because it is reasonable to think that without a visual aid on map,
// the simple sonar sound alone is an halved solution.
//
void MapWindow::DoSonar(void) {

  static unsigned long lSonarCnt = 0;

  if (!Sonar_IsEnabled || DrawInfo.NAVWarning)return;


  CAirspace *aspfound = CAirspaceManager::Instance().GetNearestAirspaceForSideview();

  if( aspfound == NULL ) {
	#if DEBUG_SONAR
	StartupStore(_T("SONAR: no aspfound, return\n"));
	#endif
	return;
  }
  CAirspace near_airspace;
  near_airspace = CAirspaceManager::Instance().GetAirspaceCopy(aspfound);

  // we dont use these at all
  bool bAS_Inside;
  int iAS_HorDistance=5000;
  int iAS_Bearing=0;
  int iAS_VertDistance=0;

  if ( near_airspace.GetDistanceInfo(bAS_Inside, iAS_HorDistance, iAS_Bearing, iAS_VertDistance) ) {
	int iSonarLevel=0;
	#if TESTBENCH
	if(1)
	#else
	if(DerivedDrawInfo.FreeFlying)
	#endif
	{
		AirSpaceSideViewSTRUCT tmpasp;
		tmpasp.psAS =  &near_airspace;
		iSonarLevel = CalcSonarDelay( 1, &tmpasp, 
			(int)DerivedDrawInfo.AltitudeAGL, (int)DerivedDrawInfo.NavAltitude);

		#if DEBUG_SONAR
		StartupStore(_T(".. iSonarLevel=%d\n"),iSonarLevel);
		#endif

		lSonarCnt++;
		if((iSonarLevel >=0) && (iSonarLevel < 10)) {
			if( lSonarCnt > (unsigned)sSonarLevel[iSonarLevel].iSoundDelay)
			{
	  			lSonarCnt = 0;
          			// StartupStore(_T("... level=%d PLAY <%s>\n"),iSonarLevel,&sSonarLevel[iSonarLevel].szSoundFilename);
	  			LKSound((TCHAR*) &(sSonarLevel[iSonarLevel].szSoundFilename));
			}
		}
	}
  }
  #if DEBUG_SONAR
  else StartupStore(_T("SONAR: no near_airspace, return\n"));
  #endif

}

