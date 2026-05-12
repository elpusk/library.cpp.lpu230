#include "StdAfx.h"
#include "UserPara.h"

const _tstring CUserPara::NullString;

CUserPara::CUserPara(void)
{
}


CUserPara::CUserPara( INT32 argc, _TCHAR* argv[] )
{
	Load( argc, argv );
}

CUserPara::~CUserPara(void)
{
	m_vArg.clear();
}

const _tstring & CUserPara::GetParameter( INT32 nArg )
{
	if( nArg > static_cast<INT32>(m_vArg.size()) )
		return NullString;
	//
	return m_vArg[nArg];
}

void CUserPara::Load(  INT32 argc, _TCHAR* argv[] )
{
	if( argc <=0 )
		return;
	//
	if( argv == NULL )
		return;

	if( m_vArg.size() > 0 )
		m_vArg.clear();

	for( INT32 i = 0; i< argc; i++ ){

		if( argv[i] ){
			m_vArg.push_back( _tstring( argv[i] ) );
		}
	}//end for

}
