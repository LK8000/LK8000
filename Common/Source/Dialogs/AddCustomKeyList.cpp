/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgCustomKeys.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKInterface.h"


void AddCustomKeyList( DataFieldEnum* dfe) {

	// Careful, order must respect the enum list in lk8000.h CustomKeyMode_t

	for(int i = 0; i < ckTOP; i++ ) {
		dfe->addEnumTextNoLF(MsgToken(CustomKeyLabel[i].Name));
	}

    dfe->Sort(0);

}



