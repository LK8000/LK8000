/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "RGB.h"


extern Topology* TopoStore[MAXTOPOLOGY];



double ReadZoomTopology(int iCategory) {

  for (int z=0; z<MAXTOPOLOGY; z++) {
	if (TopoStore[z]) {
		if ( TopoStore[z]->scaleCategory == iCategory ) {
			return (TopoStore[z]->scaleThreshold);
		}
	}
  }
  return 0.0;
}

bool HaveZoomTopology(int iCategory) {

  for (int z=0; z<MAXTOPOLOGY; z++) {
	if (TopoStore[z]) {
		if ( TopoStore[z]->scaleCategory == iCategory ) {
			return true;
		}
	}
  }
  return false;
}

// mode: 0 normal Change the topology scale of Category with newScale.
// mode: 1 zoom more or less for the category with newScale (newScale is the zoom increment).
// mode: 2 zoom more or less for all categories (tCategory is ignored, newScale is the zoom increment for all items)
// mode: 3 reset default zoom for Category (newScale is ignored)
// mode: 4 reset default zoom for all Categories (Category is ignored, newScale is ignored)
void ChangeZoomTopology(int iCategory, double newScale, short cztmode)
{
  if (LKTopo<1) return;

  LockTerrainDataGraphics();

  if (cztmode==0) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			if ( TopoStore[z]->scaleCategory == iCategory ) {
				#if DEBUG_LKTOPO
				StartupStore(_T("... ChangeZoomTopology zindex=%d, categ=%d oldscale=%f newscale=%f%s"),z,iCategory,
				TopoStore[z]->scaleThreshold, newScale,NEWLINE);
				#endif
				TopoStore[z]->scaleThreshold = newScale;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==1) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			if ( TopoStore[z]->scaleCategory == iCategory ) {
				#if DEBUG_LKTOPO
				StartupStore(_T("... ChangeZoomTopology: zindex=%d, categ=%d oldscale=%f increment=%f%s"),z,iCategory,
				TopoStore[z]->scaleThreshold, newScale,NEWLINE);
				#endif
				TopoStore[z]->scaleThreshold += newScale;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==2) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			#if DEBUG_LKTOPO
			StartupStore(_T("... ChangeZoomTopology for all: zindex=%d, categ=%d oldscale=%f increment=%f%s"),z,iCategory,
			TopoStore[z]->scaleThreshold, newScale,NEWLINE);
			#endif
			TopoStore[z]->scaleThreshold += newScale;
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==3) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			if ( TopoStore[z]->scaleCategory == iCategory ) {
				#if DEBUG_LKTOPO
				StartupStore(_T("... ChangeZoomTopology default zindex=%d, categ=%d oldscale=%f default=%f%s"),
				z,iCategory, TopoStore[z]->scaleThreshold, TopoStore[z]->scaleDefaultThreshold,NEWLINE);
				#endif
				TopoStore[z]->scaleThreshold = TopoStore[z]->scaleDefaultThreshold;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  if (cztmode==4) {
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			#if DEBUG_LKTOPO
			StartupStore(_T("... ChangeZoomTopology all default zindex=%d, categ=%d oldscale=%f default=%f%s"),
			z,iCategory, TopoStore[z]->scaleThreshold, TopoStore[z]->scaleDefaultThreshold,NEWLINE);
			#endif
			// 130207 categories 5 and 10 are using DefaultThreshold for polygon paint,
			// and scaleThreshold for labels only. Upon reset, we should use the original
			// LKTopoZoom settings, because the Default is always fixed to 100!
			// Otherwise on reset we get 99.. even if we set LKTopoZoom to something else.
			// This is needed if we want to have a startup setting different from default
			// for these 05 and 10 categories only.
			// Sorry but water areas and labels are not easy to manage because we want to paint
			// blue all the way, but not also their labels.
			// Notice that since LKTopoZoom is saved to configuration, we cannot use it for reset!
			// This is why we have a special define DEFAULT_WATER_LABELS_THRESHOLD
			switch(TopoStore[z]->scaleCategory) {
				case 5:
					TopoStore[z]->scaleThreshold = DEFAULT_WATER_LABELS_THRESHOLD;
					break;
				case 10:
					TopoStore[z]->scaleThreshold = DEFAULT_WATER_LABELS_THRESHOLD;
					break;
				default:
					TopoStore[z]->scaleThreshold = TopoStore[z]->scaleDefaultThreshold;
					break;
			}
		}
	}
	UnlockTerrainDataGraphics();
	return;
  }

  UnlockTerrainDataGraphics();
}
