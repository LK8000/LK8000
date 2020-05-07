/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef _DLGFLXIGCDOWNLOAD_H_
#define _DLGFLXIGCDOWNLOAD_H_

  ListElement* dlgLX_IGCSelectListShowModal(  void) ;
  void AddElement(TCHAR Line1[], TCHAR Line2[]);



  enum error_states
  {
   REC_NO_ERROR      ,
   REC_TIMEOUT_ERROR ,
   REC_CRC_ERROR     ,
   REC_ABORTED       ,
   FILENAME_ERROR    ,
   FILE_OPEN_ERROR   ,
   IGC_RECEIVE_ERROR ,
   REC_NO_DEVICE     ,
   REC_NOMSG         ,
  };

#endif
