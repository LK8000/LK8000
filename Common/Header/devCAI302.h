
#ifndef	DEVCAI302_H
#define	DEVCAI302_H



BOOL cai302Register(void);
BOOL cai302PutBugs(PDeviceDescriptor_t d, double Bugs);
BOOL cai302PutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL cai302PutBallast(PDeviceDescriptor_t d, double Ballast);
#endif
