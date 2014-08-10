/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   CObexPush.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 1 novembre 2013, 20:04
 */
#ifndef COBEXPUSH_H
#define	COBEXPUSH_H

#ifdef UNDER_CE
#include "win32_obex.h"
#include "boost/noncopyable.hpp" 
#include <list>

class CObexPush : boost::noncopyable {
	typedef std::list<IObexDevice*> ObexDeviceList_t;
public:
	
	CObexPush();
	virtual ~CObexPush();
	
	bool Startup();
	void Shutdown();
	
	size_t LookupDevice();
	
	bool GetDeviceName(size_t DeviceIdx, TCHAR* szFileName, size_t cb);
	bool SendFile(size_t DeviceIdx, const TCHAR* szFileName);
	
	void DumpsDeviceProperty(size_t DeviceIdx);
	
private:
	void ClearDeviceList();
	
	IObex* _pObex;
	ObexDeviceList_t _LstDevice;
	int _SavedBtState;
};
#endif
#endif	/* COBEXPUSH_H */

