/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "Bitmaps.h"
#include "RGB.h"
#include "ScreenProjection.h"

void MapWindow::DrawAirspaceLabels(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT& Orig_Aircraft)
{
  static short int label_sequencing_divider = 0;
  CAirspaceList::const_iterator it;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetAirspacesForWarningLabels();
  
  if (label_sequencing_divider) --label_sequencing_divider;

  // Draw warning position and label on top of all airspaces
  if (1) {
  ScopeLock guard(CAirspaceManager::Instance().MutexRef());
  for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->WarningLevel() > awNone) {
          double lon;
          double lat;
          int vdist;
          AirspaceWarningDrawStyle_t vlabeldrawstyle, hlabeldrawstyle;
          bool distances_ready = (*it)->GetWarningPoint(lon, lat, hlabeldrawstyle, vdist, vlabeldrawstyle);
          TCHAR hbuf[NAME_SIZE+16], vDistanceText[16];
          TextInBoxMode_t TextDisplayMode = {};
          bool hlabel_draws = false;
          bool vlabel_draws = false;
          
          // Horizontal warning point
          if (distances_ready && (hlabeldrawstyle > awsHidden) && PointVisible(lon, lat)) {

              const POINT sc = _Proj.ToRasterPoint(lat, lon);
              hAirspaceWarning.Draw(Surface, sc.x - NIBLSCALE(5), sc.y - NIBLSCALE(5), IBLSCALE(10), IBLSCALE(10));              
              
              Units::FormatAltitude(vdist, vDistanceText, sizeof(vDistanceText)/sizeof(vDistanceText[0]));
              lk::strcpy(hbuf, (*it)->Name());
              _tcscat(hbuf, TEXT(" "));
              _tcscat(hbuf, vDistanceText);
              
              switch (hlabeldrawstyle) {
                default:
                case awsHidden:
                case awsBlack:
                  TextDisplayMode.Color = RGB_BLACK;
                  break;
                case awsAmber:
                  TextDisplayMode.Color = RGB_ORANGE;
                  break;
                case awsRed:
                  TextDisplayMode.Color = RGB_RED;
                  break;
              } // sw
              TextDisplayMode.SetTextColor = 1;
              TextDisplayMode.AlligneCenter = 1;
              if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
                  TextDisplayMode.Border = 1;
              } else {
		  if (TextDisplayMode.Color==RGB_BLACK)
                     TextDisplayMode.WhiteBold = 0; // no black outline for black background..
                  else
                     TextDisplayMode.WhiteBold = 1; // outlined  
              }

              hlabel_draws = TextInBox(Surface, &rc, hbuf, sc.x, sc.y+NIBLSCALE(15), &TextDisplayMode, true);
           }
           
          // Vertical warning point
          if (distances_ready && vlabeldrawstyle > awsHidden) {

              Units::FormatAltitude(vdist, vDistanceText, sizeof(vDistanceText)/sizeof(vDistanceText[0]));
              lk::strcpy(hbuf, (*it)->Name());
              _tcscat(hbuf, TEXT(" "));
              _tcscat(hbuf, vDistanceText);
              
              switch (vlabeldrawstyle) {
                default:
                case awsHidden:
                case awsBlack:
                  TextDisplayMode.Color = RGB_BLACK;
                  break;
                case awsAmber:
                  TextDisplayMode.Color = RGB_ORANGE;
                  break;
                case awsRed:
                  TextDisplayMode.Color = RGB_RED;
                  break;
              } // sw
              TextDisplayMode.SetTextColor = 1;
              TextDisplayMode.AlligneCenter = 1;
              if ( (MapBox == (MapBox_t)mbBoxed) || (MapBox == (MapBox_t)mbBoxedNoUnit)) {
                  TextDisplayMode.Border = 1;
              } else {
		  if (TextDisplayMode.Color==RGB_BLACK)
                     TextDisplayMode.WhiteBold = 0; // no black outline for black background..
                  else
                     TextDisplayMode.WhiteBold = 1; // outlined  
              }

              vlabel_draws = TextInBox(Surface, &rc, hbuf, Orig_Aircraft.x, Orig_Aircraft.y+NIBLSCALE(15), &TextDisplayMode, true);
           }
           if (!label_sequencing_divider) CAirspaceManager::Instance().AirspaceWarningLabelPrinted(**it, hlabel_draws || vlabel_draws);
           
         }// if warnlevel>awnone
  }//for
  }// if(1) mutex
  if (!label_sequencing_divider) label_sequencing_divider=3;		// Do label sequencing slower than update rate
}


