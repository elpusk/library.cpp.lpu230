#pragma once

#include "IniItemLoader.h"
#include <memory>
#include <list>

class CLoadIni
{
public:
	typedef	 tr1::shared_ptr<CIniItemLoader>		ItemType;
	typedef	list< CLoadIni::ItemType >				listType;

public:
	CLoadIni(void);
	virtual ~CLoadIni(void);

	virtual bool Load( const _tstring sInifileName ) = 0;
	void Unload();
	virtual void DisplayItems();

	const listType & GetItemList()
	{
		return m_listItem;
	}


protected:
	listType m_listItem;
	bool m_bLoaded;

private:

};

