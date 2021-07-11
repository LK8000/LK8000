/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "TestContest.h"
#include "Tools.h"
#include <iomanip>
#include <stdexcept>


CTestContest::CKMLWrapper::CKMLWrapper(const std::string &path):
  _stream(path.c_str())
{
  if(_stream.fail())
    throw std::invalid_argument("Cannot open file '" + path + "'");
  
  _stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  _stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl;
  _stream << "  <Document>" << std::endl;
  _stream << "    <name>" << path << "</name>" << std::endl;
  _stream << "    <open>1</open>" << std::endl;
  _stream << "    <StyleMap id=\"trace\">" << std::endl;
  _stream << "      <Pair>" << std::endl;
  _stream << "        <key>normal</key>" << std::endl;
  _stream << "        <styleUrl>#yellow_line</styleUrl>" << std::endl;
  _stream << "      </Pair>" << std::endl;
  _stream << "      <Pair>" << std::endl;
  _stream << "        <key>highlight</key>" << std::endl;
  _stream << "        <styleUrl>#yellow_line</styleUrl>" << std::endl;
  _stream << "      </Pair>" << std::endl;
  _stream << "    </StyleMap>" << std::endl;
  _stream << "    <StyleMap id=\"result\">" << std::endl;
  _stream << "      <Pair>" << std::endl;
  _stream << "        <key>normal</key>" << std::endl;
  _stream << "        <styleUrl>#red_line</styleUrl>" << std::endl;
  _stream << "      </Pair>" << std::endl;
  _stream << "      <Pair>" << std::endl;
  _stream << "        <key>highlight</key>" << std::endl;
  _stream << "        <styleUrl>#red_line</styleUrl>" << std::endl;
  _stream << "      </Pair>" << std::endl;
  _stream << "    </StyleMap>" << std::endl;
  _stream << "    <Style id=\"red_line\">" << std::endl;
  _stream << "      <LineStyle>" << std::endl;
  _stream << "        <color>ff0000ff</color>" << std::endl;
  _stream << "        <width>3</width>" << std::endl;
  _stream << "      </LineStyle>" << std::endl;
  _stream << "    </Style>" << std::endl;
  _stream << "    <Style id=\"yellow_line\">" << std::endl;
  _stream << "      <LineStyle>" << std::endl;
  _stream << "        <color>ff00ffff</color>" << std::endl;
  _stream << "        <width>3</width>" << std::endl;
  _stream << "      </LineStyle>" << std::endl;
  _stream << "    </Style>" << std::endl;
}


CTestContest::CKMLWrapper::~CKMLWrapper()
{
  _stream << "  </Document>" << std::endl;
  _stream << "</kml>" << std::endl;
}


void CTestContest::CKMLWrapper::Dump(const CTrace &trace) const
{
  _stream << "    <Placemark>" << std::endl;
  _stream << "      <name>Trace</name>" << std::endl;
  _stream << "      <visibility>0</visibility>" << std::endl;
  _stream << "      <styleUrl>#trace</styleUrl>" << std::endl;
  _stream << "      <LineString>" << std::endl;
  _stream << "        <tessellate>1</tessellate>" << std::endl;
  _stream << "        <altitudeMode>absolute</altitudeMode>" << std::endl;
  _stream << "        <coordinates>" << std::endl;
  const CTrace::CPoint *point=trace.Front();
  do {
    _stream << std::fixed << std::setprecision(8) << point->GPS().Longitude() << "," << point->GPS().Latitude() << "," << std::setprecision(0) << point->GPS().Altitude() << " " << std::endl;
    point = point->Next();
  } while(point);
  _stream << "        </coordinates>" << std::endl;
  _stream << "      </LineString>" << std::endl;
  _stream << "    </Placemark>" << std::endl;

  // point=trace.Front();
  // do {
  //   _stream << "      <Placemark>" << std::endl;
  //   _stream << "        <visibility>0</visibility>" << std::endl;
  //   _stream << "        <description>" << std::endl;
  //   _stream << "          <![CDATA[" << std::endl;
  //   _stream << "            <b>Longitude:</b> " << CoordToString(point->_gps->Longitude(), false) << "<br>" << std::endl;
  //   _stream << "            <b>Latitude:</b> " << CoordToString(point->_gps->Latitude(), true) << "<br>" << std::endl;
  //   _stream << "            <b>Altitude:</b> " << std::setprecision(0) << point->_gps->Altitude() << "m" << "<br>" << std::endl;
  //   _stream << "            <b>Time:</b> " << TimeToString(point->_gps->Time()) << "<br>" << std::endl;
  //   _stream << "            <b>Prev distance:</b> " << point->_prevDistance << "m<br>" << std::endl;
  //   _stream << "            <b>Inhertited cost:</b> " << point->_inheritedCost << "m<br>" << std::endl;
  //   _stream << "            <b>Distance cost:</b> " << point->_distanceCost << "m<br>" << std::endl;
  //   _stream << "            <b>Time cost:</b> " << point->_timeCost << std::endl;
  //   _stream << "          ]]>"  << std::endl;
  //   _stream << "        </description>"  << std::endl;
  //   _stream << "        <Point>" << std::endl;
  //   _stream << "          <altitudeMode>absolute</altitudeMode>" << std::endl;
  //   _stream << "          <coordinates>" << std::setprecision(8) << point->_gps->Longitude() << "," << point->_gps->Latitude() << "," << point->_gps->Altitude() << "</coordinates>" << std::endl;
  //   _stream << "        </Point>" << std::endl;
  //   _stream << "      </Placemark>" << std::endl;
  //   point = point->Next();
  // } while(point);
}



void CTestContest::CKMLWrapper::Dump(const CContestMgr::CResult &result) const
{
  _stream << "    <Folder>" << std::endl;
  std::wstring typeStr = CContestMgr::TypeToString(result.Type());
  _stream << "    <name>" << std::string(typeStr.begin(), typeStr.end()) << "</name>" << std::endl;
  _stream << "    <description>" << std::endl;
  _stream << "      <![CDATA[" << std::endl;
  _stream << "        <b>Points:</b> " << std::setprecision(2) << result.Score() << "<br>" << std::endl;
  _stream << "        <b>Distance:</b> " << std::setprecision(1) << result.Distance() / 1000.0 << " km<br>" << std::endl;
  _stream << "        <b>Speed:</b> " << std::setprecision(1) << std::fixed << result.Speed() * 3600 / 1000.0 << " km/h<br>" << std::endl;
  _stream << "        <b>Duration:</b> " << TimeToString(result.Duration()) << std::endl;
  _stream << "      ]]>"  << std::endl;
  _stream << "    </description>"  << std::endl;
  _stream << "    <visibility>0</visibility>" << std::endl;
  _stream << "      <Placemark>" << std::endl;
  _stream << "        <name>" << std::string(typeStr.begin(), typeStr.end()) << ": Trace" << "</name>" << std::endl;
  _stream << "        <visibility>0</visibility>" << std::endl;
  _stream << "        <styleUrl>#result</styleUrl>" << std::endl;
  _stream << "        <LineString>" << std::endl;
  _stream << "          <tessellate>0</tessellate>" << std::endl;
  _stream << "          <coordinates>" << std::endl;
  for(CPointGPSArray::const_iterator it=result.PointArray().begin(); it!=result.PointArray().end(); ++it) {
    _stream << std::setprecision(8) << it->Longitude() << "," << it->Latitude() << "," << std::setprecision(0) << it->Altitude() << " " << std::endl;
  }
  _stream << "          </coordinates>" << std::endl;
  _stream << "        </LineString>" << std::endl;
  _stream << "      </Placemark>" << std::endl;
  
  unsigned i=0;
  for(CPointGPSArray::const_iterator it=result.PointArray().begin(); it!=result.PointArray().end(); ++it, i++) {
    _stream << "      <Placemark>" << std::endl;
    std::string name;
    if(it == result.PointArray().begin())
      name = "Start";
    else if(it + 1 == result.PointArray().end())
      name = "Finish";
    else
      name = "WP" + Convert(i);
    _stream << "        <name>" << std::string(typeStr.begin(), typeStr.end()) << ": " << name << "</name>" << std::endl;
    _stream << "        <visibility>0</visibility>" << std::endl;
    _stream << "        <description>" << std::endl;
    _stream << "          <![CDATA[" << std::endl;
    _stream << "            <b>Longitude:</b> " << CoordToString(it->Longitude(), false) << "<br>" << std::endl;
    _stream << "            <b>Latitude:</b> " << CoordToString(it->Latitude(), true) << "<br>" << std::endl;
    _stream << "            <b>Altitude:</b> " << std::setprecision(0) << it->Altitude() << "m" << "<br>" << std::endl;
    _stream << "            <b>Time:</b> " << TimeToString(it->Time()) << std::endl;
    _stream << "          ]]>"  << std::endl;
    _stream << "        </description>"  << std::endl;
    _stream << "        <Point>" << std::endl;
    _stream << "          <altitudeMode>absolute</altitudeMode>" << std::endl;
    _stream << "          <coordinates>" << std::setprecision(8) << it->Longitude() << "," << it->Latitude() << "," << it->Altitude() << "</coordinates>" << std::endl;
    _stream << "        </Point>" << std::endl;
    _stream << "      </Placemark>" << std::endl;
  }
  _stream << "    </Folder>" << std::endl;
}
