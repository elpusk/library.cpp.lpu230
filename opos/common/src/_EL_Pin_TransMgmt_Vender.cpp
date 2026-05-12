// EL_Pin_TransMgmt_Vender.cpp: implementation of the CEL_Pin_TransMgmt_Vender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EL_Pin_TransMgmt_Vender.h"
#include "EL_Pin_ReCode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//Constants
#define	CHK_LOOP_DEFAULT_COUNTER			14
#define	DUMMY_PASS_LOOP_DEFAULT_COUNTER		2

#define	MAX_IDLE_CHECK_CNT					15
#define	CMD_HEADER_SIZE						10
#define	DEF_CMD_BUF_SIZE					64	//default command buffer size
#define	DEF_READ_BUF_SIZE					64	//default reading buffer size

#define	DATA_FIELD_SIZE_IN_SEC_PACKET		64	//the data field size on the seocnd or later packet
#define	DATA_FIELD_SIZE_IN_FIR_PACKET		54	//the data field size on the first packet

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEL_Pin_TransMgmt_Vender::CEL_Pin_TransMgmt_Vender()
{
	//ini
	m_bIsIng=FALSE;
}

CEL_Pin_TransMgmt_Vender::~CEL_Pin_TransMgmt_Vender()
{
}

void CEL_Pin_TransMgmt_Vender::Initial()//equl to constructure.
{
	m_bIsIng=FALSE;
}


//from PC to device.
//bPair is true - transaction pair Out/in
//pData includes header.......
DWORD CEL_Pin_TransMgmt_Vender::OutTransaction( 
											   enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/,
											   unsigned char *pData/*=NULL*/,
											   int *pnSize/*=NULL*/
											   )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	static unsigned long ulCntPack=0;
	static unsigned long ulRemainderFieldSize=0;
	unsigned long ulRemainder;
	static unsigned char *pCurData=NULL;
	int nSize=0;
	BOOL bResult=TRUE;
	DWORD dwReadSize=0;
	unsigned long uSecLen=0;
	//
	if( pData!=NULL && pnSize!=NULL ){//request starting
		if( m_bIsIng ){//failure of starting transaction.
			dwResult=KLP110UF_E_INVAILD_PARA;
			return dwResult;
		}
	}

	if( !m_bIsIng ){//the first phase of transaction.......
		if( *pnSize<=0 ){
			dwResult=KLP110UF_E_INVAILD_PARA;
			return dwResult;
		}

		if( m_bCurFileMode ){//for file transfer mode.......
			if( m_hDataFile==NULL ){
				dwResult=KLP110UF_E_INVAILD_PARA;
				return dwResult;
			}

			//In file trnasfer mode,size must be the size of header
			if( *pnSize!=sizeof(EL_PACKET_H64) ){
				dwResult=KLP110UF_E_INVAILD_PARA;
				return dwResult;
			}
			
			SetFilePointer( m_hDataFile,0,NULL,FILE_BEGIN ); 
		}

		//Free pre-allocation memory.
		if( m_pTransBuffer )
			free( m_pTransBuffer );
		//allication buffer memory for OUT-data
		m_pTransBuffer=(unsigned char*)malloc(*pnSize);
		if( m_pTransBuffer==NULL )
			return KLP110UF_E_UNKOWN;
		
		//save OUT-data to buffer.
		memcpy( m_pTransBuffer,pData,*pnSize );

		m_bIsIng=TRUE;

		//copy header of packet
		memset( &m_CurHeader,0x00,sizeof(EL_PACKET_H64) );
		memcpy( &m_CurHeader,pData,CMD_HEADER_SIZE );

		//setting loop counter
		if( m_CurHeader.ulLength>DATA_FIELD_SIZE_IN_FIR_PACKET ){
			uSecLen=m_CurHeader.ulLength-DATA_FIELD_SIZE_IN_FIR_PACKET;
			ulCntPack=1;
			//
			ulCntPack+=(unsigned long)(uSecLen/DATA_FIELD_SIZE_IN_SEC_PACKET);
			ulRemainder=uSecLen%DATA_FIELD_SIZE_IN_SEC_PACKET;
		}
		else{
			ulCntPack=0;
			ulRemainder=m_CurHeader.ulLength;
		}
		if( ulRemainder!=0 )
			ulCntPack++;

		ulRemainderFieldSize=m_CurHeader.ulLength;

		TRACE(_T("=========================\n"));
		TRACE(_T("ulCntPack=%d\n"),ulCntPack);
		TRACE(_T("ulRemainder=%d\n"),ulRemainder);
		TRACE(_T("ulRemainderFieldSize=%d\n"),ulRemainderFieldSize);

		if( m_bCurFileMode )
			pCurData=NULL;//use file-pointer on File transfer mode.......
		else{
			if( ulRemainderFieldSize>0 )
				pCurData=&(m_pTransBuffer[CMD_HEADER_SIZE]);
			else
				pCurData=NULL;
		}

		//the first phase of transaction.......

		//reset current packet.......
		memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
		//copy header
		memcpy( &m_CurOutPacket,&m_CurHeader,CMD_HEADER_SIZE );

		//copy data field
		if( m_bCurFileMode ){//reading file to buffer
			bResult=ReadFile( 
				m_hDataFile,//file handle
				&m_CurOutPacket.header.cDataStart,// data buffer
				DATA_FIELD_SIZE_IN_FIR_PACKET,// number of bytes to read
				&dwReadSize,// number of bytes read
				NULL
				);
			if( !bResult ){//stop transaction by readfile error
				m_bIsIng=FALSE;
				if( m_pTransBuffer ){
					free(m_pTransBuffer);
					m_pTransBuffer=NULL;
				}
				dwResult=KLP110UF_E_UNKOWN;
				return dwResult;
			}
		}
		else
			memcpy( &m_CurOutPacket.header.cDataStart,pCurData,DATA_FIELD_SIZE_IN_FIR_PACKET );

		ulRemainderFieldSize=ulRemainderFieldSize-DATA_FIELD_SIZE_IN_FIR_PACKET;

		if( !m_bCurFileMode )
			pCurData=pCurData+DATA_FIELD_SIZE_IN_FIR_PACKET;


	}
	else{//the middle phase of transaction...... 
		//None processing.......

		if( ulRemainderFieldSize>DATA_FIELD_SIZE_IN_SEC_PACKET ){
			//reset current packet.......
			memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );

			//copy data field
			if( m_bCurFileMode ){//reading file to buffer
				bResult=ReadFile( 
					m_hDataFile,//file handle
					&m_CurOutPacket,// data buffer
					DATA_FIELD_SIZE_IN_SEC_PACKET,// number of bytes to read
					&dwReadSize,// number of bytes read
					NULL
					);
				if( !bResult ){//stop transaction by readfile error
					m_bIsIng=FALSE;
					if( m_pTransBuffer ){
						free(m_pTransBuffer);
						m_pTransBuffer=NULL;
					}
					dwResult=KLP110UF_E_UNKOWN;
					return dwResult;
				}
			}
			else
				memcpy( &m_CurOutPacket,pCurData,DATA_FIELD_SIZE_IN_SEC_PACKET );

			ulRemainderFieldSize=ulRemainderFieldSize-DATA_FIELD_SIZE_IN_SEC_PACKET;

			if( !m_bCurFileMode )
				pCurData=pCurData+DATA_FIELD_SIZE_IN_SEC_PACKET;
		}
		else{//last phase
			memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );

			if( ulRemainderFieldSize>0 ){
				if( m_bCurFileMode ){
					//reading file to buffer
					bResult=ReadFile( 
						m_hDataFile,//file handle
						&m_CurOutPacket,// data buffer
						ulRemainderFieldSize,// number of bytes to read
						&dwReadSize,// number of bytes read
						NULL
						);
					if( !bResult ){//stop transaction by readfile error
						m_bIsIng=FALSE;
						if( m_pTransBuffer ){
							free(m_pTransBuffer);
							m_pTransBuffer=NULL;
						}
						dwResult=KLP110UF_E_UNKOWN;
						return dwResult;
					}
				}
				else
					memcpy( &m_CurOutPacket,pCurData,ulRemainderFieldSize );
			}

			//reset control variables
			pCurData=0;	ulRemainderFieldSize=0;

			if( m_pTransBuffer ){
				free(m_pTransBuffer);
				m_pTransBuffer=NULL;
			}
		}
	}

	nSize=sizeof(EL_PACKET64);
	dwResult=Write( (unsigned char*)&m_CurOutPacket,&nSize );
	if( dwResult==KLP110UF_S_SUCCESS ){//write success
		
		if( ulRemainderFieldSize>0 )
			dwResult=KLP110UF_W_MORE_PROCESS;//request more processing
		else{//write transaction complete......

			m_bIsIng=FALSE;//indicates complete of OUT-phase

			if( m_pTransBuffer ){
				free(m_pTransBuffer);
				m_pTransBuffer=NULL;
			}

			if( TransType==PIN_TRANS_TYPE_S_OUT ){//single transaction type
				//may be chek idle response
				dwResult=OutTransChk( TransType );//single type transaction
			}
		}
	}
	else{//transaction stopped by write error
		m_bIsIng=FALSE;
		if( m_pTransBuffer ){
			free(m_pTransBuffer);
			m_pTransBuffer=NULL;
		}
	}
	//
	return dwResult;
}

//check phase response on out transaction
//bSingle is true - single transaction type.
DWORD CEL_Pin_TransMgmt_Vender::OutTransChk( enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )
{
	return CheckIdleResponse();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//from Device to PC.
//IN  : *pnSize - pData buffer size
//OUT : *pnSize - data size of pData buffer 
//OUT : pData - including packet header.
DWORD CEL_Pin_TransMgmt_Vender::InTransaction( unsigned char *pData/*=NULL*/,int *pnSize/*=NULL*/ )
{
	static unsigned char *pCurData=NULL;
	static unsigned long ulRemainderFieldSize=0;
	static enum enumV_HOST_IN_status CurStatus=V_HINS_GETPIN_RESPONSE;
	static EL_PACKET_H64 FirstHeader;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	enum enumV_HOST_IN_pin_event event=V_HIN_UNKNOWN;
	BYTE t=0;
	BOOL bCleanup=FALSE;
	//
	if( pData==NULL || pnSize==NULL ){
		dwResult=KLP110UF_E_INVAILD_PARA;
		return dwResult;
	}
	//get response
	memset( &m_CurInPacket,0x00,sizeof(EL_PACKET64) );
	nSize=sizeof(EL_PACKET64);
	dwResult=Read( (unsigned char*)&m_CurInPacket,&nSize );
	if( dwResult!=KLP110UF_S_SUCCESS ){//Error IO
		if( m_pTransBuffer )
			free( m_pTransBuffer );
		//
		m_pTransBuffer=NULL;
		ulRemainderFieldSize=0;
		CurStatus=V_HINS_GETPIN_RESPONSE;
		m_bIsIng=FALSE;
		return dwResult;
	}
	
	//save current header from packet
	memcpy( &m_CurHeader,&m_CurInPacket,CMD_HEADER_SIZE );

	//save header for passing dummy
	memcpy( &m_PreInHeader,&m_CurHeader,CMD_HEADER_SIZE);

	t=m_CurHeader.cType;

	event=GetPinEvent_inPhase( &m_CurHeader,CurStatus );
	//
	switch( CurStatus ){
		case V_HINS_STATUS_IDLE://check only command Queue
		case V_HINS_STATUS_CHECK_LINE://Check command Queue and USB line
			//check event
			switch(event){
				case V_HIN_IDLE://idle
				case V_HIN_PIN://pin response
				case V_HIN_ACCOUNT://account response
				case V_HIN_ICC_ST://ICC in/out response
				case V_HIN_DIS_MSG://display message response
				case V_HIN_VER://version response
				default://V_HIN_UNKNOWN://not ready any event
					dwResult=KLP110UF_S_SUCCESS;
					CurStatus=V_HINS_GETPIN_RESPONSE;
					bCleanup=TRUE;
					break;
			}//end switch
			break;
		case V_HINS_GETPIN_RESPONSE://check command Queue,GetPin,GetPinEx and GetPinExAuto response check
			//check event
			switch(event){
				case V_HIN_IDLE://idle
					dwResult=KLP110UF_S_SUCCESS;
					CurStatus=V_HINS_GETPIN_RESPONSE;
					bCleanup=TRUE;
					break;
				case V_HIN_PIN://pin response
				case V_HIN_ACCOUNT://account response
				case V_HIN_ICC_ST://ICC in/out response
				case V_HIN_VER://version response

					if( m_bIsIng ){
						//the second or later response.......
					
						if( ulRemainderFieldSize>DATA_FIELD_SIZE_IN_SEC_PACKET ){
							pCurData=pCurData+DATA_FIELD_SIZE_IN_SEC_PACKET;
							ulRemainderFieldSize=ulRemainderFieldSize-DATA_FIELD_SIZE_IN_SEC_PACKET;
							dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
							CurStatus=V_HINS_GETPIN_RESPONSE;

							memcpy( pCurData,&m_CurInPacket,DATA_FIELD_SIZE_IN_SEC_PACKET);
						}
						else{//last....... packet
							//
							memcpy( pCurData,&m_CurInPacket,ulRemainderFieldSize);
							pCurData=NULL;
							ulRemainderFieldSize=0;
							dwResult=KLP110UF_S_SUCCESS;
							CurStatus=V_HINS_GETPIN_RESPONSE;
							m_bIsIng=FALSE;
							bCleanup=TRUE;

							memcpy( pData,m_pTransBuffer,DEF_CMD_BUF_SIZE+FirstHeader.ulLength );
							*pnSize=DEF_CMD_BUF_SIZE+FirstHeader.ulLength;
						}
					}
					else{//the first response
						m_bIsIng=TRUE;//indicates. 

						//memory allocation for receving response.
						m_pTransBuffer=(unsigned char*)malloc(m_CurHeader.ulLength+DEF_CMD_BUF_SIZE);
						if( m_pTransBuffer==NULL ){//memory allocation error
							CancelTransaction();//cancel transaction
							m_bIsIng=FALSE;
							dwResult=KLP110UF_E_UNKOWN;
							bCleanup=TRUE;
							break;//exit switch
						}
					
						memcpy( m_pTransBuffer,&m_CurInPacket,sizeof(EL_PACKET64));

						if( m_CurHeader.ulLength>DATA_FIELD_SIZE_IN_FIR_PACKET ){
							pCurData=m_pTransBuffer+DEF_CMD_BUF_SIZE+DATA_FIELD_SIZE_IN_FIR_PACKET;
							ulRemainderFieldSize=m_CurHeader.ulLength-DATA_FIELD_SIZE_IN_FIR_PACKET;
							dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
							CurStatus=V_HINS_GETPIN_RESPONSE;

							memcpy( &FirstHeader,&m_CurHeader,sizeof(EL_PACKET_H64) );
						}
						else{//last....... one packet
							pCurData=NULL;
							ulRemainderFieldSize=0;
							dwResult=KLP110UF_S_SUCCESS;
							CurStatus=V_HINS_GETPIN_RESPONSE;
							m_bIsIng=FALSE;
							bCleanup=TRUE;
						}
					}
					break;
				case V_HIN_DIS_MSG://display message response
					break;
				default://V_HIN_UNKNOWN://not ready any event
					break;
			}//end switch
			break;
		case V_HINS_STATUS_RECOVER_LINE://Recover usb line
		case V_HINS_STATUS_RECOVER_LINE_RETRY://Recover usb line and retry
		default:
			dwResult=KLP110UF_E_UNKOWN;
			bCleanup=TRUE;
			CurStatus=V_HINS_GETPIN_RESPONSE;
			break;
	}//end swith


	if( bCleanup ){//cleanup processing

		if( m_pTransBuffer )
			free( m_pTransBuffer );
		//
		m_pTransBuffer=NULL;
		ulRemainderFieldSize=0;
		CurStatus=V_HINS_GETPIN_RESPONSE;
		m_bIsIng=FALSE;
	}

	return dwResult;
}


DWORD CEL_Pin_TransMgmt_Vender::InTransChk()
{
	return CheckIdleResponse();
}

//cancel Current transaction
DWORD CEL_Pin_TransMgmt_Vender::CancelTransaction()
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE sCmd[DEF_CMD_BUF_SIZE];
	int nTX;
	//
	memset( &sCmd,0x00,DEF_CMD_BUF_SIZE);
	sCmd[0]='X';//cancel commnad
	//
	nTX=CMD_HEADER_SIZE;

	dwResult=Write( (unsigned char*)&sCmd,&nTX );//send cancel transaction
	if( dwResult!=KLP110UF_S_SUCCESS )
		return dwResult;

	//Pass dummy response.
	dwResult=PassDummyResponse();
	if( dwResult==KLP110UF_S_SUCCESS )
		m_bIsIng=FALSE;//indicates Stop-transaction

	return dwResult;
}

DWORD CEL_Pin_TransMgmt_Vender::PassDummyResponse()
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	BYTE buf[DEF_READ_BUF_SIZE];
	int nRX;
	int i;
	HANDLE hDelayEvent;

	hDelayEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
	if( hDelayEvent==NULL )
		return KLP110UF_E_UNKNOWN;

	memset( &buf,0x00,sizeof(DEF_READ_BUF_SIZE) );
	nRX=DEF_READ_BUF_SIZE;

	for( i=0; i<DUMMY_PASS_LOOP_DEFAULT_COUNTER; i++ ){//dummy pass 3 times

		//delay
		WaitForSingleObject( hDelayEvent,20 );
		ResetEvent( hDelayEvent );	//clear event.

		dwResult=Read( (unsigned char*)buf,&nRX );//get response
		if( dwResult!=KLP110UF_S_SUCCESS ){
			CloseHandle( hDelayEvent );
			return dwResult;
		}
		else//save header for passing dummy
			memcpy( &m_PreInHeader,buf,DEF_READ_BUF_SIZE );

	}//end for

	CloseHandle( hDelayEvent );
	return dwResult;
}

enum enumV_HOST_IN_pin_event CEL_Pin_TransMgmt_Vender::GetPinEvent_inPhase( PEL_PACKET_H64 pHeader,enum enumV_HOST_IN_status CurStatus )
{
	enum enumV_HOST_IN_pin_event event=V_HIN_UNKNOWN;
	BYTE t=0;
	BYTE T=0;
	BYTE s=0;
	ULONG c=0;
	BYTE m=0;
	//
	if( pHeader==NULL )
		return event;
	//get type
	t=pHeader->cType;

	switch( t ){//check type of response
		case V_HIN_IDLE://V_HIN_UNKNOWN==V_HIN_IDLE
			event=(enumV_HOST_IN_pin_event)t;
			break;
		case V_HIN_PIN://pin response
			event=(enumV_HOST_IN_pin_event)t;
			break;
		case V_HIN_ACCOUNT://account response
			event=(enumV_HOST_IN_pin_event)t;
			break;
		case V_HIN_ICC_ST://ICC in/out response
			event=(enumV_HOST_IN_pin_event)t;
			break;
		case V_HIN_DIS_MSG://display message response
			event=(enumV_HOST_IN_pin_event)t;
			break;
		case V_HIN_VER://version response
			event=(enumV_HOST_IN_pin_event)t;
			break;
		default:
			event=V_HIN_IDLE;
			break;
	}//end switch
	//
	return event;
}

DWORD CEL_Pin_TransMgmt_Vender::CheckIdleResponse()
{
	EL_PACKET64 InPacket;
	int nSize;
	BOOL bIng=TRUE;
	int nMaxCnt=MAX_IDLE_CHECK_CNT;
	DWORD dwResult;

	memset( &InPacket,0x00,sizeof(EL_PACKET64) );

	do{
		nSize=sizeof(EL_PACKET64);
		dwResult=Read( (unsigned char*)&InPacket,&nSize );
		if( dwResult!=KLP110UF_S_SUCCESS )//Error IO
			return dwResult;

		if( InPacket.header.cType==0 &&
			InPacket.header.ulChain==0 && 
			InPacket.header.cSeq==0 ){

			memcpy( &m_PreInHeader,&(InPacket.header),sizeof(EL_PACKET_H64));

			bIng=FALSE;//OK idle response
		}
		else{
			memcpy( &m_PreInHeader,&(InPacket.header),sizeof(EL_PACKET_H64));
			nMaxCnt--;
		}

	}while( bIng && nMaxCnt>0 );

	return KLP110UF_S_SUCCESS;//check to idle processing ???????

}

