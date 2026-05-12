
#pragma once

#include <list>
#include <string>
#include <tchar.h>
#include "ng_DDL_Def.h"

using namespace std;

typedef int (CALLBACK* DEV_GetListType)( list<wstring> & ListDev, int nVid, int nPid, int nInf  );
typedef HANDLE	(CALLBACK* DEV_OpenType)( const wstring & sPath );
typedef BOOL (CALLBACK* DEV_CloseType)( HANDLE hDev );
typedef int (CALLBACK* DEV_TxType)( HANDLE hDev, LPBYTE lpData, int nTx, int nOutReportSize  );
typedef int (CALLBACK* DEV_RxType)( HANDLE hDev, LPBYTE lpData, int nRx, int nInReportSize  );
typedef BOOL (CALLBACK* DEV_TxRxType)( HANDLE hDev, LPBYTE lpTxData, int nTx, int nOutReportSize, LPBYTE lpRxData, int * nRx,  int nInReportSize   );

typedef DWORD (WINAPI* DDL_probeType)( EnumDDLProperty Property,DWORD dwBufSize,LPVOID lpProperty );
typedef HANDLE (WINAPI* DDL_openType)( LPCTSTR szDevicePath);
typedef BOOL (WINAPI* DDL_closeType)( HANDLE hDev);
typedef BOOL (WINAPI* DDL_resetType)( LPng_DDL_Parameter	pParam );
typedef DWORD (WINAPI* DDL_writeType)(	LPng_DDL_Parameter	pParam );
typedef DWORD (WINAPI* DDL_readType)(	LPng_DDL_Parameter	pParam );
