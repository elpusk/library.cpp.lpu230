#pragma once

#include <windows.h>
#include <Userenv.h>
#include <Wtsapi32.h>

namespace secondgeneration
{
class CWarpHandle
{
public:
	explicit CWarpHandle( HANDLE hHandle )
	{
		m_hHandle = hHandle;
	}
	~CWarpHandle()
	{
		try{
			if( m_hHandle )
				CloseHandle( m_hHandle );
			}
		catch(...){}

	}

	HANDLE GetHandle()
	{
		return m_hHandle;
	}

	BOOL Reset(){	return ResetEvent( m_hHandle );}
	BOOL Set(){		return SetEvent( m_hHandle );	}

	DWORD Wait( DWORD dwMilliseconds ){	return WaitForSingleObject( m_hHandle, dwMilliseconds );	}
private:
	HANDLE m_hHandle;

	CWarpHandle();
	CWarpHandle( const CWarpHandle &);
	CWarpHandle & operator=( const CWarpHandle &);

};

class CWarpEvtLogHandle{
public:
	explicit CWarpEvtLogHandle( HANDLE hHandle )
	{
		m_hHandle = hHandle;
	}
	~CWarpEvtLogHandle()
	{
		try{
			if( m_hHandle )
				DeregisterEventSource( m_hHandle );
			}
		catch(...){}

	}

	HANDLE GetHandle()
	{
		return m_hHandle;
	}

private:
	HANDLE m_hHandle;

	CWarpEvtLogHandle();
	CWarpEvtLogHandle( const CWarpEvtLogHandle &);
	CWarpEvtLogHandle & operator=( const CWarpEvtLogHandle &);
};

class CWarpRegKeyHandle
{
public:
	explicit CWarpRegKeyHandle( const HKEY & hKey )
	{
		memcpy( &m_hKey, &hKey, sizeof(m_hKey) );
	}

	~CWarpRegKeyHandle()
	{
		try{
			RegCloseKey( m_hKey );
		}
		catch(...){}
	}

	HKEY & GetHandle()
	{
		return m_hKey;
	}


private:
	HKEY m_hKey;

	CWarpRegKeyHandle();
	CWarpRegKeyHandle( const CWarpRegKeyHandle& );
	CWarpRegKeyHandle & operator=( const CWarpRegKeyHandle& );
};

class CWarpEnvBlock
{
public:
	explicit CWarpEnvBlock( LPVOID lpEnv )
	{
		m_lpEnv= lpEnv;
	}

	~CWarpEnvBlock()
	{
		try{
			if( m_lpEnv )
				DestroyEnvironmentBlock( m_lpEnv );
		}
		catch(...){}
	}

	LPVOID GetBlock()
	{
		return m_lpEnv;
	}


private:
	LPVOID m_lpEnv;

	CWarpEnvBlock();
	CWarpEnvBlock( const CWarpEnvBlock& );
	CWarpEnvBlock & operator=( const CWarpEnvBlock& );
};

#include <winsvc.h>

class Cns_WarpServiceHandle
{
public:
	explicit Cns_WarpServiceHandle( SC_HANDLE hHandle )
	{
		m_hHandle = hHandle;
	}
	~Cns_WarpServiceHandle()
	{
		try{
			if( m_hHandle )
				CloseServiceHandle( m_hHandle );
			}
		catch(...){}

	}

	SC_HANDLE GetHandle()
	{
		return m_hHandle;
	}

private:
	SC_HANDLE m_hHandle;

	Cns_WarpServiceHandle();
	Cns_WarpServiceHandle( const Cns_WarpServiceHandle &);

};

#include <WtsApi32.h>

	class CWarpWTSMem
	{
	public:
		explicit CWarpWTSMem( PWTS_SESSION_INFO pSessionInfo )
		{
			m_pSessionInfo= pSessionInfo;
		}

		~CWarpWTSMem()
		{
			try{
				if( m_pSessionInfo )
					WTSFreeMemory( m_pSessionInfo );
			}
			catch(...){}
		}

		PWTS_SESSION_INFO GetSessionInfo()
		{
			return m_pSessionInfo;
		}


	private:
		PWTS_SESSION_INFO m_pSessionInfo;

		CWarpWTSMem();
		CWarpWTSMem( const CWarpWTSMem& );
		CWarpWTSMem & operator=( const CWarpWTSMem& );
	};

}