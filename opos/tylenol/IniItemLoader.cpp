#include "StdAfx.h"
#include "IniItemLoader.h"
#include <vector>
#include "StringTokenizer.h"
#include <algorithm>

CIniItemLoader::~CIniItemLoader(void)
{
}

CIniItemLoader::CIniItemLoader(void):
	m_nKey(0),
	m_bIsSuccess(false),
	m_bIsStringType(true),
	m_TokenFormat(ef_decimal)
{
}

CIniItemLoader::CIniItemLoader( 
	 const _tstring & sSectionName,
	 const _tstring & sKeyName,
	  const _tstring & sIniFileName, 
	 bool bIsStringType /*= true*/ 
	 )
{
	Load( sSectionName, sKeyName, sIniFileName, bIsStringType );
}

CIniItemLoader::CIniItemLoader( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, INT32 nDefault )
{
	Load( sSectionName, sKeyName, sIniFileName, false );

	if( !m_bIsSuccess ){
		//loading default value
		m_nKey = nDefault;
		m_bIsSuccess = true;
	}
}

CIniItemLoader::CIniItemLoader( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, const _tstring & sDefault )
{
	Load( sSectionName, sKeyName, sIniFileName, true );

	if( !m_bIsSuccess ){
		//loading default value
		m_sKey = sDefault;
		m_bIsSuccess = true;
	}

}


CIniItemLoader::CIniItemLoader(const CIniItemLoader & Item )
{
	m_sIniFileName = Item.m_sIniFileName;
	m_sSectionName = Item.m_sSectionName;
	m_sKeyName = Item.m_sKeyName;
	m_nKey = Item.m_nKey;
	m_sKey = Item.m_sKey;
	m_bIsSuccess = Item.m_bIsSuccess;
	m_bIsStringType = Item.m_bIsStringType;
}

CIniItemLoader & CIniItemLoader::operator= ( const CIniItemLoader & rhs )
{
	if( this != &rhs ){
		m_sIniFileName = rhs.m_sIniFileName;
		m_sSectionName = rhs.m_sSectionName;
		m_sKeyName = rhs.m_sKeyName;
		m_nKey = rhs.m_nKey;
		m_sKey = rhs.m_sKey;
		m_bIsSuccess = rhs.m_bIsSuccess;
		m_bIsStringType = rhs.m_bIsStringType;
	}

	return *this;
}

bool CIniItemLoader::Load( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, bool bIsStringType /*= true*/ )
{
	m_sIniFileName = sIniFileName;
	m_sSectionName = sSectionName;
	m_sKeyName = sKeyName;
	m_nKey = 0;
	m_bIsSuccess = true;
	m_bIsStringType = bIsStringType;


	if( bIsStringType ){

		//string type & array
		vector<TCHAR> vKeyValue(256);
		INT32 nVal = ::GetPrivateProfileString( sSectionName.c_str(), sKeyName.c_str(), _T("ERROR"), &vKeyValue[0], 256, sIniFileName.c_str() );
		_tstring sKeyValue( vKeyValue.begin(), vKeyValue.begin() + nVal );

		if( sKeyValue == _T("ERROR") ){
			m_bIsSuccess = false;
		}
		else{
			m_sKey = sKeyValue;
		}

		SetArrayKey();
	}
	else{

		//intergal
		INT32 nVal = ::GetPrivateProfileInt( sSectionName.c_str(), sKeyName.c_str(), 999, sIniFileName.c_str() );
		if( nVal == 999 ){
			m_bIsSuccess = false;
		}
		else{
			m_nKey = nVal;
		}
	}

	return m_bIsSuccess;
}

bool CIniItemLoader::SetArrayKey()
{
	if( m_sKey.empty() )
		return false;
	//

	m_vKey.clear();

	//fill m_vKey vector from m_sKey
	CStringTokenizer tokenizer( m_sKey, _tstring(_T(" ")) );

	_tstring sToken;
	INT32 nVal = 0;

	while( tokenizer.hasMoreTokens() ){

		sToken = tokenizer.nextToken();

		if( !IsAvaliedFormat( sToken ) ){
			m_vKey.clear();
			return false;
		}
		else{

			nVal = GetByteFromToken( sToken );

			if( m_TokenFormat == ef_ascii )
				m_vKey.push_back( 0xff );

			m_vKey.push_back( static_cast<unsigned char>( nVal ) );
		}

	}//end while

	return true;
}

//check a format of token, decimal , hexcimal or character
bool CIniItemLoader::IsAvaliedFormat( const _tstring & sToken )
{
	INT32 nResult = GetByteFromToken( sToken );

	if( nResult < 0 )
		return false;
	else
		return true;
}

//get one byte data from token
// return negative is error.
// this method set a token format( m_TokenFormat memeber ) 
INT32 CIniItemLoader::GetByteFromToken( const _tstring & sToken )
{
	INT32 nResult = -1;

	// case 1 : 0~9
	// case 2 : starting with 0x, 0~9, a~f, A~F, max size 4
	// case 3 : start and end with ' , max size 3 
	// case 4 : raw ascii code. max size 1

	_tstring::size_type Pos = sToken.find_first_not_of( _tstring( _T("0123456789") ) );

	if( Pos == _tstring::npos ){
		//case 1 : decimal
		nResult = stoi( sToken );

		if( nResult > 255 || nResult < 0 ){
			nResult = -1;
		}
		else{
			//success
			m_TokenFormat = ef_decimal;
		}
		return nResult;
	}

	Pos = sToken.find_first_not_of( _tstring( _T("0123456789abcdefABCDEF") ) );

	if( Pos == 1 ){	//'x' of "0x"

		if( sToken[Pos] == _T('x') ){
			// case 2 : starting with 0x, 0~9, a~f, A~F, max size 4
			_tstring stemp = sToken.substr( 2 );
			nResult = stoi( stemp, 0, 16 );

			if( nResult > 255 || nResult < 0 ){
				nResult = -1;
			}
			else{
				//success
				m_TokenFormat = ef_heximal;
			}
		}
		else
			nResult = -1;

		return nResult;
	}

	//
	if( sToken.size() == 3 ){

		//case 3
		_tstring::const_reference cFirst = sToken[0];
		_tstring::const_reference cLast = sToken[2];

		if( (cFirst != _T('\'')) || (cLast != _T('\'')) )
			nResult = -1;
		else{
			nResult = static_cast<UINT32>( sToken[1] );
			if( nResult > 255 )
				nResult = -1;
			else{
				//success
				m_TokenFormat = ef_ascii;
			}
		}

		return nResult;
	}

	if( sToken.size() == 1 ){
		// case 4
		nResult = static_cast<UINT32>( sToken[0] );
		if( nResult > 255 )
			nResult = -1;
		else{
			//success
			m_TokenFormat = ef_ascii;
		}
	}
	else{
		nResult = -1;
	}

	return nResult;
}

static void functor_printArray( unsigned char c );

void functor_printArray( unsigned char c )
{
	_tout << hex << c << _T(",") ;
}

void CIniItemLoader::PrintArray()
{
	if( m_vKey.empty() ){
		_tout << _T("NO array data.") << endl;
		return;
	}

	for_each( m_vKey.begin(), m_vKey.end(), functor_printArray );

	_tout << endl;
	
}
