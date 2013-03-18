#if !defined(AFX_TASK_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_TASK_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define CIRCLE 0
#define SECTOR 1

typedef struct _START_POINT
{
  int Index;
  double OutBound;
  double SectorStartLat;
  double SectorStartLon;
  double SectorEndLat;
  double SectorEndLon;
  POINT	 Start;
  POINT	 End;
  bool Active;
  bool InSector;
} START_POINT;


typedef struct _TASK_POINT
{
  int Index;
  double InBound;
  double OutBound;
  double Bisector;
  double Leg;
  double SectorStartLat;
  double SectorStartLon;
  double SectorEndLat;
  double SectorEndLon;
  POINT	 Start;
  POINT	 End;		
  int	 AATType;
  double AATCircleRadius;
  double AATSectorRadius;
  double AATStartRadial;
  double AATFinishRadial;
  double AATStartLat;
  double AATStartLon;
  double AATFinishLat;
  double AATFinishLon;
  POINT	 AATStart;
  POINT	 AATFinish;
  double AATTargetOffsetRadius;
  double AATTargetOffsetRadial;
  double AATTargetLat;
  double AATTargetLon;
  POINT	 Target;
  bool   AATTargetLocked;
  bool	 OutCircle;
}TASK_POINT;

typedef TASK_POINT Task_t[MAXTASKPOINTS +1];
typedef START_POINT Start_t[MAXSTARTPOINTS +1];

typedef struct _TASKSTATS_POINT
{
  double LengthPercent;
  double IsoLine_Latitude[MAXISOLINES];
  double IsoLine_Longitude[MAXISOLINES];
  bool IsoLine_valid[MAXISOLINES];
  POINT IsoLine_Screen[MAXISOLINES];
}TASKSTATS_POINT;

typedef TASKSTATS_POINT TaskStats_t[MAXTASKPOINTS +1];

extern bool TaskModified;
extern bool TargetModified;
extern TCHAR LastTaskFileName[MAX_PATH];

void ReplaceWaypoint(int index);
void InsertWaypoint(int index, unsigned short append=0);
void SwapWaypoint(int index);
void RemoveWaypoint(int index);
void RemoveTaskPoint(int index);
void FlyDirectTo(int index);
double AdjustAATTargets(double desired);
void RefreshTaskWaypoint(int i);
void RefreshTask(void);
void CalculateTaskSectors(void);
void CalculateAATTaskSectors(void);

void guiStartLogger(bool noAsk = false);
void guiStopLogger(bool noAsk = false);
void guiToggleLogger(bool noAsk = false);

void LoadNewTask(LPCTSTR FileName);
bool LoadCupTask(LPCTSTR FileName);
void SaveTask(TCHAR *FileName);
void DefaultTask(void);
void ClearTask(void);
void RotateStartPoints(void);
bool ValidTaskPoint(int i);
bool ValidWayPoint(int i);
bool ValidNotResWayPoint(int i);
bool ValidResWayPoint(int i);

double FindInsideAATSectorRange(double latitude,
                                double longitude,
                                int taskwaypoint, 
                                double course_bearing,
                                double p_found);
double FindInsideAATSectorDistance(double latitude,
                                double longitude,
                                int taskwaypoint, 
                                double course_bearing,
                                double p_found=0.0);

double DoubleLegDistance(int taskwaypoint,
                         double longitude,
                         double latitude);

void CalculateAATIsoLines(void);

void SaveDefaultTask(void);

const WAYPOINT* TaskWayPoint(size_t idx);
#endif
