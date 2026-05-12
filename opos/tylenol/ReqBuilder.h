#pragma once

#include "info_sys.h"
#include <stddef.h>
#include "info_sys_cnst.h"
#include <vector>
#include "Windows.h"
#include "LoadMsrIni.h"
#include <map>
#include "cmdset.h"

// class for building request

using namespace std;

class CReqBuilder
{
public:
	typedef	vector<BYTE>		BUFFERTYPE;

	enum
	{	
		def_buffer_size	=	1024
	};

	struct Value{
		_tstring sIniKey;

		UINT32 dwOffset;
		UINT32 dwSize;
	};

	const static Value NullValue;

#define	OFFSETOF(m)		offsetof(SYSINFO,m)
#define	SIZEOF(m)		sizeofstructmember(SYSINFO,m)

	static const Value & GetValue( const _tstring & sKey );

public:
	CReqBuilder(void);
	virtual ~CReqBuilder(void);

	virtual bool Build( const CLoadMsrIni::ItemType & Item, bool bSet =true );

	BUFFERTYPE & GetRxBuffer();
	BUFFERTYPE & GetTxBuffer();

	UINT32 GetRxSize(){	return m_nRx;	}
	UINT32 GetTxSize(){	return m_nTx;	}

	bool IsSet(){	return m_bSet;	}

	const MSR_HOST_PACKET & GetRequest(){		return m_Req;	}

	PMSR_HOST_PACKET GetResponseBuffer(){		return &m_Resp;	}

	void AnalysisResponse();

	bool IsGoodResponse();

	void DisplayResponse();

protected:

	//CReqBuilder( const CReqBuilder & );

	void SetRx( UINT32 nRx, UINT32 nOffsetRx ){	m_nRx = nRx;	m_nOffsetRx = nOffsetRx;	}
	void SetTx( UINT32 nTx, UINT32 nOffsetTx ){	m_nTx = nTx;	m_nOffsetTx = nOffsetTx;	}

	const _tstring & GenerateKey( const _tstring & sSectionName, const _tstring & sKeyName ); 

	static map<UINT32, Value> & GetMappingMap();

	static CReqBuilder::Value & SetValue( Value & value, const _tstring & sIniKey, UINT32 dwOffset, UINT32 dwSize );

	bool IsAllZero( const CIniItemLoader::array_type & v );
private:
	////////////////////////////////////////
	//variables
public:
protected:

	MSR_HOST_PACKET m_Req;
	MSR_HOST_PACKET m_Resp;

	//BUFFERTYPE m_vTx;
	//BUFFERTYPE m_vRx;

	UINT32 m_nTx;
	UINT32 m_nRx;
	UINT32 m_nOffsetTx, m_nOffsetRx;

	bool m_bSet;	//set or get request.
private:
};

class CReqBuilderOld :public CReqBuilder
{
public:
	#define	OFFSETOF_OLD(m)		offsetof(SYSINFO_OLD,m)
	#define	SIZEOF_OLD(m)		sizeofstructmember(SYSINFO_OLD,m)

public:
	CReqBuilderOld(void):CReqBuilder(){}

	virtual ~CReqBuilderOld(void){}

	virtual bool Build( const CLoadMsrIni::ItemType & Item, bool bSet =true );
	static const Value & GetValueOld( const _tstring & sKey );
protected:
	static map<UINT32, Value> & GetMappingMapOld();
};

class CReqBuilderNew :public CReqBuilder
{
public:
	CReqBuilderNew(void):CReqBuilder(){}

	virtual ~CReqBuilderNew(void){}


};

class CReqBuilderStd :public CReqBuilder
{
public:
	#define	OFFSETOF_STD(m)		offsetof(SYSINFO_STD,m)
	#define	SIZEOF_STD(m)		sizeofstructmember(SYSINFO_STD,m)

public:
	CReqBuilderStd(void):CReqBuilder(){}

	virtual ~CReqBuilderStd(void){}

	virtual bool Build( const CLoadMsrIni::ItemType & Item, bool bSet =true );
	static const Value & GetValueStd( const _tstring & sKey );
protected:
	static map<UINT32, Value> & GetMappingMapStd();

};

