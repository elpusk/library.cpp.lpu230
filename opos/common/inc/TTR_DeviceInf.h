#ifndef __DEVICE_INTERFACE_H_
#define __DEVICE_INTERFACE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// this class works only windows2000 ,Xp or later
// Use compiler /Gz option
#include <setupapi.h>
#include <tchar.h>


//////////////////////////////////////////////////////////////////////////////
// class CDevInterfaceObj
//
class CDevInterfaceObj
{
public:
	CDevInterfaceObj(GUID* pClassGuid, PDWORD status);
	~CDevInterfaceObj(void);
	GUID* GetGuid(void)      { return &m_Guid; }
	HDEVINFO GetHandle(void) { return m_hInfo; }

protected:
	HDEVINFO m_hInfo;
	GUID m_Guid;
};

//////////////////////////////////////////////////////////////////////////////
// class CDevInterface
//
class CDevInterface
{
public:
	CDevInterface( CDevInterfaceObj* pClassObject, DWORD Index, PDWORD Error );
	~CDevInterface(void);
	TCHAR* DevicePath(void);

protected:
	CDevInterfaceObj *m_Class;
	SP_DEVICE_INTERFACE_DATA m_Data;
	PSP_INTERFACE_DEVICE_DETAIL_DATA m_Detail;
};

#endif	//#ifndef __DEVICE_INTERFACE_H_
