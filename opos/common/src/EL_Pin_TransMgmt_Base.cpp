// CEL_Pin_TransMgmt_Base.cpp: implementation of the CEL_Pin_TransMgmt_Base class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EL_Pin_TransMgmt_Base.h"
#include "EL_Pin_ReCode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEL_Pin_TransMgmt_Base::CEL_Pin_TransMgmt_Base()
{
	//ini
	//initial Base pinpad IO object......
	m_PinBaseIO.Initial();
	
	m_cCurTransaction=0;//the current transaction

	memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
	memset( &m_CurInPacket,0x00,sizeof(EL_PACKET64) );
	memset( &m_CurHeader,0x00,sizeof(EL_PACKET_H64) );
	memset( &m_PreInHeader,0x00,sizeof(EL_PACKET_H64) );

	m_pTransBuffer=NULL;

	m_bCurFileMode=FALSE;
	m_hDataFile=NULL;

}

CEL_Pin_TransMgmt_Base::~CEL_Pin_TransMgmt_Base()
{
	if( m_pTransBuffer )
		free( m_pTransBuffer );
}

void CEL_Pin_TransMgmt_Base::Initial()//equl to constructure.
{
	//initial Base pinpad IO object......
	m_PinBaseIO.Initial();
	
	m_cCurTransaction=0;//the current transaction

	memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
	memset( &m_CurInPacket,0x00,sizeof(EL_PACKET64) );
	memset( &m_CurHeader,0x00,sizeof(EL_PACKET_H64) );
	memset( &m_PreInHeader,0x00,sizeof(EL_PACKET_H64) );

	m_pTransBuffer=NULL;

	m_bCurFileMode=FALSE;
	m_hDataFile=NULL;
}

void CEL_Pin_TransMgmt_Base::SetTransMode( BOOL bIsFileMode/*=FALSE*/,HANDLE hDataFile/*=NULL*/ )
{	
	m_bCurFileMode=bIsFileMode;
	m_hDataFile=hDataFile;
}

