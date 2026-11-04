/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "Dialogs/dlgProgress.h"
#include "utils/zzip_file_stream.h"
#include "utils/printf.h"
#include "utils/charset_helper.h"
#include "LocalPath.h"
#include "MapDraw/ZoomTopology.h"

#define Log(...) TopoLog(_T("[OpenClose]") __VA_ARGS__)
#define LogT(...) TopoLogT(_T("[OpenClose]") __VA_ARGS__)
#define LogD(...) TopoLogD(_T("[OpenClose]") __VA_ARGS__)


extern Topology* TopoStore[MAXTOPOLOGY];


void OpenTopology() {
  CreateProgressDialog(MsgToken<902>()); // Loading Topology File...

  // Start off by getting the names and paths
  TCHAR szFile[MAX_PATH] = TEXT("\0");
  TCHAR Directory[MAX_PATH] = TEXT("\0");

  LKTopo=0;
  LKWaterTopology=false;

  LocalPath(Directory, _T(LKD_MAPS), szMapFile);

  Log(_T(". OpenTopology <%s>"), Directory);

  _tcscat(Directory, _T("/"));
  // Look for the file within the map zip file...
  int ret = lk::snprintf(szFile, _T("%stopology.tpl"), Directory);
  if(ret >= (MAX_PATH-1)) {
    Log(_T(". Invalid topology path : <%s>"), szFile);
    return;
  }


  std::lock_guard lock(CritSec_TerrainDataGraphics);

  std::fill(std::begin(TopoStore), std::end(TopoStore), nullptr);

  // Ready to open the file now..
  zzip_file_stream stream(szFile, "rt");
  if (!stream) {
    Log(_T(". No topology file <%s>"), szFile);
    return;
  }

  TCHAR ctemp[80];
  TCHAR ShapeName[80];
  double ShapeRange;
  long ShapeIcon;
  long ShapeField;
  TCHAR wShapeFilename[std::size(Directory) + std::size(ShapeName) + 3];
  TCHAR wCPGFilename[std::size(Directory) + std::size(ShapeName) + 3];
  int numtopo = 0;
  auto shapeIndex = topology_category::reserved;

  std::string src_line;
  std::istream istream(&stream);
  while(std::getline(istream, src_line)) {
    trim_inplace(src_line);
    //skip empty lines
    if (src_line.empty()) {
      continue; // Skip empty line
    }
    if (src_line.front() == '*') {
      continue; // Skip Comment
    }

    tstring Text = from_unknown_charset(src_line.c_str());
    const TCHAR* TempString = Text.data();

    // filename,range,icon,field
    // File name
    PExtractParameter(TempString, ctemp, 0);
    lk::strcpy(ShapeName, ctemp);

    lk::snprintf(wShapeFilename, _T("%s%s.shp"), Directory, ShapeName);
    lk::snprintf(wCPGFilename, _T("%s%s.cpg"), Directory, ShapeName);

    // Shape range
    PExtractParameter(TempString, ctemp, 1);
    ShapeRange = StrToDouble(ctemp,NULL);

    // Normally ShapeRange is indicating km threshold for items to be drawn.
    // If over 5000, we identify an LKmap topology and subtract 5000 to get the
    // type.
    //
    // Category
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
    if (ShapeRange > 5000 && ShapeRange < 6000) {
      shapeIndex = static_cast<topology_category>(ShapeRange - 5000);
      if (shapeIndex == topology_category::coast_areas) {
          // Coast Area
          LKWaterTopology = true;
      }

      // ShapeRange is belonging to a LKMAPS topology..
      if (LKTopo == -1) {
        // Problem. Mixed topology file. No good..
        Log(_T("------ INVALID MIXED OLD/NEW TOPOLOGY FILES - IGNORED"));
        LKTopo = 0;
        return;
      }
      LogD(_T("... LKMAPS new topo file"));
      LKTopo++;
    }
    else {
      LogD(_T("... OLD XCS topo file"));
      LKTopo = -1;
      shapeIndex = topology_category::reserved;
    }

    // Shape icon
    PExtractParameter(TempString, ctemp, 2);
    ShapeIcon = _tcstol(ctemp, nullptr, 10);

    // Shape field for text display

    // sjt 02NOV05 - field parameter enabled
    PExtractParameter(TempString, ctemp, 3);
    if (_istalnum(ctemp[0])) {
      ShapeField = _tcstol(ctemp, nullptr, 10);
      ShapeField--;
    }
    else {
      ShapeField = -1;
    }

    // Red component of line / shading colour
    PExtractParameter(TempString, ctemp, 4);
    auto red = static_cast<BYTE>(_tcstol(ctemp, nullptr, 10));

    // Green component of line / shading colour
    PExtractParameter(TempString, ctemp, 5);
    auto green = static_cast<BYTE>(_tcstol(ctemp, nullptr, 10));

    // Blue component of line / shading colour
    PExtractParameter(TempString, ctemp, 6);
    auto blue = static_cast<BYTE>(_tcstol(ctemp, nullptr, 10));

    if ((red == 64) && (green == 96) && (blue == 240)) {
      // JMW update colours to ICAO standard
      red = 85;  // water colours
      green = 160;
      blue = 255;
    }

    if (numtopo >= MAXTOPOLOGY) {
      Log(_T("------ Too many topology entries, ignoring the remainder"));
      break;
    }

    TopoStore[numtopo] = new Topology(wShapeFilename, ShapeField);

    TopoStore[numtopo]->Category = shapeIndex;
    TopoStore[numtopo]->scaleThreshold = ShapeRange;

    if (ShapeIcon != 0) {
      TopoStore[numtopo]->loadBitmap(ShapeIcon);
    }
    else {
      // Careful not to use hPen and hBrush then! Always check that it is
      // not null
      TopoStore[numtopo]->loadPenBrush(LKColor(red, green, blue));
    }

    numtopo++;
  }

  if (LKTopo > 0) {
    Log(_T(". LKMAPS Advanced Topology file found"));
  }
  else {
    LKTopo = 0;
  }
}

void CloseTopology() {
  LogT(TEXT(". Close"));

  std::lock_guard lock(CritSec_TerrainDataGraphics);
  std::for_each(std::begin(TopoStore), std::end(TopoStore), safe_delete());
}
