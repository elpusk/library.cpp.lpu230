#include "StdAfx.h"
#include "ReqBuilder.h"
#include <algorithm>

const CReqBuilder::Value CReqBuilder::NullValue = { _T(""), 0, 0 };

static void functor_PrintMap( pair<UINT32,CReqBuilder::Value> item );

void functor_PrintMap( pair<UINT32,CReqBuilder::Value> item )
{
	//_tout << _T("INI KEY : ") << item.second.sIniKey << _T("(") << item.first<< _T(",") << item.second.dwSize<< _T(")") << endl;
}

map<UINT32, CReqBuilder::Value> & CReqBuilder::GetMappingMap()
{
	//#define	INSERT_TO_MAP(MAP,VAL,INIKEY,M)		( MAP.insert( pair<UINT32, Value>( OFFSETOF(M), SetValue( VAL, INIKEY, OFFSETOF(M) , SIZEOF(M) ) ) ) )

	static map<UINT32, Value> mapMapping;
	static bool bIsFirst = true;

	if( bIsFirst ){
		bIsFirst = false;

		////////////////////////////////////////////////////////////////////
		//set offset & size vector
		_tstring sSection;
		vector<_tstring> vKey;
		vector<UINT32> vSize;
		vector<UINT32> vOff;

		//
		sSection = _T("common");

		vKey.push_back( sSection + _T("&") + _T("sysv" ) );
		vSize.push_back( SIZEOF(sSysVer) );
		vOff.push_back( OFFSETOF(sSysVer) );

		vKey.push_back( sSection + _T("&") + _T("interface" ) );
		vSize.push_back( SIZEOF(Interface) );
		vOff.push_back( OFFSETOF(Interface) );

		vKey.push_back( sSection + _T("&") + _T("buzzer" ) );
		vSize.push_back( SIZEOF(nBuzzerFrequency) );
		vOff.push_back( OFFSETOF(nBuzzerFrequency) );

		vKey.push_back( sSection + _T("&") + _T("wdt" ) );
		vSize.push_back( SIZEOF(nNormalWDT) );
		vOff.push_back( OFFSETOF(nNormalWDT) );

		//
		vKey.push_back( sSection + _T("&") + _T("boottime" ) );
		vSize.push_back( SIZEOF(nBootRunTime) );
		vOff.push_back( OFFSETOF(nBootRunTime) );

		vKey.push_back( sSection + _T("&") + _T("keymap" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.KeyMap.nMappingTableIndex) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.KeyMap.nMappingTableIndex) );

		vKey.push_back( sSection + _T("&") + _T("pretag" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.TagPre) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.TagPre) );

		vKey.push_back( sSection + _T("&") + _T("posttag" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.TagPost) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.TagPost) );

		vKey.push_back( sSection + _T("&") + _T("prefix" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.GlobalPrefix) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.GlobalPrefix) );

		vKey.push_back( sSection + _T("&") + _T("postfix" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.GlobalPostfix) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.GlobalPostfix) );

		vKey.push_back( sSection + _T("&") + _T("cpdmin" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.nCpdSysTickMin) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.nCpdSysTickMin) );

		vKey.push_back( sSection + _T("&") + _T("cpdmax" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.nCpdSysTickMax) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.nCpdSysTickMax) );

		vKey.push_back( sSection + _T("&") + _T("gfixmode" ) );
		vSize.push_back( SIZEOF(ContainerInfoMsrObj.nGlobalTagCondition) );
		vOff.push_back( OFFSETOF(ContainerInfoMsrObj.nGlobalTagCondition) );

		for( INT32 i=0; i<3; i++ ){
			//object i+1 common
			sSection = _T("object") + to_tstring( (unsigned long long)(i+1) );

			vKey.push_back( sSection + _T("&") + _T("enable" ) );
			vSize.push_back( SIZEOF(InfoMsr[i].cEnableTrack) );
			vOff.push_back( OFFSETOF(InfoMsr[i].cEnableTrack) );

			vKey.push_back( sSection + _T("&") + _T("encrypt" ) );
			vSize.push_back( SIZEOF(InfoMsr[i].bEnableEncryption) );
			vOff.push_back( OFFSETOF(InfoMsr[i].bEnableEncryption) );

			vKey.push_back( sSection + _T("&") + _T("buffersize" ) );
			vSize.push_back( SIZEOF(InfoMsr[i].nBufSize) );
			vOff.push_back( OFFSETOF(InfoMsr[i].nBufSize) );

			for( INT32 j=0; j<3; j++ ){
				//object i+1 , combination j
				sSection = _T("object") + to_tstring( (unsigned long long)(i+1) ) + to_tstring( (unsigned long long)j );

				vKey.push_back( sSection + _T("&") + _T("keymap" ) );
				vSize.push_back( SIZEOF(InfoMsr[i].KeyMap[j].nMappingTableIndex) );
				vOff.push_back( OFFSETOF(InfoMsr[i].KeyMap[j].nMappingTableIndex) );

				vKey.push_back( sSection + _T("&") + _T("prefix" ) );
				vSize.push_back( SIZEOF(InfoMsr[i].PrivatePrefix[j]) );
				vOff.push_back( OFFSETOF(InfoMsr[i].PrivatePrefix[j]) );

				vKey.push_back( sSection + _T("&") + _T("postfix" ) );
				vSize.push_back( SIZEOF(InfoMsr[i].PrivatePostfix[j]) );
				vOff.push_back( OFFSETOF(InfoMsr[i].PrivatePostfix[j]) );
			}//end for
		}//end for

		//ini map
		Value val;

		for( UINT32 i=0; i<vOff.size(); i++ ){
			mapMapping.insert( pair<UINT32, Value>( vOff[i], SetValue( val, vKey[i], vOff[i] , vSize[i] ) ) );
		}//end for

	}

	return mapMapping;
}

const CReqBuilder::Value & CReqBuilder::GetValue( const _tstring & sKey )
{
	map<UINT32, CReqBuilder::Value> & mapping = GetMappingMap();

	if( sKey.empty() ){
		//
		for_each( mapping.begin(), mapping.end(), functor_PrintMap );

		return CReqBuilder::NullValue;
	}

	map<UINT32, Value>::iterator iter;

	for( iter = mapping.begin(); iter != mapping.end(); ++iter ){

		if( iter->second.sIniKey == sKey ){
			break;
		}
	}//end for

	if( iter == mapping.end() ){
		//not found key
		return CReqBuilder::NullValue;	//return null value
	}

	//found key
	return iter->second;
}

CReqBuilder::CReqBuilder(void) :
m_nTx(0),m_nRx(0),
m_nOffsetTx(0),m_nOffsetRx(0),
m_bSet(true)
{
	::memset( &m_Req, 0, sizeof(m_Req) );
	::memset( &m_Resp, 0, sizeof(m_Resp) );
}

CReqBuilder::~CReqBuilder(void)
{
}

const _tstring & CReqBuilder::GenerateKey( const _tstring & sSectionName, const _tstring & sKeyName )
{
	static _tstring sKey;

	sKey = sSectionName + _T("&") + sKeyName;

	return sKey;
}

CReqBuilder::Value & CReqBuilder::SetValue( CReqBuilder::Value & value, const _tstring & sIniKey, UINT32 dwOffset, UINT32 dwSize )
{
	value.sIniKey = sIniKey;

	value.dwOffset = dwOffset;
	value.dwSize = dwSize;

	return value;
}

bool CReqBuilder::Build( const CLoadMsrIni::ItemType & Item, bool bSet /*=true*/)
{
#define	EQ_OFFSET(v,s)		(v.dwOffset==OFFSETOF(s))

	bool bResult = false;

	if( Item.get() ){

		_tstring sKey = GenerateKey( Item.get()->GetSectionName(), Item.get()->GetKeyName() );

		const CReqBuilder::Value & val = GetValue( sKey );

		if( val.sIniKey.empty() ){
			return bResult;	//return for failure
		}

		bResult = true;	// change default result to true.......

		m_bSet = bSet;

		// 나는 Item.get() 키값을 val.dwOffset, val.dwSize에 따라 m_Req 에 적절하게 넣고 싶다.
		m_Req.cCmd = Cmd_Config;
		m_Req.cLen = 0;

		m_nTx = 0;

		//set offset to data field.
		memcpy( &m_Req.sData[m_nTx], &val.dwOffset, sizeof(val.dwOffset) );	m_nTx += sizeof(val.dwOffset);

		//data field : [offset value : 4bytes] $ [size value : 4bytes] $ [value : size value]

		//set size to data field.
		memcpy( &m_Req.sData[m_nTx], &val.dwSize, sizeof(val.dwSize) );		m_nTx += sizeof(val.dwSize);

		if( m_bSet == true )
			m_Req.cSub = static_cast<unsigned char>(SysReqConfig_Set);
		else{
			//get section
			m_Req.cSub = static_cast<unsigned char>(SysReqConfig_Get);
			m_Req.cLen = static_cast<unsigned char>(m_nTx);
			m_nTx += sizeof( m_Req );
			return bResult;
		}

		map<UINT32, CReqBuilder::Value> & mapping = GetMappingMap();
		CIniItemLoader::array_type sTag(MSROBJ_INFO_DEF_TAG_SIZE);


		if( EQ_OFFSET(val,sSysVer) ){
			//Item->GetStringKey( sKey );

		}
		else if( 
			EQ_OFFSET(val,Interface) ||
			EQ_OFFSET(val,nBuzzerFrequency) ||
			EQ_OFFSET(val,nNormalWDT) ||
			EQ_OFFSET(val,nBootRunTime) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.KeyMap.nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[0].KeyMap[0].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[0].KeyMap[1].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[0].KeyMap[2].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[1].KeyMap[0].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[1].KeyMap[1].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[1].KeyMap[2].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[2].KeyMap[0].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[2].KeyMap[1].nMappingTableIndex) ||
			EQ_OFFSET(val,InfoMsr[2].KeyMap[2].nMappingTableIndex) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.nCpdSysTickMin) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.nCpdSysTickMax) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.nGlobalTagCondition) ||
			EQ_OFFSET(val,InfoMsr[0].cEnableTrack ) ||
			EQ_OFFSET(val,InfoMsr[1].cEnableTrack ) ||
			EQ_OFFSET(val,InfoMsr[2].cEnableTrack ) ||
			EQ_OFFSET(val,InfoMsr[0].bEnableEncryption ) ||
			EQ_OFFSET(val,InfoMsr[1].bEnableEncryption ) ||
			EQ_OFFSET(val,InfoMsr[2].bEnableEncryption ) ||
			EQ_OFFSET(val,InfoMsr[0].nBufSize ) ||
			EQ_OFFSET(val,InfoMsr[1].nBufSize ) ||
			EQ_OFFSET(val,InfoMsr[2].nBufSize )

			){//intergal value.
			UINT32 nKey = Item.get()->GetIntKey();
			memcpy( &m_Req.sData[m_nTx],  &nKey, val.dwSize );	m_nTx += val.dwSize;
		}
		else if(	 
			EQ_OFFSET(val,ContainerInfoMsrObj.TagPre) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.TagPost) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.GlobalPrefix) ||
			EQ_OFFSET(val,ContainerInfoMsrObj.GlobalPostfix) ||

			EQ_OFFSET(val,InfoMsr[0].PrivatePrefix[0]) ||
			EQ_OFFSET(val,InfoMsr[0].PrivatePrefix[1]) ||
			EQ_OFFSET(val,InfoMsr[0].PrivatePrefix[2]) ||
			EQ_OFFSET(val,InfoMsr[0].PrivatePostfix[0]) ||
			EQ_OFFSET(val,InfoMsr[0].PrivatePostfix[1]) ||
			EQ_OFFSET(val,InfoMsr[0].PrivatePostfix[2]) ||
			
			EQ_OFFSET(val,InfoMsr[1].PrivatePrefix[0]) ||
			EQ_OFFSET(val,InfoMsr[1].PrivatePrefix[1]) ||
			EQ_OFFSET(val,InfoMsr[1].PrivatePrefix[2]) ||
			EQ_OFFSET(val,InfoMsr[1].PrivatePostfix[0]) ||
			EQ_OFFSET(val,InfoMsr[1].PrivatePostfix[1]) ||
			EQ_OFFSET(val,InfoMsr[1].PrivatePostfix[2]) ||

			EQ_OFFSET(val,InfoMsr[2].PrivatePrefix[0]) ||
			EQ_OFFSET(val,InfoMsr[2].PrivatePrefix[1]) ||
			EQ_OFFSET(val,InfoMsr[2].PrivatePrefix[2]) ||
			EQ_OFFSET(val,InfoMsr[2].PrivatePostfix[0]) ||
			EQ_OFFSET(val,InfoMsr[2].PrivatePostfix[1]) ||
			EQ_OFFSET(val,InfoMsr[2].PrivatePostfix[2])

			){//array value

			if( !Item->GetArrayKey().empty() ){
				if( IsAllZero( Item->GetArrayKey() ) )
					m_Req.sData[m_nTx] = 0;
				else
					m_Req.sData[m_nTx] = static_cast<unsigned char>(Item->GetArrayKey().size());
				
				++m_nTx;
				copy( Item->GetArrayKey().begin(), Item->GetArrayKey().end(), sTag.begin() );
				sTag.resize( MSROBJ_INFO_DEF_TAG_SIZE );
			}
			else{
				m_Req.sData[m_nTx] = 0;	++m_nTx;
			}
			memcpy( &m_Req.sData[m_nTx],  &sTag[0], val.dwSize );	m_nTx += val.dwSize;
		}
		else{//unsupport item
			bResult = false;
		}

		if( bResult ){
			m_Req.cLen = static_cast<unsigned char>(m_nTx);
			m_nTx += SIZE_HOST_PACKET_HEADER;
		}
	}

	return bResult;
}

void CReqBuilder::AnalysisResponse()
{
	//TODO :-)
}

bool CReqBuilder::IsGoodResponse()
{
	if( m_Resp.cCmd == MSR_RESP_PREFIX ){

		if( m_Resp.cSub == Resp_Good ){
			return true;
		}
	}

	return false;
}


struct functor_PrintResponse
{
	void operator () ( unsigned char c )
	{
		_tout << _T("0x") << hex << c << _T(",");
	}
};

void CReqBuilder::DisplayResponse()
{
	if( m_Resp.cCmd != MSR_RESP_PREFIX )
		return;
	if( m_Resp.cSub != Resp_Good )
		return;
	//

	//TODO :-) display data from device.......
	vector<unsigned char> vResp( &m_Resp.sData[0], &m_Resp.sData[m_Resp.cLen]  );

	if( vResp.size() == 4 ){
		UINT32 dwVal = 0;

		memcpy( &dwVal, &vResp[0], vResp.size() );

		_tout << dec << _T("(uint)") << to_tstring( static_cast<unsigned long long>(dwVal) ) << _T(",");
	}
	else{
		for_each( vResp.begin(), vResp.end(), functor_PrintResponse() );
	}

	_tout << endl;
}


bool CReqBuilder::IsAllZero( const CIniItemLoader::array_type & v )
{
	if( v.empty() )
		return true;

	CIniItemLoader::array_type::const_iterator iter = v.begin();

	for( ; iter != v.end(); ++iter ){

		if( *iter != 0x00 )
			return false;
	}//end for

	return true;
}

/**************************************************
* CReqBuilderOld class
*
***************************************************/
bool CReqBuilderOld::Build( const CLoadMsrIni::ItemType & Item, bool bSet /*=true*/)
{
#define	EQ_OFFSET_OLD(v,s)		(v.dwOffset==OFFSETOF_OLD(s))

	bool bResult = false;

	if( Item.get() ){

		_tstring sKey = GenerateKey( Item.get()->GetSectionName(), Item.get()->GetKeyName() );

		const CReqBuilderOld::Value & val = GetValueOld( sKey );

		if( val.sIniKey.empty() ){
			return bResult;	//return for failure
		}

		bResult = true;	// change default result to true.......

		m_bSet = bSet;

		// 나는 Item.get() 키값을 val.dwOffset, val.dwSize에 따라 m_Req 에 적절하게 넣고 싶다.
		m_Req.cCmd = Cmd_Config;
		m_Req.cLen = 0;

		m_nTx = 0;

		UINT32 dwOffset, dwSize;

		if( EQ_OFFSET_OLD(val,ContainerInfoMsrObj.KeyMap.pMappingTable) ){
			//for old compatblilty resetting
			dwOffset = OFFSETOF_OLD(ContainerInfoMsrObj.KeyMap.pMappingTable);
			dwSize = SIZEOF_OLD(ContainerInfoMsrObj.KeyMap.pMappingTable);

			//set offset to data field.
			memcpy( &m_Req.sData[m_nTx], &dwOffset, sizeof(dwOffset) );	m_nTx += sizeof(dwOffset);

			//data field : [offset value : 4bytes] $ [size value : 4bytes] $ [value : size value]

			//set size to data field.
			memcpy( &m_Req.sData[m_nTx], &dwSize, sizeof(dwSize) );		m_nTx += sizeof(dwSize);
		}
		else{
			//set offset to data field.
			memcpy( &m_Req.sData[m_nTx], &val.dwOffset, sizeof(val.dwOffset) );	m_nTx += sizeof(val.dwOffset);

			//data field : [offset value : 4bytes] $ [size value : 4bytes] $ [value : size value]

			//set size to data field.
			memcpy( &m_Req.sData[m_nTx], &val.dwSize, sizeof(val.dwSize) );		m_nTx += sizeof(val.dwSize);
		}

		if( m_bSet == true )
			m_Req.cSub = static_cast<unsigned char>(SysReqConfig_Set);
		else{
			//get section

			if( 	EQ_OFFSET_OLD(val,InfoMsr[0].KeyMap[0].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[0].KeyMap[1].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[0].KeyMap[2].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[1].KeyMap[0].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[1].KeyMap[1].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[1].KeyMap[2].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[2].KeyMap[0].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[2].KeyMap[1].pMappingTable) ||
				EQ_OFFSET_OLD(val,InfoMsr[2].KeyMap[2].pMappingTable)
			 )
			{	//for old compatblilty, Not supported item.
				bResult = false;
				return bResult;
			}

			m_Req.cSub = static_cast<unsigned char>(SysReqConfig_Get);
			m_Req.cLen = static_cast<unsigned char>(m_nTx);
			m_nTx += sizeof( m_Req );
			return bResult;
		}

		map<UINT32, CReqBuilder::Value> & mapping = GetMappingMap();
		CIniItemLoader::array_type sTag(MSROBJ_INFO_DEF_TAG_SIZE);


		if( EQ_OFFSET_OLD(val,sSysVer) ){
			//Item->GetStringKey( sKey );

		}
		else if( EQ_OFFSET_OLD(val,ContainerInfoMsrObj.KeyMap.pMappingTable) )
		{	//for old compatblilty resetting
			UINT32 nKey = Item.get()->GetIntKey();
			memcpy( &m_Req.sData[m_nTx],  &nKey, dwSize );	m_nTx += dwSize;

		}
		else if( 
			EQ_OFFSET_OLD(val,Interface) ||
			EQ_OFFSET_OLD(val,nBuzzerFrequency) ||
			EQ_OFFSET_OLD(val,nNormalWDT) ||
			EQ_OFFSET_OLD(val,nBootRunTime) ||
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.nCpdSysTickMin) ||
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.nCpdSysTickMax) ||
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.nGlobalTagCondition) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].cEnableTrack ) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].cEnableTrack ) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].cEnableTrack ) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].bEnableEncryption ) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].bEnableEncryption ) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].bEnableEncryption ) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].nBufSize ) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].nBufSize ) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].nBufSize )

			){//intergal value.
			UINT32 nKey = Item.get()->GetIntKey();
			memcpy( &m_Req.sData[m_nTx],  &nKey, val.dwSize );	m_nTx += val.dwSize;
		}
		else if(	 
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.TagPre) ||
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.TagPost) ||
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.GlobalPrefix) ||
			EQ_OFFSET_OLD(val,ContainerInfoMsrObj.GlobalPostfix) ||

			EQ_OFFSET_OLD(val,InfoMsr[0].PrivatePrefix[0]) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].PrivatePrefix[1]) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].PrivatePrefix[2]) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].PrivatePostfix[0]) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].PrivatePostfix[1]) ||
			EQ_OFFSET_OLD(val,InfoMsr[0].PrivatePostfix[2]) ||
			
			EQ_OFFSET_OLD(val,InfoMsr[1].PrivatePrefix[0]) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].PrivatePrefix[1]) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].PrivatePrefix[2]) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].PrivatePostfix[0]) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].PrivatePostfix[1]) ||
			EQ_OFFSET_OLD(val,InfoMsr[1].PrivatePostfix[2]) ||

			EQ_OFFSET_OLD(val,InfoMsr[2].PrivatePrefix[0]) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].PrivatePrefix[1]) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].PrivatePrefix[2]) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].PrivatePostfix[0]) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].PrivatePostfix[1]) ||
			EQ_OFFSET_OLD(val,InfoMsr[2].PrivatePostfix[2])

			){//array value

			if( !Item->GetArrayKey().empty() ){
				if( IsAllZero( Item->GetArrayKey() ) )
					m_Req.sData[m_nTx] = 0;
				else
					m_Req.sData[m_nTx] = static_cast<unsigned char>(Item->GetArrayKey().size());
				
				++m_nTx;
				copy( Item->GetArrayKey().begin(), Item->GetArrayKey().end(), sTag.begin() );
				sTag.resize( MSROBJ_INFO_DEF_TAG_SIZE );
			}
			else{
				m_Req.sData[m_nTx] = 0;	++m_nTx;
			}
			memcpy( &m_Req.sData[m_nTx],  &sTag[0], val.dwSize );	m_nTx += val.dwSize;
		}
		else{//unsupport item
			bResult = false;
		}

		if( bResult ){
			m_Req.cLen = static_cast<unsigned char>(m_nTx);
			m_nTx += SIZE_HOST_PACKET_HEADER;
		}
	}

	return bResult;
}

map<UINT32, CReqBuilderOld::Value> & CReqBuilderOld::GetMappingMapOld()
{
	//#define	INSERT_TO_MAP_OLD(MAP,VAL,INIKEY,M)		( MAP.insert( pair<UINT32, Value>( OFFSETOF_OLD(M), SetValue( VAL, INIKEY, OFFSETOF_OLD(M) , SIZEOF_OLD(M) ) ) ) )

	static map<UINT32, Value> mapMapping;
	static bool bIsFirst = true;

	if( bIsFirst ){
		bIsFirst = false;

		////////////////////////////////////////////////////////////////////
		//set offset & size vector
		_tstring sSection;
		vector<_tstring> vKey;
		vector<UINT32> vSize;
		vector<UINT32> vOff;

		//
		sSection = _T("common");

		vKey.push_back( sSection + _T("&") + _T("sysv" ) );
		vSize.push_back( SIZEOF_OLD(sSysVer) );
		vOff.push_back( OFFSETOF_OLD(sSysVer) );

		vKey.push_back( sSection + _T("&") + _T("interface" ) );
		vSize.push_back( SIZEOF_OLD(Interface) );
		vOff.push_back( OFFSETOF_OLD(Interface) );

		vKey.push_back( sSection + _T("&") + _T("buzzer" ) );
		vSize.push_back( SIZEOF_OLD(nBuzzerFrequency) );
		vOff.push_back( OFFSETOF_OLD(nBuzzerFrequency) );

		vKey.push_back( sSection + _T("&") + _T("wdt" ) );
		vSize.push_back( SIZEOF_OLD(nNormalWDT) );
		vOff.push_back( OFFSETOF_OLD(nNormalWDT) );

		//
		vKey.push_back( sSection + _T("&") + _T("boottime" ) );
		vSize.push_back( SIZEOF_OLD(nBootRunTime) );
		vOff.push_back( OFFSETOF_OLD(nBootRunTime) );

		vKey.push_back( sSection + _T("&") + _T("keymap" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.KeyMap.pMappingTable) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.KeyMap.pMappingTable) );

		vKey.push_back( sSection + _T("&") + _T("pretag" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.TagPre) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.TagPre) );

		vKey.push_back( sSection + _T("&") + _T("posttag" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.TagPost) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.TagPost) );

		vKey.push_back( sSection + _T("&") + _T("prefix" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.GlobalPrefix) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.GlobalPrefix) );

		vKey.push_back( sSection + _T("&") + _T("postfix" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.GlobalPostfix) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.GlobalPostfix) );

		vKey.push_back( sSection + _T("&") + _T("cpdmin" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.nCpdSysTickMin) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.nCpdSysTickMin) );

		vKey.push_back( sSection + _T("&") + _T("cpdmax" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.nCpdSysTickMax) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.nCpdSysTickMax) );

		vKey.push_back( sSection + _T("&") + _T("gfixmode" ) );
		vSize.push_back( SIZEOF_OLD(ContainerInfoMsrObj.nGlobalTagCondition) );
		vOff.push_back( OFFSETOF_OLD(ContainerInfoMsrObj.nGlobalTagCondition) );

		for( INT32 i=0; i<3; i++ ){
			//object i+1 common
			sSection = _T("object") + to_tstring( (unsigned long long)(i+1) );

			vKey.push_back( sSection + _T("&") + _T("enable" ) );
			vSize.push_back( SIZEOF_OLD(InfoMsr[i].cEnableTrack) );
			vOff.push_back( OFFSETOF_OLD(InfoMsr[i].cEnableTrack) );

			vKey.push_back( sSection + _T("&") + _T("encrypt" ) );
			vSize.push_back( SIZEOF_OLD(InfoMsr[i].bEnableEncryption) );
			vOff.push_back( OFFSETOF_OLD(InfoMsr[i].bEnableEncryption) );

			vKey.push_back( sSection + _T("&") + _T("buffersize" ) );
			vSize.push_back( SIZEOF_OLD(InfoMsr[i].nBufSize) );
			vOff.push_back( OFFSETOF_OLD(InfoMsr[i].nBufSize) );

			for( INT32 j=0; j<3; j++ ){
				//object i+1 , combination j
				sSection = _T("object") + to_tstring( (unsigned long long)(i+1) ) + to_tstring( (unsigned long long)j );

				vKey.push_back( sSection + _T("&") + _T("prefix" ) );
				vSize.push_back( SIZEOF_OLD(InfoMsr[i].PrivatePrefix[j]) );
				vOff.push_back( OFFSETOF_OLD(InfoMsr[i].PrivatePrefix[j]) );

				vKey.push_back( sSection + _T("&") + _T("postfix" ) );
				vSize.push_back( SIZEOF_OLD(InfoMsr[i].PrivatePostfix[j]) );
				vOff.push_back( OFFSETOF_OLD(InfoMsr[i].PrivatePostfix[j]) );
			}//end for
		}//end for

		//ini map
		Value val;

		for( UINT32 i=0; i<vOff.size(); i++ ){
			mapMapping.insert( pair<UINT32, Value>( vOff[i], SetValue( val, vKey[i], vOff[i] , vSize[i] ) ) );
		}//end for

	}

	return mapMapping;
}

const CReqBuilderOld::Value & CReqBuilderOld::GetValueOld( const _tstring & sKey )
{
	map<UINT32, CReqBuilderOld::Value> & mapping = GetMappingMapOld();

	if( sKey.empty() ){
		//
		for_each( mapping.begin(), mapping.end(), functor_PrintMap );

		return CReqBuilder::NullValue;
	}

	map<UINT32, Value>::iterator iter;

	for( iter = mapping.begin(); iter != mapping.end(); ++iter ){

		if( iter->second.sIniKey == sKey ){
			break;
		}
	}//end for

	if( iter == mapping.end() ){
		//not found key
		return CReqBuilder::NullValue;	//return null value
	}

	//found key
	return iter->second;
}


/**************************************************
* CReqBuilderStd class
*
***************************************************/
bool CReqBuilderStd::Build( const CLoadMsrIni::ItemType & Item, bool bSet /*=true*/)
{
#define	EQ_OFFSET_STD(v,s)		(v.dwOffset==OFFSETOF_STD(s))

	bool bResult = false;

	if( Item.get() ){

		_tstring sKey = GenerateKey( Item.get()->GetSectionName(), Item.get()->GetKeyName() );

		const CReqBuilderStd::Value & val = GetValueStd( sKey );

		if( val.sIniKey.empty() ){
			return bResult;	//return for failure
		}

		bResult = true;	// change default result to true.......

		m_bSet = bSet;

		// 나는 Item.get() 키값을 val.dwOffset, val.dwSize에 따라 m_Req 에 적절하게 넣고 싶다.
		m_Req.cCmd = Cmd_Config;
		m_Req.cLen = 0;

		m_nTx = 0;

		//set offset to data field.
		memcpy( &m_Req.sData[m_nTx], &val.dwOffset, sizeof(val.dwOffset) );	m_nTx += sizeof(val.dwOffset);

		//data field : [offset value : 4bytes] $ [size value : 4bytes] $ [value : size value]

		//set size to data field.
		memcpy( &m_Req.sData[m_nTx], &val.dwSize, sizeof(val.dwSize) );		m_nTx += sizeof(val.dwSize);

		if( m_bSet == true )
			m_Req.cSub = static_cast<unsigned char>(SysReqConfig_Set);
		else{
			//get section
			m_Req.cSub = static_cast<unsigned char>(SysReqConfig_Get);
			m_Req.cLen = static_cast<unsigned char>(m_nTx);
			m_nTx += sizeof( m_Req );
			return bResult;
		}

		map<UINT32, CReqBuilderStd::Value> & mapping = GetMappingMapStd();
		CIniItemLoader::array_type sTag(MSROBJ_INFO_DEF_TAG_SIZE);


		if( EQ_OFFSET_STD(val,sSysVer) ){
			//Item->GetStringKey( sKey );

		}
		else if( 
			EQ_OFFSET_STD(val,Interface) ||
			EQ_OFFSET_STD(val,nBuzzerFrequency) ||
			EQ_OFFSET_STD(val,nNormalWDT) ||
			EQ_OFFSET_STD(val,nBootRunTime) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.KeyMap.nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[0].KeyMap[0].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[0].KeyMap[1].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[0].KeyMap[2].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[1].KeyMap[0].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[1].KeyMap[1].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[1].KeyMap[2].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[2].KeyMap[0].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[2].KeyMap[1].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,InfoMsr[2].KeyMap[2].nMappingTableIndex) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.nCpdSysTickMin) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.nCpdSysTickMax) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.nGlobalTagCondition) ||
			EQ_OFFSET_STD(val,InfoMsr[0].cEnableTrack ) ||
			EQ_OFFSET_STD(val,InfoMsr[1].cEnableTrack ) ||
			EQ_OFFSET_STD(val,InfoMsr[2].cEnableTrack ) ||
			EQ_OFFSET_STD(val,InfoMsr[0].bEnableEncryption ) ||
			EQ_OFFSET_STD(val,InfoMsr[1].bEnableEncryption ) ||
			EQ_OFFSET_STD(val,InfoMsr[2].bEnableEncryption ) ||
			EQ_OFFSET_STD(val,InfoMsr[0].nBufSize ) ||
			EQ_OFFSET_STD(val,InfoMsr[1].nBufSize ) ||
			EQ_OFFSET_STD(val,InfoMsr[2].nBufSize )

			){//intergal value.
			UINT32 nKey = Item.get()->GetIntKey();
			memcpy( &m_Req.sData[m_nTx],  &nKey, val.dwSize );	m_nTx += val.dwSize;
		}
		else if(	 
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.TagPre) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.TagPost) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.GlobalPrefix) ||
			EQ_OFFSET_STD(val,ContainerInfoMsrObj.GlobalPostfix) ||

			EQ_OFFSET_STD(val,InfoMsr[0].PrivatePrefix[0]) ||
			EQ_OFFSET_STD(val,InfoMsr[0].PrivatePrefix[1]) ||
			EQ_OFFSET_STD(val,InfoMsr[0].PrivatePrefix[2]) ||
			EQ_OFFSET_STD(val,InfoMsr[0].PrivatePostfix[0]) ||
			EQ_OFFSET_STD(val,InfoMsr[0].PrivatePostfix[1]) ||
			EQ_OFFSET_STD(val,InfoMsr[0].PrivatePostfix[2]) ||
			
			EQ_OFFSET_STD(val,InfoMsr[1].PrivatePrefix[0]) ||
			EQ_OFFSET_STD(val,InfoMsr[1].PrivatePrefix[1]) ||
			EQ_OFFSET_STD(val,InfoMsr[1].PrivatePrefix[2]) ||
			EQ_OFFSET_STD(val,InfoMsr[1].PrivatePostfix[0]) ||
			EQ_OFFSET_STD(val,InfoMsr[1].PrivatePostfix[1]) ||
			EQ_OFFSET_STD(val,InfoMsr[1].PrivatePostfix[2]) ||

			EQ_OFFSET_STD(val,InfoMsr[2].PrivatePrefix[0]) ||
			EQ_OFFSET_STD(val,InfoMsr[2].PrivatePrefix[1]) ||
			EQ_OFFSET_STD(val,InfoMsr[2].PrivatePrefix[2]) ||
			EQ_OFFSET_STD(val,InfoMsr[2].PrivatePostfix[0]) ||
			EQ_OFFSET_STD(val,InfoMsr[2].PrivatePostfix[1]) ||
			EQ_OFFSET_STD(val,InfoMsr[2].PrivatePostfix[2]) ||

			EQ_OFFSET_STD(val,InfoIButton.TagPre) ||
			EQ_OFFSET_STD(val,InfoIButton.TagPost) ||
			EQ_OFFSET_STD(val,InfoIButton.GlobalPrefix) ||
			EQ_OFFSET_STD(val,InfoIButton.GlobalPostfix) ||

			EQ_OFFSET_STD(val,InfoUart.TagPre) ||
			EQ_OFFSET_STD(val,InfoUart.TagPost) ||
			EQ_OFFSET_STD(val,InfoUart.GlobalPrefix) ||
			EQ_OFFSET_STD(val,InfoUart.GlobalPostfix)

			){//array value

			if( !Item->GetArrayKey().empty() ){
				if( IsAllZero( Item->GetArrayKey() ) )
					m_Req.sData[m_nTx] = 0;
				else
					m_Req.sData[m_nTx] = static_cast<unsigned char>(Item->GetArrayKey().size());
				
				++m_nTx;
				copy( Item->GetArrayKey().begin(), Item->GetArrayKey().end(), sTag.begin() );
				sTag.resize( MSROBJ_INFO_DEF_TAG_SIZE );
			}
			else{
				m_Req.sData[m_nTx] = 0;	++m_nTx;
			}
			memcpy( &m_Req.sData[m_nTx],  &sTag[0], val.dwSize );	m_nTx += val.dwSize;
		}
		else{//unsupport item
			bResult = false;
		}

		if( bResult ){
			m_Req.cLen = static_cast<unsigned char>(m_nTx);
			m_nTx += SIZE_HOST_PACKET_HEADER;
		}
	}

	return bResult;
}

map<UINT32, CReqBuilderStd::Value> & CReqBuilderStd::GetMappingMapStd()
{
	//#define	INSERT_TO_MAP(MAP,VAL,INIKEY,M)		( MAP.insert( pair<UINT32, Value>( OFFSETOF(M), SetValue( VAL, INIKEY, OFFSETOF(M) , SIZEOF(M) ) ) ) )

	static map<UINT32, Value> mapMapping;
	static bool bIsFirst = true;

	if( bIsFirst ){
		bIsFirst = false;

		////////////////////////////////////////////////////////////////////
		//set offset & size vector
		_tstring sSection;
		vector<_tstring> vKey;
		vector<UINT32> vSize;
		vector<UINT32> vOff;

		//
		sSection = _T("common");

		vKey.push_back( sSection + _T("&") + _T("sysv" ) );
		vSize.push_back( SIZEOF_STD(sSysVer) );
		vOff.push_back( OFFSETOF_STD(sSysVer) );

		vKey.push_back( sSection + _T("&") + _T("interface" ) );
		vSize.push_back( SIZEOF_STD(Interface) );
		vOff.push_back( OFFSETOF_STD(Interface) );

		vKey.push_back( sSection + _T("&") + _T("buzzer" ) );
		vSize.push_back( SIZEOF_STD(nBuzzerFrequency) );
		vOff.push_back( OFFSETOF_STD(nBuzzerFrequency) );

		vKey.push_back( sSection + _T("&") + _T("wdt" ) );
		vSize.push_back( SIZEOF_STD(nNormalWDT) );
		vOff.push_back( OFFSETOF_STD(nNormalWDT) );

		//
		vKey.push_back( sSection + _T("&") + _T("boottime" ) );
		vSize.push_back( SIZEOF_STD(nBootRunTime) );
		vOff.push_back( OFFSETOF_STD(nBootRunTime) );

		vKey.push_back( sSection + _T("&") + _T("keymap" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.KeyMap.nMappingTableIndex) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.KeyMap.nMappingTableIndex) );

		vKey.push_back( sSection + _T("&") + _T("pretag" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.TagPre) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.TagPre) );

		vKey.push_back( sSection + _T("&") + _T("posttag" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.TagPost) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.TagPost) );

		vKey.push_back( sSection + _T("&") + _T("prefix" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.GlobalPrefix) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.GlobalPrefix) );

		vKey.push_back( sSection + _T("&") + _T("postfix" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.GlobalPostfix) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.GlobalPostfix) );

		vKey.push_back( sSection + _T("&") + _T("cpdmin" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.nCpdSysTickMin) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.nCpdSysTickMin) );

		vKey.push_back( sSection + _T("&") + _T("cpdmax" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.nCpdSysTickMax) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.nCpdSysTickMax) );

		vKey.push_back( sSection + _T("&") + _T("gfixmode" ) );
		vSize.push_back( SIZEOF_STD(ContainerInfoMsrObj.nGlobalTagCondition) );
		vOff.push_back( OFFSETOF_STD(ContainerInfoMsrObj.nGlobalTagCondition) );

		for( INT32 i=0; i<3; i++ ){
			//object i+1 common
			sSection = _T("object") + to_tstring( (unsigned long long)(i+1) );

			vKey.push_back( sSection + _T("&") + _T("enable" ) );
			vSize.push_back( SIZEOF_STD(InfoMsr[i].cEnableTrack) );
			vOff.push_back( OFFSETOF_STD(InfoMsr[i].cEnableTrack) );

			vKey.push_back( sSection + _T("&") + _T("encrypt" ) );
			vSize.push_back( SIZEOF_STD(InfoMsr[i].bEnableEncryption) );
			vOff.push_back( OFFSETOF_STD(InfoMsr[i].bEnableEncryption) );

			vKey.push_back( sSection + _T("&") + _T("buffersize" ) );
			vSize.push_back( SIZEOF_STD(InfoMsr[i].nBufSize) );
			vOff.push_back( OFFSETOF_STD(InfoMsr[i].nBufSize) );

			for( INT32 j=0; j<3; j++ ){
				//object i+1 , combination j
				sSection = _T("object") + to_tstring( (unsigned long long)(i+1) ) + to_tstring( (unsigned long long)j );

				vKey.push_back( sSection + _T("&") + _T("keymap" ) );
				vSize.push_back( SIZEOF_STD(InfoMsr[i].KeyMap[j].nMappingTableIndex) );
				vOff.push_back( OFFSETOF_STD(InfoMsr[i].KeyMap[j].nMappingTableIndex) );

				vKey.push_back( sSection + _T("&") + _T("prefix" ) );
				vSize.push_back( SIZEOF_STD(InfoMsr[i].PrivatePrefix[j]) );
				vOff.push_back( OFFSETOF_STD(InfoMsr[i].PrivatePrefix[j]) );

				vKey.push_back( sSection + _T("&") + _T("postfix" ) );
				vSize.push_back( SIZEOF_STD(InfoMsr[i].PrivatePostfix[j]) );
				vOff.push_back( OFFSETOF_STD(InfoMsr[i].PrivatePostfix[j]) );
			}//end for
		}//end for


		//iButton
		sSection = _T("ibutton");

		vKey.push_back( sSection + _T("&") + _T("pretag" ) );
		vSize.push_back( SIZEOF_STD(InfoIButton.TagPre) );
		vOff.push_back( OFFSETOF_STD(InfoIButton.TagPre) );

		vKey.push_back( sSection + _T("&") + _T("posttag" ) );
		vSize.push_back( SIZEOF_STD(InfoIButton.TagPost) );
		vOff.push_back( OFFSETOF_STD(InfoIButton.TagPost) );

		vKey.push_back( sSection + _T("&") + _T("prefix" ) );
		vSize.push_back( SIZEOF_STD(InfoIButton.GlobalPrefix) );
		vOff.push_back( OFFSETOF_STD(InfoIButton.GlobalPrefix) );

		vKey.push_back( sSection + _T("&") + _T("postfix" ) );
		vSize.push_back( SIZEOF_STD(InfoIButton.GlobalPostfix) );
		vOff.push_back( OFFSETOF_STD(InfoIButton.GlobalPostfix) );

		//iButton
		sSection = _T("RS232");

		vKey.push_back( sSection + _T("&") + _T("pretag" ) );
		vSize.push_back( SIZEOF_STD(InfoUart.TagPre) );
		vOff.push_back( OFFSETOF_STD(InfoUart.TagPre) );

		vKey.push_back( sSection + _T("&") + _T("posttag" ) );
		vSize.push_back( SIZEOF_STD(InfoUart.TagPost) );
		vOff.push_back( OFFSETOF_STD(InfoUart.TagPost) );

		vKey.push_back( sSection + _T("&") + _T("prefix" ) );
		vSize.push_back( SIZEOF_STD(InfoUart.GlobalPrefix) );
		vOff.push_back( OFFSETOF_STD(InfoUart.GlobalPrefix) );

		vKey.push_back( sSection + _T("&") + _T("postfix" ) );
		vSize.push_back( SIZEOF_STD(InfoUart.GlobalPostfix) );
		vOff.push_back( OFFSETOF_STD(InfoUart.GlobalPostfix) );

		//ini map
		Value val;

		for( UINT32 i=0; i<vOff.size(); i++ ){
			mapMapping.insert( pair<UINT32, Value>( vOff[i], SetValue( val, vKey[i], vOff[i] , vSize[i] ) ) );
		}//end for

	}

	return mapMapping;
}

const CReqBuilderStd::Value & CReqBuilderStd::GetValueStd( const _tstring & sKey )
{
	map<UINT32, CReqBuilder::Value> & mapping = GetMappingMapStd();

	if( sKey.empty() ){
		//
		for_each( mapping.begin(), mapping.end(), functor_PrintMap );

		return CReqBuilder::NullValue;
	}

	map<UINT32, Value>::iterator iter;

	for( iter = mapping.begin(); iter != mapping.end(); ++iter ){

		if( iter->second.sIniKey == sKey ){
			break;
		}
	}//end for

	if( iter == mapping.end() ){
		//not found key
		return CReqBuilder::NullValue;	//return null value
	}

	//found key
	return iter->second;}