/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Sizes.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/
#ifndef SIZES_H
#define SIZES_H

/*
 *  THIS FILE IS INCLUDED AUTOMATICALLY BY DEFINES.H
 */

// Number of InfoBoxes
#define NUMDATAOPTIONS_MAX                      156


#define DISTANCE_ROUNDING 20.0
// Rounding of task distances for entry (sector radius etc)
// 10.0 means rounding to 0.1 user units

#define MINFREESTORAGE 500
// 500 kb must be free for logger to be active this is based on rough
// estimate that a long flight will detailed logging is about 200k,
// and we want to leave a little free.

// max length airspace and waypoint names
#define NAME_SIZE 30
#define EXT_NAMESIZE (NAME_SIZE+16+1)

// define max string length for substring search in airspace description
#define EXT_SEARCH_SIZE 1024
// Do not change, it is fixed in AddSnailPoint.
#define NUMSNAILCOLORS 15

// max URL length
#define MAX_URL_LEN  50

// arbitrary, for setting a limit
#define MAX_NMEA_LEN		160
#define MAX_NMEA_PARAMS		40

// max length of waypoint comment names
#define COMMENT_SIZE 250


// Max number of Infobox groups configurables, should be 8 +1?
#define MAXINFOWINDOWS 14

#define POLARSIZE 3


// task points enlarged from 10 to 20 and then from 20 to 50
#define MAXTASKPOINTS 50
#define MAXSTARTPOINTS 20

#define LONGTRAILSIZE 600
// 1000 points at 3.6 seconds average = one hour
#define TRAILSIZE 1000
// short trail is 10 minutes approx
#define TRAILSHRINK 5


// number of points along final glide to scan for terrain
#define NUMFINALGLIDETERRAIN 30

// ratio of border size to trigger shape cache reload
#define BORDERFACTOR 1.4

// maximum number of topologies
#define MAXTOPOLOGY 20

// timeout in quarter seconds of menu button
#define MENUTIMEOUTMAX 8*4

// invalid value for terrain, we can store inside terrain altitude being unsigned short
#define TERRAIN_INVALID 32767

#define NUMAIRSPACECOLORS 17
#ifdef HAVE_HATCHED_BRUSH
#define NUMAIRSPACEBRUSHES 8
#else
#define NUMAIRSPACEBRUSHES NUMAIRSPACECOLORS
#endif

#define NUMBUTTONLABELS 16

// change 300 to 1023 to fix problem with airspace lines and long comments
#define READLINE_LENGTH 1024

// Size of Status message cache - Note 1000 messages may not be enough...
// TODO If we continue with the reading one at a time - then consider using
// a pointer structure and build on the fly, thus no limit, but also only
// RAM used as required - easy to do with struct above - just point to next.
// (NOTE: This is used for all the caches for now - temporary)
#define MAXSTATUSMESSAGECACHE 1000

#define MAXNEARESTTOPONAME 50

#define MAXISOLINES 32

#define ERROR_TIME 1.0e6

#endif // SIZES_H
