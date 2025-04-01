/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <ctype.h>
#include "Terrain.h"
#include "RGB.h"
#include "LKProfiles.h"
#include "Dialogs/dlgProgress.h"
#include "utils/zzip_stream.h"
#include "utils/printf.h"
#include "LocalPath.h"


extern Topology* TopoStore[MAXTOPOLOGY];


void OpenTopology() {
  CreateProgressDialog(MsgToken<902>()); // Loading Topology File...

  // Start off by getting the names and paths
  TCHAR szFile[MAX_PATH] = TEXT("\0");
  TCHAR Directory[MAX_PATH] = TEXT("\0");

  LKTopo=0;
  LKWaterTopology=false;

  LocalPath(Directory, _T(LKD_MAPS), szMapFile);

  StartupStore(_T(". OpenTopology <%s>"), Directory);

  _tcscat(Directory, _T("/"));
  // Look for the file within the map zip file...
  int ret = lk::snprintf(szFile, _T("%stopology.tpl"), Directory);
  if(ret >= (MAX_PATH-1)) {
    StartupStore(_T(". Invalid topology path : <%s>"), szFile);
    return;
  }


  LockTerrainDataGraphics();

  std::fill(std::begin(TopoStore), std::end(TopoStore), nullptr);

  // Ready to open the file now..
  zzip_stream stream(szFile, "rt");

  if (!stream) {
    UnlockTerrainDataGraphics();
    StartupStore(TEXT(". No topology file <%s>%s"), szFile,NEWLINE);
    return;
  }

  TCHAR ctemp[80];
  TCHAR TempString[READLINE_LENGTH+1];
  TCHAR ShapeName[80];
  double ShapeRange;
  long ShapeIcon;
  long ShapeField;
  TCHAR wShapeFilename[std::size(Directory) + std::size(ShapeName) + 3];
  TCHAR wCPGFilename[std::size(Directory) + std::size(ShapeName) + 3];
  TCHAR *Stop;
  int numtopo = 0;
  int shapeIndex=0;

  while(stream.read_line(TempString)) {

    if(TempString[0] == _T('\0')) {
      continue; // Skip empty line
    }
    if(TempString[0] == _T('*')) {
      continue; // Skip Comment
    }

    BYTE red, green, blue;
    // filename,range,icon,field

    // File name
    PExtractParameter(TempString, ctemp, 0);
    lk::strcpy(ShapeName, ctemp);

    _stprintf(wShapeFilename, _T("%s%s.shp"), Directory, ShapeName);
    _stprintf(wCPGFilename, _T("%s%s.cpg"), Directory, ShapeName);

    // Shape range
    PExtractParameter(TempString, ctemp, 1);
    ShapeRange = StrToDouble(ctemp,NULL);

	// Normally ShapeRange is indicating km threshold for items to be drawn.
	// If over 5000, we identify an LKmap topology and subtract 5000 to get the type.
	//
	// SCALE CATEGORIES scaleCategory
	// 0 is reserved
	// 1 is marked locations
	// 05 for coast areas
	// 10 for water area
	// 20 for water line
	// 30 for big road
	// 40 for medium road
	// 50 for small road
	// 60 for railroad
	// 70 for big city
	// 80 for medium city
	// 90 for small city
	// 100 for very small city
	// 110 for city area polyline with no name
	if ( ShapeRange>5000 && ShapeRange<6000 ) {
		shapeIndex=(int)ShapeRange-5000;
		// Load default values
		switch(shapeIndex) {
			case 5:
				// Coast Area
				ShapeRange=100;
				LKWaterTopology = true;
				break;
			case 10:
				// Water Area
				ShapeRange=100;
				break;
			case 20:
				// Water line
				ShapeRange=7;
				break;
			case 30:
				// Big road
				ShapeRange=25;
				break;
			case 40:
				// Medium road
				ShapeRange=6;
				break;
			case 50:
				// Small road
				ShapeRange=3;
				break;
			case 60:
				// Railroad
				ShapeRange=8;
				break;
			case 70:
				// Big city
				ShapeRange=15;
				break;
			case 80:
				// Med city
				ShapeRange=10;
				break;
			case 90:
				// Small city
				ShapeRange=6;
				break;
			case 100:
				// Very small city
				ShapeRange=3;
				break;
			case 110:
				// City polyline area
				ShapeRange=15;
				break;
			default:
				// UNKNOWN
				ShapeRange=100;
				break;
		}
		// ShapeRange is belonging to a LKMAPS topology..
		if (LKTopo == -1) {
			// Problem. Mixed topology file. No good..
			StartupStore(_T("------ INVALID MIXED OLD/NEW TOPOLOGY FILES- TOPOLOGY IGNORED%s"),NEWLINE);
			UnlockTerrainDataGraphics();
			LKTopo=0;
			return;
		}
		#if DEBUG_LKTOPO
		StartupStore(_T("... LKMAPS new topo file%s"),NEWLINE);
		#endif
		LKTopo++;

	} else {
		#if DEBUG_LKTOPO
		StartupStore(_T("... OLD XCS topo file%s"),NEWLINE);
		#endif
		LKTopo=-1;
	}


        // Shape icon
        PExtractParameter(TempString, ctemp, 2);
        ShapeIcon = _tcstol(ctemp, &Stop, 10);

        // Shape field for text display

        // sjt 02NOV05 - field parameter enabled
        PExtractParameter(TempString, ctemp, 3);
        if (_istalnum(ctemp[0])) {
          ShapeField = _tcstol(ctemp, &Stop, 10);
          ShapeField--;
        } else {
          ShapeField = -1;
	}

        // Red component of line / shading colour
        PExtractParameter(TempString, ctemp, 4);
        red = (BYTE)_tcstol(ctemp, &Stop, 10);

        // Green component of line / shading colour
        PExtractParameter(TempString, ctemp, 5);
        green = (BYTE)_tcstol(ctemp, &Stop, 10);

        // Blue component of line / shading colour
        PExtractParameter(TempString, ctemp, 6);
        blue = (BYTE)_tcstol(ctemp, &Stop, 10);

        if ((red==64)
            && (green==96)
            && (blue==240)) {
          // JMW update colours to ICAO standard
          red =    85; // water colours
          green = 160;
          blue =  255;
        }

        TopoStore[numtopo] = new Topology(wShapeFilename, ShapeField);

        TopoStore[numtopo]->scaleCategory = shapeIndex;
        TopoStore[numtopo]->scaleDefaultThreshold = ShapeRange;
        TopoStore[numtopo]->scaleThreshold = ShapeRange;

        if (ShapeIcon!=0)
          TopoStore[numtopo]->loadBitmap(ShapeIcon);
	else {
	  // Careful not to use hPen and hBrush then! Always check that it is not null
          TopoStore[numtopo]->loadPenBrush(LKColor(red,green,blue));
	}

	if (shapeIndex ==  5) if ( LKTopoZoomCat05 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat05;
	if (shapeIndex == 10) if ( LKTopoZoomCat10 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat10;
	if (shapeIndex == 20) if ( LKTopoZoomCat20 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat20;
	if (shapeIndex == 30) if ( LKTopoZoomCat30 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat30;
	if (shapeIndex == 40) if ( LKTopoZoomCat40 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat40;
	if (shapeIndex == 50) if ( LKTopoZoomCat50 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat50;
	if (shapeIndex == 60) if ( LKTopoZoomCat60 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat60;
	if (shapeIndex == 70) if ( LKTopoZoomCat70 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat70;
	if (shapeIndex == 80) if ( LKTopoZoomCat80 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat80;
	if (shapeIndex == 90) if ( LKTopoZoomCat90 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat90;
	if (shapeIndex == 100) if ( LKTopoZoomCat100 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat100;
	if (shapeIndex == 110) if ( LKTopoZoomCat110 <=100 ) TopoStore[numtopo]->scaleThreshold = LKTopoZoomCat110;

	#if DEBUG_LKTOPO
	StartupStore(_T("... TopoStore[%d] scaleCategory=%d Threshold=%f defaultthreshold=%f%s"),numtopo,shapeIndex,
		TopoStore[numtopo]->scaleThreshold,TopoStore[numtopo]->scaleDefaultThreshold,NEWLINE);
	#endif

    numtopo++;
  }

  if (LKTopo>0) {
	StartupStore(_T(". LKMAPS Advanced Topology file found%s"),NEWLINE);
  } else {
	LKTopo=0;
  }

  UnlockTerrainDataGraphics();

}




void CloseTopology() {
  TestLog(TEXT(". CloseTopology"));

  LockTerrainDataGraphics();
  std::for_each(std::begin(TopoStore), std::end(TopoStore), safe_delete());
  UnlockTerrainDataGraphics();
}
