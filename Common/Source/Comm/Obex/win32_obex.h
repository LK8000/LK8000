/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   win32_obex.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 01 novembre 2013, 16:15
 */

#ifndef _win32_obex_h__
#define _win32_obex_h__

#include <ocidl.h>

#ifndef _COM_interface
#define _COM_interface struct
#endif

#define OBFTP_MAX_STRING 256
#define OBEX_TYPE_UNICODE 0x00
#define OBEX_TYPE_BYTESEQ 0x40
#define OBEX_TYPE_BYTE 0x80
#define OBEX_TYPE_DWORD 0xc0
#define OBEX_HID_CONNECTIONID (0x0b | OBEX_TYPE_DWORD)


typedef _COM_interface IDeviceEnum *LPIDEVICEENUM;
typedef _COM_interface IPropertyBagEnum *LPIPROPERTYBAGENUM;
typedef _COM_interface IPropertyBag *LPIPROPERTYBAG;
typedef _COM_interface IObexService *LPIOBEXSERVICE;
typedef _COM_interface IObexDevice *LPIOBEXDEVICE;
typedef _COM_interface IHeaderCollection *LPIHEADERCOLLECTION;
typedef _COM_interface IHeaderEnum *LPIHEADERENUM;


typedef struct _OBEX_HEADER {
	byte bId;
	union {
		LPWSTR pszData;
		DWORD dwData;
		byte bData;
		struct {
			DWORD dwSize;
			byte *pbaData;
		} ba;
	} value;
} OBEX_HEADER;


#undef INTERFACE

const CLSID CLSID_Obex = {0x30a7bc00,0x59b6, 0x40bb, {0xaa,0x2b,0x89,0xeb,0x49,0xef,0x27,0x4e}};
const IID IID_IObex = {0x0C5A5B12,0x2979,0x42D1, {0x9E,0x15,0xA6,0x3E,0x34,0x38,0x3B,0x58}};
#define INTERFACE IObex
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Initialize)(THIS) PURE;
	STDMETHOD(Shutdown)(THIS) PURE;
	STDMETHOD(EnumDevices)(THIS_ IDeviceEnum**, REFCLSID) PURE;
	STDMETHOD(EnumTransports)(THIS_ IPropertyBagEnum**) PURE;
    STDMETHOD(RegisterService)(THIS_ IPropertyBag*,IObexService**)PURE;
	STDMETHOD(BindToDevice)(THIS_ IPropertyBag*, IObexDevice**) PURE;
	STDMETHOD(StartDeviceEnum)(THIS) PURE;
	STDMETHOD(StopDeviceEnum)(THIS) PURE;
};
#undef INTERFACE

#define INTERFACE IDeviceEnum
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
    STDMETHOD(Next)(THIS_ ULONG, IObexDevice**,ULONG*) PURE;
	STDMETHOD(Skip)(THIS_ ULONG) PURE;
	STDMETHOD(Reset)(THIS) PURE;
	STDMETHOD(Clone)(THIS_ IDeviceEnum**) PURE;
};
#undef INTERFACE

#define INTERFACE IObexDevice
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Connect)(THIS_ LPCWSTR, DWORD, IHeaderCollection*) PURE;
	STDMETHOD(Disconnect)(THIS_ IHeaderCollection*) PURE;
    STDMETHOD(Get)(THIS_ IHeaderCollection*, IStream**) PURE;
    STDMETHOD(Put)(THIS_ IHeaderCollection*, IStream**) PURE;
    STDMETHOD(Abort)(THIS_ IHeaderCollection*) PURE;
    STDMETHOD(SetPath)(THIS_ LPCWSTR, DWORD, IHeaderCollection*) PURE;
    STDMETHOD(EnumProperties)(THIS_ REFIID, void **) PURE;
    STDMETHOD(SetPassword)(THIS_ LPCWSTR) PURE;
    STDMETHOD(BindToStorage)(THIS_ DWORD, IStorage**) PURE;
};
#undef INTERFACE

const CLSID CLSID_HeaderCollection = {0x30a7bc01,0x59b6,0x40bb, {0xaa,0x2b,0x89,0xeb,0x49,0xef,0x27,0x4e}};
const IID IID_IHeaderCollection = {0x6561D66B,0x8CC1,0x49F9, {0x80,0x71,0x63,0x2D,0x28,0x8E,0xDA,0xF3}};
#define INTERFACE IHeaderCollection
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(AddByteArray)(THIS_ byte, unsigned long, byte*) PURE;
	STDMETHOD(AddLong)(THIS_ byte, unsigned long) PURE;
	STDMETHOD(AddByte)(THIS_ byte, byte) PURE;
	STDMETHOD(AddUnicodeString)(THIS_ byte, LPCWSTR) PURE;
	STDMETHOD(Remove)(THIS_  byte) PURE;
	STDMETHOD(RemoveAll)(THIS) PURE;
	STDMETHOD(AddCount)(THIS_  unsigned long) PURE;
	STDMETHOD(AddName)(THIS_ LPCWSTR) PURE;
	STDMETHOD(AddType)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(AddLength)(THIS_ unsigned long) PURE;
	STDMETHOD(AddTimeOld)(THIS_ unsigned long) PURE;
	STDMETHOD(AddTime)(THIS_ FILETIME*) PURE;
	STDMETHOD(AddDescription)(THIS_ LPCWSTR) PURE;
	STDMETHOD(AddTarget)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(AddHTTP)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(AddBody)(THIS_ unsigned long, byte*pData) PURE;
	STDMETHOD(AddEndOfBody)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(AddWho)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(AddConnectionId)(THIS_ unsigned long) PURE;
	STDMETHOD(AddAppParams)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(AddObjectClass)(THIS_ unsigned long, byte*) PURE;
	STDMETHOD(EnumHeaders)(THIS_ IHeaderEnum**) PURE;
};
#undef INTERFACE

EXTERN_C const IID IID_IHeaderEnum;
#define INTERFACE IHeaderEnum
DECLARE_INTERFACE_(INTERFACE, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(Next)(THIS_ ULONG, OBEX_HEADER**, ULONG*) PURE;
	STDMETHOD(Skip)(THIS_ ULONG) PURE;
	STDMETHOD(Reset)(THIS) PURE;
	STDMETHOD(Clone)(THIS_ IHeaderEnum**) PURE;

};
#undef INTERFACE

#endif // _win32_obex_h__
