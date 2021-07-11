/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"



double AdjustAATTargets(double desired) {
  int i, istart, inum;
  double av=0;
  istart = max(1,ActiveTaskPoint);
  inum=0;

  LockTaskData();
  for(i=istart;i<MAXTASKPOINTS-1;i++)
    {
      if(ValidTaskPoint(i)&&ValidTaskPoint(i+1) && !Task[i].AATTargetLocked)
	{
          Task[i].AATTargetOffsetRadius = max(-1.0,min(1.0,
                                          Task[i].AATTargetOffsetRadius));
	  av += Task[i].AATTargetOffsetRadius;
	  inum++;
	}
    }
  if (inum>0) {
    av/= inum;
  }
  if (fabs(desired)>1.0) {
    // don't adjust, just retrieve.
    goto OnExit;
  }

  // TODO accuracy: Check here for true minimum distance between
  // successive points (especially second last to final point)

  // Do this with intersection tests

  desired = (desired+1.0)/2.0; // scale to 0,1
  av = (av+1.0)/2.0; // scale to 0,1

  for(i=istart;i<MAXTASKPOINTS-1;i++)
    {
      if((Task[i].Index >=0)&&(Task[i+1].Index >=0) && !Task[i].AATTargetLocked)
	{
	  double d = (Task[i].AATTargetOffsetRadius+1.0)/2.0;
          // scale to 0,1

          if (av>0.01) {
            d = desired;
	    // 20080615 JMW
	    // was (desired/av)*d;
	    // now, we don't want it to be proportional
          } else {
            d = desired;
          }
          d = min(1.0, max(d, 0.0))*2.0-1.0;
          Task[i].AATTargetOffsetRadius = d;
	}
    }
 OnExit:
  UnlockTaskData();
  return av;
}
