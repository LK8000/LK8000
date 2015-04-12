#if !defined(AFX_AIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_AIRSPACE_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define OTHER                           0
#define RESTRICT                        1
#define PROHIBITED                      2
#define DANGER                          3
#define CLASSA				4
#define CLASSB				5
#define CLASSC				6
#define CLASSD				7
#define	NOGLIDER			8
#define CTR                             9
#define WAVE				10
#define AATASK				11
#define CLASSE				12
#define CLASSF				13
#define CLASSG				14
#define CLASSTMZ            		15
#define CLASSRMZ			16
#define AIRSPACECLASSCOUNT              17

#define ALLON 0
#define CLIP 1
#define AUTO 2
#define ALLBELOW 3
#define INSIDE 4
#define ALLOFF 5


void ReadAirspace(void);


//*******************************************************************************
// experimental: new dialog based warning system


#define OUTSIDE_CHECK_INTERVAL 4

#endif
