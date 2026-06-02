#pragma once

#include "resource.h"       // 주 기호입니다.

#include "cmdset.h"
#include "Sync.h"
#include "FireEvent.h"
#include "info_msr.h"
#include "DevUart.h"

#include "Lpu237Dll.h"

#include <OposMsr.hi>
#include <OposMsr.h>
#include "el_opos/el_msr_opos.h"

#include <Deque>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include "Log.h"
#include "GlobalVar.h"

//#define	LPU230_SO_VERSION	1008004		//support UART mode. 1.8.4
//#define	LPU230_SO_VERSION	1008006		//support UART mode. 1.8.6
														//support suspend & hibernation and auto release at terminating program 1.9.6

//#define	LPU230_SO_VERSION	1008008		// fix code missing suspend & hibernation and auto release at terminating funtionality.
//#define	LPU230_SO_VERSION	1008010		// fix code missing return error code in removed device.
//#define	LPU230_SO_VERSION	1008012		// after kill thread, send leaveOpos Mode command.
														// Don't send leaveOpos Mode command at reference counter 1.

//#define	LPU230_SO_VERSION	1008014		// add OposLpu230.ini file enable/ disable event fire.
//#define	LPU230_SO_VERSION	1008016		// fix error get device cap.

//#define	LPU230_SO_VERSION	1008017		// supports Elpusk\\lpu230\\lpu230.ini and Easyset\\lpu230\\lpu230.ini
//#define	LPU230_SO_VERSION	1008018		// change Hid library 
//#define	LPU230_SO_VERSION	1008019		// fix device detecting code missing.
//#define	LPU230_SO_VERSION	1008022		// support ng_devmanager.
//#define	LPU230_SO_VERSION	1008023		// support lpu238
#define	LPU230_SO_VERSION	2000000		// using cf2

using namespace ATL;
using namespace SYNC;

#pragma pack(push,1)
typedef struct tagMSR_DATA_PACKET{

	unsigned char cID;	//report ID

	char cErrorSize[3];	//object' error(negative),  data size(positive)

	unsigned char sData[1];	//the staring point of data field.

}MSR_DATA_PACKET, *PMSR_DATA_PACKET, *LPMSR_DATA_PACKET;
#pragma pack(pop)

class CSoMsr
{
private:
	typedef	std::vector<CString>	TrackDataType;
	typedef	std::shared_ptr<TrackDataType>		PtrTrackDataType;
	typedef	std::deque<PtrTrackDataType>		TrackDataQType;

public:

	enum MsReadTransaction{
		MSR_Complete,		// idle status 
		MSR_Waits,			//waits response of msr
		MSR_Error			//communication error
	};

	enum{
		CONST_MAX_TRACK = 4,
		CONST_SUPPORT_TRACK = 3
	};

public:
	static CSoMsr *GetInstance( bool bFree = false )
	{
		static CSoMsr *pSoMsr = NULL;

		if( bFree ){

			if( pSoMsr )
				delete pSoMsr;

			pSoMsr = NULL;
		}
		else{
			if( pSoMsr == NULL )
				pSoMsr = new CSoMsr();
		}
		
		return pSoMsr;
	}

	
	~CSoMsr()
	{
		ATLTRACE( _T(" $ CSoMsr()::~CSoMsr().\n") );
		CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CSoMsr::~CSoMsr()\n") );

		if( m_pWorker ){
			delete( m_pWorker );
		}

		if( m_pDeviceQ ){
			delete( m_pDeviceQ );
		}

		if( m_pFireEvent ){
			delete( m_pFireEvent );
		}
		
	}

	HANDLE get_device_handle()
	{
		return m_hDev;
	}
private:
	CSoMsr( const CSoMsr & );

	CSoMsr()
		: m_bDeviceChanged(true)
		, m_bDeviceIsUart(false)
		, m_lOpenResultCode(OPOS_E_NOHARDWARE)
		, m_lResultCode(OPOS_E_CLOSED)
		, m_bAutoDisable(FALSE)
		, m_lBinaryConversion(OPOS_BC_NONE)
		, m_lCapPowerReporting(OPOS_PR_NONE)
		, m_bClaimed(FALSE)
		, m_lDataCount(0)
		, m_bDeviceEnabled(FALSE)
		, m_lPowerNotify(OPOS_PN_DISABLED)
		, m_lPowerState(	OPOS_PS_UNKNOWN)
		, m_lResultCodeExtended(0)
		, m_lServiceObjectVersion(LPU230_SO_VERSION)
		, m_lState(OPOS_S_CLOSED)
		, m_bCapISO(TRUE)
		, m_bCapJISOne(FALSE)
		, m_lCapJISTwo(FALSE)
		, m_lCapTransmitSentinels(TRUE)
		, m_bDecodeData(TRUE)
		, m_lErrorReportingType(MSR_ERT_CARD)
		, m_bParseDecodeData(FALSE)
		, m_lTracksToRead(MSR_TR_1_2_3)
		, m_bTransmitSentinels(FALSE)
		, m_DevName(_T("Lpu230MSR"))
		, m_DeviceDescription(_T("Elpusk Magnetic card reader"))
		, m_CheckHealthText(_T("[Error]"))
		, m_SODescription(_T("OPOS service object for Lpu230 Magnetic card reader."))
		, m_sDeviceName(_T(""))
		, m_vTrackData(CONST_MAX_TRACK)
		, m_sAccountNumber(_T(""))
		, m_bIsATrackError(false)
		, m_sExpirationDate(_T(""))
		, m_sFirstName(_T(""))
		, m_sMiddleInitial(_T(""))
		, m_sServiceCode(_T(""))
		, m_sSuffix(_T(""))
		, m_sSurname(_T(""))
		, m_sTitle(_T(""))
		, m_sTrack1DiscretionaryData(_T(""))
		, m_sTrack2DiscretionaryData(_T(""))
		, m_hDev(NULL)
		, m_pFireEvent(NULL)
		, m_lCapDataEncryption(MSR_DE_NONE) //EL_MSR_DE_AES_DUKPT|MSR_DE_NONE
		, m_bCapTrackDataMasking(FALSE)//MMD1000 dosn't support each track masking
		, m_lDataEncryptionAlgorithm(MSR_DE_NONE)
		, m_vTrackEncryptedData(CONST_MAX_TRACK)
	{
		ATLTRACE( _T(" $ CSoMsr()::CSoMsr().\n") );

		m_pLog = CLog::GetLog();
		ATLASSERT(m_pLog!=NULL);
		//ATLTRACE( _T(" * CSoLpu230()::m_pLog = 0x%x.\n"),m_pLog );

		CLpu237Dll::get_instance( CGlobalVar::GetInstance()->get_msr_dll_path() );

		CLpu237Dll::get_instance().LPU237_dll_on();

		for( int i=0; i<CONST_MAX_TRACK; i++ ){
			m_bTrackError[i] = false;
			m_cErrorCode[i] = 0;
		}

		m_pFireEvent = new CFireEvent();

		m_pDeviceQ = new CWorkQueue();
		m_pWorker = new CWorker( this );
	}

private:

	//Hardware error code
	enum : signed char{
		C_MSR_GOOD				=0,	//magnetic card None Error(including blank )
		C_MSR_ERROR_PARITY		=-32,	//magnetic card parity Error(0xe0)
		C_MSR_ERROR_LRC			=-33,	//magnetic card LRC Error(0xdf)
		C_MSR_ERROR_STX			=-34,	//magnetic card STX Error(0xde)
		C_MSR_ERROR_ETX			=-35,	//magnetic card ETX Error(0xdd)
		C_MSR_WARN_OVER		=-64	//magnetic overflowing data size on ISO spec(0xc0)
	};

	enum : unsigned char{
		C_MSR_JIS2_STXETX =	0xFF,
	};

	enum : unsigned char{
		C_MSR_ISO1_STX = 0x05,	//% (0x25)
		C_MSR_ISO1_ETX =	0x1F	//? (0x3F)
	};
	enum : unsigned char{
		C_MSR_ISO2_STX = 0x0b,	//; (0x3B)
		C_MSR_ISO2_ETX =	0x0F	//? (0x3F)
	};
	enum : unsigned char{
		C_MSR_ISO3_STX = 0x0b,	//; (0x3B)
		C_MSR_ISO3_ETX =	0x0F	//? (0x3F)
	};

	enum{
		CONST_SYNC_WAITE_TIME = 2000
	};

	typedef	vector< _tstring >		DevListType;

	/////////////////////////
	//device event class
	class CDevEvent{

	public:
		typedef	std::vector<BYTE>		DataType;

		enum{
			MaxDataSize = 256
		};

		enum EventType{
			Data,
			Error
		};

	public:
		CDevEvent( EventType Type, BYTE cCmd, BYTE cSub, DWORD dwSize, const DataType & vData, HANDLE hCompleteEvent )
			: m_Type(Type), m_CompleteEvent(hCompleteEvent)
		{
			::memset( &m_TxData, 0, sizeof(m_TxData) );

			m_TxData.cCmd = cCmd;
			m_TxData.cSub = cSub;
			m_TxData.cLen = (BYTE)dwSize;
			std::copy( vData.begin(), vData.begin()+dwSize, m_TxData.sData );
		}

		~CDevEvent()	{}

		CDevEvent & operator = ( const CDevEvent & Event )
		{
			m_Type = Event.m_Type;
			::memcpy( &m_TxData, static_cast<const void*>(&Event.m_TxData), sizeof(m_TxData) );
			return *this;
		}

		//build Tx Data as the givin format.
		LPMSR_HOST_PACKET GetTxData(){	return &m_TxData;	}

		MSR_HostCmd GetCmd(){	return static_cast<MSR_HostCmd>(m_TxData.cCmd); }

		void Complete()
		{
			m_CompleteEvent.Set();
		}

		DWORD WaitComplete( DWORD dwTimeOut = INFINITE )
		{
			return ::WaitForSingleObject( m_CompleteEvent.GetHandle(), dwTimeOut );
		}

	private:
		CDevEvent();
		EventType m_Type;
		MSR_HOST_PACKET m_TxData;

		CEventCase m_CompleteEvent;
	};

	typedef	std::shared_ptr<CDevEvent>	sp_DevEvent;

	/////////////////////////
	//work Queue
	class CWorkQueue{
	public:
		CWorkQueue()
		{
			ATLTRACE( _T(" $ CWorkQueue()::CWorkQueue().\n") );
			ATLASSERT( m_Locker.IsIniOk() );
		}

		~CWorkQueue()
		{
			//::DeleteCriticalSection( &m_CsEvent );
			ATLTRACE( _T(" $ CWorkQueue()::~CWorkQueue().\n") );
			CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CWorkQueue::~CWorkQueue()\n") );
		}

		bool EnQ( const sp_DevEvent &Event )
		{
			bool bResult = true;

			//::EnterCriticalSection( &m_CsEvent );
			m_Locker.Lock(INFINITE);
			m_DeviceEvent.push_back( Event );
			m_Locker.Unlock();
			//::LeaveCriticalSection( &m_CsEvent );
			return bResult;
		}

		bool DeQ( sp_DevEvent &Event )
		{
			bool bResult = true;

			//::EnterCriticalSection( &m_CsEvent );
			m_Locker.Lock(INFINITE);
			
			if( !m_DeviceEvent.empty() ){
				Event = m_DeviceEvent.front();
				m_DeviceEvent.pop_front();
			}
			else
				bResult = false;

			m_Locker.Unlock();
			//::LeaveCriticalSection( &m_CsEvent );
			return bResult;
		}

		void QClear()
		{
			//::EnterCriticalSection( &m_CsEvent );
			m_Locker.Lock(INFINITE);
			m_DeviceEvent.clear();
			m_Locker.Unlock();
			//::LeaveCriticalSection( &m_CsEvent );
		}

		bool IsEmpty()
		{
			bool bResult = true;

			//::EnterCriticalSection( &m_CsEvent );
			m_Locker.Lock(INFINITE);
			bResult = m_DeviceEvent.empty();
			m_Locker.Unlock();
			//::LeaveCriticalSection( &m_CsEvent );

			return bResult;
		}

	private:
		//Queue
		std::deque<sp_DevEvent> m_DeviceEvent;
		//CRITICAL_SECTION m_CsEvent;
		CMutex m_Locker;
	};

	/////////////////////////
	//worker class
	class CWorker{
	public:
		enum{
			Evt_Kill			= WAIT_OBJECT_0,
			Evt_Wakeup		= WAIT_OBJECT_0+1,
			Evt_UsbReadDone = WAIT_OBJECT_0+2,
			Evt_Msg			= WAIT_OBJECT_0+3,
			Evt_ReadDone	= WAIT_IO_COMPLETION,
			Evt_TimeOut		= WAIT_TIMEOUT
		};

		CWorker( LPVOID lpParameter ) : m_hWorker(NULL)
			, m_dwIdWorker(0)
			, m_bRunning(false)
			, m_dwRead(1+3+76+37+104)
			, m_bReqExecuteRead(false)
			, m_pSoMsr(NULL)
		{
			ATLTRACE( _T(" $ CWorker()::CWorker().\n") );

			m_lpParameter = lpParameter;
			m_dwLastError = ERROR_SUCCESS;
			m_bOpen = false;
			m_bSuspend = false;
			m_bRecover = false;

			::memset( m_sRead, 0, sizeof(m_sRead) );
			::memset( &m_Ov, 0, sizeof(m_Ov) );

			m_ReadDoneEvent.SetHandle( ::CreateEvent( NULL, TRUE, FALSE, NULL ) );
			m_Ov.hEvent = m_ReadDoneEvent.GetHandle();

			m_KillEvent.SetHandle( ::CreateEvent( NULL, TRUE, FALSE, NULL ) );

			m_WakeupEvent.SetHandle( ::CreateEvent( NULL, FALSE, FALSE, NULL ) );

			m_UsbMsrDoneEvent.SetHandle( CreateEvent( NULL, FALSE, FALSE, NULL ) );

			//m_hWorker = ::CreateThread( NULL, 0, MsrWoker, lpParameter, 0, &m_dwIdWorker );
			m_hWorker = (HANDLE)::_beginthreadex( NULL, 0, MsrWoker, lpParameter, 0, &m_dwIdWorker );
		
		}

		~CWorker()
		{
			ATLTRACE( _T(" $ CWorker()::~CWorker().\n") );
			CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CWorker::~CWorker()\n") );
			Kill();
		}

		bool Wakeup()
		{
			if( m_hWorker == NULL )
				return false;

			m_WakeupEvent.Set();
			return true;
		}

		void Kill();

	private:
		CWorker();

	private:
		CSoMsr *m_pSoMsr;

		bool m_bReqExecuteRead;
		LPVOID m_lpParameter;

		HANDLE m_hWorker;
		//DWORD m_dwIdWorker;
		unsigned m_dwIdWorker;

		CEventCase m_KillEvent;
		CEventCase m_WakeupEvent;
		CEventCase m_UsbMsrDoneEvent;

		volatile bool m_bRunning;

		DWORD m_dwRead;
		OVERLAPPED m_Ov;
		BYTE m_sRead[256];
		CEventCase m_ReadDoneEvent;

		static volatile bool m_bRecover;
		static volatile DWORD m_dwLastError;
		static volatile bool m_bOpen;
		static volatile bool m_bSuspend;

		bool MsrWoker_Normal_Uart( CSoMsr *pSo,  DWORD dwWaitResult  );//return false, exit thread
		bool MsrWoker_Abnormal_Uart( CSoMsr *pSo, DWORD dwWaitResult  );

		bool MsrWoker_Normal_Usb( CSoMsr *pSo,  DWORD dwWaitResult  );//return false, exit thread
		bool MsrWoker_Abnormal_Usb( CSoMsr *pSo, DWORD dwWaitResult  );

		bool MsWorker_Normal_Usb_get_msr_data( unsigned int n_result_index );

		//static DWORD WINAPI MsrWoker( LPVOID lpParam );
		static unsigned __stdcall MsrWoker( LPVOID lpParam );
		static VOID WINAPI MsrReadDoneComplete(
			DWORD dwErrorCode,
			DWORD dwNumberOfBytesTransfered,
			LPOVERLAPPED lpOverlapped
			);

		static void WINAPI MsrReadDoneCompleteUsb(void*pParam);

		static void CALLBACK EventCB_PowerSuspend(LPVOID lpCBParameter);
		static void CALLBACK EventCB_PowerResume(LPVOID lpCBParameter);
		static void CALLBACK EventCB_DeviceChanged_PlugIn(LPVOID lpCBParameter);
		static void CALLBACK EventCB_DeviceChanged_PlugOut(LPVOID lpCBParameter);
	};
	
public:

	HRESULT OpenService(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, long* pRC);
	HRESULT GetPropertyNumber(long PropIndex, long* pNumber);
	HRESULT SetPropertyNumber(long PropIndex, long Number);
	HRESULT GetPropertyString(long PropIndex, BSTR* pString);
	HRESULT SetPropertyString(long PropIndex, BSTR bstrString);
	HRESULT GetOpenResult(long* pRC);
	HRESULT COFreezeEvents(VARIANT_BOOL Freeze, long* pRC);
	HRESULT CheckHealth(long Level, long* pRC);
	HRESULT ClaimDevice(long lTimeout, long* pRC);
	HRESULT ClearInput(long* pRC);
	HRESULT Close(long* pRC);
	HRESULT DirectIO(long Command, long* pData, BSTR* pString, long* pRC);
	HRESULT ReleaseDevice(long* pRC);
	HRESULT ResetStatistics(BSTR StatisticsBuffer, long* pRC);
	HRESULT RetrieveStatistics(BSTR* StatisticsBuffer, long* pRC);
	HRESULT UpdateStatistics(BSTR StatisticsBuffer, long* pRC);

	//for encryption
	HRESULT UpdateKey( BSTR Key, BSTR KeyName, long* pRC );

private:
	static int GetStringFromMultiString( list<_tstring> & listStr, LPCTSTR szMultiStr );

private:
	volatile bool m_bDeviceChanged;

	bool m_bDeviceIsUart;

	// the last Open result value of processing
	long m_lOpenResultCode;
	// the last result value of processing
	long m_lResultCode;
	// mutex object
	CMutex m_Locker;
	// AutoDisable Property  R/W
	BOOL m_bAutoDisable;
	// BinaryConversion Property  R/W
	long m_lBinaryConversion;
	// the reporting capabilities of the device.
	long m_lCapPowerReporting;
	// If TRUE, the device is claimed for exclusive access.
	BOOL m_bClaimed;
	// Holds the number of enqueued DataEvents at the control
	long m_lDataCount;

	CFireEvent	*m_pFireEvent;
	
	// TRUE, the device has been placed in an operational state
	BOOL m_bDeviceEnabled; 
	
	// the type power notification selection made by the Application.
	long m_lPowerNotify;
	// Contains the current power condition, if it can be determined.
	long m_lPowerState;
	// When the ResultCode is set to OPOS_E_EXTENDED, this property is set to a class-specific value, and must match one of the values given in this document under the appropriate device class section.
	long m_lResultCodeExtended;
	// Service object version number.
	long m_lServiceObjectVersion;
	// the current state of the Control.
	volatile long m_lState;
	// If TRUE, the MSR device supports ISO cards
	BOOL m_bCapISO;
	// If TRUE, the MSR device supports JIS Type-I cards
	BOOL m_bCapJISOne;
	// If TRUE, the MSR device supports JIS Type-II cards
	BOOL m_lCapJISTwo;
	// If TRUE, the device is able to transmit the start and end sentinels. 
	BOOL m_lCapTransmitSentinels;
	// If FALSE, the Track1Data, Track2Data, Track3Data and Track4Data properties contain the original encoded bit sequence, known as “raw format”.
	BOOL m_bDecodeData;
	// An error is reported by an ErrorEvent when a card is swiped, and one or more of the tracks specified by the TracksToRead property contains data with errors.
	long m_lErrorReportingType;
	// If TRUE, the decoded data contained within the Track1Data and Track2Data properties is further separated into fields for access via various other properties.  Track3Data is not parsed because its data content is of an open format defined by the card issuer.  JIS-I Track 1 Format C and ISO Track 1 Format C data are not parsed for similar reasons. Track4Data is also not parsed.
	BOOL m_bParseDecodeData;
	// Indicates the track data that the application wishes to have placed into the Track1Data, Track2Data, Track3Data and Track4Data properties following a card swipe.
	long m_lTracksToRead;
	// If TRUE, the Track1Data, Track2Data, Track3Data and Track4Data properties contain start sentinel and end sentinel values.
	BOOL m_bTransmitSentinels;
	// BSTR device name
	CString m_DevName;
	// device description
	CString m_DeviceDescription;
	// CheckHealthText
	CString m_CheckHealthText;
	//service object description
	CString m_SODescription;

	//for encryption.......
	long m_lCapDataEncryption;
	BOOL m_bCapTrackDataMasking;
	ATL::CString m_sAdditionalSecurityInformation;//I use DUKPT, contain transaction number.
	long m_lDataEncryptionAlgorithm;
	TrackDataType m_vTrackEncryptedData;

	//
	ATL::CString m_sAccountNumber;//
	ATL::CString m_sExpirationDate;//
	ATL::CString m_sFirstName;
	ATL::CString m_sMiddleInitial;
	ATL::CString m_sServiceCode;//
	ATL::CString m_sSuffix;
	ATL::CString m_sSurname;
	ATL::CString m_sTitle;
	ATL::CString m_sTrack1DiscretionaryData;//
	ATL::CString m_sTrack2DiscretionaryData;//

	//mutex
	CMutex m_ClaimMutex;

	//device name
	CString m_sDeviceName;

	//Queue
	CWorkQueue *m_pDeviceQ;

	//Device full path
	CString m_sDevFullPath;

	HANDLE m_hDev;

	//Worker thread
	CWorker *m_pWorker;

	vector< _tstring > m_vDevPath;//device path list for finding function

	//event dipatch id
	//std::vector<DISPID> m_vEventDispIds;

	//ATL string format. card data
	TrackDataType m_vTrackData;

	//Map for track data.......
	TrackDataQType m_qTrackData;

	MSR_HOST_PACKET m_Resp;	//MSR response buffer

	MSR_TAG m_PrivatePreTag[CONST_SUPPORT_TRACK];
	MSR_TAG m_PrivatePostTag[CONST_SUPPORT_TRACK];

	bool m_bTrackError[CONST_MAX_TRACK];
	bool m_bIsATrackError;
	signed char m_cErrorCode[CONST_MAX_TRACK];

	CDevUart m_Uart;

	void setDeviceChanged( bool bChanged = true ){	m_bDeviceChanged = bChanged;	}
	bool getDeviceChanged(){	return m_bDeviceChanged;	}

	bool IsPathNameUart( const _tstring & sDevPath );

	bool Msr_Open(void);
	bool Msr_Close(void);
	bool Msr_Enable(bool bEnable = true);
	bool Msr_FreezeEvents( bool bFreeze = true );
	bool Msr_EnableDecode( bool bEnable = true );

	MsReadTransaction Msr_GetMsData(void);

	bool Msr_SendRequest( const MSR_HostCmd Cmd, const unsigned char cSub,INT32 nData, const unsigned char *sData, unsigned char *sResp = NULL, HANDLE hDev = NULL  );
	bool Msr_SendRequest( const MSR_HOST_PACKET & Req,  PMSR_HOST_PACKET Resp = NULL, HANDLE hDev = NULL  );

	bool FindOpenEnterOpos( bool bFlashHandle = true );

	void SaveMsrData( LPMSR_DATA_PACKET pMsData );

	CFireEvent::CMsrEvent::EventType Fire( DWORD dwErrorCode );

	void QClear();
	DevListType & GetDeviceList();

	unsigned char ChangeErrorCode( signed char cError );

	void ParsingCardData(  LPMSR_DATA_PACKET pMsData );

	BOOL Write(
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten=NULL,
		LPOVERLAPPED lpOverlapped =NULL,
		HANDLE hDev = NULL
	);

	BOOL Read(
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped =NULL,
		HANDLE hDev = NULL
	);

	bool EnQTrackData( const PtrTrackDataType & pTrackData );

	bool DeQTrackData( PtrTrackDataType & pTrackData );

	void ErrorProcessWithErrorResponse( long lError );

	//for debugging
	CString & Deb_GetStringFromPropertyNumber( long PropIndex );
	CString & Deb_GetStringFromPropertyString( long PropIndex );
	CString & Deb_GetStringFromCheckHealthLevel( long Level );
	void Deb_Printf( const TCHAR *_Format, ...);

	CLog *m_pLog;
	void SOTrace( bool bStemp, CLog::Level nLevel, const TCHAR *_Format, ...);
};

