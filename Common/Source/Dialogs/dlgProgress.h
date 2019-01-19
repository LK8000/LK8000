/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgProgress.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 novembre 2014, 17:36
 */

#ifndef DLGPROGRESS_H
#define	DLGPROGRESS_H

void CreateProgressDialog(const TCHAR *text);
void CloseProgressDialog();
void ProgressDialogText(const TCHAR* text);

#endif	/* DLGPROGRESS_H */
