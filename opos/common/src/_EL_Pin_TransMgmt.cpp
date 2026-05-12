// EL_Pin_TansMgmt.cpp: implementation of the CEL_Pin_TransMgmt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EL_Pin_TransMgmt.h"
#include "EL_Pin_ReCode.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//Constants
#define	CHK_LOOP_DEFAULT_COUNTER			14
#define	DUMMY_PASS_LOOP_DEFAULT_COUNTER		4

#define	MAX_IDLE_CHECK_CNT					15

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEL_Pin_TransMgmt::CEL_Pin_TransMgmt()
{
	//ini
	m_bIsIng=FALSE;
}

CEL_Pin_TransMgmt::~CEL_Pin_TransMgmt()
{
}

void CEL_Pin_TransMgmt::Initial()//equl to constructure.
{
	m_bIsIng=FALSE;
}


//from PC to device.
//bPair is true - transaction pair Out/in
DWORD CEL_Pin_TransMgmt::OutTransaction( enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/,unsigned char *pData/*=NULL*/,int *pnSize/*=NULL*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	static unsigned long ulCntPack=0;
	static unsigned long ulRemainderFieldSize=0;
	unsigned long ulRemainder;
	static unsigned char *pCurData=NULL;
	int nSize=0;
	BOOL bResult=TRUE;
	DWORD dwReadSize=0;
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

		//save data
		if( m_pTransBuffer )
			free( m_pTransBuffer );
		m_pTransBuffer=(unsigned char*)malloc(*pnSize);
		if( m_pTransBuffer==NULL )
			return KLP110UF_E_UNKOWN;
		//
		memcpy( m_pTransBuffer,pData,*pnSize );

		m_bIsIng=TRUE;
		//copy header of packet
		memset( &m_CurHeader,0x00,sizeof(EL_PACKET_H64) );
		memcpy( &m_CurHeader,pData,sizeof(EL_PACKET_H64) );
		m_CurHeader.cSeq=GetIncTransaction();//generates transaction number
		m_CurHeader.ulChain=0;//reset chain

		ulCntPack=(unsigned long)(m_CurHeader.ulLength/DATA_FIELD_SIZE_IN_PACKET);
		ulRemainder=m_CurHeader.ulLength%DATA_FIELD_SIZE_IN_PACKET;
		if( ulRemainder!=0 )
			ulCntPack++;

		ulRemainderFieldSize=m_CurHeader.ulLength;

		TRACE(_T("=========================\n"));
		TRACE(_T("ulCntPack=%d\n"),ulCntPack);
		TRACE(_T("ulRemainder=%d\n"),ulRemainder);
		TRACE(_T("ulRemainderFieldSize=%d\n"),ulRemainderFieldSize);


		if( m_bCurFileMode )
			pCurData=NULL;
		else{
			if( ulRemainderFieldSize>0 )
				pCurData=&(m_pTransBuffer[sizeof(EL_PACKET_H64)]);
			else
				pCurData=NULL;
		}
	}
	else{//the middle phase of transaction...... 
		//None processing.......
	}

	if( ulCntPack>0 ){
		m_CurHeader.ulChain++;
		if( m_CurHeader.ulChain==ulCntPack ){
			//the last phase of transaction
			m_CurHeader.ulChain=0;//indicate last phase
		}
	}

	if( ulRemainderFieldSize>DATA_FIELD_SIZE_IN_PACKET ){
		//reset current packet.......
		memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
		//copy header
		memcpy( &m_CurOutPacket,&m_CurHeader,sizeof(EL_PACKET_H64) );
		//copy data field
		if( m_bCurFileMode ){//reading file to buffer
			bResult=ReadFile( 
				m_hDataFile,//file handle
				m_CurOutPacket.sData,// data buffer
				DATA_FIELD_SIZE_IN_PACKET,// number of bytes to read
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
			memcpy( m_CurOutPacket.sData,pCurData,DATA_FIELD_SIZE_IN_PACKET );

		ulRemainderFieldSize=ulRemainderFieldSize-DATA_FIELD_SIZE_IN_PACKET;

		if( !m_bCurFileMode )
			pCurData=pCurData+DATA_FIELD_SIZE_IN_PACKET;
	}
	else{//last phase
		memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
		memcpy( &m_CurOutPacket,&m_CurHeader,sizeof(EL_PACKET_H64) );
		if( ulRemainderFieldSize>0 ){
			if( m_bCurFileMode ){
				//reading file to buffer
				bResult=ReadFile( 
					m_hDataFile,//file handle
					m_CurOutPacket.sData,// data buffer
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
				memcpy( m_CurOutPacket.sData,pCurData,ulRemainderFieldSize );
		}

		//reset control variables
		pCurData=0;	ulRemainderFieldSize=0;

		if( m_pTransBuffer ){
			free(m_pTransBuffer);
			m_pTransBuffer=NULL;
		}
	}

	nSize=sizeof(EL_PACKET64);
	dwResult=Write( (unsigned char*)&m_CurOutPacket,&nSize );
	if( dwResult==KLP110UF_S_SUCCESS ){//write success
		dwResult=OutTransChk();//check phase complete
		if( dwResult==KLP110UF_S_SUCCESS ){
			if( ulRemainderFieldSize>0 )
				dwResult=KLP110UF_W_MORE_PROCESS;//request more processing
			else{//write transaction complete......
				m_bIsIng=FALSE;
				if( m_pTransBuffer ){
					free(m_pTransBuffer);
					m_pTransBuffer=NULL;
				}

				if( TransType==PIN_TRANS_TYPE_S_OUT ){//single transaction type
					dwResult=OutTransChk( TransType );//single type transaction
				}
			}
		}
		else{//transaction stopped by phase complete checking error
			m_bIsIng=FALSE;
			if( m_pTransBuffer ){
				free(m_pTransBuffer);
				m_pTransBuffer=NULL;
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
DWORD CEL_Pin_TransMgmt::OutTransChk( enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	int nSize;
	int i;
	//
	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		//single type transaction last control
		memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
		nSize=sizeof(EL_PACKET64);
		dwResult=Write( (unsigned char*)&m_CurOutPacket,&nSize );
		if( dwResult!=KLP110UF_S_SUCCESS )
			return dwResult;
	}

	for( i=0; i<CHK_LOOP_DEFAULT_COUNTER; i++ ){
		memset( &m_CurInPacket,0x00,sizeof(EL_PACKET64) );
		nSize=sizeof(EL_PACKET64);
		dwResult=Read( (unsigned char*)&m_CurInPacket,&nSize );
		if( dwResult==KLP110UF_S_SUCCESS ){
			if(m_CurInPacket.header.cType==m_PreInHeader.cType && 
				m_CurInPacket.header.cSeq==m_PreInHeader.cSeq &&
				m_CurInPacket.header.ulChain==m_PreInHeader.ulChain ){
				//pass previous response by driver buffering
			}
			else if( m_CurInPacket.header.cType==0 || 
				m_CurInPacket.header.cSeq==m_CurOutPacket.header.cSeq ||
				m_CurInPacket.header.ulChain==m_CurOutPacket.header.ulChain ){
				//save header for passing dummy
				memcpy( &m_PreInHeader,&(m_CurInPacket.header),sizeof(EL_PACKET_H64));
				return dwResult;//tx phase complete.......
			}
			else{//Check Error response(including cancel)
				//save header for passing dummy
				memcpy( &m_PreInHeader,&(m_CurInPacket.header),sizeof(EL_PACKET_H64));

				//send idle-command for chaning status of pinpad.
				dwResult=Write();
				if( dwResult==KLP110UF_S_SUCCESS ){
					dwResult=PassDummyResponse();
					if( dwResult==KLP110UF_S_SUCCESS ){
						dwResult=KLP110UF_E_INVAILD_RESP;
						return dwResult;
					}
					else
						return dwResult;
				}
				else
					return dwResult;
			}
		}
		else{//Error read
			return dwResult;
		}
	}//end for
	//
	dwResult=KLP110UF_E_INVAILD_RESP;
	return dwResult;
}


//from Device to PC.
//IN  : *pnSize - pData buffer size
//OUT : *pnSize - data size of pData buffer 
//OUT : pData - including packet header.
//DWORD CEL_Pin_TransMgmt::InTransaction( unsigned char *pData/*=NULL*/,int *pnSize/*=NULL*/ )
/*
{
	static unsigned char *pCurData=NULL;
	static unsigned long ulRemainderFieldSize=0;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( pData==NULL || pnSize==NULL ){
		dwResult=KLP110UF_E_INVAILD_PARA;
		return dwResult;
	}
	//
	memset( &m_CurInPacket,0x00,sizeof(EL_PACKET64) );
	nSize=sizeof(EL_PACKET64);
	dwResult=Read( (unsigned char*)&m_CurInPacket,&nSize );
	if( dwResult==KLP110UF_S_SUCCESS ){
		//save current header
		memcpy( &m_CurHeader,&m_CurInPacket,sizeof(EL_PACKET_H64) );
		
		if( m_CurInPacket.header.cSeq==0 ){//current response is idle-reponse
			*pnSize=0;//reponse size is zero.
			return dwResult;//complete InTransaction by idle reponse.......
		}
		else{//
			if( m_CurInPacket.header.cSeq!=m_CurOutPacket.header.cSeq ){
				//mismatching IN/OUT pair transaction
				//previous IN/OUT pair transaction may be Error.

				//cancel previous error transaction
				dwResult=CancelTransaction();
				if( dwResult==KLP110UF_S_SUCCESS )
					dwResult=KLP110UF_E_INVAILD_RESP;
				//
				return dwResult;
			}
			else{//IN traction of IN/OUT pair
				if( m_CurInPacket.header.cType==0 ){
					//current device is busy for command processing.
					dwResult=KLP110UF_W_MORE_PROCESS;
					return dwResult;
				}
				else if(m_CurInPacket.header.cType==m_CurOutPacket.header.cType){
					//IN traction of IN/OUT pair general case
					if( m_CurInPacket.header.ulChain==0 ){
						if( !m_bIsIng ){//single packet transaction
							if( m_pTransBuffer )
								free(m_pTransBuffer);
							//
							m_pTransBuffer=(unsigned char*)malloc(m_CurHeader.ulLength+sizeof(EL_PACKET_H64));
							if( m_pTransBuffer==NULL ){//memory allocation error
								CancelTransaction();//cancel transaction
								return KLP110UF_E_UNKOWN;
							}

							memcpy( m_pTransBuffer,&m_CurHeader,sizeof(EL_PACKET_H64) );
							memcpy( &(m_pTransBuffer[sizeof(EL_PACKET_H64)]),m_CurInPacket.sData,m_CurHeader.ulLength );
						}
						else{//copy remainder
							memcpy( pCurData,m_CurInPacket.sData,ulRemainderFieldSize );
						}

						m_bIsIng=FALSE;
						//Last phase of IN transaction
						dwResult=InTransChk();
						if( dwResult==KLP110UF_S_SUCCESS )
							memcpy( pData,m_pTransBuffer,sizeof(EL_PACKET_H64)+m_CurHeader.ulLength );
						//
						free( m_pTransBuffer );
						m_pTransBuffer=NULL;
						ulRemainderFieldSize=0;
						return dwResult;
					}
					else if( m_CurInPacket.header.ulChain==1 ){
						//the first phase of transaction
						m_bIsIng=TRUE;
						if( m_pTransBuffer )
							free( m_pTransBuffer );

						m_pTransBuffer=(unsigned char*)malloc(m_CurHeader.ulLength+sizeof(EL_PACKET_H64));
						if( m_pTransBuffer==NULL ){//memory allocation error
							CancelTransaction();//cancel transaction
							m_bIsIng=FALSE;
							return KLP110UF_E_UNKOWN;
						}

						dwResult=InTransChk();
						if( dwResult==KLP110UF_S_SUCCESS ){//copy data
							memcpy( m_pTransBuffer,&m_CurInPacket,sizeof(EL_PACKET_H64) );
							pCurData=m_pTransBuffer+sizeof(EL_PACKET_H64);
							ulRemainderFieldSize=m_CurHeader.ulLength-DATA_FIELD_SIZE_IN_PACKET;
							dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
						}
						else{//check phase error
							free( m_pTransBuffer );
							m_pTransBuffer=NULL;
							ulRemainderFieldSize=0;
							return dwResult;
						}

					}
					else{//chain number is greater then two.......
						dwResult=InTransChk();
						if( dwResult==KLP110UF_S_SUCCESS ){//copy data
							memcpy( pCurData,m_CurInPacket.sData,DATA_FIELD_SIZE_IN_PACKET );
							pCurData=pCurData+DATA_FIELD_SIZE_IN_PACKET;
							ulRemainderFieldSize=m_CurHeader.ulLength-DATA_FIELD_SIZE_IN_PACKET;
						}
						else{//check phase error
							free( m_pTransBuffer );
							m_pTransBuffer=NULL;
							ulRemainderFieldSize=0;
							return dwResult;
						}
					}
				}
				else{//mismatching IN/OUT pair transaction
					//cancel previous error transaction
					dwResult=CancelTransaction();
					if( dwResult==KLP110UF_S_SUCCESS )
						dwResult=KLP110UF_E_INVAILD_RESP;
					//
					return dwResult;
				}
			}
		}

	}
	else{//Error read
		if( m_pTransBuffer )
			free( m_pTransBuffer );
		//
		m_pTransBuffer=NULL;
		ulRemainderFieldSize=0;
		return dwResult;
	}
	//
	return dwResult;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//from Device to PC.
//IN  : *pnSize - pData buffer size
//OUT : *pnSize - data size of pData buffer 
//OUT : pData - including packet header.
DWORD CEL_Pin_TransMgmt::InTransaction( unsigned char *pData/*=NULL*/,int *pnSize/*=NULL*/ )
{
	static unsigned char *pCurData=NULL;
	static unsigned long ulRemainderFieldSize=0;
	static enum enumHOST_IN_status CurStatus=HINS_S010;
	int nSize=0;
	DWORD dwResult=KLP110UF_S_SUCCESS;
	enum enumHOST_IN_pin_event event=HIN_UNKNOWN;
	BYTE t=0;
	BYTE s=0;
	ULONG c=0;
	BYTE m=0;
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
		CurStatus=HINS_S010;
		return dwResult;
	}
	
	//save current header from packet
	memcpy( &m_CurHeader,&m_CurInPacket,sizeof(EL_PACKET_H64) );
	m=m_CurOutPacket.header.cSeq;

	if( m_PreInHeader.cType==m_CurHeader.cType &&
		m_PreInHeader.cSeq==m_CurHeader.cSeq && 
		m_PreInHeader.ulChain==m_CurHeader.ulChain ){
		//pass previous response
		dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
		return dwResult;
	}

	//save header for passing dummy
	memcpy( &m_PreInHeader,&m_CurHeader,sizeof(EL_PACKET_H64));

	t=m_CurHeader.cType;	s=m_CurHeader.cSeq;	c=m_CurHeader.ulChain;

	event=GetPinEvent_inPhase( &m_CurHeader,CurStatus );
	//
	switch( CurStatus ){
		case HINS_S000://idle
			switch(event){
				case HIN_R008://idle response
					dwResult=KLP110UF_S_SUCCESS;
					CurStatus=HINS_S000;
					bCleanup=TRUE;
					break;
				case HIN_R000://busy pinad
				case HIN_R001://starting response
				case HIN_R002://middle response
				case HIN_R003://last response
				case HIN_R004://unknown starting response
				case HIN_R005://unknown middle response
				case HIN_R006://response of cancel-cmd
				case HIN_R007://unknown idle response
					dwResult=Write();//send idle configuration
					CurStatus=HINS_S000;
					bCleanup=TRUE;
					break;
				default:
					dwResult=KLP110UF_E_UNKOWN;
					bCleanup=TRUE;
					break;
			}//end event switch
			break;
		case HINS_S010://waits
			switch(event){
				case HIN_R000://busy pinad
					dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
					CurStatus=HINS_S010;
					break;
				case HIN_R001://starting response
					//the first phase of transaction
					m_bIsIng=TRUE;
					if( m_pTransBuffer )
						free( m_pTransBuffer );

					m_pTransBuffer=(unsigned char*)malloc(m_CurHeader.ulLength+sizeof(EL_PACKET_H64));
					if( m_pTransBuffer==NULL ){//memory allocation error
						CancelTransaction();//cancel transaction
						m_bIsIng=FALSE;
						dwResult=KLP110UF_E_UNKOWN;
						bCleanup=TRUE;
						break;//exit switch
					}

					dwResult=InTransChk();
					if( dwResult==KLP110UF_S_SUCCESS ){//copy data
						memcpy( m_pTransBuffer,&m_CurInPacket,sizeof(EL_PACKET_H64)+DATA_FIELD_SIZE_IN_PACKET );
						pCurData=m_pTransBuffer+sizeof(EL_PACKET_H64)+DATA_FIELD_SIZE_IN_PACKET;
						ulRemainderFieldSize=m_CurHeader.ulLength-DATA_FIELD_SIZE_IN_PACKET;
						dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
						CurStatus=HINS_S111;
					}
					else{//check phase error
						m_bIsIng=FALSE;
						bCleanup=TRUE;
					}
					break;
				case HIN_R003://last response
					if( !m_bIsIng ){//single packet transaction
						if( m_pTransBuffer )
							free(m_pTransBuffer);
						//
						m_pTransBuffer=(unsigned char*)malloc(m_CurHeader.ulLength+sizeof(EL_PACKET_H64));
						if( m_pTransBuffer==NULL ){//memory allocation error
							m_bIsIng=TRUE;
							CancelTransaction();//cancel transaction
							m_bIsIng=FALSE;
							dwResult=KLP110UF_E_UNKOWN;
							bCleanup=TRUE;
							break;//exit switch
						}
						memcpy( m_pTransBuffer,&m_CurHeader,sizeof(EL_PACKET_H64) );
						memcpy( &(m_pTransBuffer[sizeof(EL_PACKET_H64)]),m_CurInPacket.sData,m_CurHeader.ulLength );
					}
					else{//copy remainder
						memcpy( pCurData,m_CurInPacket.sData,ulRemainderFieldSize );
					}

					m_bIsIng=FALSE;
					//Last phase of IN transaction
					dwResult=InTransChk();
					if( dwResult==KLP110UF_S_SUCCESS ){
						memcpy( pData,m_pTransBuffer,sizeof(EL_PACKET_H64)+m_CurHeader.ulLength );
						*pnSize=sizeof(EL_PACKET_H64)+m_CurHeader.ulLength;
					}
					//
					CurStatus=HINS_S110;
					bCleanup=TRUE;
					break;
				case HIN_R002://middle response
				case HIN_R004://unknown starting response
				case HIN_R005://unknown middle response
				case HIN_R006://response of cancel-cmd
				case HIN_R007://unknown idle response
				case HIN_R008://idle response
					m_bIsIng=TRUE;
					dwResult=CancelTransaction();
					m_bIsIng=FALSE;
					if( dwResult==KLP110UF_S_SUCCESS ){
						Write();//send idle configuration
						dwResult=KLP110UF_E_INVAILD_RESP;
					}
					//
					bCleanup=TRUE;
					break;
				default:
					dwResult=KLP110UF_E_UNKOWN;
					bCleanup=TRUE;
					break;
			}//end event switch
			break;
		case HINS_S111://receiving
			switch(event){
				case HIN_R002://middle response
					dwResult=InTransChk();
					if( dwResult==KLP110UF_S_SUCCESS ){//copy data
						memcpy( pCurData,m_CurInPacket.sData,DATA_FIELD_SIZE_IN_PACKET );
						pCurData=pCurData+DATA_FIELD_SIZE_IN_PACKET;
						ulRemainderFieldSize=ulRemainderFieldSize-DATA_FIELD_SIZE_IN_PACKET;
						dwResult=KLP110UF_W_MORE_PROCESS;//<<<<<
					}
					else{//check phase error
						bCleanup=TRUE;
						m_bIsIng=FALSE;
					}
					break;
				case HIN_R003://last response
					memcpy( pCurData,m_CurInPacket.sData,ulRemainderFieldSize );

					m_bIsIng=FALSE;
					//Last phase of IN transaction
					dwResult=InTransChk();
					if( dwResult==KLP110UF_S_SUCCESS ){
						memcpy( pData,m_pTransBuffer,sizeof(EL_PACKET_H64)+m_CurHeader.ulLength );
						*pnSize=sizeof(EL_PACKET_H64)+m_CurHeader.ulLength;
					}
					//
					CurStatus=HINS_S110;
					bCleanup=TRUE;
					break;
				case HIN_R000://busy pinad
				case HIN_R001://starting response
				case HIN_R004://unknown starting response
				case HIN_R005://unknown middle response
				case HIN_R006://response of cancel-cmd
				case HIN_R007://unknown idle response
				case HIN_R008://idle response
					m_bIsIng=TRUE;
					dwResult=CancelTransaction();
					m_bIsIng=FALSE;
					if( dwResult==KLP110UF_S_SUCCESS ){
						Write();//send idle configuration
						dwResult=KLP110UF_E_INVAILD_RESP;
					}
					//
					bCleanup=TRUE;
					break;
				default:
					dwResult=KLP110UF_E_UNKOWN;
					bCleanup=TRUE;
					break;
			}//end event switch
			break;
		case HINS_S110://last response
			switch(event){
				case HIN_R008://idle response
					dwResult=Write();//send idle configuration
					CurStatus=HINS_S000;
					bCleanup=TRUE;
					break;
				case HIN_R000://busy pinad
				case HIN_R001://starting response
				case HIN_R002://middle response
				case HIN_R003://last response
				case HIN_R004://unknown starting response
				case HIN_R005://unknown middle response
				case HIN_R006://response of cancel-cmd
				case HIN_R007://unknown idle response
					m_bIsIng=TRUE;
					dwResult=CancelTransaction();
					m_bIsIng=FALSE;
					if( dwResult==KLP110UF_S_SUCCESS ){
						Write();//send idle configuration
						dwResult=KLP110UF_E_INVAILD_RESP;
					}
					//
					bCleanup=TRUE;
					break;
				default:
					dwResult=KLP110UF_E_UNKOWN;
					bCleanup=TRUE;
					break;
			}//end event switch
			break;
		case HINS_S100://cancel
			switch(event){
				case HIN_R006://response of cancel-cmd
				case HIN_R000://busy pinad
				case HIN_R001://starting response
				case HIN_R002://middle response
				case HIN_R003://last response
				case HIN_R004://unknown starting response
				case HIN_R005://unknown middle response
				case HIN_R007://unknown idle response
				case HIN_R008://idle response
					dwResult=Write();//send idle configuration
					CurStatus=HINS_S000;
					bCleanup=TRUE;
					break;
				default:
					dwResult=KLP110UF_E_UNKOWN;
					bCleanup=TRUE;
					break;
			}//end event switch
			break;
		default:
			dwResult=KLP110UF_E_UNKOWN;
			bCleanup=TRUE;
			break;
	}//end switch

	if( bCleanup ){//cleanup processing

		if( m_pTransBuffer )
			free( m_pTransBuffer );
		//
		m_pTransBuffer=NULL;
		ulRemainderFieldSize=0;
		CurStatus=HINS_S010;
		m_bIsIng=FALSE;
	}

	return dwResult;
}


DWORD CEL_Pin_TransMgmt::InTransChk()
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	int nSize;
	//
	memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );

	m_CurOutPacket.header.cType=0;
	m_CurOutPacket.header.cSeq=m_CurHeader.cSeq;
	m_CurOutPacket.header.ulChain=m_CurHeader.ulChain;

	nSize=sizeof(EL_PACKET64);
	dwResult=Write( (unsigned char*)&m_CurOutPacket,&nSize );

	if( dwResult==KLP110UF_S_SUCCESS ){
		if( m_CurHeader.ulChain==0 ){
			//the last phase
			//assign idle-response to device.......
			memset( &m_CurOutPacket,0x00,sizeof(EL_PACKET64) );
			nSize=sizeof(EL_PACKET64);
			dwResult=Write( (unsigned char*)&m_CurOutPacket,&nSize );
		}
	}

	return dwResult;
}

//cancel Current transaction
DWORD CEL_Pin_TransMgmt::CancelTransaction()
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	EL_PACKET64 CancelPack;

	int nTX;
	//
	if( m_bIsIng ){
		//send cancel trasaction
		memset( &CancelPack,0x00,sizeof(EL_PACKET64) );
		CancelPack.header.cType=EL_PIN_CMD_TYPE_STOP_CMD;
		CancelPack.header.ulLength=0;
		CancelPack.header.cSlot=0;
		CancelPack.header.cSeq=GetIncTransaction();
		CancelPack.header.cSubType1=0;
		CancelPack.header.cSubType2=0;
		CancelPack.header.cSubType3=0;
		CancelPack.header.ulChain=0;
	}
	else
		memset( &CancelPack,0x00,sizeof(EL_PACKET64));
	//
	nTX=sizeof(EL_PACKET64);
	dwResult=Write( (unsigned char*)&CancelPack,&nTX );//send cancel transaction

	if( dwResult!=KLP110UF_S_SUCCESS )
		return dwResult;

	if( m_pTransBuffer ){
		free(m_pTransBuffer);
		m_pTransBuffer=NULL;
	}

	//Pass dummy response.
	dwResult=PassDummyResponse();
	if( dwResult==KLP110UF_S_SUCCESS )
		m_bIsIng=FALSE;//indicates Stop-transaction

	return dwResult;
}

DWORD CEL_Pin_TransMgmt::PassDummyResponse()
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	EL_PACKET64 Pack;
	int nRX;
	int i;

	memset( &Pack,0x00,sizeof(EL_PACKET64) );
	nRX=sizeof(EL_PACKET64);

	for( i=0; i<DUMMY_PASS_LOOP_DEFAULT_COUNTER; i++ ){//dummy pass 3 times
		dwResult=Read( (unsigned char*)&Pack,&nRX );//get response
		if( dwResult!=KLP110UF_S_SUCCESS )
			return dwResult;
		else//save header for passing dummy
			memcpy( &m_PreInHeader,&(Pack.header),sizeof(EL_PACKET_H64));

	}//end for

	return dwResult;
}

enum enumHOST_IN_pin_event CEL_Pin_TransMgmt::GetPinEvent_inPhase( PEL_PACKET_H64 pHeader,enum enumHOST_IN_status CurStatus )
{
	enum enumHOST_IN_pin_event event=HIN_UNKNOWN;
	BYTE t=0;
	BYTE T=0;
	BYTE s=0;
	ULONG c=0;
	BYTE m=0;
	//
	if( pHeader==NULL )
		return event;
	//
	m=m_CurOutPacket.header.cSeq;
	T=m_CurOutPacket.header.cType;//get Out-phase type
	t=pHeader->cType;	s=pHeader->cSeq;	c=pHeader->ulChain;

	switch( CurStatus ){
		case HINS_S000://idle
			if( t==0 && s==0 && c==0 )
				event=HIN_R008;
			else
				event=HIN_R007;
			break;
		case HINS_S010://waits
			if( t==0 && s==m && c==0 )
				event=HIN_R000;
			else if( t=='X' )
				event=HIN_R006;//cancel response
			else if( t!=0 && s==m && c==1 )
				event=HIN_R001;
			else if( t!=0 && s==m && c==0 )
				event=HIN_R003;
			else
				event=HIN_R004;
			break;
		case HINS_S111://receiving
			if( t=='X' )
				event=HIN_R006;//cancel response
			else if( t!=0 && s==m ){
				if( c!=0 ){//check chaining number.
					//here checking code -_-;;
					event=HIN_R002;
				}
				else
					event=HIN_R003;
			}
			else
				event=HIN_R005;
			break;
		case HINS_S110://last response
			if( t==0 && s==0 && c==0 )
				event=HIN_R008;
			else
				event=HIN_R007;
			break;
		case HINS_S100://cancel
				event=HIN_R006;
			break;
		default:
			break;
	}//end switch

	//
	return event;
}

DWORD CEL_Pin_TransMgmt::CheckIdleResponse()
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