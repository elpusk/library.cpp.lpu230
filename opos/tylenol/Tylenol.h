#pragma once

#include "cmdset.h"
#include "TTR_DevHid.h"
#include <vector>
#include <string>

using namespace std;

class CTylenol
{
public:
	typedef	vector< _tstring >		DevListType;

	enum enumErrorCode{
		ErrorOpen,
		ErrorNoDevice,
		ErrorSendCmd,
		ErrorGetResponse,
		ErrorUnknownResponse,
		ErrorCmdGotoBoot,
		ErrorCmdApply,
		ErrorCmdLeave,
		ErrorCmdEnter
	};

public:
	virtual ~CTylenol(void);

	static void OpenDevice();
	static void EnterConfigMode();
	static void LeaveConfigMode();
	static void ApplyConfigMode();
	static void GotoBootLoaderConfigMode();
	static void CloseDevice();
	static void SendRequest( const MSR_HostCmd Cmd, const unsigned char cSub,const INT32 nData, const unsigned char *sData, unsigned char *sResp=NULL );
	static void SendRequest( const MSR_HOST_PACKET & Req, PMSR_HOST_PACKET Resp = NULL );
	static void GotoBoot( const _tstring & sDevPath );
	static void GotoBoot();
	static void PrintError( enumErrorCode code );

	static DevListType & GetDeviceList();

	static DevListType & GetDeviceList( int nVid, int nPid, int nInf = -1 );

	static void OpenDevice( const _tstring & sDevPath );

	static void EnableCuiMode( bool bEnable = true ){	m_bIsCuiMode = bEnable;		}

	static HANDLE GetHandle(){	return m_HidDevice.GetHandle(); }

protected:
private:
	CTylenol(void);
	CTylenol( const CTylenol & );

	static CTTR_DeviceHid m_HidDevice;

	static vector< _tstring > m_vDevPath;

	static bool m_bIsCuiMode;
};

