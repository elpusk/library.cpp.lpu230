#include "StdAfx.h"
#include "LoadIni.h"
#include <algorithm>


static void functor_display( CLoadIni::ItemType & item );

void functor_display( CLoadIni::ItemType & item )
{
	if( item.get() ){
		if( item.get()->IsLoadSuccess() ){
			_tout << item.get()->GetIniFileName() << _T( " : " ) << item.get()->GetSectionName() << _T(" : ") << item.get()->GetKeyName() << _T(" : ");

			if( item.get()->IsStringType() ){

				if( item.get()->IsArray() ){
					item.get()->PrintArray();
				}
				else{
					_tout << item.get()->GetStringKey() << endl;
				}
			}
			else{
				_tout << dec << item.get()->GetIntKey() << endl;
			}
		}
		else{
			_tout << _T("Loading faile of Item") <<endl;
		}
	}
}


CLoadIni::CLoadIni(void) :
m_bLoaded(false)
{
}


CLoadIni::~CLoadIni(void)
{
}

void CLoadIni::Unload()
{
	m_bLoaded = false;
	m_listItem.clear();
}

void CLoadIni::DisplayItems()
{
	if( !m_bLoaded ){
		_tout << _T("unloaded items") <<endl;
		return;
	}
	//

	for_each( m_listItem.begin(), m_listItem.end(), functor_display );
}