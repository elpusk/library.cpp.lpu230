#pragma once

#include <Windows.h>

namespace SYNC
{
	class CMutex
	{
	public:
		CMutex() : m_hMutex(NULL), m_IniOk(false)
		{
			m_hMutex = ::CreateMutex( NULL, FALSE, NULL );
			if( m_hMutex )
				m_IniOk = true;
		}

		~CMutex()
		{
			if( m_IniOk )
				::CloseHandle( m_hMutex );
		}

		bool Lock( DWORD dwTimeOut = 0)
		{
			if( WAIT_OBJECT_0 == ::WaitForSingleObject( m_hMutex, dwTimeOut ) )
				return true;
			else
				return false;
		}

		void Unlock()
		{
			::ReleaseMutex( m_hMutex );
		}

		bool IsIniOk(){	return m_IniOk;	}

	private:
		HANDLE m_hMutex;
		bool m_IniOk;
	};

	class CEventCase{
	public:

		CEventCase() : m_hEvent(NULL){}

		~CEventCase()
		{
			if( m_hEvent )
				::CloseHandle( m_hEvent );
		}

		CEventCase( HANDLE hEvent ) : m_hEvent(hEvent)	{}

		void Set()
		{
			if( m_hEvent )
				::SetEvent( m_hEvent );
		}

		void Reset()
		{
			::ResetEvent( m_hEvent );
		}

		HANDLE GetHandle(){	return m_hEvent;	}

		void SetHandle( HANDLE hEvent )
		{	
			if( m_hEvent )
				::CloseHandle( m_hEvent );

			m_hEvent = hEvent;	
		}

	private:
		HANDLE m_hEvent;
	};

}
