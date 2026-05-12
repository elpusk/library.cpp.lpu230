#pragma once

#include "TTR_tchar.h"
#include <vector>

using namespace std;

class CIniItemLoader
{
public:
	typedef	std::vector<unsigned char>	array_type;

public:
	~CIniItemLoader(void);
	
	CIniItemLoader( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, INT32 nDefault );
	CIniItemLoader( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, const _tstring & sDefault );
	CIniItemLoader(void);
	CIniItemLoader(const CIniItemLoader & Item );

	bool Load( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, bool bIsStringType = true );

	INT32 GetIntKey(){	return m_nKey;	}
	const _tstring & GetStringKey(){	return m_sKey;	}
	void GetStringKey( _tstring & sKey ){	sKey = m_sKey;	}
	const array_type & GetArrayKey(){	return m_vKey;	}

	const _tstring & GetIniFileName(){	return m_sIniFileName;	}
	const _tstring & GetSectionName(){	return m_sSectionName;	}
	const _tstring & GetKeyName(){		return m_sKeyName;	}

	bool IsStringType(){		return m_bIsStringType;	}
	bool IsLoadSuccess(){	return m_bIsSuccess;	}

	CIniItemLoader & operator= ( const CIniItemLoader & rhs );

	bool IsArray(){	return !m_vKey.empty();	}

	void PrintArray();


private:
	CIniItemLoader( const _tstring & sSectionName, const _tstring & sKeyName,  const _tstring & sIniFileName, bool bIsStringType = true );

	
	bool SetArrayKey();

	enum enumFormat{
		ef_decimal,
		ef_heximal,
		ef_ascii
	};

	bool IsAvaliedFormat( const _tstring & sToken );

	INT32 GetByteFromToken( const _tstring & sToken );

private:
	_tstring m_sIniFileName;
	_tstring m_sSectionName;
	_tstring m_sKeyName;
	INT32 m_nKey;
	_tstring m_sKey;
	bool m_bIsSuccess;
	bool m_bIsStringType;

	array_type m_vKey;

	enumFormat m_TokenFormat;
};

