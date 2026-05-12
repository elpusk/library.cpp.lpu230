#include "StdAfx.h"
#include "LoadMsrIni.h"
#include <fstream>
#include <algorithm>
#include <vector>


CLoadMsrIni::CLoadMsrIni(void)
{
}


CLoadMsrIni::~CLoadMsrIni(void)
{
	Unload();
}

bool CLoadMsrIni::Load( const _tstring sInifileName )
{
	if( sInifileName.empty() ){
		return false;
	}

	//check exist file
	_tifstream ifs( sInifileName.c_str(), ios::in );
	if( ifs.bad() ){
		return false;
	}

	ifs.close();

	//unload already file
	if( m_bLoaded ){
		Unload();
	}

	_tstring sSection( _T("common") );
	vector< pair<_tstring,bool> > vCommonKey;
	vCommonKey.push_back( pair<_tstring,bool>(_T("sysv"),true) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("interface"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("buzzer"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("wdt"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("boottime"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("keymap"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("pretag"),true) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("posttag"),true) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("prefix"),true) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("postfix"),true) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("cpdmin"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("cpdmax"),false) );
	vCommonKey.push_back( pair<_tstring,bool>(_T("gfixmode"),false) );

	vector< pair<_tstring,bool> > vTrackKey;
	vTrackKey.push_back( pair<_tstring,bool>(_T("enable"),false) );
	vTrackKey.push_back( pair<_tstring,bool>(_T("encrypt"),false) );
	vTrackKey.push_back( pair<_tstring,bool>(_T("buffersize"),false) );

	vector< pair<_tstring,bool> > vCombiKey;
	vCombiKey.push_back( pair<_tstring,bool>(_T("keymap"),false) );
	vCombiKey.push_back( pair<_tstring,bool>(_T("prefix"),true) );
	vCombiKey.push_back( pair<_tstring,bool>(_T("postfix"),true) );

	ItemType Item;

	//load key from file
	for( UINT32 i=0; i< vCommonKey.size(); i++ ){

		Item.reset( new CIniItemLoader() );
		if( Item->Load( sSection, vCommonKey[i].first, sInifileName, vCommonKey[i].second ) )
			m_listItem.push_back( Item );

	}//end for

	//
	vector<_tstring> vPrivatePre;

	vPrivatePre.push_back( _T("") );
	vPrivatePre.push_back( _T("") );
	vPrivatePre.push_back( _T("") );

	vector<_tstring> vPrivatePost;

	vPrivatePost.push_back( _T("") );
	vPrivatePost.push_back( _T("") );
	vPrivatePost.push_back( _T("") );

	for( unsigned long long nTrack =1; nTrack <4; nTrack++ ){
		// each track common section
		sSection = _T("object") + to_tstring( nTrack );

		for( UINT32 i=0; i<vTrackKey.size(); i++ ){
			Item.reset( new CIniItemLoader() );
			if( Item->Load( sSection, vTrackKey[i].first, sInifileName, vTrackKey[i].second ) )
				m_listItem.push_back( Item );
		}

		for( unsigned long long nComb = 0; nComb < 3; nComb++ ){

			//each track combination section
			sSection = _T("object") + to_tstring( nTrack ) + to_tstring( nComb );

			for( UINT32 i=0; i<vCombiKey.size(); i++ ){
				Item.reset( new CIniItemLoader() );
				if( Item->Load( sSection, vCombiKey[i].first, sInifileName, vCombiKey[i].second ) )
					m_listItem.push_back( Item );
			}//end for
		}//end for
	}//end for

	m_bLoaded = true;

	//check load status
	for( list< CLoadIni::ItemType >::iterator iter = m_listItem.begin() ; iter != m_listItem.end(); ++iter ){

		if( iter->get() ){
			if( ! iter->get()->IsLoadSuccess() )
				return false;
		}
	}

	return true;
}
