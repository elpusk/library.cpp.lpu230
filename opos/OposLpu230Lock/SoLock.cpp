#include "StdAfx.h"
#include "SoLock.h"
#include <list>
#include <string>
#include <varargs.h>
#include "DetectWnd.h"
#include "EL_Support.h"
#include "GlobalVar.h"

#pragma comment( lib, "setupapi.lib" )

using namespace std;


HRESULT CSoLock::OpenService(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, long* pRC)
{
	Deb_Printf( _T("++ OpenService.\n")  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ OpenService(%s,%s).\n"),DeviceClass,DeviceName );

	do{
		if( pRC == NULL || pDispatch == NULL ){
			m_lResultCode = OPOS_E_ILLEGAL;
			continue;
		}
		//
		CString sClass( DeviceClass );

		if( sClass != _T(OPOS_CLASSKEY_LOCK) ){
			m_lResultCode = *pRC = OPOS_E_ILLEGAL;
			continue;
		}

		CString sDev( DeviceName );

		if( sDev != _T("Lpu230Lock") ){
			m_lOpenResultCode = m_lResultCode = *pRC = OPOS_E_NOEXIST;
			continue;
		}
		// save CCO dispatch for callback event
		m_pFireEvent->setDispatch( pDispatch );

		m_lOpenResultCode = m_lResultCode = *pRC = OPOS_SUCCESS;

		// Initialize the current state 
		m_lState = OPOS_S_IDLE;

		m_lBinaryConversion = OPOS_BC_NONE;
		if (CGlobalVar::GetInstance()->is_generate_exception_in_set_binary_conversion()) {
			m_lBinaryConversion = OPOS_BC_NIBBLE;
		}
		m_lCapPowerReporting = OPOS_PR_NONE;

		m_bDeviceEnabled = FALSE;

		m_pFireEvent->setFreezeEvents(false);
		m_lPowerNotify = OPOS_PN_DISABLED;
		m_lPowerState = 	OPOS_PS_UNKNOWN;
		m_lResultCodeExtended = 0;
	
		//Open Lock device here
		if( !CSoLock::CShare::get_instance().is_dll_ok() ){
			m_lOpenResultCode = m_lResultCode = *pRC = OPOS_E_NOSERVICE;
		}

		if( !CSoLock::CShare::get_instance().Lock_Open( this ) ){
			m_lOpenResultCode = m_lResultCode = *pRC = OPOS_E_NOSERVICE;
		}
	}while(0);


	Deb_Printf( _T("-- OpenService.\n")  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- OpenService.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::GetPropertyNumber(long PropIndex, long* pNumber)
{
	Deb_Printf( _T("++ GetPropertyNumber(%s).\n"),Deb_GetStringFromPropertyNumber(PropIndex)  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ GetPropertyNumber(%s).\n"),Deb_GetStringFromPropertyNumber(PropIndex)  );

	if( pNumber == NULL ){
		m_lResultCode = OPOS_E_ILLEGAL;
	}
	else{
		//support to veriosn 1.5
		HRESULT hResult = OPOS_SUCCESS; 

		switch( PropIndex ){
			case PIDX_DeviceEnabled:	*pNumber = get_enable();			break;
			case PIDX_FreezeEvents:	*pNumber = m_pFireEvent->getFreezeEvents();	break;
			case PIDX_ResultCode:		*pNumber = m_lResultCode;		break;
			case PIDX_ResultCodeExtended:	*pNumber = m_lResultCodeExtended;	break;
			case PIDX_ServiceObjectVersion:		*pNumber = m_lServiceObjectVersion;		break;
			case PIDX_State:				*pNumber = m_lState;				break;
			case PIDX_BinaryConversion:	*pNumber = m_lBinaryConversion;	break;
			case PIDX_PowerNotify:		*pNumber = m_lPowerNotify;			break;
			case PIDX_PowerState:			*pNumber = m_lPowerState;			break;
			case PIDX_CapPowerReporting:	*pNumber = m_lCapPowerReporting;		break;
			case PIDXLock_CapKeylockType:	*pNumber = m_lCapKeylockType;	break;
			case PIDXLock_KeyPosition:	*pNumber = m_lKeyPosition;		break;
			case PIDXLock_PositionCount:		*pNumber = m_lPositionCount;	break;
			default:	hResult = OPOS_E_ILLEGAL;
				break;
		}//end switch

		m_lResultCode = hResult;
	}

	if( pNumber ){
		Deb_Printf( _T("-- GetPropertyNumber(0x%x).\n"),*pNumber  );
		CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyNumber.(Value = 0x%x)\n"), *pNumber );
	}
	else{
		Deb_Printf( _T("-- GetPropertyNumber(NULL).\n")  );
		CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyNumber.(Value = NULL)\n") );
	}

	return S_OK;
}

HRESULT CSoLock::SetPropertyNumber(long PropIndex, long Number)
{
	Deb_Printf( _T("++ SetPropertyNumber(%s,0x%x).\n"),Deb_GetStringFromPropertyNumber(PropIndex),Number  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL,_T("++ SetPropertyNumber(%s,0x%x).\n"),Deb_GetStringFromPropertyNumber(PropIndex),Number  );

	//support to veriosn 1.5
	HRESULT hResult = OPOS_SUCCESS;

	bool bResult = true;

	switch( PropIndex ){
		case PIDX_Claimed:	hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_DeviceEnabled:
			set_enable( (BOOL)Number );
			break;//
		case PIDX_FreezeEvents:

			if( Number ){
				m_pFireEvent->setFreezeEvents();
			}
			else{
				m_pFireEvent->setFreezeEvents(false);
			}
			break;//
		case PIDX_ResultCode:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_ResultCodeExtended:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_ServiceObjectVersion:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_State:							hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_BinaryConversion:
			if (!CGlobalVar::GetInstance()->is_generate_exception_in_set_binary_conversion()) {
				m_lBinaryConversion = Number;
				break;
			}
			hResult = OPOS_E_ILLEGAL;
			break;
		case PIDX_PowerNotify:
			if( Number == OPOS_PN_ENABLED && m_lCapPowerReporting == OPOS_PR_NONE )
				hResult = OPOS_E_ILLEGAL;
			else if( m_lPowerNotify == OPOS_PN_ENABLED && Number == OPOS_PN_ENABLED )
				hResult = OPOS_E_ILLEGAL;	//already enable 
			else{
				m_lPowerNotify = Number;
			}	
			break;//
		case PIDX_PowerState:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_CapPowerReporting:	hResult = OPOS_E_ILLEGAL;		break;//
		default:	hResult = OPOS_E_ILLEGAL;
			break;
	}//end switch

	m_lResultCode = hResult;

	Deb_Printf( _T("-- SetPropertyNumber.\n")  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- SetPropertyNumber.(Result = 0x%x)\n"), hResult );

	return S_OK;
}

CString CSoLock::get_formatted_electronic_key_value()
{
	CSoLock::KeyDataType value, value2;
	CString s_key;
	CString s_temp;
	size_t i = 0;

	do{
		value = CSoLock::CShare::get_instance().get_electronic_key_value_by_binary();
		std::vector<wchar_t> v_wvalue;
		switch(m_lBinaryConversion){
				case OPOS_BC_NONE:
					if (!CGlobalVar::GetInstance()->is_generate_exception_in_set_binary_conversion()) {
						//new style
						std::transform(std::begin(value), std::end(value), std::back_inserter(v_wvalue), [=](char c)->wchar_t {
							return (wchar_t)c;
							});
						//
						s_key.SetString(&v_wvalue[0], v_wvalue.size());
					}
					break;
				case OPOS_BC_NIBBLE:
					if (!CGlobalVar::GetInstance()->is_generate_exception_in_set_binary_conversion()) {
						//new style
						for (i = 0; i < value.size(); i++) {
							value2.push_back(0x30 + (value[i] >> 4));
							value2.push_back(0x30 + (value[i]&0x0F));
						}//end for
						std::transform(std::begin(value2), std::end(value2), std::back_inserter(v_wvalue), [=](char c)->wchar_t {
							return (wchar_t)c;
							});
						//
						s_key.SetString(&v_wvalue[0], v_wvalue.size());
						break;
					}
					for( i=0; i<value.size(); i++ ){
						s_temp.Format( _T("%02X"), value[i] );
						s_key += s_temp;
					}//end for
					break;
				case OPOS_BC_DECIMAL:
					for( i=0; i<value.size(); i++ ){
						s_temp.Format( _T("%03u"), (unsigned int)value[i] );
						s_key += s_temp;
					}//end for
					break;
				default:	break;
		}//end switch.

	}while(0);

	return s_key;

}

HRESULT CSoLock::GetPropertyString(long PropIndex, BSTR* pString)
{
	Deb_Printf( _T("++ GetPropertyString(%s).\n"),Deb_GetStringFromPropertyString(PropIndex)  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ GetPropertyString(%s).\n"),Deb_GetStringFromPropertyString(PropIndex)  );

	//support to veriosn 1.5
	HRESULT hResult = OPOS_SUCCESS; 

	m_lResultCode = OPOS_SUCCESS;
	m_lResultCodeExtended = OPOS_SUCCESS;

	CString sEmpty(_T("") );

	static KeyDataType vTrackData(CONST_MAX_KEY);

	if( pString==NULL )
		hResult = OPOS_E_ILLEGAL;
	else{

		switch( PropIndex ){
			case PIDX_CheckHealthText: 
				m_CheckHealthText.SetSysString( pString );	break;
			case PIDX_DeviceDescription:
				m_DeviceDescription.SetSysString( pString );	break;
			case PIDX_DeviceName:
				m_DevName.SetSysString( pString );	break;
			case PIDX_ServiceObjectDescription:
				m_SODescription.SetSysString( pString );	break;
			case PIDXLock_ElectronicKeyValue:
				get_electronic_key_value().SetSysString( pString );
				break;
			default:
				hResult = OPOS_E_ILLEGAL;
				break;
		}//end switch

	}

	m_lResultCode = hResult;

	if( pString ){
		Deb_Printf( _T("-- GetPropertyString(%s).\n"),*pString  );
		CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyString.(Value = %s)\n"), *pString );
	}
	else{
		Deb_Printf( _T("-- GetPropertyString(NULL).\n")  );
		CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyString.(Value = NULL)\n") );
	}

	return S_OK;
}

HRESULT CSoLock::SetPropertyString(long PropIndex, BSTR bstrString)
{
	Deb_Printf( _T("++ SetPropertyString(%s,%s).\n"),Deb_GetStringFromPropertyString(PropIndex),bstrString  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ SetPropertyString(%s,%s).\n"),Deb_GetStringFromPropertyString(PropIndex),bstrString  );

	m_lResultCode = OPOS_E_ILLEGAL; 

	Deb_Printf( _T("-- SetPropertyString.\n")  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- SetPropertyString.(Result = 0x%x)\n"), m_lResultCode );

	return S_FALSE;
}

HRESULT CSoLock::COFreezeEvents(VARIANT_BOOL Freeze, long* pRC)
{
	if( Freeze ){
		m_pFireEvent->setFreezeEvents();
	}
	else{
		m_pFireEvent->setFreezeEvents( false );
	}
	//
	m_lResultCode = *pRC = OPOS_SUCCESS;   
	return S_OK;
}

HRESULT CSoLock::CheckHealth(long Level, long* pRC)
{
	Deb_Printf( _T("++ CheckHealth.\n") );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ CheckHealth(%s).\n"), Deb_GetStringFromCheckHealthLevel(Level) );

	*pRC = OPOS_SUCCESS;   
	m_lResultCode = OPOS_SUCCESS;   
   
	if (Level == OPOS_CH_INTERNAL){
		m_CheckHealthText.Format( _T("Internal HCheck: Complete") );
	}
	else if (Level == OPOS_CH_EXTERNAL){
		m_CheckHealthText.Format( _T("External HCheck: Complete") );
	}
	else if (Level == OPOS_CH_INTERACTIVE)
		m_CheckHealthText.Format( _T("Interactive HCheck: Complete") );   
	else {   
		m_CheckHealthText.Format( _T("[Error]") );   
		*pRC = OPOS_E_ILLEGAL;
		m_lResultCode = OPOS_E_ILLEGAL;   
	} 

	Deb_Printf( _T("-- CheckHealth.\n") );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- CheckHealth.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::ClaimDevice(long lTimeout, long* pRC)
{
	Deb_Printf( _T("++ ClaimDevice.\n") );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ ClaimDevice.(%d)\n"), lTimeout);

	*pRC = m_lResultCode = OPOS_E_ILLEGAL;

	Deb_Printf( _T("-- ClaimDevice.\n") );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- ClaimDevice.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::Close(long* pRC)
{
	Deb_Printf( _T("++ Close.\n") );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ Close.\n") );

	if( pRC == NULL ){
		m_lResultCode = OPOS_E_ILLEGAL;
	}
	else{

		if( CSoLock::CShare::get_instance().Lock_Close( this ) ){
			m_lState = OPOS_S_CLOSED;
			*pRC = OPOS_SUCCESS;
			m_lResultCode = OPOS_E_CLOSED;
		}
		else{
			m_lResultCode = *pRC = OPOS_E_FAILURE;
		}
	}

	Deb_Printf( _T("-- Close.\n") );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- Close.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::DirectIO(long Command, long* pData, BSTR* pString, long* pRC)
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ DirectIO.\n") );

	if( !get_enable() ){
		m_lResultCode = *pRC = OPOS_E_ILLEGAL;
	}
	else{
		m_lResultCode = *pRC = OPOS_SUCCESS;
	}

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- DirectIO.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::ReleaseDevice(long* pRC)
{
	Deb_Printf( _T("++ ReleaseDevice.\n")  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ ReleaseDevice.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;;

	Deb_Printf( _T("-- ReleaseDevice(0x%x).\n"),m_lResultCode  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- ReleaseDevice.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::ResetStatistics(BSTR StatisticsBuffer, long* pRC)
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ ResetStatistics.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- ResetStatistics.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::RetrieveStatistics(BSTR* StatisticsBuffer, long* pRC)
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ RetrieveStatistics.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- RetrieveStatistics.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::UpdateStatistics(BSTR StatisticsBuffer, long* pRC)
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ UpdateStatistics.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- UpdateStatistics.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::CompareFirmwareVersion( BSTR FirmwareFileName, LONG* pResult, long* pRC )
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ CompareFirmwareVersion.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- CompareFirmwareVersion.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::UpdateFirmware( BSTR FirmwareFileName,  long* pRC )
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ UpdateFirmware.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- UpdateFirmware.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoLock::WaitForKeylockChange( long lKeyPosition, long lTimeout, long* pRC)
{
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ WaitForKeylockChange.\n") );

	do{
		if( pRC == NULL ){
			m_lResultCode = *pRC = OPOS_E_ILLEGAL;
			continue;
		}
		if( lKeyPosition != LOCK_KP_ANY ){
			m_lResultCode = *pRC = OPOS_E_ILLEGAL;
			continue;
		}
		//
		if( !CSoLock::CShare::get_instance().is_open() ){
			m_lResultCode = *pRC = OPOS_E_CLOSED;
			continue;
		}
		if( !get_enable() ){
			m_lResultCode = *pRC = OPOS_E_DISABLED;
			continue;
		}
		if( m_pFireEvent->getFreezeEvents() ){
			m_lResultCode = *pRC = OPOS_E_FAILURE;
			continue;
		}
		//
		CShare::type_ptr_event evt_wait( new CShare::type_ptr_event::element_type( CreateEvent( NULL, TRUE, FALSE, NULL ) ) );
		CShare::get_instance().en_queue( evt_wait );

		DWORD dw_timeout = lTimeout;
		if( -1 == lTimeout ) //FOREVER
			dw_timeout = INFINITE;
		//
		DWORD dw_result = evt_wait->Wait( dw_timeout );
		switch( dw_result ){
		case WAIT_OBJECT_0:
			m_lResultCode = *pRC = OPOS_SUCCESS;
			break;
		case WAIT_TIMEOUT:
			CShare::get_instance().remove_queue( evt_wait );
			m_lResultCode = *pRC = OPOS_E_TIMEOUT;
			break;
		default:
			CShare::get_instance().remove_queue( evt_wait );
			m_lResultCode = *pRC = OPOS_E_FAILURE;
			break;
		}//end switch

		
	}while(0);

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- WaitForKeylockChange.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

CFireEvent::CKeyEvent::EventType CSoLock::Fire( DWORD dw_status )
{
	Deb_Printf( _T("++ *Fire(0x%x).\n"),dw_status  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("++ Fire(0x%x).\n"),dw_status );

	DWORD dwStatus = dw_status;	//LOCK_KP_ELECTRONIC;

	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] Fire : SUCCESS.\n")  );

	CFireEvent::CKeyEvent::EventType EvtType = CFireEvent::CKeyEvent::TypeStatusUpdate;
	CFireEvent::PtrKeyEvent KeyEvent = CFireEvent::PtrKeyEvent( new CFireEvent::CKeyEvent( EvtType, dwStatus ) );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("[DETAIL] Start Fire.\n")  );
	m_pFireEvent->Fire( KeyEvent );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("[DETAIL] End Fire.\n")  );

	Deb_Printf( _T("-- *Fire.\n")  );
	CSoLock::CShare::get_instance().SOTrace( true, CLog::LEV_HIGH, _T("-- Fire.\n") );

	return EvtType;
}

CString & CSoLock::Deb_GetStringFromPropertyNumber( long PropIndex )
{
	static CString sResult;

	switch( PropIndex ){
		case PIDX_Claimed:		sResult =  _T("PIDX_Claimed");		break;
		case PIDX_DataEventEnabled:	sResult =  _T("PIDX_DataEventEnabled");		break;
		case PIDX_DeviceEnabled:	sResult =  _T("PIDX_DeviceEnabled");		break;
		case PIDX_FreezeEvents:	sResult =  _T("PIDX_FreezeEvents");		break;
		case PIDX_OutputID:		sResult =  _T("PIDX_OutputID");		break;
		case PIDX_ResultCode:		sResult =  _T("PIDX_ResultCode");		break;
		case PIDX_ResultCodeExtended:	sResult =  _T("PIDX_ResultCodeExtended");		break;
		case PIDX_ServiceObjectVersion:		sResult =  _T("PIDX_ServiceObjectVersion");		break;
		case PIDX_State:				sResult =  _T("PIDX_State");		break;
		case PIDX_AutoDisable:		sResult =  _T("PIDX_AutoDisable");		break;
		case PIDX_BinaryConversion:	sResult =  _T("PIDX_BinaryConversion");		break;
		case PIDX_DataCount:			sResult =  _T("PIDX_DataCount");		break;
		case PIDX_PowerNotify:		sResult =  _T("PIDX_PowerNotify");		break;
		case PIDX_PowerState:			sResult =  _T("PIDX_PowerState");		break;
		case PIDX_CapPowerReporting:	sResult =  _T("PIDX_CapPowerReporting");		break;
		default:	sResult.Format( _T("Unknown(%d)"), PropIndex );		break;
	}//end switch

	return sResult;
}

CString & CSoLock::Deb_GetStringFromPropertyString( long PropIndex )
{
	static CString sResult;

		switch( PropIndex ){
			case PIDX_CheckHealthText: 
				sResult = _T("PIDX_CheckHealthText");	break;
			case PIDX_DeviceDescription:
				sResult = _T("PIDX_DeviceDescription");	break;
			case PIDX_DeviceName:
				sResult = _T("PIDX_DeviceName");	break;
			case PIDX_ServiceObjectDescription:
				sResult = _T("PIDX_ServiceObjectDescription");	break;
			case PIDXLock_ElectronicKeyValue:
				sResult = _T("PIDXLock_ElectronicKeyValue");	break;
			default:
				sResult.Format( _T("Unknown(%d)"), PropIndex );		break;
				break;
		}//end switch

	return  sResult;
}

CString & CSoLock::Deb_GetStringFromCheckHealthLevel( long Level )
{
	static CString sResult;

	if (Level == OPOS_CH_INTERNAL)
		sResult = _T("OPOS_CH_INTERNAL");
	else if (Level == OPOS_CH_EXTERNAL)
		sResult = _T("OPOS_CH_EXTERNAL");
	else if (Level == OPOS_CH_INTERACTIVE)
		sResult = _T("OPOS_CH_INTERACTIVE");
	else
		sResult.Format( _T("Unknown(%d)"), Level );	

	return sResult;
}

void CSoLock::Deb_Printf( const TCHAR *_Format, ...)
{
#ifdef		_YSS_ENABLE_STDOUT_FOR_DEBUG_	
	va_list args;

	va_start(args, _Format);

	int nLen = _vsctprintf( _Format, args )+1;

	TCHAR *buffer = new TCHAR[nLen];

	_vstprintf_s( buffer, nLen, _Format, args );

	_tprintf( _T("%s"), buffer );

	va_end(args);

	delete [] buffer;
#endif			//_YSS_ENABLE_STDOUT_FOR_DEBUG_
}

void CSoLock::set_enable( BOOL b_enable )
{
	m_enable_locker.Lock(50);
	if( b_enable )
		m_s_electronic_key_value = _T("");
	//
	m_bDeviceEnabled = b_enable;
	m_enable_locker.Unlock();
}

BOOL CSoLock::get_enable()
{
	BOOL b_enable(TRUE);

	m_enable_locker.Lock(50);
	b_enable = m_bDeviceEnabled;
	m_enable_locker.Unlock();
	return b_enable;
}

void CSoLock::set_electronic_key_value( const CString & value )
{
	m_enable_locker.Lock(50);
	m_s_electronic_key_value = value;
	m_enable_locker.Unlock();
}

CString CSoLock::get_electronic_key_value()
{
	CString value;
	m_enable_locker.Lock(50);
	value = m_s_electronic_key_value;
	m_enable_locker.Unlock();

	if( CGlobalVar::GetInstance()->get_lock_key_size() == 6 ){
		value = value.Mid( 1*2,6*2 );
	}

	return value;
}




