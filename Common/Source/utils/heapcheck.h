/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
/*____________________________________________________________________________*/

#ifndef __heapcheck_h__
#define __heapcheck_h__

/*____________________________________________________________________________*/

/* include header for heap allocation checker */

#ifdef HC_DMALLOC
  #include "../../../../dmalloc/dmalloc.h"
#endif

#ifdef HC_DUMA
  #include "../../../../duma/duma.h"
#endif

#endif /* __heapcheck_h__ */
