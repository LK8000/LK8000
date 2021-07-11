/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgProgress.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 novembre 2014, 17:36
 */

#ifndef DLG_IGC_PROGRESS_H
#define	DLG_IGC_PROGRESS_H

void CreateIGCProgressDialog();
void CloseIGCProgressDialog();
void IGCProgressDialogText(const TCHAR* text);
void OnIGCAbortClicked(WndButton* pWnd);
#endif	/* DLG_IGC_PROGRESS_H */
