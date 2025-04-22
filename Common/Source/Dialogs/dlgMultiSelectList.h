/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgMultiSelectList.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 13 april 2025
 */

#ifndef DIALOGS_DLMULTISELECTLIST_H
#define DIALOGS_DLMULTISELECTLIST_H

#include "Event/object_identifier.h"


namespace DlgMultiSelect {

void ShowModal();
void AddItem(im_object_variant&& elmt, double distance);
int GetItemCount();

} // namespace DlgMultiSelect


#endif // DIALOGS_DLMULTISELECTLIST_H
