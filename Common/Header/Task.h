#if !defined(AFX_TASK_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_TASK_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////
// Task Type
#define TSK_DEFAULT     0
#define TSK_AAT         1
#define TSK_GP          2 // Race To Goal in PG Mode
/////////////////

#define CIRCLE 0
#define SECTOR 1
#define DAe    2 // only Exist for not AAT and Not PGTask.
#define LINE   3 // only Used for Save Start and Finish Type in xml file.
#define CONE   4 // Only Used In PG Optimized Task
#define ESS_CIRCLE   5 // Only Used In PG Optimized Task

typedef struct _START_POINT
{
  int Index;
  double OutBound;
  bool Active;
  bool InSector;
} START_POINT;


struct TASK_POINT
{
  int Index;
  double InBound;
  double OutBound;
  double Bisector;
  double Leg;
  int	 AATType;
  double AATCircleRadius;
  double AATSectorRadius;
  double AATStartRadial;
  double AATFinishRadial;
  double AATTargetOffsetRadius;
  double AATTargetOffsetRadial;
  double AATTargetLat;
  double AATTargetLon;
  POINT	 Target;
  bool   AATTargetLocked;
  bool	 OutCircle;
  double AATTargetAltitude;
  double PGConeSlope; // Slope Ratio for PG Cone Turn point
  double PGConeBase; // Base Altitude of Cone Turn Point
  double PGConeBaseRadius; // radius At Base Altitude of Cone Turn Point
};

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
BOOL CheckFAILeg(double leg, double total);

void CalculateTaskSectors(void);
void CalculateTaskSectors(int Idx);

void CalculateAATTaskSectors(void);

void guiStartLogger(bool noAsk = false);
void guiStopLogger(bool noAsk = false);
void guiToggleLogger(bool noAsk = false);

bool LoadCupTask(LPCTSTR FileName);
bool LoadGpxTask(LPCTSTR FileName);
void SaveTask(const TCHAR *FileName);
void DefaultTask(void);
void ClearTask(void);
void RotateStartPoints(void);
bool ValidTaskPoint(int i);
bool ValidWayPoint(int i);
bool ValidNotResWayPoint(int i);
bool ValidResWayPoint(int i);
bool ValidStartPoint(size_t i);

int GetTaskSectorParameter(int TskIdx, int *SecType, double *SecRadius);

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

void ReverseTask();
#endif
