#include <algorithm>
#include <map>

#include <mp_cconvert.h>

#include <GemCoreScrHelper.h>


namespace gemcore_scr_interface
{


void CGemCoreScr_Helper::build_ex_PC_to_RDR_XfrBlock( _mp::type_v_buffer & vtx, unsigned char cBWI, const std::wstring & sPdu )
{
	_mp::type_v_buffer vdata(0);
	_mp::cconvert::binary_from_hex_string(vdata, sPdu, L" ");
	build_PC_to_RDR_XfrBlock( vtx, cBWI, vdata);
}

void CGemCoreScr_Helper::build_ex_PC_to_RDR_SetParameters( 
	_mp::type_v_buffer & vtx,
	typeIccProtocol protocol,
	unsigned char bmFindexDindex,
	unsigned char bmTCCKST,
	unsigned char bGuardTime,
	unsigned char bWaitingInteger,
	unsigned char bIFSC /*= 0	//T1 only*/
	)
{
	_mp::type_v_buffer vdata;

	if( protocol == Protocol_T0 ){
		vdata.resize( 5, 0 );	vdata.assign(vdata.size(), 0);
	}
	else{ //protocol == Protocol_T1 
		vdata.resize( 7, 0 ); vdata.assign(vdata.size(), 0);
		vdata[6] = bIFSC;
	}

	int i(0);
	vdata[i++] = bmFindexDindex;
	vdata[i++] = bmTCCKST;
	vdata[i++] = bGuardTime;
	vdata[i++] = bWaitingInteger;

	build_PC_to_RDR_SetParameters( vtx, protocol, vdata );
}

// get report string from extentions
std::wstring CGemCoreScr_Helper::get_atr_report( const CCardInfo & CardInfo, const wchar_t cDelimitor /*= L'\n'*/ )
{
	std::wstring sReport( L"none atr data" );

	if( CardInfo.get_atr().empty() ){
		return sReport;
	}
	//

	std::wstring satr;
	_mp::cconvert::hex_string_from_binary(satr, CardInfo.get_atr(), L" ");
	sReport = L"atr : " + satr;
	//
	if( !CardInfo.get_historical_bytes().empty() ){
		std::wstring shistory;
		_mp::cconvert::hex_string_from_binary(shistory, CardInfo.get_historical_bytes(), L" ");
			
		sReport += cDelimitor;
		sReport += L"historical bytes : ";
		sReport += shistory;
	}
	//
	if( CardInfo.get_conversion() == Convention_inverse ){
		sReport += cDelimitor;		sReport += L"convention : inverse";
	}
	else{
		sReport += cDelimitor;		sReport += L"convention : direct";
	}
	//
	CGemCoreScr_Helper::CCardInfo::typeVectorParameters vParameters( CardInfo.get_parameters() );
	_mp::type_v_buffer vValue;
	std::wstring s_data;

	if( !vParameters.empty() ){

		long long index(0);

		for_each( begin(vParameters), end(vParameters), [&]( CGemCoreScr_Helper::CCardInfo::typeVectorParameters::value_type value ){
			std::wstring svalue;

			sReport += cDelimitor;		sReport += L"index : ";	sReport += std::to_wstring(index++);

			sReport += cDelimitor;
			if( value.bUsedDefaultValue )
				sReport += L"using default value.";
			else
				sReport += L"using specific value.";

			if( value.cProtocol == 0 ){	
				sReport += cDelimitor;		sReport += L"protocol : T0";
				vValue.resize(0);
				vValue.push_back( value.cFI );
				sReport += cDelimitor;		sReport += L"FI : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				vValue.resize(0);
				vValue.push_back( value.cDI );
				sReport += cDelimitor;		sReport += L"DI : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				vValue.resize(0);
				vValue.push_back( value.cN );
				sReport += cDelimitor;		sReport += L"N : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				vValue.resize(0);
				vValue.push_back( value.cWI );
				sReport += cDelimitor;		sReport += L"WI : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;
			}
			else if( value.cProtocol == 1 ){
				sReport += cDelimitor;		sReport += L"protocol : T1";
				vValue.resize(0);
				vValue.push_back( value.cCWI );
				sReport += cDelimitor;		sReport += L"CWI : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				vValue.resize(0);
				vValue.push_back( value.cBWI );
				sReport += cDelimitor;		sReport += L"BWI : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				vValue.resize(0);
				vValue.push_back( value.cN );
				sReport += cDelimitor;		sReport += L"N : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				vValue.resize(0);
				vValue.push_back( value.cIFSC );
				sReport += cDelimitor;		sReport += L"IFSC : ";
				s_data.clear();
				_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
				sReport += s_data;

				if( value.cChckSumType == 0 ){
					sReport += cDelimitor;		sReport += L"checksum type : LRC";
				}
				else if( value.cChckSumType == 1 ){
					sReport += cDelimitor;		sReport += L"checksum type : CRC";
				}
			}
			else{
				sReport += cDelimitor;		sReport += L"protocol : unknown";
			}
			//
		});// for_each

	}
	else{
		unsigned char cVal(0);
		sReport += cDelimitor;		sReport += L"protocol(default) : T0";

		
		if( CardInfo.get_global_FIDI( &cVal ) ){
			vValue.resize(0);
			vValue.push_back( cVal>>4 );
			sReport += cDelimitor;		sReport += L"FI : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;

			vValue.resize(0);
			vValue.push_back( cVal&0x0F );
			sReport += cDelimitor;		sReport += L"DI : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;
		}
		else{
			vValue.resize(0);
			vValue.push_back( CGemCoreScr_Helper::CCardInfo::default_FI );
			sReport += cDelimitor;		sReport += L"FI(default) : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;

			vValue.resize(0);
			vValue.push_back( CGemCoreScr_Helper::CCardInfo::default_DI );
			sReport += cDelimitor;		sReport += L"DI(default) : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;
		}

		if( CardInfo.get_global_N( &cVal ) ){
			vValue.resize(0);	vValue.push_back( cVal );
			sReport += cDelimitor;		sReport += L"N : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;
		}
		else{
			vValue.resize(0);	vValue.push_back( CGemCoreScr_Helper::CCardInfo::default_N );
			sReport += cDelimitor;		sReport += L"N(default) : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;
		}

		if( CardInfo.get_specific_WI( &cVal ) ){
			vValue.resize(0);	vValue.push_back( cVal );
			sReport += cDelimitor;		sReport += L"WI : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;
		}
		else{
			vValue.resize(0);	vValue.push_back( CGemCoreScr_Helper::CCardInfo::default_WI );
			sReport += cDelimitor;		sReport += L"WI(default) : ";	
			s_data.clear();
			_mp::cconvert::hex_string_from_binary(s_data, vValue, L"");
			sReport += s_data;
		}
	}

	return sReport;
}

bool CGemCoreScr_Helper::decode_atr( CCardInfo & OutCardInfo, const _mp::type_v_buffer & vAtr )
{
	return OutCardInfo.analysis_atr( vAtr );
}

unsigned char CGemCoreScr_Helper::get_sequence_number()
{
	static unsigned char cSequence = 0;
	cSequence++;
	return cSequence;
}

void CGemCoreScr_Helper::build_PC_to_RDR( _mp::type_v_buffer & vtx,  PC_to_RDR cMessageType, unsigned char cP1, const _mp::type_v_buffer & vdata/*= _mp::type_v_buffer(0)*/ )
{
	vtx.resize( sizeof(scr_message)+vdata.size(), 0 ); vtx.assign(vtx.size(), 0);
	//
	scr_message *pMsg = reinterpret_cast<scr_message *>(&vtx[0]);

	pMsg->cSync = SCR_SYNC_VALUE;
	pMsg->cCtrl = SCR_ACK_VALUE;
	pMsg->ccid_hdr_pc_to_rdr.cMessageType = cMessageType;
	pMsg->ccid_hdr_pc_to_rdr.dwLength = (unsigned long)vdata.size();
	pMsg->ccid_hdr_pc_to_rdr.cSlot = 0;
	pMsg->ccid_hdr_pc_to_rdr.cSeq = get_sequence_number();
	pMsg->ccid_hdr_pc_to_rdr.asCmd[0] = cP1;
	pMsg->ccid_hdr_pc_to_rdr.asCmd[1] = 0;
	pMsg->ccid_hdr_pc_to_rdr.asCmd[2] = 0;

	if( vdata.size() > 0 )
		std::copy( begin(vdata), end(vdata), begin(vtx)+sizeof(scr_message)-1 );

	unsigned char bcc(0);
	for_each( begin(vtx), begin(vtx)+vtx.size()-1, [&]( _mp::type_v_buffer::value_type value ){
		bcc ^= value;
	});

	if( vdata.size() > 0 )
		vtx[vtx.size()-1] = bcc;//set LRC
	else
		pMsg->sData[0] = bcc;//set LRC
}

void CGemCoreScr_Helper::build_PC_to_RDR_IccPowerOn( _mp::type_v_buffer & vtx, typeIccPower power )
{
	build_PC_to_RDR( vtx, PC_to_RDR_IccPowerOn, power );
}

void CGemCoreScr_Helper::build_PC_to_RDR_IccPowerOff( _mp::type_v_buffer & vtx )
{
	build_PC_to_RDR( vtx, PC_to_RDR_IccPowerOff, 0 );
}

void CGemCoreScr_Helper::build_PC_to_RDR_XfrBlock( _mp::type_v_buffer & vtx, unsigned char cBWI, const _mp::type_v_buffer & vdata )
{
	build_PC_to_RDR( vtx, PC_to_RDR_XfrBlock, cBWI, vdata );
}

void CGemCoreScr_Helper::build_PC_to_RDR_SetParameters( _mp::type_v_buffer & vtx, typeIccProtocol protocol,  const _mp::type_v_buffer & vdata )
{
	build_PC_to_RDR( vtx, PC_to_RDR_SetParameters, protocol, vdata );
}

void CGemCoreScr_Helper::build_PC_to_RDR_ResetParameters( _mp::type_v_buffer & vtx )
{
	build_PC_to_RDR( vtx, PC_to_RDR_ResetParameters, 0 );
}

void CGemCoreScr_Helper::build_PC_to_RDR_Abort( _mp::type_v_buffer & vtx )
{
	build_PC_to_RDR( vtx, PC_to_RDR_Abort, 0 );
}

void CGemCoreScr_Helper::build_PC_to_RDR_GetParameters( _mp::type_v_buffer & vtx )
{
	build_PC_to_RDR( vtx, PC_to_RDR_GetParameters, 0 );
}

void CGemCoreScr_Helper::build_PC_to_RDR_GetSlotStatus( _mp::type_v_buffer & vtx )
{
	build_PC_to_RDR( vtx, PC_to_RDR_GetSlotStatus, 0 );
}

void CGemCoreScr_Helper::build_PC_to_RDR_Escape( _mp::type_v_buffer & vtx, const _mp::type_v_buffer & vdata )
{
	build_PC_to_RDR( vtx, PC_to_RDR_Escape, 0, vdata );
}

CGemCoreScr_Helper::CGemCoreScr_Helper(void)
{
}

CGemCoreScr_Helper::~CGemCoreScr_Helper(void)
{
}

//////////////////////////////////////////////
// CResponse class

void CGemCoreScr_Helper::CResponse::ini()
{
	m_sDescription = L"no response";
	m_sMessageType = L"";
	m_sParameters = L"";
}

CGemCoreScr_Helper::CResponse::CResponse()
{
	ini();
}

CGemCoreScr_Helper::CResponse::CResponse( const CResponse & rep )
{
	*this = rep;
}

CGemCoreScr_Helper::CResponse & CGemCoreScr_Helper::CResponse::operator=( const CResponse & rep )
{
	m_sDescription = rep.m_sDescription;
	m_vRaw.resize( rep.m_vRaw.size() );
	std::copy( std::begin(rep.m_vRaw), std::end(rep.m_vRaw), std::begin(m_vRaw) );
	//
	m_sMessageType = rep.m_sMessageType;
	m_sParameters = rep.m_sParameters;
	return *this;
}

CGemCoreScr_Helper::CResponse::CResponse( const _mp::type_v_buffer & vrx )
{
	m_vRaw.resize( vrx.size() );
	std::copy(std::begin(vrx), std::end(vrx), std::begin(m_vRaw) );
	if( !analysis() )
		m_vRaw.resize(0);
}

void CGemCoreScr_Helper::CResponse::setRaw( const _mp::type_v_buffer & vrx )
{
	m_vRaw.resize( vrx.size() );
	std::copy(std::begin(vrx), std::end(vrx), std::begin(m_vRaw) );
	if( !analysis() )
		m_vRaw.resize(0);
}

CGemCoreScr_Helper::CResponse::~CResponse()
{
}

bool CGemCoreScr_Helper::CResponse::analysis()
{
	bool bResult(false);
	//
	if( m_vRaw.size() < const_size_min ){
		ini();
		return bResult;
	}
	else if( m_vRaw.size() < const_size_min+10 ){
		ini();
		return true;
	}

	scr_message *pmsg = (scr_message*)&m_vRaw[0];
	//
	switch( pmsg->ccid_hdr_rdr_to_pc.cMessageType ){
		case RDR_to_PC_DataBlock:
			m_sMessageType = L"RDR_TO_PC_DATABLOCK";
			break;
		case RDR_to_PC_SlotStatus:
			m_sMessageType = L"RDR_TO_PC_SLOTSTATUS";
			break;
		case RDR_to_PC_Parameters:
			m_sMessageType = L"RDR_TO_PC_PARAMETERS";
			break;
		case RDR_to_PC_Escape:
			m_sMessageType = L"RDR_TO_PC_ESCAPE";
			break;
		case RDR_to_PC_DataRateAndClockFrequency:
			m_sMessageType = L"RDR_TO_PC_DATARATEANDCLOCKFREQUENCY";
			break;
		default:
			ini();
			return bResult;
	}//end switch

	ccid_rdr_to_pc_header *header = &(pmsg->ccid_hdr_rdr_to_pc);

	if( m_vRaw.size() != header->dwLength + const_size_min + 10 ){
			ini();
			return bResult;
	}
	//
	std::wstring sICCStatus;
	std::wstring sCommandStatus;

	switch( header->cStatus & 0x03 ){
		case 0:	sICCStatus = L"An ICC is present and active";		break;
		case 1:	sICCStatus = L"An ICC is present and inactive";		break;
		case 2:	sICCStatus = L"No ICC is present";		break;
		default:	sICCStatus = L"RFU ";		break;
	}//end switch

	switch( header->cStatus & 0xC0 ){
		case 0x00:	sCommandStatus = L"Processed without error";		break;
		case 0x40:	sCommandStatus = L"Failed";		break;
		case 0x80:	sCommandStatus = L"Time Extension is requested";		break;
		default:	sCommandStatus = L"RFU ";		break;
	}//end switch

	std::wstring sError;

	switch( header->cError ){
		case CMD_ABORTED:	sError = L"CMD_ABORTED";	break;
		case ICC_MUTE:	sError = L"ICC_MUTE";	break;
		case XFR_PARITY_ERROR:	sError = L"XFR_PARITY_ERROR";	break;
		case XFR_OVERRUN:	sError = L"XFR_OVERRUN";	break;
		case HW_ERROR:	sError = L"HW_ERROR";	break;
		case BAD_ATR_TS:	sError = L"BAD_ATR_TS";	break;
		case BAD_ATR_TCK:	sError = L"BAD_ATR_TCK";	break;
		case ICC_PROTOCOL_NOT_SUPPORTED:	sError = L"ICC_PROTOCOL_NOT_SUPPORTED";	break;
		case ICC_CLASS_NOT_SUPPORTED:	sError = L"ICC_CLASS_NOT_SUPPORTED";	break;
		case PROCEDURE_BYTE_CONFLICT:	sError = L"PROCEDURE_BYTE_CONFLICT";	break;
		case DEACTIVATED_PROTOCOL:	sError = L"DEACTIVATED_PROTOCOL";	break;
		case BUSY_WITH_AUTO_SEQUENCE:	sError = L"BUSY_WITH_AUTO_SEQUENCE";	break;
		case PIN_TIMEOUT:	sError = L"PIN_TIMEOUT";	break;
		case PIN_CANCELLED:	sError = L"PIN_CANCELLED";	break;
		case CMD_SLOT_BUSY:	sError = L"CMD_SLOT_BUSY";	break;
		case 0:	sError = L"Command not supported";	break;
		default:
			if( header->cError <= 0xC0 && header->cError >= 0x81 ){
				sError = L"User Defined";
			}
			else if( header->cError <= 0x7F && header->cError >= 0x01 ){
				sError = L"Index of not supported / incorrect message parameter";
			}
			else{
				sError = L"Reserved for future use";
			}
			break;
	}//end switch

	m_sDescription = m_sMessageType + L" : " +sICCStatus+ L" : " + sCommandStatus + L" : " + sError;

	//m_sParameters
	if( header->cMessageType == RDR_to_PC_Parameters ){
		if( header->dwLength == 5  ){
			m_sParameters = 
				L"protocol="
				+ _mp::cconvert::get_string( header->asCmd[0],16 )
				+ L", bmFindexDindex="
				+ _mp::cconvert::get_string( header->asCmd[1], 16)
				+ L", bmTCCKST0="
				+ _mp::cconvert::get_string( header->asCmd[2], 16)
				+ L", bGuardTimeT0="
				+ _mp::cconvert::get_string( header->asCmd[3], 16)
				+ L", bWaitingIntegerT0="
				+ _mp::cconvert::get_string( header->asCmd[4], 16)
				+ L", bClockStop="
				+ _mp::cconvert::get_string( header->asCmd[5], 16);
		}
		else if( header->dwLength == 7 ){
			m_sParameters =
				L"protocol="
				+ _mp::cconvert::get_string(header->asCmd[0], 16)
				+ L", bmFindexDindex="
				+ _mp::cconvert::get_string(header->asCmd[1], 16)
				+ L", bmTCCKST1="
				+ _mp::cconvert::get_string(header->asCmd[2], 16)
				+ L", bGuardTimeT1="
				+ _mp::cconvert::get_string(header->asCmd[3], 16)
				+ L", bWaitingIntegerT1="
				+ _mp::cconvert::get_string(header->asCmd[4], 16)
				+ L", bClockStop="
				+ _mp::cconvert::get_string(header->asCmd[5], 16)
				+ L", bIFSC="
				+ _mp::cconvert::get_string(header->asCmd[6], 16)
				+ L", bNadValue="
				+ _mp::cconvert::get_string(header->asCmd[7], 16);
		}
		else{
			m_sParameters = L"";
		}
	}
	else{
		m_sParameters = L"";
	}

	bResult = true;
	return bResult;
}

_mp::type_v_buffer CGemCoreScr_Helper::CResponse::getCcidRaw()
{
	_mp::type_v_buffer vccid(0);

	if( m_vRaw.size() <= 3 )
		return vccid;
	//
	vccid.resize( m_vRaw.size()-3, 0 );
	std::copy( begin(m_vRaw)+2, end(m_vRaw)-1, begin(vccid) );
	return vccid;
}

_mp::type_v_buffer CGemCoreScr_Helper::CResponse::getCcidDataField()
{
	_mp::type_v_buffer data(0);

	if( m_vRaw.size() <= 3+10 )
		return data;
	//
	scr_message *pmsg = (scr_message*)&m_vRaw[0];

	if( pmsg->ccid_hdr_rdr_to_pc.dwLength + 13 != m_vRaw.size() )
		return data;

	data.resize( pmsg->ccid_hdr_rdr_to_pc.dwLength );
	std::copy( begin(m_vRaw)+12, end(m_vRaw)-1, begin(data) );
	return data;
}

//////////////////////////////////////////////
// CCardInfo class

CGemCoreScr_Helper::CCardInfo::CCardInfo()
{
	ini();
}

CGemCoreScr_Helper::CCardInfo::~CCardInfo()
{
}

CGemCoreScr_Helper::CCardInfo::CCardInfo( const _mp::type_v_buffer & vAtr )
{
	analysis_atr( vAtr );
}

CGemCoreScr_Helper::CCardInfo::CCardInfo( const CCardInfo & info )
{
	*this = info;
}

CGemCoreScr_Helper::CCardInfo & CGemCoreScr_Helper::CCardInfo::operator=( const CCardInfo & info )
{
	this->m_bTckOk = info.m_bTckOk;
	this->m_Conversion = info.m_Conversion;
	this->m_vAtr = info.m_vAtr;
	this->m_vHistory = info.m_vHistory;
	this->m_vTCK = info.m_vTCK;
	this->m_mapAtr = info.m_mapAtr;
	this->m_vParameters = info.m_vParameters;
	return *this;
}

CGemCoreScr_Helper::CCardInfo & CGemCoreScr_Helper::CCardInfo::operator=( _mp::type_v_buffer & vAtr )
{
	analysis_atr( vAtr );
	return *this;
}

void CGemCoreScr_Helper::CCardInfo::ini()
{
	m_bTckOk = true;
	m_Conversion = Convention_direct;

	m_vAtr.resize(0);
	m_vHistory.resize(0);
	m_vTCK.resize(0);

	m_mapAtr.clear();
	m_vParameters.resize(0);
}

bool CGemCoreScr_Helper::CCardInfo::analysis_atr( const _mp::type_v_buffer & vAtr )
{
	ini();

	if (vAtr.size() < 2){	return false;	/* Atr must have TS and T0	*/	}

	size_t nIndex(0);
	m_mapAtr[L"TS"] = vAtr[nIndex++];
	m_mapAtr[L"T0"] = vAtr[nIndex++];
	//
	bool bEnd(false);
	unsigned long long i(1);
	unsigned char TD(vAtr[1]);
	int nAdd(0);

	do{
		if( TD & 0x10 )	nAdd++;
		if( TD & 0x20 )	nAdd++;
		if( TD & 0x40 )	nAdd++;
		if( TD & 0x80 )	nAdd++;
		else	bEnd = true;

		if( vAtr.size() < nIndex+nAdd ){
			ini();
			return false;
		}

		if( TD & 0x10 ){		m_mapAtr[L"TA"+std::to_wstring(i)] = vAtr[nIndex++];		}
		if( TD & 0x20 ){		m_mapAtr[L"TB"+std::to_wstring(i)] = vAtr[nIndex++];		}
		if( TD & 0x40 ){		m_mapAtr[L"TC"+std::to_wstring(i)] = vAtr[nIndex++];		}
		if( TD & 0x80 ){		m_mapAtr[L"TD"+std::to_wstring(i)] = vAtr[nIndex];		TD = vAtr[nIndex++];	}

		i++;
		nAdd = 0;
	}while( !bEnd );

	int nLastIndex( static_cast<int>(i-1) );

	m_vTCK.resize(0);
	m_vHistory.resize(0);

	// TS section
	if( m_mapAtr[L"TS"] == 0x3B ){		m_Conversion = Convention_direct;		}
	else if( m_mapAtr[L"TS"] == 0x3F ){	m_Conversion = Convention_inverse;	}
	else return false;
	
	// T0 section
	m_vHistory.resize(  static_cast<int>(m_mapAtr[L"T0"] & 0x0F), 0 ); m_vHistory.assign(m_vHistory.size(), 0);
	if( vAtr.size() < m_vHistory.size()+nIndex )	{	ini();	return false;	}
	else{
		std::copy(std::begin(vAtr)+nIndex, std::begin(vAtr)+m_vHistory.size()+nIndex, std::begin(m_vHistory) );

		if( vAtr.size() > m_vHistory.size()+nIndex ){
			m_vTCK.resize( 1, vAtr[m_vHistory.size()+nIndex] );

			unsigned char cCalTck(0);
			std::for_each(std::begin(vAtr), std::begin(vAtr)+m_vHistory.size()+nIndex, [&]( _mp::type_v_buffer::value_type value ){
				cCalTck = cCalTck ^ value;
			});

			if( cCalTck != m_vTCK[0] )	m_bTckOk = false;
			else							m_bTckOk = true;
		}
	}

	m_vAtr.resize( vAtr.size() );
	std::copy(std::begin(vAtr), std::end(vAtr), std::begin(m_vAtr) );

	//
	SMARTCARD_PARAMEMTER para;
	memset( &para, 0, sizeof(para) );
	unsigned char cVal(0);
	bool bNotDefault(false); 
	bool bResult(true); 

	typeMapAtr::iterator it = m_mapAtr.find( L"TD1" );
	if( it != end(m_mapAtr) ){
		if( (it->second & 0x0F) == 0 ){//T0
			para.cProtocol = 0;
			bNotDefault = bResult = get_global_FIDI( &cVal );
			if( bResult ){		para.cFI = cVal >> 4;	para.cDI =  cVal & 0x0F;	}
			else{				para.cFI = default_FI;	para.cDI =  default_DI;	}
			
			bResult = get_global_N( &cVal );		bNotDefault |= bResult;
			if( bResult ){	para.cN = cVal;		}
			else{			para.cN = default_N;		}

			bResult = get_specific_WI( &cVal );		bNotDefault |= bResult;
			if( bResult ){	para.cWI = cVal;	}
			else{			para.cWI = default_WI;	}

			para.bUsedDefaultValue = !bNotDefault;
		}
		else if( (it->second & 0x0F) == 1 ){//T1
			para.cProtocol = 1;
			bNotDefault = bResult = get_global_FIDI( &cVal );
			if( bResult ){		para.cFI = cVal >> 4;	para.cDI =  cVal & 0x0F;	}
			else{				para.cFI = default_FI;	para.cDI =  default_DI;	}
			
			bResult = get_global_N( &cVal );		bNotDefault |= bResult;
			if( bResult ){	para.cN = cVal;		}
			else{			para.cN = default_N;		}

			para.cBWI = default_BWI;
			para.cCWI = default_CWI;
			para.cIFSC = default_IFSC;
			para.cChckSumType = deafult_CheckSumType;
			para.bUsedDefaultValue = !bNotDefault;
		}
		else{
			para.cProtocol = it->second & 0x0F;//unsupported protocol
			para.bUsedDefaultValue = false;
		}
		m_vParameters.push_back( para );
	}

	//nLastIndex
	std::wstring sInfChara;
	for( auto j = 3;	j<=nLastIndex;	j++ ){
		memset( &para, 0, sizeof(para) );
		cVal = 0;
		bNotDefault = false;

		sInfChara = L"TD"+std::to_wstring(static_cast<long long>(j-1));
		it = m_mapAtr.find( sInfChara );
		if( it != m_mapAtr.end() ){
			if( (it->second & 0x0F) == 0 ){
				para.cProtocol = 0;
				bNotDefault = bResult = get_global_FIDI( &cVal );
				if( bResult ){	para.cFI = cVal >> 4;	para.cDI =  cVal & 0x0F;	}
				else{			para.cFI = default_FI;	para.cDI =  default_DI;	}

				bResult = get_global_N( &cVal );		bNotDefault |= bResult;
				if( bResult ){	para.cN = cVal;		}
				else{			para.cN = default_N;		}
				
				para.cWI = default_WI;
				para.bUsedDefaultValue = !bNotDefault;
			}
			else if( (it->second & 0x0F) == 1 ){// J >= 3
				para.cProtocol = 1;
				bNotDefault = bResult = get_global_FIDI( &cVal );
				if( bResult ){	para.cFI = cVal >> 4;	para.cDI =  cVal & 0x0F;	}
				else{			para.cFI = default_FI;	para.cDI =  default_DI;	}

				bResult = get_global_N( &cVal );		bNotDefault |= bResult;
				if( bResult ){	para.cN = cVal;		}
				else{			para.cN = default_N;		}

				sInfChara = L"TA"+std::to_wstring(static_cast<long long>(j));
				it = m_mapAtr.find( sInfChara );
				if( it != end(m_mapAtr) ){
					bNotDefault |= true;
					para.cIFSC = it->second;
				}
				else{
					para.cIFSC = default_IFSC;
				}

				sInfChara = L"TB"+std::to_wstring(static_cast<long long>(j));
				it = m_mapAtr.find( sInfChara );
				if( it != end(m_mapAtr) ){
					bNotDefault |= true;
					para.cBWI = it->second >> 4;
					para.cCWI = it->second & 0x0F;
				}
				else{
					para.cBWI = default_BWI;
					para.cCWI = default_CWI;
				}

				sInfChara = L"TC"+std::to_wstring(static_cast<long long>(j));
				it = m_mapAtr.find( sInfChara );
				if( it != end(m_mapAtr) ){
					bNotDefault |= true;

					if( it->second & 0x01 ){
						para.cChckSumType = 1;//CRC
					}
					else{
						para.cChckSumType = 0;//LRC
					}
				}
				else{
					para.cChckSumType = deafult_CheckSumType;
				}
				
				para.bUsedDefaultValue = !bNotDefault;
			}
			else{
				para.cProtocol = it->second & 0x0F;//unsupported protocol
				para.bUsedDefaultValue = false;
			}

			m_vParameters.push_back( para );
		}
	}//end for

	return true;
}

bool CGemCoreScr_Helper::CCardInfo::get_global_FIDI(  unsigned char *pcFIDI /*= NULL*/  ) const
{
	typeMapAtr::const_iterator iter = m_mapAtr.find( L"TA1" );
	if( iter == end(m_mapAtr) )
		return false;
	else{
		if( pcFIDI )
			*pcFIDI = iter->second;
		return true;
	}
}

bool CGemCoreScr_Helper::CCardInfo::get_global_N( unsigned char *pcN /*= NULL*/ ) const
{
	typeMapAtr::const_iterator iter = m_mapAtr.find( L"TC1" );
	if( iter == end(m_mapAtr) )
		return false;
	else{
		if( pcN )
			*pcN = iter->second;
		return true;
	}
}

bool CGemCoreScr_Helper::CCardInfo::get_specific_WI( unsigned char *pcWI /*= NULL*/ ) const
{
	typeMapAtr::const_iterator iter = m_mapAtr.find( L"TC2" );
	if( iter == end(m_mapAtr) )
		return false;
	else{
		if( pcWI )
			*pcWI = iter->second;
		return true;
	}
}

bool CGemCoreScr_Helper::CCardInfo::get_TA2( unsigned char *pcTA2 /*= NULL*/ ) const
{
	typeMapAtr::const_iterator iter = m_mapAtr.find( L"TA2" );
	if( iter == end(m_mapAtr) )
		return false;
	else{
		if( pcTA2 )
			*pcTA2 = iter->second;
		return true;
	}
}

bool CGemCoreScr_Helper::CCardInfo::Is_allow_mode_change_by_TA2()
{
	unsigned char cTA2(0);

	if( get_TA2( &cTA2 ) ){
		if( cTA2 & 0x80 )
			return false;	//disallowed
		else
			return true;	//allowed
	}
	else{
		return false;
	}
}

bool CGemCoreScr_Helper::CCardInfo::Is_explicit_parameters_by_TA2()
{
	unsigned char cTA2(0);

	if( get_TA2( &cTA2 ) ){
		if( cTA2 & 0x10 )
			return false;	//implicit
		else
			return true;	//explicit
	}
	else{
		return false;
	}
}


}//namespace gemcore_scr_interface