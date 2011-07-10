//
// LKincludetexttime.cpp
//
// code used by ALPHA debug messages, inlined because not thread safe
//

  TCHAR time_temp[60];
  TCHAR tlocal[20];
  TCHAR tutc[20];

  Units::TimeToText(tlocal, (int)TimeLocal((int)(GPS_INFO.Time))),
  Units::TimeToText(tutc, (int)GPS_INFO.Time);
  wsprintf(time_temp, _T("@%s (UTC %s)"), tlocal, tutc);
