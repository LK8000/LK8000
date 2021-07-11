/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Topology.h"

using std::min;

bool CheckRectOverlap(const RECT *rc1, const RECT *rc2) {
  if(rc1->left >= rc2->right) return(false);
  if(rc1->right <= rc2->left) return(false);
  if(rc1->top >= rc2->bottom) return(false);
  if(rc1->bottom <= rc2->top) return(false);
  return(true);
}
