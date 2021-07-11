/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef _DLGFLARMIGCDOWNLOAD_H_
#define _DLGFLARMIGCDOWNLOAD_H_

  ListElement* dlgIGCSelectListShowModal(  DeviceDescriptor_t *d) ;
  void StartIGCReadThread();
  void StopIGCReadThread(void) ;
  void LeaveBinModeWithReset(DeviceDescriptor_t *d);
#endif


