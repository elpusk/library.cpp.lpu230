#pragma once

#include "resource.h"       // 주 기호입니다.

#include "Sync.h"
#include "FireEvent.h"

#include <OposLock.hi>
#include <OposLock.h>

using namespace ATL;
using namespace SYNC;

#include <Deque>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <map>
#include "Log.h"

#include "sg_warpobject.h"
#include "Dll.h"
#include "GlobalVar.h"

//#define	LPU230LOCK_SO_VERSION		1000000//1008019		// initial version
#define	LPU230LOCK_SO_VERSION_1_14_1		1014001//1.14.1		// initial version
//#define	LPU230LOCK_SO_VERSION		1014002//1.14.2		// support lpu238
#define	LPU230LOCK_SO_VERSION		1014030//1.14.30		// using cf2

class CSoLock
{
public:
	typedef	std::vector<unsigned char>		KeyDataType;
	typedef	std::shared_ptr<KeyDataType>	PtrKeyDataType;
	typedef	std::deque<PtrKeyDataType>		KeyDataQType;

	enum{
		CONST_MAX_KEY = 8,
		CONST_SYNC_WAITE_TIME = 2000
	};

	class CShare{
	public:
		typedef	std::shared_ptr< secondgeneration::CWarpHandle > type_ptr_event;
		typedef	std::deque<type_ptr_event> type_queue_notify_event;
		typedef	vector< _tstring >	type_dev_list;
		typedef	std::map<CSoLock*, DWORD>				type_map_result_buffer_index;

		typedef	std::set< CSoLock* >	type_set_pointer_so;

	public:
		CShare() : 
			m_n_open_counter(0)
			, m_h_dev(NULL)
			, m_bDeviceIsUart(false)
			, m_b_is_dll_ok(false)
			, m_pLog(NULL)
			, m_dw_result_buffer_index(-1)
		{
			bool b_load_ok(true);

			m_pLog = CLog::GetLog();
			ATLASSERT(m_pLog!=NULL);

			do{
				if( CDll::get_instance( CGlobalVar::GetInstance()->get_ibutton_dll_path() ).is_load_ok() )
					continue;
				//
				b_load_ok = false;
			}while(0);

			m_b_is_dll_ok = b_load_ok = CDll::get_instance().is_load_ok();

			CDll::get_instance().dll_on();
		}

		~CShare()
		{
			//CDll::get_instance().dll_off();
			for_each( begin(m_q_evt), end(m_q_evt), [=]( type_queue_notify_event::value_type c ){
				c->Set();
			});
		}
		//
		static CShare & get_instance()
		{
			static CShare share;
			return share;
		}

		void en_queue(type_ptr_event &evt )
		{
			m_q_locker.Lock( 2000 );
			m_q_evt.push_back( evt );
			m_q_locker.Unlock();
		}

		type_ptr_event de_queue()
		{
			m_q_locker.Lock( 2000 );
			type_ptr_event evt(m_q_evt.front());
			m_q_evt.pop_front();
			m_q_locker.Unlock();
			return evt;
		}

		void remove_queue( const type_ptr_event & evt )
		{
			m_q_locker.Lock( 2000 );
			type_queue_notify_event::iterator it = find( begin(m_q_evt), end(m_q_evt), evt );
			if( it != end(m_q_evt) ){
				m_q_evt.erase( it );
			}
			m_q_locker.Unlock();
		}

		void remove_queue()
		{
			m_q_locker.Lock( 2000 );
			while( !m_q_evt.empty() ){
				type_ptr_event evt = m_q_evt.front();
				evt->Set();
				m_q_evt.pop_front();
			}
			m_q_locker.Unlock();
		}

		bool empty()
		{
			bool b_result = false;

			m_q_locker.Lock( 2000 );
			if (m_q_evt.empty()) {
				b_result = true;
				ATLTRACE(L"=== empty()\n");
			}
			else {
				ATLTRACE(L"=== not empty()\n");
			}
			m_q_locker.Unlock();

			return b_result;
		}

		void remove_so_of_set( CSoLock *pLock )
		{
			if( pLock ){
				m_set_locker.Lock(100);
				m_set_so.erase( pLock );
				m_set_locker.Unlock();
			}
		}

		bool Lock_Open( CSoLock *pLock )
		{
			bool b_result(false);

			do{
				if( !open() ){
					m_set_locker.Lock(100);
					m_set_so.insert( pLock );
					m_set_locker.Unlock();
					b_result = true;
					continue;
				}

				type_dev_list & vDevList =  GetDeviceList();

				if( vDevList.empty() ){
					SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Lock_Open : NO DEVICE.\n") );
					continue;
				}

				type_dev_list::iterator iDev = vDevList.begin();

				for( ;iDev != vDevList.end(); ++iDev ){

					if( IsPathNameUart( iDev->c_str() ) ){
						m_bDeviceIsUart = true;
						break;	//exit for
					}
					else{
						m_bDeviceIsUart = false;
					}
				}//end for

				if( !m_bDeviceIsUart ){
					iDev = vDevList.begin();
					ATLTRACE( _T(" + OPEN HID MODE\n") );
				}
				else{
					ATLTRACE( _T(" + OPEN UART MODE\n") );
				}

				m_h_dev = CDll::get_instance().open( iDev->c_str() );
				if( m_h_dev == NULL || m_h_dev == INVALID_HANDLE_VALUE ){
					SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Lock_Open : open.\n") );
					continue;
				}

				DWORD dw_result = CDll::get_instance().enable( m_h_dev );
				if( dw_result != CDll::dll_result_success ){
					CDll::get_instance().close( m_h_dev );
					m_h_dev = NULL;
					SOTrace( true, CLog::LEV_LOW, _T("[ERROR] enable : open.\n") );
					continue;
				}

				dw_result = CDll::get_instance().wait_key_with_callback( m_h_dev, CSoLock::CShare::key_callback_readdone, NULL );
				if( dw_result == CDll::dll_result_error ){
					CDll::get_instance().close( m_h_dev );
					m_h_dev = NULL;
					SOTrace( true, CLog::LEV_LOW, _T("[ERROR] enable : wait_key_with_callback.\n") );
					continue;
				}

				m_dw_result_buffer_index = dw_result;
				SOTrace( true, CLog::LEV_NORMAL, _T("[INFO]wait_key_with_callback = %d\n"), dw_result );

				//save device path
				m_sDevFullPath.Format( _T("%s"), iDev->c_str() );

				if( m_bDeviceIsUart ){
					::Sleep(70);// In Uart mode, need time-delay 100 msec. NOTICE !!!!!!!
				}

				m_set_locker.Lock(100);
				m_set_so.insert( pLock );
				m_set_locker.Unlock();

				b_result = true;
			}while(0);

			return b_result;
		}
		
		bool Lock_Close( CSoLock *pLock )
		{
			bool b_result(false);

			do{
				if( !close() ){
					b_result = true;
					remove_so_of_set( pLock );
					continue;
				}

				CSoLock::CShare::remove_queue();
				remove_so_of_set( pLock );

				CDll::get_instance().close( m_h_dev );
				m_h_dev = NULL;
				b_result = true;
			}while(0);

			return b_result;
		}

		bool is_open()
		{
			bool b_open(false);

			m_open_locker.Lock(100);
			if( m_n_open_counter > 0 ){
				b_open = true;
			}
			m_open_locker.Unlock();

			return b_open;
		}

		bool is_dll_ok()
		{
			return m_b_is_dll_ok;
		}

		KeyDataType get_electronic_key_value_by_binary()
		{
			KeyDataType value;

			m_key_locker.Lock(500);
			value.resize( m_vElectronicKeyValue.size(), 0 );
			copy( begin(m_vElectronicKeyValue), end(m_vElectronicKeyValue), begin(value) );
			m_key_locker.Unlock();

			return value;
		}

		void set_electronic_key_value_by_binary(const KeyDataType & value)
		{
			m_key_locker.Lock(500);
			m_vElectronicKeyValue.resize( value.size(), 0 );
			copy( begin(value), end(value), begin(m_vElectronicKeyValue) ); 
			m_key_locker.Unlock();
		}

		void SOTrace( bool bStemp, CLog::Level nLevel, const TCHAR *_Format, ...)
		{
			CLog *pLog = CLog::GetLog();

			ATLASSERT(pLog!=NULL);

			if( pLog ){
				va_list args;
				va_start(args, _Format);

				int nLen = _vsctprintf( _Format, args )+1;

				TCHAR *buffer = new TCHAR[nLen];

				_vstprintf_s( buffer, nLen, _Format, args );

				pLog->Log( bStemp, nLevel, _T("%s"), buffer );

				va_end(args);

				delete [] buffer;
			}
		}

		static void WINAPI key_callback_readdone(void *p_obj)
		{
			do{
				CSoLock::CShare & obj = CSoLock::CShare::get_instance();
				DWORD dw_index = obj.m_dw_result_buffer_index;
				//
				CSoLock::KeyDataType value( CONST_MAX_KEY, 0 );

				DWORD dw_result = CDll::get_instance().get_data( dw_index, &value[0] );
				if( dw_result == CDll::dll_result_error ){
					continue;
				}

				obj.set_electronic_key_value_by_binary( value );

				// wakeup all waiter in waiting queue.
				while (!obj.empty()) {
					obj.de_queue()->Set();//signal
				}//end while

				obj.m_set_locker.Lock(100);

				std::for_each( begin(obj.m_set_so), end(obj.m_set_so), [&]( CSoLock *pso ){
					if( pso->get_enable() ){
						pso->set_electronic_key_value( pso->get_formatted_electronic_key_value() );
						pso->Fire(LOCK_KP_ELECTRONIC);
					}
				});

				dw_result = CDll::get_instance().wait_key_with_callback( obj.m_h_dev, CSoLock::CShare::key_callback_readdone, NULL );
				if( dw_result != CDll::dll_result_error ){
					obj.m_dw_result_buffer_index = dw_result;
					obj.SOTrace( true, CLog::LEV_NORMAL, _T("[INFO]wait_key_with_callback = %d\n"), dw_result );
				}
				else{
					obj.m_dw_result_buffer_index = -1;
				}

				obj.m_set_locker.Unlock();

			}while(0);
		}

	private:
		bool open()
		{
			bool b_run(false);

			m_open_locker.Lock(100);
			if( m_n_open_counter == 0 )
				b_run = true;

			m_n_open_counter++;
			m_open_locker.Unlock();
			return b_run;
		}

		bool close()
		{
			bool b_run(false);

			m_open_locker.Lock(100);
			if( m_n_open_counter > 0 ){
				if( m_n_open_counter==1)
					b_run = true;

				m_n_open_counter--;
			}

			m_open_locker.Unlock();
			return b_run;
		}

		type_dev_list & GetDeviceList()
		{
			m_v_devPath.clear();

			bool bResult = false;

			// 1. find connected device from HID.
			static list<_tstring> HidDevPathList;
			static INT32 nHidDev = 0;


			do{
				nHidDev = CDll::get_instance().get_list( NULL );
				if( nHidDev <= 0 ){
					ATLTRACE( _T("* NOT FOUND HID DEVICE.\n") );
					continue;
				}
				//
				vector<BYTE> vPaths( nHidDev, 0 );
				nHidDev = CDll::get_instance().get_list( reinterpret_cast<LPTSTR>( &vPaths[0] ) );
				if( nHidDev <= 0 ){
					ATLTRACE( _T("* NOT FOUND HID DEVICE.\n") );
					continue;
				}
				GetStringFromMultiString( HidDevPathList, reinterpret_cast<LPCTSTR>( &vPaths[0] ) );
				m_v_devPath.resize( HidDevPathList.size() );
				copy( HidDevPathList.begin(), HidDevPathList.end(), m_v_devPath.begin() );
			}while(0);

			return m_v_devPath;
		}

		int GetStringFromMultiString( std::list<_tstring> &listStr,LPCTSTR szMultiStr )
		{
			LPCTSTR pDest;
			_tstring stemp;
			INT nCount =0;
			INT nOffset = 0;

			if( szMultiStr == NULL )
				return 0;
			else
				listStr.clear();
			//

			while( szMultiStr[nOffset] != NULL ){

				pDest = &(szMultiStr[nOffset]);
				stemp=pDest;
				listStr.push_back( stemp );

				nOffset += stemp.length()+1;//for passing null termination
				nCount++;
			}//while

			return nCount;
		}

		bool IsPathNameUart( const _tstring & sDevPath )
		{
			bool bResult = false;

			//_T("//./COM")

			if( sDevPath.size() > 3 ){

				if( sDevPath[0] == _T('C') && sDevPath[1] == _T('O') && sDevPath[2] == _T('M') ){
						bResult = true;
				}

			}

			return bResult;
		}

		bool FindReOpen()
		{
			bool bResult = true;

			if( m_h_dev ){
				CDll::get_instance().close( m_h_dev );
				m_h_dev = NULL;
			}

			//find device. & open
			type_dev_list & vDevList =  GetDeviceList();

			if( !vDevList.empty() ){

				type_dev_list::iterator iDev = vDevList.begin();

				for( ;iDev != vDevList.end(); ++iDev ){

					if( IsPathNameUart( iDev->c_str() ) ){
						m_bDeviceIsUart = true;
						break;	//exit for
					}
					else
						m_bDeviceIsUart = false;
				}

				if( !m_bDeviceIsUart )
					iDev = vDevList.begin();

				m_h_dev = CDll::get_instance().open( iDev->c_str() );
				if( m_h_dev != NULL && m_h_dev != INVALID_HANDLE_VALUE ){
					//save device path
					m_sDevFullPath.Format( _T("%s"), iDev->c_str() );
				}
				else
					bResult = false;
			}
			else{
				ATLTRACE( _T(" = FindReOpen : NOT FOUND DEVICE.\n") );
				bResult = false;
			}
			return bResult;
		}

		bool EnQKeyData( const PtrKeyDataType & pKeyData )
		{
			bool bResult = true;
			//
			m_qKeyData.push_back( pKeyData );

			return bResult;
		}

		bool DeQKeyData( PtrKeyDataType & pKeyData )
		{
			bool bResult = true;

			if( !m_qKeyData.empty() ){
				pKeyData = m_qKeyData.front();
				m_qKeyData.pop_front();
			}
			else
				bResult = false;

			return bResult;
		}

	private:
		CMutex m_set_locker;
		type_set_pointer_so m_set_so;

		type_queue_notify_event m_q_evt;
		CMutex m_q_locker;
		//
		unsigned long m_n_open_counter;

		CMutex m_open_locker;
		HANDLE m_h_dev;
		vector< _tstring > m_v_devPath;//device path list for finding function

		bool m_bDeviceIsUart;

		bool m_b_is_dll_ok;

		CString m_sDevFullPath;		//Device full path

		DWORD m_dw_result_buffer_index;

		CLog *m_pLog;

		KeyDataType m_vElectronicKeyValue;
		CMutex m_key_locker;
	
		KeyDataQType m_qKeyData;	//Map for i-button SN.......
	private:
		CShare( const CShare &);
		CShare operator=( const CShare & );
	};


public:
	CSoLock(void) :
		m_lOpenResultCode(OPOS_E_NOHARDWARE)
		, m_lResultCode(OPOS_E_CLOSED)
		, m_lBinaryConversion(OPOS_BC_NONE)
		, m_lCapPowerReporting(OPOS_PR_NONE)
		, m_bDeviceEnabled(FALSE)
		, m_lPowerNotify(OPOS_PN_DISABLED)
		, m_lPowerState(	OPOS_PS_UNKNOWN)
		, m_lResultCodeExtended(0)
		, m_lServiceObjectVersion(LPU230LOCK_SO_VERSION)
		, m_lState(OPOS_S_CLOSED)
		, m_DevName(_T("Lpu230Lock"))
		, m_DeviceDescription(_T("Elpusk electronic keylock"))
		, m_CheckHealthText(_T("[Error]"))
		, m_SODescription(_T("OPOS service object for Lpu230Lock electronic keylock."))
		, m_pFireEvent(NULL)
		,m_lCapKeylockType(LOCK_KT_ELECTRONIC)
		,m_lKeyPosition(LOCK_KP_ELECTRONIC)
		,m_lPositionCount(0)
	{
		ATLTRACE( _T(" $ CSoLock()::CSoLock().\n") );
		bool b_dll_ok = CSoLock::CShare::get_instance().is_dll_ok();

		ATLASSERT(b_dll_ok);

		if (CGlobalVar::GetInstance()->is_generate_exception_in_set_binary_conversion()) {
			m_lBinaryConversion = OPOS_BC_NIBBLE;//
			m_lServiceObjectVersion = LPU230LOCK_SO_VERSION_1_14_1;//old version setting
		}

		m_pFireEvent = new CFireEvent();
	}

	~CSoLock(void)
	{
		ATLTRACE( _T(" $ CSoLock()::~CSoLock().\n") );
		CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CSoLock::~CSoLock()\n") );

		set_enable( FALSE );

		if( m_pFireEvent ){
			delete( m_pFireEvent );
		}
	}

private:
	//don't call these methods.
	CSoLock( const CSoLock & );
	CSoLock operator=( const CSoLock & );

public:
	HRESULT OpenService(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, long* pRC);
	HRESULT Close(long* pRC);
	HRESULT ClaimDevice(long lTimeout, long* pRC);
	HRESULT ReleaseDevice(long* pRC);
	HRESULT CheckHealth(long Level, long* pRC);
	HRESULT DirectIO(long Command, long* pData, BSTR* pString, long* pRC);
	HRESULT ResetStatistics(BSTR StatisticsBuffer, long* pRC);
	HRESULT RetrieveStatistics(BSTR* StatisticsBuffer, long* pRC);
	HRESULT UpdateStatistics(BSTR StatisticsBuffer, long* pRC);

	HRESULT WaitForKeylockChange( long lKeyPosition, long lTimeout, long* pRC);

	HRESULT GetPropertyNumber(long PropIndex, long* pNumber);
	HRESULT SetPropertyNumber(long PropIndex, long Number);
	HRESULT GetPropertyString(long PropIndex, BSTR* pString);
	HRESULT SetPropertyString(long PropIndex, BSTR bstrString);
	HRESULT COFreezeEvents(VARIANT_BOOL Freeze, long* pRC);
    HRESULT CompareFirmwareVersion( BSTR FirmwareFileName, LONG* pResult, long* pRC );
    HRESULT UpdateFirmware( BSTR FirmwareFileName,  long* pRC );
private:
	CString get_formatted_electronic_key_value();

	CFireEvent::CKeyEvent::EventType Fire( DWORD dw_status );


	//for debugging
	CString & Deb_GetStringFromPropertyNumber( long PropIndex );
	CString & Deb_GetStringFromPropertyString( long PropIndex );
	CString & Deb_GetStringFromCheckHealthLevel( long Level );
	void Deb_Printf( const TCHAR *_Format, ...);

	void set_enable( BOOL b_enable );
	BOOL get_enable();
	void set_electronic_key_value( const CString & value );
	CString get_electronic_key_value();

private:
	CMutex m_enable_locker;

	CString m_s_electronic_key_value;

	// the last Open result value of processing
	long m_lOpenResultCode;
	// the last result value of processing
	long m_lResultCode;

	//
	// BinaryConversion Property  R/W
	long m_lBinaryConversion;
	// the reporting capabilities of the device.
	long m_lCapPowerReporting;
	// CheckHealthText
	CString m_CheckHealthText;
	//DataEventEnabled - Not support.
	CFireEvent	*m_pFireEvent;//for DataEventEnabled, FreezeEvents
	//OutputID  - Not support.
	// TRUE, the device has been placed in an operational state
	volatile BOOL m_bDeviceEnabled; 
	// the type power notification selection made by the Application.
	long m_lPowerNotify;
	// Contains the current power condition, if it can be determined.
	long m_lPowerState;
	// the current state of the Control.
	volatile long m_lState;
	//DeviceControlDescription
	//DeviceControlVersion
	//service object description
	CString m_SODescription;
	// Service object version number.
	long m_lServiceObjectVersion;
	// device description
	CString m_DeviceDescription;
	// BSTR device name
	CString m_DevName;

	//////////////////////////////////////////////////////////////////
	//CapKeylockType: int32 { read-only } 1.11 open
	long m_lCapKeylockType;

	//ElectronicKeyValue: binary { read-only } 1.11 open & enable
	//KeyDataType m_vElectronicKeyValue;--> moved to CShare.

	//KeyPosition: int32 { read-only } 1.0 open & enable
	long m_lKeyPosition;

	//PositionCount: int32 { read-only } 1.0 open
	long m_lPositionCount;
	
	// When the ResultCode is set to OPOS_E_EXTENDED, this property is set to a class-specific value, and must match one of the values given in this document under the appropriate device class section.
	long m_lResultCodeExtended;

	//event dipatch id
	//std::vector<DISPID> m_vEventDispIds;
};

