/*
 * DrawFAIOpti.h
 *
 *  Created on: Feb 13, 2017
 *      Author: Ulli
 */

#ifndef FAI_SECTOR_H_
#define FAI_SECTOR_H_



#ifdef PNA
  #define FAI_SECTOR_STEPS 11
#else
  #define FAI_SECTOR_STEPS 21
#endif

typedef std::list<GeoPoint> GPS_Track;


struct GPS_Gridline_t
{
  GeoPoint GridLine[FAI_SECTOR_STEPS];
  TCHAR szLable[20];
  double fValue;
};

typedef std::list<GPS_Gridline_t> GPS_Gridlines;

class Projection;

class FAI_Sector
{
public:
  GPS_Track     m_FAIShape;
  GPS_Track     m_FAIShape2;
  GPS_Gridlines m_FAIGridLines;
  double        m_lat1,m_lat2,m_lon1,m_lon2;
  double        m_fGrid;
  int           m_Side;
  FAI_Sector(void);
 ~FAI_Sector(void);
 void FreeFAISectorMem(void);
 bool CalcSectorCache(double lat1, double lon1, double lat2, double lon2, double fGrid, int iOpposite);

 /**
	* Specialised For MapWindow
	*/
 void DrawFAISector (LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const LKColor& InFfillcolor);
 
 /**
	* For Specialised for Analysis Dialog
	*/
 void AnalysisDrawFAISector (LKSurface& Surface, const RECT& rc, const GeoPoint& center, const LKColor& InFfillcolor);

} ;







#endif /* FAI_SECTOR_H_ */
