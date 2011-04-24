/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef	DEVIMI_H
#define	DEVIMI_H
 
#include "devBase.h"

class CDevIMI : public DevBase
{
  // IMI interface
  static bool Connect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);
  static bool Disconnect(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[]);
  static bool DeclarationWrite(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[]);
  
  // LK interface
  static const TCHAR *GetName();
  static BOOL DeclareTask(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufSize, TCHAR errBuf[]);
  static BOOL Install(PDeviceDescriptor_t d);
  
public:
  static bool Register();
};

#endif /* DEVIMI_H */
