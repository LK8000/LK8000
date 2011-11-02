/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: ClimbAverageCalculator.cpp,v 8.2 2010/12/10 23:46:41 root Exp root $
*/

#include "ClimbAverageCalculator.h"

#include "utils/heapcheck.h"

ClimbAverageCalculator::ClimbAverageCalculator(void)
{
	newestValIndex = -1;

	for(int i=0; i<MAX_HISTORY; i++)
	{
		history[i].altitude = -99999;
		history[i].time = -99999;
	}
}

ClimbAverageCalculator::~ClimbAverageCalculator(void)
{
}


double ClimbAverageCalculator::GetAverage(double curTime, double curAltitude, int averageTime)
{	  
  double result = 0;
  int i;
  int bestHistory;
  int oldestValIndex = 0;
  
  
  newestValIndex = newestValIndex < MAX_HISTORY-1 ? newestValIndex+1 : 0;
  oldestValIndex = bestHistory =  newestValIndex < MAX_HISTORY-1 ? newestValIndex+1 : 0;
  
  // add the new sample
  history[newestValIndex].time = curTime;
  history[newestValIndex].altitude = curAltitude;
  
  // initialy bestHistory is the current...
  bestHistory = newestValIndex;
  
  // now run through the history and find the best sample for average period within the average time period
  for(i=0; i<MAX_HISTORY; i++)
    {
      if (history[i].time != -99999)
	{
	  if (history[i].time + averageTime >= curTime) // inside the period ?
	    {
	      if (history[i].time < history[bestHistory].time) // is the sample older (and therefore better) than the current found ?
		{
		  bestHistory = i;
		}
	    }
	}
    }	
  
  // calculate the average !
  if (bestHistory != newestValIndex)
    {
      result = (curAltitude - history[bestHistory].altitude) / (curTime - history[bestHistory].time);
    }
  
  return result;
}
