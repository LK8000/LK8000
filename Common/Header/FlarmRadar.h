#ifndef FLARMRADAR_H
#define FLARMRADAR_H


#define NO_VARIO_COLORS 9 

#define NO_TRACE_PTS 5000
#define GC_TRACE_TIME_SKIP 2

typedef struct
{
	double fLat;
	double fLon;
	double fAlt;
//	double fIntegrator;
	int iColorIdx;
} LastPositions;




#endif

