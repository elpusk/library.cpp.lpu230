#include "StdAfx.h"
#include "SoMsr.h"
#include "info_sys.h"
#include "info_sys_cnst.h"
#include "KBConst.h"
#include <list>
#include <string>
#include <varargs.h>
#include "DetectWnd.h"
#include "EL_Support.h"
#include "GlobalVar.h"
#include "SysVer.h"

#include <Dbt.h>

#pragma comment( lib, "setupapi.lib" )

using namespace std;


HRESULT CSoMsr::OpenService(BSTR DeviceClass, BSTR DeviceName, IDispatch* pDispatch, long* pRC)
{
	Deb_Printf( _T("++ OpenService.\n")  );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ OpenService(%s,%s).\n"),DeviceClass,DeviceName );

	if( pRC == NULL || pDispatch == NULL ){
		m_lResultCode = OPOS_E_ILLEGAL;
	}
	else{
		CString sClass( DeviceClass );

		if( sClass != _T(OPOS_CLASSKEY_MSR) ){
			m_lResultCode = *pRC = OPOS_E_ILLEGAL;
		}
		else{

			CString sDev( DeviceName );

			if( sDev != _T("Lpu230MSR") ){
				m_lOpenResultCode = m_lResultCode = *pRC = OPOS_E_NOEXIST;
			}
			else{
				// save CCO dispatch for callback event
				m_pFireEvent->setDispatch( pDispatch );

				m_lOpenResultCode = m_lResultCode = *pRC = OPOS_SUCCESS;
				m_sDeviceName = sDev;

				// Initialize the current state 
				m_lState = OPOS_S_IDLE;

				m_bAutoDisable = FALSE;
				m_lBinaryConversion = OPOS_BC_NONE;
				m_lCapPowerReporting = OPOS_PR_NONE;
				m_lDataCount = 0;

				m_pFireEvent->setDataEventEnable( false );
				m_bDeviceEnabled = FALSE;

				m_pFireEvent->setFreezeEvents(false);
				m_lPowerNotify = OPOS_PN_DISABLED;
				m_lPowerState = 	OPOS_PS_UNKNOWN;
				m_lResultCodeExtended = 0;
				m_bCapISO = TRUE;
				m_bCapJISOne = FALSE;
				m_lCapJISTwo = FALSE;
				m_lCapTransmitSentinels = TRUE;
				m_bDecodeData = TRUE;
				m_lErrorReportingType = MSR_ERT_CARD;
				m_bParseDecodeData = TRUE;
				m_lTracksToRead = MSR_TR_1_2_3;
				m_bTransmitSentinels = FALSE;	//remarked for getting real value. <>
	
				//Open MSR device here
				if( !Msr_Open() ){
					m_lOpenResultCode = m_lResultCode = *pRC = OPOS_E_NOSERVICE;
				}
			}
		}
	}

	Deb_Printf( _T("-- OpenService.\n")  );
	SOTrace( true, CLog::LEV_HIGH, _T("-- OpenService.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::GetPropertyNumber(long PropIndex, long* pNumber)
{
	Deb_Printf( _T("++ GetPropertyNumber(%s).\n"),Deb_GetStringFromPropertyNumber(PropIndex)  );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ GetPropertyNumber(%s).\n"),Deb_GetStringFromPropertyNumber(PropIndex)  );

	if( pNumber == NULL ){
		m_lResultCode = OPOS_E_ILLEGAL;
	}
	else{

		//support to veriosn 1.5
		HRESULT hResult = OPOS_SUCCESS; 

		switch( PropIndex ){
			case PIDX_Claimed:		*pNumber = m_bClaimed;		break;
			case PIDX_DataEventEnabled:	*pNumber = m_pFireEvent->getDataEventEnable();	break;
			case PIDX_DeviceEnabled:	*pNumber = m_bDeviceEnabled;			break;
			case PIDX_FreezeEvents:	*pNumber = m_pFireEvent->getFreezeEvents();	break;
			case PIDX_OutputID:		*pNumber = 0;		break;//not support
			case PIDX_ResultCode:		*pNumber = m_lResultCode;		break;
			case PIDX_ResultCodeExtended:	*pNumber = m_lResultCodeExtended;	break;
			case PIDX_ServiceObjectVersion:		*pNumber = m_lServiceObjectVersion;		break;
			case PIDX_State:				*pNumber = m_lState;				break;
			case PIDX_AutoDisable:		*pNumber = m_bAutoDisable;		break;
			case PIDX_BinaryConversion:	*pNumber = m_lBinaryConversion;	break;
			case PIDX_DataCount:			*pNumber = m_lDataCount;			break;
			case PIDX_PowerNotify:		*pNumber = m_lPowerNotify;			break;
			case PIDX_PowerState:			*pNumber = m_lPowerState;			break;
			case PIDX_CapPowerReporting:	*pNumber = m_lCapPowerReporting;		break;
			case PIDXMsr_DecodeData:		*pNumber = m_bDecodeData;	break;
			case PIDXMsr_ParseDecodeData:	*pNumber = m_bParseDecodeData;		break;
			case PIDXMsr_TracksToRead:		*pNumber = m_lTracksToRead;			break;
			case PIDXMsr_ErrorReportingType:	*pNumber = m_lErrorReportingType;	break;
			case PIDXMsr_TransmitSentinels:		*pNumber = m_bTransmitSentinels;		break;
			case PIDXMsr_CapISO:		*pNumber = m_bCapISO;			break;
			case PIDXMsr_CapJISOne:	*pNumber = m_bCapJISOne;		break;
			case PIDXMsr_CapJISTwo:	*pNumber = m_lCapJISTwo;		break;
			case PIDXMsr_CapTransmitSentinels:		*pNumber = m_lCapTransmitSentinels;		break;
			default:	hResult = OPOS_E_ILLEGAL;
				break;
		}//end switch

		m_lResultCode = hResult;
	}

	if( pNumber ){
		Deb_Printf( _T("-- GetPropertyNumber(0x%x).\n"),*pNumber  );
		SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyNumber.(Value = 0x%x)\n"), *pNumber );
	}
	else{
		Deb_Printf( _T("-- GetPropertyNumber(NULL).\n")  );
		SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyNumber.(Value = NULL)\n") );
	}

	return S_OK;
}


HRESULT CSoMsr::SetPropertyNumber(long PropIndex, long Number)
{
	Deb_Printf( _T("++ SetPropertyNumber(%s,0x%x).\n"),Deb_GetStringFromPropertyNumber(PropIndex),Number  );
	SOTrace( true, CLog::LEV_NORMAL,_T("++ SetPropertyNumber(%s,0x%x).\n"),Deb_GetStringFromPropertyNumber(PropIndex),Number  );

	//support to veriosn 1.5
	HRESULT hResult = OPOS_SUCCESS;

	bool bResult = true;

	switch( PropIndex ){
		case PIDX_Claimed:	hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_DataEventEnabled:
			if( Number ){// see 010220-OPOS-APG-(Rel-1.5).doc	46 page, DataEventEnabled Property.
				m_pFireEvent->setDataEventEnable( true );
			}
			else{
				m_pFireEvent->setDataEventEnable( false );
			}
			break;
		case PIDX_DeviceEnabled:

			if( !m_bClaimed ){
				hResult = OPOS_E_NOTCLAIMED;
				break;
			}
			
			if( Number )
				bResult = Msr_Enable( true );
			else
				bResult = Msr_Enable( false );

			m_bDeviceEnabled = Number;
			break;//
		case PIDX_FreezeEvents:

			if( Number ){
				Msr_FreezeEvents();
				m_pFireEvent->setFreezeEvents();
			}
			else{
				Msr_FreezeEvents( false );
				m_pFireEvent->setFreezeEvents(false);
			}
			break;//
		case PIDX_OutputID:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_ResultCode:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_ResultCodeExtended:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_ServiceObjectVersion:		hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_State:							hResult = OPOS_E_ILLEGAL;		break;//
		case PIDX_AutoDisable:				m_bAutoDisable = Number;		break;//
		case PIDX_BinaryConversion:
			switch( Number ){
				case OPOS_BC_NONE:
				case OPOS_BC_NIBBLE:
				case OPOS_BC_DECIMAL:	m_lBinaryConversion = Number;	break;
				default:	hResult = OPOS_E_ILLEGAL;	break;
			}//end switch
			break;//
		case PIDX_DataCount:	hResult = OPOS_E_ILLEGAL;		break;//
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
		case PIDXMsr_DecodeData:
			m_bDecodeData = Number;
			
			if( m_bDecodeData )
				Msr_EnableDecode();
			else{
				Msr_EnableDecode( false );
				m_bParseDecodeData = FALSE;
			}
			break;//
		case PIDXMsr_ParseDecodeData:		m_bParseDecodeData = Number;	break;//
		case PIDXMsr_TracksToRead:
			if( MSR_TR_1_2_3_4 < Number )
				hResult = OPOS_E_ILLEGAL;
			else
				m_lTracksToRead = Number;
			break;//
		case PIDXMsr_ErrorReportingType:

			if( Number != MSR_ERT_CARD && Number != MSR_ERT_TRACK )
				hResult = OPOS_E_ILLEGAL;
			else
				m_lErrorReportingType = Number;
			break;//
		case PIDXMsr_TransmitSentinels:
			
			if( !m_lCapTransmitSentinels )
				hResult = OPOS_E_ILLEGAL;
			else{
				if( Number ){
					m_bTransmitSentinels = TRUE;
				}
				else{
					m_bTransmitSentinels = FALSE;
				}
			}
			break;//

		////read only
		case PIDXMsr_CapISO:
		case PIDXMsr_CapJISOne:
		case PIDXMsr_CapJISTwo:
		case PIDXMsr_CapTransmitSentinels:	
			hResult = OPOS_E_ILLEGAL;		break;//
		default:	hResult = OPOS_E_ILLEGAL;
			break;
	}//end switch

	m_lResultCode = hResult;

	Deb_Printf( _T("-- SetPropertyNumber.\n")  );
	SOTrace( true, CLog::LEV_HIGH, _T("-- SetPropertyNumber.(Result = 0x%x)\n"), hResult );

	return S_OK;
}

HRESULT CSoMsr::GetPropertyString(long PropIndex, BSTR* pString)
{
	Deb_Printf( _T("++ GetPropertyString(%s).\n"),Deb_GetStringFromPropertyString(PropIndex)  );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ GetPropertyString(%s).\n"),Deb_GetStringFromPropertyString(PropIndex)  );

	//support to veriosn 1.5
	HRESULT hResult = OPOS_SUCCESS; 

	m_lResultCode = OPOS_SUCCESS;
	m_lResultCodeExtended = OPOS_SUCCESS;

	CString sEmpty(_T("") );

	static TrackDataType vTrackData(CONST_MAX_TRACK);

	bool b_hide_some_data_for_security(true);

	if( pString==NULL )
		hResult = OPOS_E_ILLEGAL;
	else{

		switch( PropIndex ){
			case PIDX_CheckHealthText:
				b_hide_some_data_for_security = false;
				m_CheckHealthText.SetSysString( pString );	break;
			case PIDX_DeviceDescription:
				b_hide_some_data_for_security = false;
				m_DeviceDescription.SetSysString( pString );	break;
			case PIDX_DeviceName: 
				b_hide_some_data_for_security = false;
				m_DevName.SetSysString( pString );	break;
			case PIDX_ServiceObjectDescription:
				b_hide_some_data_for_security = false;
				m_SODescription.SetSysString( pString );	break;

			case PIDXMsr_Track1Data:

				if( m_lTracksToRead & MSR_TR_1 )
					m_vTrackData[0].SetSysString( pString );
				else
					m_vTrackData[0].Empty();
				break;
			case PIDXMsr_Track2Data:	

				if( m_lTracksToRead & MSR_TR_2 )
					m_vTrackData[1].SetSysString( pString );
				else
					m_vTrackData[1].Empty();
				break;
			case PIDXMsr_Track3Data:

				if( m_lTracksToRead & MSR_TR_3 )
					m_vTrackData[2].SetSysString( pString );
				else
					m_vTrackData[2].Empty();
				break;
			case PIDXMsr_Track4Data:
				//m_vTrackData[3].SetSysString( pString );
				m_vTrackData[3].Empty();
				break;
			case PIDXMsr_AccountNumber:	m_sAccountNumber.SetSysString( pString );	break;
			case PIDXMsr_ExpirationDate:		m_sExpirationDate.SetSysString( pString );		break;
			case PIDXMsr_FirstName:			m_sFirstName.SetSysString( pString );	break;
			case PIDXMsr_MiddleInitial:		m_sMiddleInitial.SetSysString( pString );	break;
			case PIDXMsr_ServiceCode:		m_sServiceCode.SetSysString( pString );	break;
			case PIDXMsr_Suffix:				m_sSuffix.SetSysString( pString );	break;
			case PIDXMsr_Surname:			m_sSurname.SetSysString( pString );	break;
			case PIDXMsr_Title:					m_sTitle.SetSysString( pString );	break;
			case PIDXMsr_Track1DiscretionaryData:	m_sTrack1DiscretionaryData.SetSysString( pString );	break;
			case PIDXMsr_Track2DiscretionaryData:	m_sTrack2DiscretionaryData.SetSysString( pString );	break;

			default:
				hResult = OPOS_E_ILLEGAL;
				break;
		}//end switch

	}

	m_lResultCode = hResult;

	if( pString ){
		Deb_Printf(_T("-- GetPropertyString(%s).\n"), *pString);

		if (b_hide_some_data_for_security && *pString != NULL) {
			BSTR p_d(*pString);
			int k = 0;
			std::vector<TCHAR> v_debug(0);
			while (p_d[k]) {
				if (k % 2 == 0) {
					v_debug.push_back(p_d[k]);
				}
				else {
					v_debug.push_back(_T('*'));
				}
				k++;
			}//end while
			v_debug.push_back((TCHAR)0);//make null terminate string
			SOTrace(true, CLog::LEV_HIGH, _T("-- GetPropertyString.(Value = %s)\n"), &v_debug[0]);
		}
		else {
			SOTrace(true, CLog::LEV_HIGH, _T("-- GetPropertyString.(Value = %s)\n"), *pString);
		}
	}
	else{
		Deb_Printf( _T("-- GetPropertyString(NULL).\n")  );
		SOTrace( true, CLog::LEV_HIGH, _T("-- GetPropertyString.(Value = NULL)\n") );
	}

	return S_OK;
}


HRESULT CSoMsr::SetPropertyString(long PropIndex, BSTR bstrString)
{
	Deb_Printf( _T("++ SetPropertyString(%s,%s).\n"),Deb_GetStringFromPropertyString(PropIndex),bstrString  );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ SetPropertyString(%s,%s).\n"),Deb_GetStringFromPropertyString(PropIndex),bstrString  );

	m_lResultCode = OPOS_E_ILLEGAL; 

	Deb_Printf( _T("-- SetPropertyString.\n")  );
	SOTrace( true, CLog::LEV_HIGH, _T("-- SetPropertyString.(Result = 0x%x)\n"), m_lResultCode );

	return S_FALSE;
}


HRESULT CSoMsr::GetOpenResult(long* pRC)
{
	Deb_Printf( _T("++ GetOpenResult.\n") );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ GetOpenResult.\n") );

	*pRC = m_lOpenResultCode;
	m_lResultCode = m_lOpenResultCode;

	Deb_Printf( _T("-- GetOpenResult.\n") );
	SOTrace( true, CLog::LEV_HIGH, _T("-- GetOpenResult.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::COFreezeEvents(VARIANT_BOOL Freeze, long* pRC)
{
	if( Freeze ){
		Msr_FreezeEvents();
		m_pFireEvent->setFreezeEvents();
	}
	else{
		Msr_FreezeEvents( false );
		m_pFireEvent->setFreezeEvents( false );
	}
	//
	m_lResultCode = *pRC = OPOS_SUCCESS;   
	return S_OK;
}


HRESULT CSoMsr::CheckHealth(long Level, long* pRC)
{
	Deb_Printf( _T("++ CheckHealth.\n") );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ CheckHealth(%s).\n"), Deb_GetStringFromCheckHealthLevel(Level) );

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
	SOTrace( true, CLog::LEV_HIGH, _T("-- CheckHealth.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::ClaimDevice(long lTimeout, long* pRC)
{
	Deb_Printf( _T("++ ClaimDevice.\n") );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ ClaimDevice.(%d)\n"), lTimeout);

	if( m_bClaimed ){
		m_lResultCode = OPOS_SUCCESS;
	}
	else{

		DWORD dwTime = lTimeout;

		if( lTimeout == OPOS_FOREVER )
			dwTime = INFINITE;

		if( m_ClaimMutex.Lock( dwTime ) ){
			m_lResultCode = OPOS_SUCCESS;
			m_bClaimed = TRUE;
		}
		else{
			m_lResultCode = OPOS_E_TIMEOUT;
		}
	}

	*pRC = m_lResultCode;

	Deb_Printf( _T("-- ClaimDevice.\n") );
	SOTrace( true, CLog::LEV_HIGH, _T("-- ClaimDevice.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::ClearInput(long* pRC)
{
	Deb_Printf( _T("++ ClearInput.\n") );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ ClearInput.\n") );

	if( m_bClaimed ){

		QClear();	//clear device Queue
		m_pFireEvent->ClearQ();
		m_lDataCount = 0;

		m_lResultCode = *pRC = OPOS_SUCCESS;
	}
	else{
		m_lResultCode = *pRC = OPOS_E_NOTCLAIMED;
	}

	Deb_Printf( _T("-- ClearInput.\n") );

	SOTrace( true, CLog::LEV_HIGH, _T("-- ClearInput.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::Close(long* pRC)
{
	Deb_Printf( _T("++ Close.\n") );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ Close.\n") );

	if( pRC == NULL ){
		m_lResultCode = OPOS_E_ILLEGAL;
	}
	else{

		if( Msr_Close() ){

			if( m_bClaimed )
				ReleaseDevice( pRC );

			m_lState = OPOS_S_CLOSED;
			*pRC = OPOS_SUCCESS;
			m_lResultCode = OPOS_E_CLOSED;
		}
		else{
			m_lResultCode = *pRC = OPOS_E_FAILURE;
		}
	}

	Deb_Printf( _T("-- Close.\n") );
	SOTrace( true, CLog::LEV_HIGH, _T("-- Close.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::DirectIO(long Command, long* pData, BSTR* pString, long* pRC)
{
	SOTrace( true, CLog::LEV_NORMAL, _T("++ DirectIO.\n") );

	if( !m_bDeviceEnabled ){
		m_lResultCode = *pRC = OPOS_E_ILLEGAL;
	}
	else{
		m_lResultCode = *pRC = OPOS_SUCCESS;
	}

	SOTrace( true, CLog::LEV_HIGH, _T("-- DirectIO.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::ReleaseDevice(long* pRC)
{
	Deb_Printf( _T("++ ReleaseDevice.\n")  );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ ReleaseDevice.\n") );

	if( m_bClaimed ){

		if( m_bDeviceEnabled ){
			Msr_Enable( false );
			m_bDeviceEnabled = FALSE;
		}

		QClear();	//clear queue.
		m_bClaimed = FALSE;

		m_ClaimMutex.Unlock();
		*pRC = OPOS_SUCCESS; 
	}
	else{
		*pRC = OPOS_E_NOTCLAIMED; 
		//*pRC = OPOS_SUCCESS;
	}

	m_lResultCode = *pRC;
	Deb_Printf( _T("-- ReleaseDevice(0x%x).\n"),m_lResultCode  );
	SOTrace( true, CLog::LEV_HIGH, _T("-- ReleaseDevice.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::ResetStatistics(BSTR StatisticsBuffer, long* pRC)
{
	SOTrace( true, CLog::LEV_NORMAL, _T("++ ResetStatistics.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	SOTrace( true, CLog::LEV_HIGH, _T("-- ResetStatistics.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::RetrieveStatistics(BSTR* StatisticsBuffer, long* pRC)
{
	SOTrace( true, CLog::LEV_NORMAL, _T("++ RetrieveStatistics.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	SOTrace( true, CLog::LEV_HIGH, _T("-- RetrieveStatistics.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}


HRESULT CSoMsr::UpdateStatistics(BSTR StatisticsBuffer, long* pRC)
{
	SOTrace( true, CLog::LEV_NORMAL, _T("++ UpdateStatistics.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	SOTrace( true, CLog::LEV_HIGH, _T("-- UpdateStatistics.(RC = 0x%x)\n"), *pRC );

	return S_OK;
}

HRESULT CSoMsr::UpdateKey( BSTR Key, BSTR KeyName, long* pRC )
{
	SOTrace( true, CLog::LEV_NORMAL, _T("++ UpdateKey.\n") );

	m_lResultCode = *pRC = OPOS_E_ILLEGAL;

	SOTrace( true, CLog::LEV_HIGH, _T("-- UpdateKey.(RC = 0x%x)\n"), *pRC );
	return S_OK;
}

bool CSoMsr::IsPathNameUart( const _tstring & sDevPath )
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

bool CSoMsr::Msr_Open(void)
{
	DevListType & vDevList =  GetDeviceList();

	if( vDevList.empty() ){
		SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_Open : NO DEVICE.\n") );
		return false;
	}

	DevListType::iterator iDev = vDevList.begin();

	for( ;iDev != vDevList.end(); ++iDev ){

		if( IsPathNameUart( iDev->c_str() ) ){
			m_bDeviceIsUart = true;
			break;	//exit for
		}
		else{
			m_bDeviceIsUart = false;
		}
	}

	if( !m_bDeviceIsUart ){
		iDev = vDevList.begin();
		ATLTRACE( _T(" + OPEN HID MODE\n") );
	}
	else{
		ATLTRACE( _T(" + OPEN UART MODE\n") );
	}

	if( m_bDeviceIsUart ){
		m_hDev = CreateFile(
				iDev->c_str(), 
				GENERIC_READ | GENERIC_WRITE, 
				0,//FILE_SHARE_READ | FILE_SHARE_WRITE
				(LPSECURITY_ATTRIBUTES)NULL, 
				OPEN_EXISTING, 
				FILE_FLAG_OVERLAPPED, 
				NULL
				);
	}
	else{
		m_hDev = CLpu237Dll::get_instance().LPU237_open( iDev->c_str() );
		CLpu237Dll::get_instance().LPU237_enable( m_hDev );
	}

	if( m_hDev == NULL || m_hDev == INVALID_HANDLE_VALUE ){
		m_hDev = NULL;
		SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_Open : CreateFile.\n") );
		return false;
	}

	if( m_bDeviceIsUart ){
		m_Uart.Config( m_hDev );
		m_Uart.SetTimeOut( m_hDev,0,0,0,0,0 );
	}

	//save device path
	m_sDevFullPath.Format( _T("%s"), iDev->c_str() );

	CSoMsr::CDevEvent::DataType vData;

	//enqueue Enter opos mode
	CSoMsr::sp_DevEvent Event( 
		new CSoMsr::CDevEvent( CDevEvent::Data,Cmd_EnterOps,0,0,vData, ::CreateEvent( NULL,TRUE, FALSE, NULL) )
		);

	m_pDeviceQ->EnQ( Event );
	m_pWorker->Wakeup();

	//wait to end of request
	DWORD dwResult = Event->WaitComplete( CONST_SYNC_WAITE_TIME );

	if( m_bDeviceIsUart ){
		::Sleep(70);// In Uart mode, need time-delay 100 msec. NOTICE !!!!!!!
	}

	return true;
}

bool CSoMsr::Msr_Close(void)
{
	CSoMsr::CDevEvent::DataType vData;

	//enqueue Leave opos mode
	CSoMsr::sp_DevEvent Event( 
		new CSoMsr::CDevEvent( CDevEvent::Data,Cmd_LeaveOps,0,0,vData, ::CreateEvent( NULL,TRUE, FALSE, NULL) )
		);

	m_pDeviceQ->EnQ( Event	 );
	m_pWorker->Wakeup();

	//wait to end of request
	DWORD dwResult = Event->WaitComplete( CONST_SYNC_WAITE_TIME );

	if( m_bDeviceIsUart ){
		CloseHandle( m_hDev );
	}
	else{
		CLpu237Dll::get_instance().LPU237_close( m_hDev );
	}
	m_hDev = NULL;

	return true;
}

bool CSoMsr::Msr_Enable(bool bEnable /*= true*/)
{
	return true;
}

bool CSoMsr::Msr_FreezeEvents( bool bFreeze /*= true*/ )
{
	return true;
}

bool CSoMsr::Msr_EnableDecode( bool bEnable /*= true*/ )
{
	return true;
}

CSoMsr::MsReadTransaction CSoMsr::Msr_GetMsData(void)
{

	return MSR_Complete;
}

int CSoMsr::GetStringFromMultiString( list<_tstring> & listStr, LPCTSTR szMultiStr )
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

CSoMsr::DevListType & CSoMsr::GetDeviceList()
{
	m_vDevPath.clear();

	vector<TCHAR> vPath(512,0);
	bool bResult = false;

	// 1. find connected device from HID.
	static list<_tstring> HidDevPathList;
	static INT32 nHidDev = 0;

	if( getDeviceChanged() ){

		setDeviceChanged( false );

		vector<TCHAR> v_buf;
		DWORD dwSize = CLpu237Dll::get_instance().LPU237_get_list( NULL );
		if( dwSize > 0 && dwSize != LPU237_DLL_RESULT_ERROR  ){
			v_buf.resize( dwSize, 0 );
			UINT nDev = CLpu237Dll::get_instance().LPU237_get_list( reinterpret_cast<LPTSTR>(&v_buf[0]) );
			CSoMsr::GetStringFromMultiString( HidDevPathList, reinterpret_cast<LPCTSTR>( &v_buf[0] ) );

			list<_tstring>::iterator dev = HidDevPathList.begin();
			for( dev = HidDevPathList.begin(); dev != HidDevPathList.end(); dev++ ){
				_tcscpy( &vPath[0], (*dev).c_str() );
				m_vDevPath.push_back( *dev );
			}//end for

			HidDevPathList.resize( m_vDevPath.size() );
			copy( m_vDevPath.begin(), m_vDevPath.end(), HidDevPathList.begin() );
		}
		else{
			ATLTRACE( _T("* NOT FOUND HID DEVICE.\n") );
		}
	}
	else{
		m_vDevPath.resize( HidDevPathList.size() );
		copy( HidDevPathList.begin(), HidDevPathList.end(), m_vDevPath.begin() );
	}

	list<_tstring> DevPathList;

	INT32 nDev = m_Uart.GetDevList( DevPathList );// search device from uart

	if( nDev > 0 ){
		list<_tstring>::iterator dev = DevPathList.begin();

		for( dev = DevPathList.begin(); dev != DevPathList.end(); dev++ ){
			m_vDevPath.push_back( *dev );
		}//end for
	}
	else{
		//ATLTRACE( _T("= NOT FOUND UART DEVICE.\n") );
	}

	return m_vDevPath;
}

void CSoMsr::QClear()
{
	m_pDeviceQ->QClear();
}

//send request & get response
bool CSoMsr::Msr_SendRequest( 
	const MSR_HostCmd Cmd, 
	const unsigned char cSub,
	INT32 nData, 
	const unsigned char *sData, 
	unsigned char *sResp /*= NULL*/,
	HANDLE hDev /*= NULL*/ )
{

	if( hDev == NULL )
		hDev = m_hDev;

	bool bResult = false;

	INT32 nBuf;
	INT32 nPacket;
	INT32 nRemainder;
	INT32 nOffset = 0;

	unsigned char sBuf[512];
	unsigned char sPacket[65];

	::memset( sBuf,0,512 );

	sBuf[0] = 0;	//report ID
	sBuf[1] = (unsigned char)Cmd;
	sBuf[2] = cSub;
	sBuf[3] = (unsigned char)nData;

	if( nData > 0 )
		::memcpy( &sBuf[4], sData,nData );

	if( nData >= sizeof(UINT32)*2 ){
		UINT32 dwOff,dwSize;
		::memcpy( &dwOff, sData, sizeof(dwOff) );
		::memcpy( &dwSize, &sData[sizeof(dwOff)], sizeof(dwSize) );
	}

	CEventCase CompleteEvent( ::CreateEvent( NULL, TRUE, FALSE, NULL) );
	OVERLAPPED ov;
	::memset( &ov, 0, sizeof(ov) );
	ov.hEvent = CompleteEvent.GetHandle();

	/////////////////////////////////////////////////////////
	//the first send
	nRemainder = nData + 3 + 1;

	::memset( sPacket, 0 , 65 );

	if( nRemainder > 65 ){
		::memcpy( sPacket, sBuf, 65 );
		nPacket = 65;
		nOffset += nPacket;
		nRemainder -= 65;
	}
	else{
		::memcpy( sPacket, sBuf, 65 );
		nPacket = nRemainder;
		nOffset += nPacket;
		nRemainder = 0;
	}

	if( !Write( sPacket, 65,(LPDWORD)&nPacket,&ov,hDev ) ){
		DWORD dwResult = ::GetLastError();

		if( ERROR_IO_PENDING != dwResult ){
			SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : Write(0x%x).\n"),dwResult );
			return false;
		}
		//
		DWORD dwTransfer = 0;
		if( !::GetOverlappedResult( hDev, &ov, &dwTransfer, TRUE) ){
			SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : Write-GetOverlappedResult(0x%x).\n"),::GetLastError() );
			return false;
		}

		//Write complete.
	}

	/////////////////////////////////////////////////////////
	//the partial send
	while( nRemainder > 0 ){

		::memset( sPacket, 0 , 65 );

		if( nRemainder > 64 ){
			::memcpy( &sPacket[1], &sBuf[nOffset], 64 );
			nPacket = 64;
			nOffset += nPacket;
			nRemainder -= 64;
		}
		else{
			::memcpy( &sPacket[1],  &sBuf[nOffset], nRemainder );
			nPacket = nRemainder;
			nOffset += nPacket;
			nRemainder = 0;
		}

		CompleteEvent.Reset();
		::memset( &ov, 0, sizeof(ov) );
		ov.hEvent = CompleteEvent.GetHandle();

		if( !Write( sPacket, 65,(LPDWORD)&nPacket,&ov,hDev ) ){
			DWORD dwResult = ::GetLastError();

			if( ERROR_IO_PENDING != dwResult ){
				SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : Write(0x%x).\n"),dwResult );
				return false;
			}
			//
			DWORD dwTransfer = 0;
			if( !::GetOverlappedResult( hDev, &ov, &dwTransfer, TRUE) ){
				SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : Write-GetOverlappedResult(0x%x).\n"),::GetLastError() );
				return false;
			}
		}
	}//end while


	/////////////////////////////////////////////////////////
	//getting response.
	nBuf = 1+3+76+37+104;
	::memset( sBuf,0,nBuf );

	CompleteEvent.Reset();
	::memset( &ov, 0, sizeof(ov) );
	ov.hEvent = CompleteEvent.GetHandle();

	DWORD dwTransfer = 0;

	if( !Read( sBuf, (DWORD)nBuf, (LPDWORD)&dwTransfer,&ov,hDev ) ){
		DWORD dwResult = ::GetLastError();

		if( ERROR_IO_PENDING != dwResult ){
			SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : Read(0x%x).\n"),dwResult );
			return false;
		}
		//
		if( !::GetOverlappedResult( hDev, &ov, &dwTransfer, TRUE) ){
			SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : Read-GetOverlappedResult(0x%x).\n"),::GetLastError() );
			return false;
		}
	}

	if( sBuf[1] == MSR_RESP_PREFIX && sBuf[2] == Resp_Good ){
		bResult = true;

		if( sResp ){
			::memcpy( sResp, &sBuf[1], nBuf-1 );
		}
	}
	else{
		SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Msr_SendRequest : UNKNOWN repones FORMAT.\n") );
		return false;
	}

	return true;
}

bool CSoMsr::Msr_SendRequest( const MSR_HOST_PACKET & Req,  PMSR_HOST_PACKET Resp /*= NULL*/, HANDLE hDev /*= NULL*/  )
{
	return Msr_SendRequest( 
		static_cast<MSR_HostCmd>(Req.cCmd),
		Req.cSub, static_cast<INT32>(Req.cLen),
		static_cast<const unsigned char*>(Req.sData),
		reinterpret_cast<unsigned char*>(Resp),
		hDev
		);
}

bool CSoMsr::FindOpenEnterOpos(  bool bFlashHandle /*= true*/ )
{
	bool bResult = true;

	if( m_hDev ){
		if( m_bDeviceIsUart )
			CloseHandle( m_hDev );
		else
			CLpu237Dll::get_instance().LPU237_close( m_hDev );
		//
		m_hDev = NULL;
	}

	if( !bFlashHandle ){
		return bResult;
	}

	//find device. & open
	DevListType & vDevList =  GetDeviceList();

	if( !vDevList.empty() ){

		DevListType::iterator iDev = vDevList.begin();

		for( ;iDev != vDevList.end(); ++iDev ){

			if( IsPathNameUart( iDev->c_str() ) ){
				m_bDeviceIsUart = true;
				break;	//exit for
			}
			else
				m_bDeviceIsUart = false;
		}

		if( !m_bDeviceIsUart ){
			iDev = vDevList.begin();
			m_hDev = CLpu237Dll::get_instance().LPU237_open( iDev->c_str() );
			if( m_hDev == INVALID_HANDLE_VALUE )
				m_hDev = NULL;
		}
		else{
			m_hDev = CreateFile(
					iDev->c_str(), 
					GENERIC_READ | GENERIC_WRITE, 
					0,//FILE_SHARE_READ | FILE_SHARE_WRITE,
					(LPSECURITY_ATTRIBUTES)NULL, 
					OPEN_EXISTING, 
					FILE_FLAG_OVERLAPPED, 
					NULL
					);
		}

		if( m_hDev != NULL ){

			if( m_bDeviceIsUart ){
				m_Uart.Config( m_hDev );
				m_Uart.SetTimeOut( m_hDev,0,0,0,0,0 );
			}

			//save device path
			m_sDevFullPath.Format( _T("%s"), iDev->c_str() );

			CSoMsr::CDevEvent::DataType vData;

			//build enter request
			MSR_HOST_PACKET Cmd;
			::memset( &Cmd, 0, sizeof(Cmd) );
			Cmd.cCmd = Cmd_EnterOps;

			if( m_bDeviceIsUart ){
				if( !Msr_SendRequest( Cmd ) )
					bResult = false;
			}
		}
		else
			bResult = false;
	}
	else{
		ATLTRACE( _T(" = FindOpenEnterOpos : NOT FOUND DEVICE.\n") );
		bResult = false;
	}


	return bResult;
}

void CSoMsr::SaveMsrData( LPMSR_DATA_PACKET pMsData )
{
	if( pMsData == NULL )
		return;

	int nOffset = 0;
	long lMask = 1;

	TCHAR cStx[]={ C_MSR_ISO1_STX, C_MSR_ISO2_STX, C_MSR_ISO3_STX };
	TCHAR cEtx[]={ C_MSR_ISO1_ETX, C_MSR_ISO2_ETX, C_MSR_ISO3_ETX };

	int i;

	if( m_bDecodeData ){
		for( i = 0; i<3; i++){

			if( i==0 ){
				cStx[i] += 0x20;	cEtx[i] += 0x20;
			}
			else{
				cStx[i] += 0x30;	cEtx[i] += 0x30;
			}
		}//end for
	}

	m_bIsATrackError = false;
	
	for( i=0; i<3; i++ ){

		if( pMsData->cErrorSize[i] > 0 ){

			if( m_lTracksToRead & lMask ){

				wchar_t *pwc = (wchar_t *)new wchar_t[(pMsData->cErrorSize[i] + 1)];
				::memset( pwc,0, sizeof(wchar_t)*(pMsData->cErrorSize[i] + 1) );

				for( signed char j = 0; j<pMsData->cErrorSize[i]; j++ ){

					if( m_bDecodeData ){
						//change format....... to ASCII
						if( i==0 )
							pMsData->sData[nOffset+j] += 0x20;	//ISO1 track.
						else
							pMsData->sData[nOffset+j] += 0x30;	//ISO 2,3 track
					}
				}//end for

				size_t nSize = ::mbstowcs( pwc, reinterpret_cast<const char *>(&(pMsData->sData[nOffset])), pMsData->cErrorSize[i] );
				pwc[pMsData->cErrorSize[i]] = NULL;

				if( m_bTransmitSentinels )
					m_vTrackData[i].Format( _T("%c%s%c"), cStx[i],pwc,cEtx[i] );
				else
					m_vTrackData[i].Format( _T("%s"), pwc );
				//	
				delete pwc;
			}
			else{
				m_vTrackData[i].Empty();
			}

			nOffset += pMsData->cErrorSize[i];
			m_bTrackError[i] = false;
		}
		else if( pMsData->cErrorSize[i] == 0 ){
			//No error, no data
			m_vTrackData[i].Empty();		m_bTrackError[i] = false;
		}
		else{//error
			m_vTrackData[i].Empty();		m_bTrackError[i] = true;
		}

		lMask <<= 1;
		m_bIsATrackError |= m_bTrackError[i];
		m_cErrorCode[i] = ChangeErrorCode( pMsData->cErrorSize[i] );	//save error
	}//end for

	if( m_bIsATrackError && m_lErrorReportingType == MSR_ERT_CARD ){
		//remove data.
		TrackDataType::iterator iter = m_vTrackData.begin();

		for( ; iter != m_vTrackData.end(); ++iter )
			iter->Empty();
	}

	if( m_bParseDecodeData )
		ParsingCardData( pMsData );
}

CFireEvent::CMsrEvent::EventType CSoMsr::Fire( DWORD dwErrorCode )
{
	Deb_Printf( _T("++ *Fire(0x%x).\n"),dwErrorCode  );
	SOTrace( true, CLog::LEV_NORMAL, _T("++ Fire(0x%x).\n"),dwErrorCode );

	CFireEvent::CMsrEvent::EventType EvtType = CFireEvent::CMsrEvent::TypeData;
	CFireEvent::PtrMsrEvent MsrEvent;
	long lErrorLocaus = OPOS_EL_INPUT;

	DWORD dwStatus = 0;
	signed char cErrorCode[3];

	bool bError = false;

	int i;
	for( i = 0; i<3 ; i++ )
		cErrorCode[i] = m_cErrorCode[i];

	//MAKELONG(low,high); //MAKEWORD(LO,HI)

	if( (m_lTracksToRead & MSR_TR_1) == 0 )
		cErrorCode[0] = 0;
	if( (m_lTracksToRead & MSR_TR_2) == 0 )
		cErrorCode[1] = 0;
	if( (m_lTracksToRead & MSR_TR_3) == 0 )
		cErrorCode[2] = 0;
	//check error
	for(  i = 0; i<3 ; i++ ){
		if( cErrorCode[i] < 0 )
			bError = true;
	}//end for

	dwStatus = MAKELONG( MAKEWORD( cErrorCode[0], cErrorCode[1] ), MAKEWORD( cErrorCode[2], 0 ) );

	if( dwErrorCode == ERROR_SUCCESS ){

		if( !bError ){
			// report success.......
			SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] Fire : SUCCESS.\n")  );

			EvtType = CFireEvent::CMsrEvent::TypeData;
			MsrEvent = CFireEvent::PtrMsrEvent( new CFireEvent::CMsrEvent( EvtType, dwStatus ) );
			SOTrace( true, CLog::LEV_HIGH, _T("[DETAIL] Start Fire.\n")  );
			m_pFireEvent->Fire( MsrEvent );
			SOTrace( true, CLog::LEV_HIGH, _T("[DETAIL] End Fire.\n")  );
		}
		else{
			// Error
			if( m_lErrorReportingType == MSR_ERT_TRACK ){
				// report error

				for( i = 0; i<3 ; i++ ){
					if( cErrorCode[i] > 0 ){
						cErrorCode[i] = 0;
					}
				}//end for

				dwStatus = MAKELONG( MAKEWORD( cErrorCode[0], cErrorCode[1] ), MAKEWORD( cErrorCode[2], 0 ) );

				if( CGlobalVar::GetInstance()->get_ctl_enable_erttrack_ms_error() ){
					EvtType = CFireEvent::CMsrEvent::TypeError;
					MsrEvent = CFireEvent::PtrMsrEvent( new CFireEvent::CMsrEvent( 
						EvtType, 
						OPOS_E_EXTENDED,
						dwStatus,
						lErrorLocaus
						) );

					m_pFireEvent->Fire( MsrEvent );
					ErrorProcessWithErrorResponse( m_pFireEvent->getLastErrorResponse() );
					SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] Fire : ERROR( STATUS,LOCAUS )=(0x%x, 0x%x). R(%d)\n"), dwStatus, lErrorLocaus,m_pFireEvent->getLastErrorResponse()  );
				}
			}
			else{//MSR_ERT_CARD
				// report error
				dwStatus = OPOSERR;
				lErrorLocaus = 0;
				long lResult = OPOS_E_EXTENDED;
				
				EvtType = CFireEvent::CMsrEvent::TypeError;

				if( CGlobalVar::GetInstance()->get_ctl_enable_ertcard_ms_error() ){
				
					MsrEvent = CFireEvent::PtrMsrEvent( new CFireEvent::CMsrEvent( 
						EvtType, 
						lResult,
						dwStatus,
						lErrorLocaus
						) );

					m_pFireEvent->Fire( MsrEvent );
					ErrorProcessWithErrorResponse( m_pFireEvent->getLastErrorResponse() );
				}
				else{
					SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] NOT Fire : ERROR.\n")  );
				}
			}
		}
	}
	else{
		//conmuincation error
		//report error

		if( CGlobalVar::GetInstance()->get_ctl_enable_commuicate_error() ){
			EvtType = CFireEvent::CMsrEvent::TypeError;
			MsrEvent = CFireEvent::PtrMsrEvent( new CFireEvent::CMsrEvent( 
				EvtType, 
				OPOS_E_NOEXIST, //result
				0,//extened
				lErrorLocaus//locals
				) );

			m_pFireEvent->Fire( MsrEvent );

			ErrorProcessWithErrorResponse( m_pFireEvent->getLastErrorResponse() );

			SOTrace( true, CLog::LEV_LOW, _T("[ERROR] Fire : COMMUICATE ERROR R(%d).\n"),m_pFireEvent->getLastErrorResponse() );
		}
	}

	m_lDataCount = m_pFireEvent->getQSize();

	Deb_Printf( _T("-- *Fire.\n")  );
	SOTrace( true, CLog::LEV_HIGH, _T("-- Fire.\n") );

	return EvtType;
}

void CSoMsr::ErrorProcessWithErrorResponse( long lError )
{
	TrackDataType::iterator iter = m_vTrackData.begin();
	CFireEvent::PtrMsrEvent MsrEvent;
	long lErrorLocaus = OPOS_EL_INPUT;
	long lResultEx = 0;

	switch( lError ){
		case OPOS_ER_RETRY:
			break;
		case OPOS_ER_CLEAR:
			//remove data.
			for( ; iter != m_vTrackData.end(); ++iter )
				iter->Empty();

			for( int i=0; i<CONST_MAX_TRACK; i++ ){
				m_bTrackError[i] = false;
				m_cErrorCode[i] = 0;
			}
			break;
		case OPOS_ER_CONTINUEINPUT:
			break;
		default:
			break;
	}//end switch
}

//change hardware error code to Opos error code
unsigned char CSoMsr::ChangeErrorCode( signed char cError )
{
	signed char cOposError = 0;

	switch( cError ){
		case C_MSR_ERROR_PARITY:
			cOposError = static_cast<unsigned char>(OPOS_EMSR_PARITY);
			break;
		case C_MSR_ERROR_LRC:
			cOposError = static_cast<unsigned char>(OPOS_EMSR_LRC);
			break;
		case C_MSR_ERROR_STX:
			cOposError = static_cast<unsigned char>(OPOS_EMSR_START);
			break;
		case C_MSR_ERROR_ETX:
			cOposError = static_cast<unsigned char>(OPOS_EMSR_END);
			break;
		case C_MSR_WARN_OVER:
			cOposError = static_cast<unsigned char>(OPOS_EMSR_END);
			break;
		case C_MSR_GOOD:
		default:
			cOposError = cError;
			break;

	}//end switch

	return cOposError;
}

//parsing card data
void CSoMsr::ParsingCardData(  LPMSR_DATA_PACKET pMsData )
{
	//reset data
	m_sAccountNumber.Empty();
	m_sExpirationDate.Empty();
	m_sFirstName.Empty();
	m_sMiddleInitial.Empty();
	m_sServiceCode.Empty();
	m_sSuffix.Empty();
	m_sSurname.Empty();
	m_sTitle.Empty();
	m_sTrack1DiscretionaryData.Empty();
	m_sTrack2DiscretionaryData.Empty();

	int nData[] = { 0, 0 };
	LPBYTE pData[] = { NULL, NULL };

	if(  pMsData->cErrorSize[0]>0 )
		nData[0] = pMsData->cErrorSize[0];
	if(  pMsData->cErrorSize[1]>0 )
		nData[1] = pMsData->cErrorSize[1];

	pData[0] = &(pMsData->sData[0]);
	pData[1] = &(pMsData->sData[ nData[0] ]);

	int i, j;
	if( !m_bDecodeData ){
		// converts ASCII code.
		for( i = 0; i<2; i++ ){

			for( j =0; j< nData[i]; j++ ){
				if( i==0 )
					pData[i][j] += 0x20;	//ISO1 track.
				else
					pData[i][j] += 0x30;	//ISO 2,3 track
			}//end for
		}//end for
	}

	enum : int{
		NM_CountryCode = 0,
		NM_Surname = 1,
		NM_FirstName = 2,
		NM_MiddleInitial = 3,
		NM_Title = 4
	};

	int CurNameMode = NM_Surname;
	int nItemCount = 0;
	////////////////////
	//parsing track1
	CString sToken, sTemp;
	char cSeparator ='^';
	int nSeparator = 0;
	int k=0;
	i = 0;	j = 0;

	while( j<nData[i] ){

		if( j == 0 && pData[i][j] != 'B' ){
			break;	//mismatch format
		}

		if( pData[i][j] != cSeparator ){

			if( nSeparator == 2 ){

				if( k >= 7 ){
					// save m_sTrack1DiscretionaryData
					sToken.Format(_T("%c"), (TCHAR)(pData[i][j]) );
					m_sTrack1DiscretionaryData += sToken;
				}

				k++;
			}
			else if( nSeparator == 1 ){
				//name
				sToken.Format(_T("%c"), (TCHAR)(pData[i][j]) );

				switch( CurNameMode ){
					case NM_CountryCode:
						nItemCount++;
						if( nItemCount==3 ){
							CurNameMode++;
							nItemCount = 0;
						}
						break;
					case NM_Surname:
						if( pData[i][j] != '/' && pData[i][j] != ' ' ){
							m_sSurname += sToken;
							nItemCount++;
						}
						else{
							CurNameMode++;
							nItemCount = 0;
						}
						break;
					case NM_FirstName:
						if( pData[i][j] != ' ' ){
							m_sFirstName += sToken;
							nItemCount++;
						}
						else{
							CurNameMode++;
							nItemCount = 0;
						}
						break;
					case NM_MiddleInitial:
						if( pData[i][j] != ' ' && pData[i][j] != '.' ){
							m_sMiddleInitial += sToken;
							nItemCount++;
						}
						else if( pData[i][j] == '.' ){
							CurNameMode++;
							nItemCount = 0;
						}
						else{
							CurNameMode+=2;
							nItemCount = 0;
						}
						break;
					case NM_Title:
						if( pData[i][j] != ' ' ){
							m_sTitle += sToken;
							nItemCount++;
						}
						break;
					default:
						break;
				}//end switch
			}
			else if( nSeparator == 0 ){
				//PA
				if( j == 2 ){
					if( pData[i][1] == '5' && pData[i][2] == '9' )
						CurNameMode = NM_CountryCode;
				}
			}
		}
		else{
			nSeparator++;
		}
		//
		j++;
	}//end while

	////////////////////
	//parsing track2
	cSeparator = '=';
	nSeparator = 0;
	i++;	
	j = 0;

	k=0;

	while( j<nData[i] ){

		if( pData[i][j] == cSeparator ){
			nSeparator++;
		}
		else{
			sToken.Format(_T("%c"), (TCHAR)(pData[i][j]) );

			if( nSeparator == 0 ){
				// save m_sAccountNumber

				if( j < 20 )
					m_sAccountNumber += sToken;
				else{
					m_sAccountNumber.Empty();
					break; //exit while invalied format.
				}
			}
			else if( nSeparator == 1 ){

				if( k >= 0 && k <4 ){
					// save m_sExpirationDate
					m_sExpirationDate += sToken;
				}

				else if( k >= 4 && k < 7 ){
					// save m_sServiceCode
					m_sServiceCode += sToken;
				}
				else{
					// save m_sTrack2DiscretionaryData
					m_sTrack2DiscretionaryData += sToken;
				}

				k++;
			}

		}

		j++;
	}//end while

}


BOOL CSoMsr::Write(
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten/*=NULL*/,
		LPOVERLAPPED lpOverlapped /*=NULL*/,
		HANDLE hDev /*= NULL*/
	)
{

	if( hDev == NULL )
		hDev = m_hDev;

	BOOL bResult = ::WriteFile( hDev,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped );
	
	return bResult;

}


BOOL CSoMsr::Read(
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped /*=NULL*/,
		HANDLE hDev /*= NULL*/
	)
{
	if( hDev == NULL )
		hDev = m_hDev;

	BOOL bResult = ::ReadFile( hDev,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped );
	
	return bResult;

}

bool CSoMsr::EnQTrackData( const PtrTrackDataType & pTrackData )
{
	bool bResult = true;
	//
	m_qTrackData.push_back( pTrackData );

	return bResult;
}

bool CSoMsr::DeQTrackData( PtrTrackDataType & pTrackData )
{
	bool bResult = true;

	if( !m_qTrackData.empty() ){
		pTrackData = m_qTrackData.front();
		m_qTrackData.pop_front();
	}
	else
		bResult = false;

	return bResult;
}

////////////////////////////////////////////////////////
// CWorker class
///////////////////////////////////////////////////////
volatile DWORD CSoMsr::CWorker::m_dwLastError = ERROR_SUCCESS;
volatile bool CSoMsr::CWorker::m_bOpen = false;
volatile bool CSoMsr::CWorker::m_bSuspend = false;
volatile bool CSoMsr::CWorker::m_bRecover = false;

void CSoMsr::CWorker::Kill()
{
	if( m_hWorker ){
		//here kill worker
		m_KillEvent.Set();

		DWORD dwResult = 0;
		int nCount = 5000/50;


		if( m_bRunning ){

			//dwResult = ::WaitForSingleObject( m_hWorker, INFINITE );
			bool bWait = true;

			do{
				dwResult = ::WaitForSingleObject( m_hWorker, 50 );
				nCount--;

				if( WAIT_TIMEOUT != dwResult ){
					bWait = false;
					CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CWorker::Kill() : dwResult = 0x%x.\n"),dwResult );
					ATLTRACE( _T("[DETAIL] : CWorker::Kill() : dwResult = 0x%x.\n"),dwResult  );
				}
				else if( !m_bRunning ){
					bWait = false;
					CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CWorker::Kill() : m_bRunning = false.\n") );
					ATLTRACE( _T("[DETAIL] : CWorker::Kill() : m_bRunning = false.\n")  );
				}
				else if( nCount<=0 ){//waits timer over
					bWait = false;
					CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] : CWorker::Kill() : WAITS TIME-OVER..\n") );
					ATLTRACE(  _T("[DETAIL] : CWorker::Kill() : WAITS TIME-OVER..\n")  );
				}

			}while( bWait );
		}

		::CloseHandle( m_hWorker );
		m_hWorker = NULL;

		if( m_bOpen )
			if( !m_bRecover ){

				if( m_pSoMsr ){
					//build leave request
					if( m_pSoMsr->m_bDeviceIsUart ){
						MSR_HOST_PACKET Cmd;
						::memset( &Cmd, 0, sizeof(Cmd) );
						Cmd.cCmd = Cmd_LeaveOps;

						::CancelIo( m_pSoMsr->m_hDev );
						m_pSoMsr->Msr_SendRequest( Cmd );
						m_pSoMsr->SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] CWorker : Kill : SEND_LEAVE_OPOS_MODE. UART\n")  );
					}
				}
			}
	}
}

// Return false, exit thread
// Normal processing.......
bool CSoMsr::CWorker::MsrWoker_Normal_Uart( CSoMsr *pSo, DWORD dwWaitResult  )
{
	CSoMsr::sp_DevEvent Req;
	bool bRun = true;
	//static bool bReqExecuteRead = false;
	LPMSR_DATA_PACKET pMsData = NULL;
	bool bFire = true;

	CWorkQueue *pDeviceQ = pSo->m_pDeviceQ;

	switch( dwWaitResult ){
		case CWorker::Evt_Kill:
			bRun = false;	//exit worker.
			pSo->SOTrace( true, CLog::LEV_HIGH, _T("[DETAIL] MsrWoker_Normal_Uart : EVENT : KILL.\n")  );
			break;
		case CWorker::Evt_Wakeup:// run job.
		case CWorker::Evt_TimeOut:

			//if( !pDeviceQ->IsEmpty() ){//removed.. 20150330
			if( pDeviceQ->DeQ( Req ) ){
						
				switch( Req->GetCmd() ){
					case Cmd_EnterOps:
						::CancelIo( pSo->m_hDev );
						pSo->Msr_SendRequest( *(Req->GetTxData()) );
						Req->Complete();
						m_bOpen = true;
						m_bReqExecuteRead = true;
						break;
					case Cmd_LeaveOps:
						::CancelIo( pSo->m_hDev );
						pSo->Msr_SendRequest( *(Req->GetTxData()) );
						Req->Complete();
						m_bOpen = false;
						m_bReqExecuteRead = false;
						break;
					default://unsupported request
						break;
				}//switch
			}// end dequeue
			else{

				if( m_bReqExecuteRead ){

					::CancelIo( pSo->m_hDev );

					m_ReadDoneEvent.Reset();
					::memset( &m_Ov, 0, sizeof(m_Ov) );
					m_Ov.hEvent = m_ReadDoneEvent.GetHandle();
						
					if( ::ReadFileEx( pSo->m_hDev, m_sRead, m_dwRead, &m_Ov, MsrReadDoneComplete ) ){
						m_bReqExecuteRead = false;
						ATLTRACE( _T("SUCCESS : MsrWoker_Normal_Uart : ReadFileEx().\n") );
					}
					else{
						ATLTRACE( _T("FAILE : MsrWoker_Normal_Uart : ReadFileEx(%d).\n"), ::GetLastError() );
						pSo->SOTrace( true, CLog::LEV_LOW, _T("FAILE : MsrWoker_Normal_Uart : ReadFileEx(%d).\n"), ::GetLastError() );
					}
				}

				if( pSo->m_pFireEvent->FireQ() )
					pSo->m_lDataCount = 0;
			}
			break;
		case CWorker::Evt_ReadDone://read done
				
			if( m_bOpen ){

				if( pSo->m_bDeviceEnabled ){
					if( ERROR_SUCCESS == m_dwLastError ){
						//get MS data.
						pMsData = reinterpret_cast<LPMSR_DATA_PACKET>(m_sRead);
						pSo->SaveMsrData( pMsData );
					}
					else{
						m_bRecover = true;	//change
						pSo->m_lState = OPOS_S_ERROR;

						::CloseHandle( pSo->m_hDev );
						pSo->m_hDev = NULL;

					}

					if( bFire ){
						//fire event
						if( pSo->Fire( m_dwLastError ) == CFireEvent::CMsrEvent::TypeData )
							if( pSo->m_bAutoDisable )
								pSo->m_bDeviceEnabled = FALSE;
					}
				}

				if( !m_bReqExecuteRead )
					m_bReqExecuteRead = true;	//repeats reading operation in next time-out.

				ATLTRACE( _T(" * Read_done.\n" ) );
			}
			break;
		default://may be error
			break;
	}//end switch

	return bRun;
}

bool CSoMsr::CWorker::MsrWoker_Abnormal_Uart( CSoMsr *pSo, DWORD dwWaitResult  )
{
	CSoMsr::sp_DevEvent Req;
	bool bRun = true;

	CWorkQueue *pDeviceQ = pSo->m_pDeviceQ;

	switch( dwWaitResult ){
		case CWorker::Evt_Kill:
			bRun = false;	//exit worker.
			break;
		case CWorker::Evt_Wakeup:// run job.
		case CWorker::Evt_TimeOut:

			//if( !pDeviceQ->IsEmpty() ){//remark 20150330
			if( pDeviceQ->DeQ( Req ) ){
						
				switch( Req->GetCmd() ){
					case Cmd_EnterOps:
						Req->Complete();
						break;
					case Cmd_LeaveOps:
						::CancelIo( pSo->m_hDev );
						Req->Complete();
						m_bOpen = false;
						m_bRecover = false;	//change, not recoverd but. bypass because of closing device.
						break;
					default://unsupported request
						break;
				}//switch
			}// end dequeue
			else{
				//find device. & reopen
				if( pSo->FindOpenEnterOpos() ){

					m_bOpen = true;
					m_bRecover = false;	//recover OK........
					pSo->m_lState = OPOS_S_IDLE;

					if( !m_bReqExecuteRead ){
						m_bReqExecuteRead = true;//recover from Power suspend.
					}
					ATLTRACE( _T("*** Recover OK.......\n") );
					CLog::GetLog()->Log( true,  CLog::LEV_NORMAL, _T("[INFO] MsrWoker_Abnormal_Uart : RECOVER : OK.\n") );
				}
			}
			break;
		case CWorker::Evt_ReadDone://read done
				
			//fire event
			pSo->Fire( m_dwLastError );

			break;
		default://may be error
			break;
	}//end switch
	return bRun;
}

// Return false, exit thread
// Normal processing.......
bool CSoMsr::CWorker::MsrWoker_Normal_Usb( CSoMsr *pSo, DWORD dwWaitResult  )
{
	static DWORD dw_result_index( LPU237_DLL_RESULT_ERROR );

	CSoMsr::sp_DevEvent Req;
	bool bRun = true;
	//static bool bReqExecuteRead = false;
	LPMSR_DATA_PACKET pMsData = NULL;
	bool bFire = true;

	CWorkQueue *pDeviceQ = pSo->m_pDeviceQ;

	switch( dwWaitResult ){
		case CWorker::Evt_Kill:
			bRun = false;	//exit worker.
			pSo->SOTrace( true, CLog::LEV_HIGH, _T("[DETAIL] MsrWoker_Normal_Usb : EVENT : KILL.\n")  );
			break;
		case CWorker::Evt_Wakeup:// run job.
		case CWorker::Evt_TimeOut:

			if( pDeviceQ->DeQ( Req ) ){
						
				switch( Req->GetCmd() ){
					case Cmd_EnterOps:
						Req->Complete();
						m_bOpen = true;
						m_bReqExecuteRead = true;
						break;
					case Cmd_LeaveOps:
						Req->Complete();
						m_bOpen = false;
						m_bReqExecuteRead = false;
						break;
					default://unsupported request
						break;
				}//switch
			}// end dequeue
			else{
				if( m_bReqExecuteRead ){

					::CancelIo( pSo->m_hDev );

					m_ReadDoneEvent.Reset();
					::memset( &m_Ov, 0, sizeof(m_Ov) );
					m_Ov.hEvent = m_UsbMsrDoneEvent.GetHandle();
						
					dw_result_index = CLpu237Dll::get_instance().LPU237_wait_swipe_with_callback( pSo->m_hDev, CSoMsr::CWorker::MsrReadDoneCompleteUsb, &m_Ov );
					if( dw_result_index != LPU237_DLL_RESULT_ERROR ){
						m_bReqExecuteRead = false;
						ATLTRACE( _T("SUCCESS : MsrWoker_Normal_Usb : LPU237_wait_swipe_with_callback().\n") );
					}
					else{
						ATLTRACE( _T("FAILE : MsrWoker_Normal_Usb : LPU237_wait_swipe_with_callback(0x%X).\n"), dw_result_index );
						pSo->SOTrace( true, CLog::LEV_LOW, _T("FAILE : MsrWoker_Normal_Usb : LPU237_wait_swipe_with_callback(0x%X).\n"), dw_result_index );
					}
				}

				if( pSo->m_pFireEvent->FireQ() )
					pSo->m_lDataCount = 0;
			}
			break;
		case CWorker::Evt_UsbReadDone://read done
				
			if( m_bOpen ){

				if( pSo->m_bDeviceEnabled ){
					if( MsWorker_Normal_Usb_get_msr_data( dw_result_index )   ){
						if( ERROR_SUCCESS == m_dwLastError ){
							//get MS data.
							pMsData = reinterpret_cast<LPMSR_DATA_PACKET>(m_sRead);
							pSo->SaveMsrData( pMsData );
						}
						else{
							m_bRecover = true;	//change
							pSo->m_lState = OPOS_S_ERROR;

							CLpu237Dll::get_instance().LPU237_close( pSo->m_hDev );
							pSo->m_hDev = NULL;

						}

						if( bFire ){	//fire event
							if( pSo->Fire( m_dwLastError ) == CFireEvent::CMsrEvent::TypeData )
								if( pSo->m_bAutoDisable )
									pSo->m_bDeviceEnabled = FALSE;
						}
					}
				}

				if( !m_bReqExecuteRead )
					m_bReqExecuteRead = true;	//repeats reading operation in next time-out.

				ATLTRACE( _T(" * Read_done.\n" ) );
			}
			break;
		default://may be error
			break;
	}//end switch

	return bRun;
}

bool CSoMsr::CWorker::MsWorker_Normal_Usb_get_msr_data( unsigned int n_result_index )
{
	bool b_fire(false);
	LPMSR_DATA_PACKET pMsData = reinterpret_cast<LPMSR_DATA_PACKET>(m_sRead);
	m_dwLastError = ERROR_SUCCESS;
	bool b_error[] = { false, false, false };

	do{
		DWORD dwCount[] = { LPU237_DLL_RESULT_ERROR, LPU237_DLL_RESULT_ERROR, LPU237_DLL_RESULT_ERROR };
		vector<BYTE> v_iso[] = { vector<BYTE>( 200,0 ), vector<BYTE>( 200,0 ), vector<BYTE>( 200,0 ) };
		unsigned char c_sub=0x30;
		unsigned int n_offset = 0;

		memset( m_sRead, 0, sizeof(m_sRead) );

		for( int n_track = 0; n_track < 3; n_track++ ){
			if( n_track == 0 )
				c_sub=0x20;
			else
				c_sub=0x30;
			//
			dwCount[n_track] = CLpu237Dll::get_instance().LPU237_get_data( n_result_index, 1+n_track, &v_iso[n_track][0] );
			if( dwCount[n_track] == LPU237_DLL_RESULT_ERROR ){
				b_error[n_track] = true;
			}
			else if( dwCount[n_track] == LPU237_DLL_RESULT_CANCEL ||  dwCount[n_track] == LPU237_DLL_RESULT_ERROR_MSR || dwCount[n_track] == LPU237_DLL_RESULT_ICC_INSERTED || dwCount[n_track] == LPU237_DLL_RESULT_ICC_REMOVED ){
			}
			else if( dwCount[n_track] > 0 ){
				v_iso[n_track].resize( dwCount[n_track] );

				pMsData->cErrorSize[n_track] = (signed char)dwCount[n_track];

				for_each( begin(v_iso[n_track]), end(v_iso[n_track]), [&]( const BYTE c ){
					pMsData->sData[n_offset++] = c-c_sub;
				});
			}

		}//end for
		//
		if( b_error[0] != true || b_error[1] != true || b_error[2] != true ){
			if( pMsData->cErrorSize[0] > 0 || pMsData->cErrorSize[1] > 0 || pMsData->cErrorSize[2] > 0 ){
				b_fire = true;
			}
		}

	}while(0);

	return b_fire;
}

bool CSoMsr::CWorker::MsrWoker_Abnormal_Usb( CSoMsr *pSo, DWORD dwWaitResult  )
{
	CSoMsr::sp_DevEvent Req;
	bool bRun = true;

	CWorkQueue *pDeviceQ = pSo->m_pDeviceQ;

	switch( dwWaitResult ){
		case CWorker::Evt_Kill:
			bRun = false;	//exit worker.
			break;
		case CWorker::Evt_Wakeup:// run job.
		case CWorker::Evt_TimeOut:

			//if( !pDeviceQ->IsEmpty() ){//remark 20150330
			if( pDeviceQ->DeQ( Req ) ){
						
				switch( Req->GetCmd() ){
					case Cmd_EnterOps:
						Req->Complete();
						break;
					case Cmd_LeaveOps:
						::CancelIo( pSo->m_hDev );
						Req->Complete();
						m_bOpen = false;
						m_bRecover = false;	//change, not recoverd but. bypass because of closing device.
						break;
					default://unsupported request
						break;
				}//switch
			}// end dequeue
			else{
				//find device. & reopen
				if( pSo->FindOpenEnterOpos() ){

					m_bOpen = true;
					m_bRecover = false;	//recover OK........
					pSo->m_lState = OPOS_S_IDLE;

					if( !m_bReqExecuteRead ){
						m_bReqExecuteRead = true;//recover from Power suspend.
					}
					ATLTRACE( _T("*** Recover OK.......\n") );
					CLog::GetLog()->Log( true,  CLog::LEV_NORMAL, _T("[INFO] MsrWoker_Abnormal_Uart : RECOVER : OK.\n") );
				}
			}
			break;
		case CWorker::Evt_ReadDone://read done
				
			//fire event
			pSo->Fire( m_dwLastError );

			break;
		default://may be error
			break;
	}//end switch
	return bRun;
}

//DWORD CSoMsr::CWorker::MsrWoker( LPVOID lpParam )
unsigned CSoMsr::CWorker::MsrWoker( LPVOID lpParam )
{
	CSoMsr *pSo = static_cast<CSoMsr*>(lpParam);
	CWorkQueue *pDeviceQ = pSo->m_pDeviceQ;
	CWorker *pWorker = pSo->m_pWorker;
	pWorker->m_pSoMsr = pSo;

	if( pDeviceQ == NULL || pWorker == NULL ){
		ATLTRACE( _T("FAILURE : RUN : MsrWoker.") );

		pSo->SOTrace( true, CLog::LEV_LOW, _T("[ERROR] MsrWoker : pDeviceQ = 0x%x, pWorker = 0x%x.\n"),pDeviceQ,pWorker  );
		//return 0;
		_endthreadex(0);
		return 0;
	}

	CDetectWnd *pDetectWnd = CDetectWnd::GetInstance();
	pDetectWnd->Register();

	pDetectWnd->setCBParameter( lpParam );
	pDetectWnd->AddEventCallBack( WM_POWERBROADCAST, PBT_APMSUSPEND, EventCB_PowerSuspend );
	pDetectWnd->AddEventCallBack( WM_POWERBROADCAST, PBT_APMRESUMESUSPEND, EventCB_PowerResume );
	pDetectWnd->AddEventCallBack( WM_DEVICECHANGE, DBT_DEVICEARRIVAL, EventCB_DeviceChanged_PlugIn );
	pDetectWnd->AddEventCallBack( WM_DEVICECHANGE, DBT_DEVICEREMOVECOMPLETE, EventCB_DeviceChanged_PlugOut );

	pWorker->m_bRunning = true;

	HANDLE EventArray[3];

	EventArray[0] = pWorker->m_KillEvent.GetHandle();
	EventArray[1] = pWorker->m_WakeupEvent.GetHandle();
	EventArray[2] = pWorker->m_UsbMsrDoneEvent.GetHandle();

	DWORD dwWaitResult;
	bool bRun = true;

	pSo->SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] MsrWoker : STARTED.\n")  );

	do{
		//dwWaitResult = ::WaitForMultipleObjectsEx( 2, EventArray, FALSE, 50, TRUE );
		dwWaitResult = ::MsgWaitForMultipleObjectsEx( 3, EventArray, 50, QS_ALLINPUT,MWMO_ALERTABLE );

		if( dwWaitResult == (WAIT_OBJECT_0+3) ){

			if( pDetectWnd ){
				pDetectWnd->Bump();
			}
		}

		if( !m_bSuspend ){

			if( pWorker->m_bRecover ){
				if( pSo->m_bDeviceIsUart )
					bRun = pWorker->MsrWoker_Abnormal_Uart( pSo, dwWaitResult );
				else
					bRun = pWorker->MsrWoker_Abnormal_Usb( pSo, dwWaitResult );
			}
			else{
				if( pSo->m_bDeviceIsUart )
					bRun = pWorker->MsrWoker_Normal_Uart( pSo, dwWaitResult );
				else
					bRun = pWorker->MsrWoker_Normal_Usb( pSo, dwWaitResult );
			}
		}

	}while( bRun );

	if( m_bOpen ){
		if( !m_bRecover ){
			//CSoMsr *pSo = static_cast<CSoMsr*>(m_lpParameter);

			CLog::GetLog()->Log( true, CLog::LEV_HIGH, _T("[DETAIL] MsrWoker : EXIT : NOT RECOVER.\n")  );

			if( pSo->m_bDeviceIsUart ){
				//build leave request
				MSR_HOST_PACKET Cmd;
				::memset( &Cmd, 0, sizeof(Cmd) );
				Cmd.cCmd = Cmd_LeaveOps;

				::CancelIo( pSo->m_hDev );
				pSo->Msr_SendRequest( Cmd );
			}
			else{
				CLpu237Dll::get_instance().LPU237_leave_opos( pSo->m_hDev );

			}
			pSo->SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] MsrWoker : EXIT : SEND_LEAVE_OPOS_MODE.\n")  );
		}

		m_bOpen = false;
	}

	ATLTRACE( _T(" * Exit worker\n" ) );
	pSo->SOTrace( true, CLog::LEV_NORMAL, _T("[INFO] MsrWoker : Exit worker\n")  );

	pDetectWnd->Unregister();
	pWorker->m_bRunning = false;

	_endthreadex(0);
	return 0;
}

VOID CSoMsr::CWorker::MsrReadDoneComplete(
			DWORD dwErrorCode,
			DWORD dwNumberOfBytesTransfered,
			LPOVERLAPPED lpOverlapped
			)
{
	m_dwLastError = dwErrorCode;

	if( ERROR_OPERATION_ABORTED == dwErrorCode ){
		ATLTRACE( _T(" .. called ERROR_OPERATION_ABORTED.\n" ) );
	}
	else if( ERROR_SUCCESS == dwErrorCode ){
		ATLTRACE( _T(" .. called ERROR_SUCCESS[RX : %d].\n" ),dwNumberOfBytesTransfered );
	}
	else if( ERROR_DEVICE_NOT_CONNECTED  == dwErrorCode ){
		ATLTRACE( _T(" .. called ERROR_DEVICE_NOT_CONNECTED.\n" ) );
	}
	else{
		//if success, get magnetic card data from 
		ATLTRACE( _T(" .. called callback[error code = %d].\n" ), dwErrorCode );
	}
}

void CSoMsr::CWorker::MsrReadDoneCompleteUsb(void*pParam)
{
	OVERLAPPED *p_ov = (OVERLAPPED *)pParam;
	do{
		if( p_ov == NULL )
			continue;
		if( p_ov->hEvent == NULL )
			continue;
		//
		ATLTRACE( _T("CALLBACK : MsrReadDoneCompleteUsb.(0x%X)\n"),p_ov->hEvent );
		SetEvent( p_ov->hEvent );
	}while(0);
}

void CALLBACK CSoMsr::CWorker::EventCB_PowerSuspend(LPVOID lpCBParameter)
{
	ATLTRACE( _T(":EventCB_PowerSuspend().\r\n") );
	m_bSuspend = true;

	CSoMsr *pSo = static_cast<CSoMsr*>(lpCBParameter);

	if( m_bOpen ){
		if( pSo->m_bDeviceIsUart ){
			CancelIo( pSo->m_hDev );
			CloseHandle( pSo->m_hDev );
		}
		else{
			CLpu237Dll::get_instance().LPU237_close( pSo->m_hDev );
		}
		pSo->m_hDev = NULL;
	}
}

void CALLBACK CSoMsr::CWorker::EventCB_PowerResume(LPVOID lpCBParameter)
{
	ATLTRACE( _T(":EventCB_PowerResume().\r\n") );
	CLog::GetLog()->Log( true,  CLog::LEV_NORMAL, _T("[INFO] EventCB_PowerResume()++.\n") );
	m_bSuspend = false;

	CSoMsr *pSo = static_cast<CSoMsr*>(lpCBParameter);

	if( m_bOpen ){

		if( pSo->FindOpenEnterOpos() ){
			m_bOpen = true;
			m_bRecover = false;	//recover OK........
			pSo->m_lState = OPOS_S_IDLE;
			pSo->m_pWorker->m_bReqExecuteRead = true;//recover from Power suspend.
			ATLTRACE( _T("*** Recover OK.......\n") );
			CLog::GetLog()->Log( true,  CLog::LEV_NORMAL, _T("[INFO] EventCB_PowerResume : FindOpenEnterOpos : OK.\n") );
		}
		else{
			m_bRecover = true;		//change
			pSo->m_lState = OPOS_S_ERROR;
			ATLTRACE( _T("*** Recover FAIL.......\n") );
			CLog::GetLog()->Log( true,  CLog::LEV_LOW, _T("[ERROR] EventCB_PowerResume : FindOpenEnterOpos : FAIL.\n") );
		}
	}
}

void CALLBACK CSoMsr::CWorker::EventCB_DeviceChanged_PlugIn(LPVOID lpCBParameter)
{
	ATLTRACE( _T(":EventCB_DeviceChanged_PlugIn().\r\n") );
	CLog::GetLog()->Log( true,  CLog::LEV_NORMAL, _T("[INFO] EventCB_DeviceChanged_PlugIn()++.\n") );

	CSoMsr *pSo = static_cast<CSoMsr*>(lpCBParameter);

	pSo->setDeviceChanged();

}

void CALLBACK CSoMsr::CWorker::EventCB_DeviceChanged_PlugOut(LPVOID lpCBParameter)
{
	ATLTRACE( _T(":EventCB_DeviceChanged_PlugOut().\r\n") );
	CLog::GetLog()->Log( true,  CLog::LEV_NORMAL, _T("[INFO] EventCB_DeviceChanged_PlugOut()++.\n") );

	CSoMsr *pSo = static_cast<CSoMsr*>(lpCBParameter);

	pSo->setDeviceChanged();

}

CString & CSoMsr::Deb_GetStringFromPropertyNumber( long PropIndex )
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
		case PIDXMsr_DecodeData:		sResult =  _T("PIDXMsr_DecodeData");		break;
		case PIDXMsr_ParseDecodeData:	sResult =  _T("PIDXMsr_ParseDecodeData");		break;
		case PIDXMsr_TracksToRead:		sResult =  _T("PIDXMsr_TracksToRead");		break;
		case PIDXMsr_ErrorReportingType:	sResult =  _T("PIDXMsr_ErrorReportingType");		break;
		case PIDXMsr_TransmitSentinels:		sResult =  _T("PIDXMsr_TransmitSentinels");		break;
		case PIDXMsr_CapISO:		sResult =  _T("PIDXMsr_CapISO");		break;
		case PIDXMsr_CapJISOne:	sResult =  _T("PIDXMsr_CapJISOne");		break;
		case PIDXMsr_CapJISTwo:	sResult =  _T("PIDXMsr_CapJISTwo");		break;
		case PIDXMsr_CapTransmitSentinels:		sResult =  _T("PIDXMsr_CapTransmitSentinels");		break;
		default:	sResult.Format( _T("Unknown(%d)"), PropIndex );		break;
	}//end switch

	return sResult;
}

CString & CSoMsr::Deb_GetStringFromPropertyString( long PropIndex )
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

			case PIDXMsr_Track1Data:
				sResult = _T("PIDXMsr_Track1Data");	break;
			case PIDXMsr_Track2Data:	
				sResult = _T("PIDXMsr_Track2Data");	break;
			case PIDXMsr_Track3Data:
				sResult = _T("PIDXMsr_Track3Data");	break;
			case PIDXMsr_Track4Data:
				sResult = _T("PIDXMsr_Track4Data");	break;
			case PIDXMsr_AccountNumber:	sResult = _T("PIDXMsr_AccountNumber");	break;
			case PIDXMsr_ExpirationDate:		sResult = _T("PIDXMsr_ExpirationDate");	break;
			case PIDXMsr_FirstName:			sResult = _T("PIDXMsr_FirstName");	break;
			case PIDXMsr_MiddleInitial:		sResult = _T("PIDXMsr_MiddleInitial");	break;
			case PIDXMsr_ServiceCode:		sResult = _T("PIDXMsr_ServiceCode");	break;
			case PIDXMsr_Suffix:				sResult = _T("PIDXMsr_Suffix");	break;
			case PIDXMsr_Surname:			sResult = _T("PIDXMsr_Surname");	break;
			case PIDXMsr_Title:					sResult = _T("PIDXMsr_Title");	break;
			case PIDXMsr_Track1DiscretionaryData:	sResult = _T("PIDXMsr_Track1DiscretionaryData");	break;
			case PIDXMsr_Track2DiscretionaryData:	sResult = _T("PIDXMsr_Track2DiscretionaryData");	break;

			default:
				sResult.Format( _T("Unknown(%d)"), PropIndex );		break;
				break;
		}//end switch

	return  sResult;
}

CString & CSoMsr::Deb_GetStringFromCheckHealthLevel( long Level )
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

void CSoMsr::Deb_Printf( const TCHAR *_Format, ...)
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

void CSoMsr::SOTrace( bool bStemp, CLog::Level nLevel, const TCHAR *_Format, ...)
{
	CLog *pLog = CLog::GetLog();
	//pLog = m_pLog;

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
