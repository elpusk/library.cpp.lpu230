// EL_Pin_IO_Base.cpp: implementation of the CEL_Pin_IO_Base class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "EL_Pin_Cmd_Base.h"
#include "EL_Pin_ReCode.h"
#include "EL_Support.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEL_Pin_Cmd_Base::CEL_Pin_Cmd_Base()
{
	//ini
	Initial();
}

CEL_Pin_Cmd_Base::~CEL_Pin_Cmd_Base()
{
	Destory();
}

void CEL_Pin_Cmd_Base::Initial()//call by constructure
{
	//create m_pTransObj object 
	if( sizeof(CEL_Pin_TransMgmt_Vender)>sizeof(CEL_Pin_TransMgmt) ){
		//create CEL_Pin_TransMgmt_Vender object
		m_pTransObj=(CEL_Pin_TransMgmt_Vender*)new CEL_Pin_TransMgmt_Vender();
	
	}
	else{
		//create CEL_Pin_TransMgmt(HID base) object
		m_pTransObj=(CEL_Pin_TransMgmt*)new CEL_Pin_TransMgmt();
	}
	
	m_pTransObj->Initial();
	//
	m_nRXSize=0;
	m_pInParaBuf=NULL;

	memset( m_sMainAliveMutexName,0x00,_MAX_PATH );
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );


	//create mutex for single object
	//Mutex name is fixed
	sprintf( m_sMainAliveMutexName,"_UNIQUE_NAME_REQ_CEL_PIN_CMD_BASE_" );
	//srand( (unsigned)time( NULL ) );
	//EL_CreateRandomName( m_sMainAliveMutexName,rand(),GetTickCount() );

	m_hMainAliveMutex=EL_CreateOneAppMutex( m_sMainAliveMutexName,FALSE );
	if( m_hMainAliveMutex==NULL ){
		//failure command sync-mutex
	}
	else{
		m_hWThreadKillEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
		if( m_hWThreadKillEvent==NULL ){
			//failure command Kill-notifier
		}

		m_hRequestEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
		if( m_hRequestEvent==NULL ){
			//failure request event
		}

		m_hInOkEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
		if( m_hInOkEvent==NULL ){
			//failure request event
		}
		//
		m_hRequestMutex=CreateMutex(NULL,FALSE,NULL);
		if( m_hRequestMutex==NULL ){
			//failure request mutex
		}

		m_hMutexProtect=CreateMutex(NULL,FALSE,NULL);
		if( m_hMutexProtect==NULL ){
			//failure request mutex
		}

		CreateWokerThread();
	}
}

void CEL_Pin_Cmd_Base::Destory()//call by destructure
{
	TerminateWokerThread();

	if( m_hMainAliveMutex )
		CloseHandle(m_hMainAliveMutex);

	if( m_hWThreadKillEvent )
		CloseHandle( m_hWThreadKillEvent );

	if( m_hRequestEvent )
		CloseHandle( m_hRequestEvent );
	//
	if( m_hRequestMutex )
		CloseHandle( m_hRequestMutex );

	if( m_hMutexProtect )
		CloseHandle( m_hMutexProtect );

	if( m_pInParaBuf )
		free( m_pInParaBuf );

	if( m_pTransObj )
		delete( m_pTransObj );
}


////////////////////////////////////////////////////////////////////////////
//command base function as function-type 

//execute sync-type function
DWORD CEL_Pin_Cmd_Base::SyncFun( BYTE *lpdata,DWORD dwSize,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwWaitResult;
	//
	if( lpdata==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//
	//you need mutex - functional protection....... 
	dwWaitResult=WaitForSingleObject( m_hRequestMutex,EVENT_WAIT_DEFAULT_TIME);
	if( dwWaitResult!=WAIT_OBJECT_0 ){
		dwResult=KLP110UF_E_UNKOWN;
		return dwResult;
	}

	//setting parameter
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );
	
	m_InParaItem.TransType=TransType;
	m_InParaItem.nOutSize=(int)dwSize;//input data size,from device
	m_InParaItem.pOutData=lpdata;//input data

	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=NULL;//Output data
	}
	else{
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=&m_pRX[0];//Output data
	}

	//worker notify end of processing to mail thread
	m_InParaItem.hNotifyEvent=CreateEvent( NULL,TRUE,FALSE,NULL );

	//for windows message type
	m_InParaItem.hTargetWnd=0;
	m_InParaItem.uTargetMsg=0;

	m_InParaItem.lpCBFun=NULL;
	m_InParaItem.lpUserPara=NULL;
	m_InParaItem.dwResult=0;//the result of processing

	//
	SetEvent( m_hRequestEvent );//request processing

	//wait pre-accepting.......
	dwResult=WaitForSingleObject( m_hInOkEvent,EVENT_WAIT_DEFAULT_TIME );
	if( dwResult==WAIT_OBJECT_0 ){
		ResetEvent( m_hInOkEvent );
		dwResult=KLP110UF_S_SUCCESS;
		ReleaseMutex( m_hRequestMutex );//release mutex
	}
	else{
		dwResult=KLP110UF_E_UNKOWN;
		ReleaseMutex( m_hRequestMutex );//release mutex
		return dwResult;
	}

	//wait result of processing
	dwResult=WaitForSingleObject( m_InParaItem.hNotifyEvent,INFINITE );
	if( dwResult==WAIT_OBJECT_0 ){
		dwResult=m_dwResult;
	}
	else{
		dwResult=KLP110UF_E_UNKOWN;
	}
	CloseHandle( m_InParaItem.hNotifyEvent );
	//

	return dwResult;
}

//execute Async-type function by polling method
DWORD CEL_Pin_Cmd_Base::AsyncFun_Pol(  BYTE *lpdata,DWORD dwSize,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( lpdata==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//

	//
	return dwResult;
}

//execute sync-type function by windows Messaging method
DWORD CEL_Pin_Cmd_Base::AsyncFun_Msg(  BYTE *lpdata,DWORD dwSize,HWND hTarget,UINT uMsg,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwWaitResult;
	//
	if( lpdata==NULL )
		return KLP110UF_E_INVAILD_PARA;

	//you need mutex - functional protection....... 
	dwWaitResult=WaitForSingleObject( m_hRequestMutex,EVENT_WAIT_DEFAULT_TIME);
	if( dwWaitResult!=WAIT_OBJECT_0 ){
		dwResult=KLP110UF_E_UNKOWN;
		return dwResult;
	}

	//setting parameter
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );
	
	m_InParaItem.TransType=TransType;
	m_InParaItem.nOutSize=(int)dwSize;//input data size,from device
	m_InParaItem.pOutData=lpdata;//input data

	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=NULL;//Output data
	}
	else{
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=&m_pRX[0];//Output data
	}

	//worker notify end of processing to mail thread
	//ParaItem.hNotifyEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
	m_InParaItem.hNotifyEvent=NULL;

	//for windows message type
	m_InParaItem.hTargetWnd=hTarget;
	m_InParaItem.uTargetMsg=uMsg;

	m_InParaItem.lpCBFun=NULL;
	m_InParaItem.lpUserPara=NULL;
	m_InParaItem.dwResult=0;//the result of processing

	//
	SetEvent( m_hRequestEvent );//request processing

	//wait pre-accepting.......
	dwResult=WaitForSingleObject( m_hInOkEvent,EVENT_WAIT_DEFAULT_TIME );
	if( dwResult==WAIT_OBJECT_0 ){
		ResetEvent( m_hInOkEvent );
		dwResult=KLP110UF_S_SUCCESS;
	}
	else
		dwResult=KLP110UF_E_UNKOWN;

	//release mutex
	 ReleaseMutex( m_hRequestMutex );
	//

	return dwResult;
}

//execute sync-type function by callback method
DWORD CEL_Pin_Cmd_Base::AsyncFun_CB(  BYTE *lpdata,DWORD dwSize,CallbackFunType lpFun,LPVOID lpUserPara,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwWaitResult;
	//
	if( lpdata==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//
	//setting parameter
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );
	
	m_InParaItem.TransType=TransType;
	m_InParaItem.nOutSize=(int)dwSize;//input data size,from device
	m_InParaItem.pOutData=lpdata;//input data

	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=NULL;//Output data
	}
	else{
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=&m_pRX[0];//Output data
	}

	//worker notify end of processing to mail thread
	//m_InParaItem.hNotifyEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
	m_InParaItem.hNotifyEvent=NULL;

	//for windows message type
	m_InParaItem.hTargetWnd=0;
	m_InParaItem.uTargetMsg=0;

	m_InParaItem.lpCBFun=lpFun;
	m_InParaItem.lpUserPara=lpUserPara;
	m_InParaItem.dwResult=0;//the result of processing

	//you need mutex - functional protection....... 
	dwWaitResult=WaitForSingleObject( m_hRequestMutex,EVENT_WAIT_DEFAULT_TIME);
	if( dwWaitResult!=WAIT_OBJECT_0 ){
		dwResult=KLP110UF_E_UNKOWN;
		return dwResult;
	}

	//
	SetEvent( m_hRequestEvent );//request processing

	//wait pre-accepting.......
	dwResult=WaitForSingleObject( m_hInOkEvent,EVENT_WAIT_DEFAULT_TIME );
	if( dwResult==WAIT_OBJECT_0 ){
		ResetEvent( m_hInOkEvent );
		dwResult=KLP110UF_S_SUCCESS;
	}
	else
		dwResult=KLP110UF_E_UNKOWN;

	//release mutex
	 ReleaseMutex( m_hRequestMutex );
	//
	return dwResult;
}

DWORD CEL_Pin_Cmd_Base::SyncFun( PEL_PACKET_H64 pHeader,char *sFileName,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )//execute sync-type function
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwWaitResult;
	HANDLE hDataFile=NULL;
	int nFileSize=0;

	if( pHeader==NULL )
		return KLP110UF_E_INVAILD_PARA;

	if( sFileName ){//given data files.......
		hDataFile=CreateFile( 
			sFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if( hDataFile==INVALID_HANDLE_VALUE ){
			return KLP110UF_E_INVAILD_PARA;
		}

		//GetFileSize
		nFileSize=GetFileSize(hDataFile,NULL);
		if( nFileSize<=0 ){
			CloseHandle( hDataFile );
			return KLP110UF_E_INVAILD_PARA;
		}

		if( pHeader->ulLength==0 ){
			pHeader->ulLength=(ULONG)nFileSize;
		}
		else if( (int)pHeader->ulLength>nFileSize ){
			CloseHandle( hDataFile );
			return KLP110UF_E_INVAILD_PARA;
		}
	}
	else
		return SyncFun( (BYTE*)pHeader,sizeof(EL_PACKET_H64),TransType );
	//

	//
	//you need mutex - functional protection....... 
	dwWaitResult=WaitForSingleObject( m_hRequestMutex,EVENT_WAIT_DEFAULT_TIME);
	if( dwWaitResult!=WAIT_OBJECT_0 ){
		dwResult=KLP110UF_E_UNKOWN;
		CloseHandle( hDataFile );
		return dwResult;
	}
	//
	//setting parameter
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );

	m_InParaItem.TransType=TransType;
	//m_InParaItem.nOutSize=sizeof(EL_PACKET_H64)+nFileSize;//input data size,from device
	//In file trnasfer mode,size must be the size of header
	m_InParaItem.nOutSize=sizeof(EL_PACKET_H64);//input data size,from device
	m_InParaItem.pOutData=(BYTE*)pHeader;//input data

	//>_< this handle is closed automatically by worker thread
	m_InParaItem.hFileData=hDataFile;//set data file
	m_InParaItem.bUsedFile=TRUE;

	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=NULL;//Output data
	}
	else{
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=&m_pRX[0];//Output data
	}

	//worker notify end of processing to mail thread
	m_InParaItem.hNotifyEvent=CreateEvent( NULL,TRUE,FALSE,NULL );

	//for windows message type
	m_InParaItem.hTargetWnd=0;
	m_InParaItem.uTargetMsg=0;

	m_InParaItem.lpCBFun=NULL;
	m_InParaItem.lpUserPara=NULL;
	m_InParaItem.dwResult=0;//the result of processing

	//
	SetEvent( m_hRequestEvent );//request processing

	//wait pre-accepting.......
	dwResult=WaitForSingleObject( m_hInOkEvent,EVENT_WAIT_DEFAULT_TIME );
	if( dwResult==WAIT_OBJECT_0 ){
		ResetEvent( m_hInOkEvent );
		dwResult=KLP110UF_S_SUCCESS;
		ReleaseMutex( m_hRequestMutex );//release mutex
	}
	else{
		dwResult=KLP110UF_E_UNKOWN;
		CloseHandle( hDataFile );
		ReleaseMutex( m_hRequestMutex );//release mutex
		return dwResult;
	}

	//wait result of processing
	dwResult=WaitForSingleObject( m_InParaItem.hNotifyEvent,INFINITE );
	if( dwResult==WAIT_OBJECT_0 ){
		dwResult=m_dwResult;
	}
	else{
		dwResult=KLP110UF_E_UNKOWN;
	}
	CloseHandle( m_InParaItem.hNotifyEvent );
	//

	return dwResult;
}

DWORD CEL_Pin_Cmd_Base::AsyncFun_Pol( PEL_PACKET_H64 pHeader,char *sFileName,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )//execute Async-type function by polling method
{
	DWORD dwResult=KLP110UF_S_SUCCESS;

	if( pHeader==NULL )
		return KLP110UF_E_INVAILD_PARA;

	return dwResult;
}

DWORD CEL_Pin_Cmd_Base::AsyncFun_Msg( PEL_PACKET_H64 pHeader,char *sFileName,HWND hTarget,UINT uMsg,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )//execute sync-type function by windows Messaging method
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwWaitResult;
	HANDLE hDataFile=NULL;
	int nFileSize=0;

	//
	if( pHeader==NULL )
		return KLP110UF_E_INVAILD_PARA;

	if( sFileName ){//given data files.......
		hDataFile=CreateFile( 
			sFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if( hDataFile==INVALID_HANDLE_VALUE ){
			return KLP110UF_E_INVAILD_PARA;
		}

		//GetFileSize
		nFileSize=GetFileSize(hDataFile,NULL);

		if( nFileSize<=0 ){
			CloseHandle( hDataFile );
			return KLP110UF_E_INVAILD_PARA;
		}

		if( pHeader->ulLength==0 ){
			pHeader->ulLength=(ULONG)nFileSize;
		}
		else if( (int)pHeader->ulLength>nFileSize ){
			CloseHandle( hDataFile );
			return KLP110UF_E_INVAILD_PARA;
		}
	}
	else
		return AsyncFun_Msg( (BYTE*)pHeader,sizeof(EL_PACKET_H64),hTarget,uMsg,TransType );
	//

	//you need mutex - functional protection....... 
	dwWaitResult=WaitForSingleObject( m_hRequestMutex,EVENT_WAIT_DEFAULT_TIME);
	if( dwWaitResult!=WAIT_OBJECT_0 ){
		dwResult=KLP110UF_E_UNKOWN;
		CloseHandle( hDataFile );
		return dwResult;
	}

	//setting parameter
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );
	
	m_InParaItem.TransType=TransType;
	//m_InParaItem.nOutSize=sizeof(EL_PACKET_H64)+nFileSize;//input data size,from device
	//In file trnasfer mode,size must be the size of header
	m_InParaItem.nOutSize=sizeof(EL_PACKET_H64);//input data size,from device
	m_InParaItem.pOutData=(BYTE*)pHeader;//input data

	//>_< this handle is closed automatically by worker thread
	m_InParaItem.hFileData=hDataFile;//set data file
	m_InParaItem.bUsedFile=TRUE;

	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=NULL;//Output data
	}
	else{
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=&m_pRX[0];//Output data
	}

	//worker notify end of processing to mail thread
	//ParaItem.hNotifyEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
	m_InParaItem.hNotifyEvent=NULL;

	//for windows message type
	m_InParaItem.hTargetWnd=hTarget;
	m_InParaItem.uTargetMsg=uMsg;

	m_InParaItem.lpCBFun=NULL;
	m_InParaItem.lpUserPara=NULL;
	m_InParaItem.dwResult=0;//the result of processing

	//
	SetEvent( m_hRequestEvent );//request processing

	//wait pre-accepting.......
	dwResult=WaitForSingleObject( m_hInOkEvent,EVENT_WAIT_DEFAULT_TIME );
	if( dwResult==WAIT_OBJECT_0 ){
		ResetEvent( m_hInOkEvent );
		dwResult=KLP110UF_S_SUCCESS;
	}
	else{
		CloseHandle( hDataFile );
		dwResult=KLP110UF_E_UNKOWN;
	}

	//release mutex
	 ReleaseMutex( m_hRequestMutex );
	//
	return dwResult;
}

DWORD CEL_Pin_Cmd_Base::AsyncFun_CB( PEL_PACKET_H64 pHeader,char *sFileName,CallbackFunType lpFun,LPVOID lpUserPara,enum enumPin_Trans_Type TransType/*=PIN_TRANS_TYPE_S_OUT_S_IN*/ )//execute sync-type function by callback method
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	DWORD dwWaitResult;
	HANDLE hDataFile=NULL;
	int nFileSize=0;

	//
	if( pHeader==NULL )
		return KLP110UF_E_INVAILD_PARA;
	//
	if( sFileName ){//given data files.......
		hDataFile=CreateFile( 
			sFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if( hDataFile==INVALID_HANDLE_VALUE ){
			return KLP110UF_E_INVAILD_PARA;
		}

		//GetFileSize
		nFileSize=GetFileSize(hDataFile,NULL);
		if( nFileSize<=0 ){
			CloseHandle( hDataFile );
			return KLP110UF_E_INVAILD_PARA;
		}

		if( pHeader->ulLength==0 ){
			pHeader->ulLength=(ULONG)nFileSize;
		}
		else if( (int)pHeader->ulLength>nFileSize ){
			CloseHandle( hDataFile );
			return KLP110UF_E_INVAILD_PARA;
		}
	}
	else
		return AsyncFun_CB( (BYTE*)pHeader,sizeof(EL_PACKET_H64),lpFun,lpUserPara,TransType );

	//setting parameter
	memset( &m_InParaItem,0x00,sizeof(ParamterItem) );
	

	m_InParaItem.TransType=TransType;
	//m_InParaItem.nOutSize=sizeof(EL_PACKET_H64)+nFileSize;//input data size,from device
	//In file trnasfer mode,size must be the size of header
	m_InParaItem.nOutSize=sizeof(EL_PACKET_H64);//input data size,from device
	m_InParaItem.pOutData=(BYTE*)pHeader;//input data

	//>_< this handle is closed automatically by worker thread
	m_InParaItem.hFileData=hDataFile;//set data file
	m_InParaItem.bUsedFile=TRUE;

	if( TransType==PIN_TRANS_TYPE_S_OUT ){
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=NULL;//Output data
	}
	else{
		m_InParaItem.nInSize=0;//Out data size,from device
		m_InParaItem.pInData=&m_pRX[0];//Output data
	}

	//worker notify end of processing to mail thread
	//m_InParaItem.hNotifyEvent=CreateEvent( NULL,TRUE,FALSE,NULL );
	m_InParaItem.hNotifyEvent=NULL;

	//for windows message type
	m_InParaItem.hTargetWnd=0;
	m_InParaItem.uTargetMsg=0;

	m_InParaItem.lpCBFun=lpFun;
	m_InParaItem.lpUserPara=lpUserPara;
	m_InParaItem.dwResult=0;//the result of processing

	//you need mutex - functional protection....... 
	dwWaitResult=WaitForSingleObject( m_hRequestMutex,EVENT_WAIT_DEFAULT_TIME);
	if( dwWaitResult!=WAIT_OBJECT_0 ){
		dwResult=KLP110UF_E_UNKOWN;
		CloseHandle( hDataFile );
		return dwResult;
	}

	//
	SetEvent( m_hRequestEvent );//request processing

	//wait pre-accepting.......
	dwResult=WaitForSingleObject( m_hInOkEvent,EVENT_WAIT_DEFAULT_TIME );
	if( dwResult==WAIT_OBJECT_0 ){
		ResetEvent( m_hInOkEvent );
		dwResult=KLP110UF_S_SUCCESS;
	}
	else{
		CloseHandle( hDataFile );
		dwResult=KLP110UF_E_UNKOWN;
	}

	//release mutex
	ReleaseMutex( m_hRequestMutex );
	//
	return dwResult;
}

BOOL CEL_Pin_Cmd_Base::LockProtect( DWORD dwTimeOut/*=INFINITE*/ )
{
	//m_hMutexProtect;//protects m_dwResult,m_nRXSize and m_pRX member.......
	BOOL bResult=TRUE;
	DWORD dwWaitResult=0;

	//Request ownership of mutex.
	dwWaitResult=WaitForSingleObject( m_hMutexProtect,dwTimeOut );

	switch (dwWaitResult) {
        case WAIT_OBJECT_0:// The thread got mutex ownership.
			bResult=TRUE;
			break;
        case WAIT_TIMEOUT:// Cannot get mutex ownership due to time-out.
        case WAIT_ABANDONED:// Got ownership of the abandoned mutex object.
		default:
            bResult=FALSE;
			break;
	}//end switch

	return bResult;
}
BOOL CEL_Pin_Cmd_Base::ReleaseProtect()
{
	BOOL bResult=TRUE;

	bResult=ReleaseMutex(m_hMutexProtect);

	return bResult;
}


//get result of command processing by sync asynac type function
DWORD CEL_Pin_Cmd_Base::GetResult(  BYTE *lpdata,DWORD *pdwSize )
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	if( lpdata==NULL && pdwSize!=NULL ){
		if( !LockProtect(PROTECT_DEFAULT_TIME_OUT) )
			return KLP110UF_E_UNKNOWN;//Get mutex error

		*pdwSize=m_nRXSize;
		dwResult=m_dwResult;
		ReleaseProtect();

		return dwResult;
	}
	if( pdwSize==NULL ){
		if( !LockProtect(PROTECT_DEFAULT_TIME_OUT) )
			return KLP110UF_E_UNKNOWN;//Get mutex error

		dwResult=m_dwResult;

		ReleaseProtect();
		return dwResult;
	}
	//
	if( !LockProtect(PROTECT_DEFAULT_TIME_OUT) )
		return KLP110UF_E_UNKNOWN;//Get mutex error

	*pdwSize=m_nRXSize;
	if( m_nRXSize>0 )
		memcpy( lpdata,m_pRX,m_nRXSize );

	dwResult=m_dwResult;
	ReleaseProtect();
	//
	return dwResult;
}

DWORD CEL_Pin_Cmd_Base::Cancel()	//cancel current processing.......
{
	DWORD dwResult=KLP110UF_S_SUCCESS;
	//
	EL_PACKET_H64 head;

	memset( &head,0x00,sizeof(EL_PACKET_H64) );

	head.cType='X';//cancel command.

	dwResult=SyncFun( (BYTE*)&head,sizeof(EL_PACKET_H64),PIN_TRANS_TYPE_S_OUT );
	return dwResult;
}

BOOL CEL_Pin_Cmd_Base::CreateWokerThread()
{
	LPWokerThreadItem pItem;
	HANDLE hStartUp;
	DWORD dwResult;
	//
	if( m_hMainAliveMutex==NULL )
		return FALSE;
	//
	pItem=(LPWokerThreadItem)malloc( sizeof(WokerThreadItem) );
	if( pItem==NULL )
		return FALSE;

	hStartUp=CreateEvent( NULL,TRUE,FALSE,NULL );
	if( hStartUp==NULL ){
		free(pItem);
		return FALSE;
	}

	memset( pItem,0x00,sizeof(WokerThreadItem) );
	strcpy( pItem->sMainALiveMutexName,m_sMainAliveMutexName );
	ResetEvent( m_hWThreadKillEvent );
	pItem->hKillMe=m_hWThreadKillEvent;
	pItem->hStartUp=hStartUp;
	pItem->hRequest=m_hRequestEvent;
	pItem->lpUserPara=this;

	//
	m_hWThreadhandle=CreateThread(
		NULL,
		0x80000,
		(LPTHREAD_START_ROUTINE)WokerThread,
		pItem,
		0,
		&m_dwThID
		);
	if( m_hWThreadhandle==NULL ){
		m_hWThreadDeadEvent=NULL;
		free( pItem );
		CloseHandle(hStartUp);
		return FALSE;
	}

	//wait to work-thread start-up
	dwResult=WaitForSingleObject( hStartUp,EVENT_WAIT_DEFAULT_TIME );
	switch( dwResult ){
		case WAIT_OBJECT_0://start up OK
			m_hWThreadDeadEvent=pItem->hIamKilled;
			if( m_hWThreadDeadEvent==NULL ){
				//work-thread failure of ini
				//work-thread suicide automatically.
			}
			else{
				strcpy( m_sDeadEventName,pItem->sIamKilledEventName );
			}
			break;
		case WAIT_TIMEOUT:
		case WAIT_ABANDONED:
		case WAIT_FAILED:
			TerminateWokerThread();
			m_hWThreadDeadEvent=NULL;
			free( pItem );
			CloseHandle(hStartUp);
			return FALSE;
			break;
	}//end switch 
	

	free( pItem );
	CloseHandle(hStartUp);
	return TRUE;
}

BOOL CEL_Pin_Cmd_Base::TerminateWokerThread()
{
	DWORD dwResult;

	if( m_hMainAliveMutex==NULL )
		return FALSE;
	//
	if( m_hWThreadhandle==NULL )
		return FALSE;
	//

	//check alive work-thread......() m_hWThreadDeadEvent
	if( !EL_AvailableEvent( m_sDeadEventName ) ){
		//woker-thread may be killed.....
		//may be this instruction is dummy. 
		//but feels good.......
		SetEvent( m_hWThreadKillEvent );
		return TRUE;
	}

	//exsit woker-thread.
	//command to woker-thread to stop.
	if( !SetEvent( m_hWThreadKillEvent ) )
		return FALSE;
	
	//wait to work-thread shut-up
	dwResult=WaitForSingleObject( m_hWThreadDeadEvent,EVENT_WAIT_DEFAULT_TIME*2 );
	switch( dwResult ){
		case WAIT_OBJECT_0://shut-up OK
			CloseHandle( m_hWThreadDeadEvent );
			m_hWThreadDeadEvent=NULL;
			break;
		case WAIT_TIMEOUT:
		case WAIT_ABANDONED:
		case WAIT_FAILED:
			m_hWThreadDeadEvent=NULL;
			return FALSE;
			break;
	}//end switch 


	return TRUE;
}

UINT CEL_Pin_Cmd_Base::WokerThread(LPVOID pParam)
{
	UINT uResult=0;
	LPWokerThreadItem pItem=(LPWokerThreadItem)pParam;
	CEL_Pin_Cmd_Base *pObj;
	CEL_Pin_TransMgmt_Base *pBase;
	HANDLE hStopEvent;
	HANDLE hStartupEvent;
	HANDLE hRequestEvent;
	HANDLE hGoodByteEvent;
	char sMainAliveMutexName[_MAX_PATH];
	DWORD dwWaitResult,dwResult;
	HANDLE HandleArrary[2];
	int nEventCnt=2;
	BOOL bIdle=TRUE;
	BOOL bPairMode=TRUE;
	BOOL bOutMode=TRUE;
	ParamterItem CurParaItem;
	//
	memset( sMainAliveMutexName,0x00,_MAX_PATH );
	pObj=(CEL_Pin_Cmd_Base*)pItem->lpUserPara;
	pBase=pObj->m_pTransObj;
	hStopEvent=pItem->hKillMe;//stop event by main-thread.......
	hStartupEvent=pItem->hStartUp;
	hRequestEvent=pItem->hRequest;
	strcpy( sMainAliveMutexName,pItem->sMainALiveMutexName );

	//
	memset( &CurParaItem,0x00,sizeof(ParamterItem) );

	//ini thread
	srand( (unsigned)time( NULL ) );
	EL_CreateRandomName( pItem->sIamKilledEventName,rand(),GetTickCount() );

	hGoodByteEvent=CreateEvent( NULL,TRUE,FALSE,pItem->sIamKilledEventName );
	pItem->hIamKilled=hGoodByteEvent;
	if( hGoodByteEvent==NULL ){
		SetEvent( hStartupEvent );
		uResult=100;
		return uResult;
	}
	else{
		HandleArrary[0]=hStopEvent;
		HandleArrary[1]=hRequestEvent;
		SetEvent( hStartupEvent );
	}
	//

	/////////////////////////////////////////////////
	//thread main.......
	while(1){//infinate loop 
		//the main part of woker.......

		//dead condition 
		//hStopEvent is signaled status. OR
		//sMainAliveMutexName mutex is killed.
		while( bIdle ){
			dwWaitResult=WaitForMultipleObjects( nEventCnt,HandleArrary,FALSE,WOKER_WAIT_DEFAULT_TIME );
			switch(dwWaitResult){
				case WAIT_OBJECT_0://request stop thread
					ResetEvent( hStopEvent );

					SetEvent( pObj->m_hInOkEvent );
					uResult=100;
					SetEvent( hGoodByteEvent );//notify will be exit.......
					return uResult;//exit thread >_<
					break;
				case WAIT_OBJECT_0+1://request command
					ResetEvent( hRequestEvent );
					bIdle=FALSE;

					//copy pararmeter of m_InParaItem
					pObj->CopyInPara( &CurParaItem );
					SetEvent( pObj->m_hInOkEvent );//notifiy to accepting input
					
					if( CurParaItem.TransType!=PIN_TRANS_TYPE_S_OUT )
						bPairMode=TRUE;
					else
						bPairMode=FALSE;
					//
					bOutMode=TRUE;

					if( CurParaItem.bUsedFile )
						pBase->SetTransMode( TRUE,CurParaItem.hFileData );
					else
						pBase->SetTransMode();

					dwResult=pBase->OutTransaction( CurParaItem.TransType,CurParaItem.pOutData,&CurParaItem.nOutSize );
					if( dwResult==KLP110UF_S_SUCCESS )
						if( !bPairMode ){
							bIdle=TRUE;

							pBase->CheckIdleResponse();
							//transaction complete.......
							NotifyResult( pObj,&CurParaItem,dwResult );
						}
						else{//start In-transaction
							bOutMode=FALSE;//indicate out transaction mode
						}
					else if( dwResult==KLP110UF_W_MORE_PROCESS ){
						//More requested Out-transaction
					}
					else{//Error
						if( dwResult==KLP110UF_E_INVAILD_RESP )
							pBase->CheckIdleResponse();

						bIdle=TRUE;
						//transaction complete....... by error
						NotifyResult( pObj,&CurParaItem,dwResult );
					}
					break;
				case WAIT_ABANDONED_0://abandoned stop thread
					break;
				case WAIT_ABANDONED_0+1://abandoned command
					break;
				case WAIT_TIMEOUT://idle
					//Line check and Main thread Alive check
					break;
				case WAIT_FAILED:
					break;
			}//end switch
		}//end while-idle processing
		/////////////////////////////////////////////////////////////
		// Here command processing
		if( bOutMode ){
			dwResult=pBase->OutTransaction( CurParaItem.TransType );
			if( dwResult==KLP110UF_S_SUCCESS )
				if( !bPairMode ){
					bIdle=TRUE;

					pBase->CheckIdleResponse();
					//transaction complete.......
					NotifyResult( pObj,&CurParaItem,dwResult );
				}
				else{//start In-transaction
					bOutMode=FALSE;//indicate out transaction mode
				}
			else if( dwResult==KLP110UF_W_MORE_PROCESS ){
				//More requested Out-transaction
			}
			else{//Error
				if( dwResult==KLP110UF_E_INVAILD_RESP )
					pBase->CheckIdleResponse();

				bIdle=TRUE;
				//transaction complete....... by error
				NotifyResult( pObj,&CurParaItem,dwResult );
			}
		}
		else{//In transaction mode
			dwResult=pBase->InTransaction( CurParaItem.pInData,&CurParaItem.nInSize );
			if( dwResult==KLP110UF_S_SUCCESS ){
				bIdle=TRUE;

				pBase->CheckIdleResponse();

				//transaction complete.......
				NotifyResult( pObj,&CurParaItem,dwResult );
			}
			else if( dwResult==KLP110UF_W_MORE_PROCESS ){
				//More requested in-transaction
			}
			else{//Error
				if( dwResult==KLP110UF_E_INVAILD_RESP )
					pBase->CheckIdleResponse();

				bIdle=TRUE;
				//transaction complete....... by error
				NotifyResult( pObj,&CurParaItem,dwResult );
			}
		}

		if( !bIdle ){
			/////////////////////////////////////////////////////////////
			dwWaitResult=WaitForMultipleObjects( nEventCnt,HandleArrary,FALSE,WOKER_WAIT_DEFAULT_TIME );
			switch(dwWaitResult){
				case WAIT_OBJECT_0://request stop thread
					ResetEvent( hStopEvent );

					SetEvent( pObj->m_hInOkEvent );

					if( !bIdle ){//cancel previous command
						pObj->Cancel();
						dwResult=KLP110UF_W_CANCEL_BY_APP;
						NotifyResult( pObj,&CurParaItem,dwResult );
					}
					//
					uResult=100;
					SetEvent( hGoodByteEvent );//notify will be exit.......
					return uResult;//exit thread >_<
					break;
				case WAIT_OBJECT_0+1://request command
					ResetEvent( hRequestEvent );

					if( !bIdle ){
						//cancel previous command
						pObj->Cancel();
						dwResult=KLP110UF_W_CANCEL_BY_APP;
						NotifyResult( pObj,&CurParaItem,dwResult );
					}

					bIdle=FALSE;

					//copy pararmeter
					pObj->CopyInPara( &CurParaItem );
					SetEvent( pObj->m_hInOkEvent );//notifiy to accepting input

					if( CurParaItem.TransType!=PIN_TRANS_TYPE_S_OUT )
						bPairMode=TRUE;
					else
						bPairMode=FALSE;

					//execute new request
					//
					bOutMode=TRUE;

					if( CurParaItem.bUsedFile )
						pBase->SetTransMode( TRUE,CurParaItem.hFileData );
					else
						pBase->SetTransMode();

					dwResult=pBase->OutTransaction( CurParaItem.TransType,CurParaItem.pOutData,&CurParaItem.nOutSize );
					if( dwResult==KLP110UF_S_SUCCESS )
						if( !bPairMode ){
							bIdle=TRUE;

							pBase->CheckIdleResponse();

							//transaction complete.......
							NotifyResult( pObj,&CurParaItem,dwResult );
						}
						else{//start In-transaction
							bOutMode=FALSE;//indicate out transaction mode
						}
					else if( dwResult==KLP110UF_W_MORE_PROCESS ){
						//More requested Out-transaction
					}
					else{//Error
						if( dwResult==KLP110UF_E_INVAILD_RESP )
							pBase->CheckIdleResponse();

						bIdle=TRUE;
						//transaction complete....... by error
						NotifyResult( pObj,&CurParaItem,dwResult );
					}
					break;
				case WAIT_ABANDONED_0://abandoned stop thread
					break;
				case WAIT_ABANDONED_0+1://abandoned command
					break;
				case WAIT_TIMEOUT://idle
					break;
				case WAIT_FAILED:
					break;
			}//end switch
		}
	}//while(1)

	//
	return uResult;
}

//static member.......
void CEL_Pin_Cmd_Base::NotifyResult( CEL_Pin_Cmd_Base *pObj,PParamterItem pCurItem,DWORD dwResult )
{
	if( pCurItem==NULL )
		return;

	if( !pObj->LockProtect(PROTECT_DEFAULT_TIME_OUT) )
		return;//Get mutex error

	//Copy result
	pObj->m_dwResult=pCurItem->dwResult=dwResult;

	//copy response
	pObj->m_nRXSize=pCurItem->nInSize;
	if( pObj->m_nRXSize>0 )
		memcpy( pObj->m_pRX,pCurItem->pInData,pObj->m_nRXSize );

	pObj->ReleaseProtect();

	if( pCurItem->bUsedFile )
		CloseHandle( pCurItem->hFileData );
	//
	if( pCurItem->hNotifyEvent!=NULL ){
		//sync type or polling type case 
		SetEvent( pCurItem->hNotifyEvent );
	}
	else if( pCurItem->lpCBFun ){
		//call abck type
		pCurItem->lpCBFun( pCurItem->lpUserPara,dwResult,pCurItem->pInData,pCurItem->nInSize );
	}
	else{//may be message type

		switch( dwResult ){
			case KLP110UF_S_SUCCESS:
				PostMessage( pCurItem->hTargetWnd,pCurItem->uTargetMsg,
					KLP110UF_WPARAM_S_SUCCESS,(LPARAM)dwResult );
				break;
			case KLP110UF_S_REMOVE_ICC:
				PostMessage( pCurItem->hTargetWnd,pCurItem->uTargetMsg,
					KLP110UF_WPARAM_W_REMOVE_ICC,(LPARAM)dwResult );
				break;
			case KLP110UF_S_INSERT_ICC:
				PostMessage( pCurItem->hTargetWnd,pCurItem->uTargetMsg,
					KLP110UF_WPARAM_W_INSERT_ICC,(LPARAM)dwResult );
				break;
			case KLP110UF_W_CANCEL_BY_APP:
				PostMessage( pCurItem->hTargetWnd,pCurItem->uTargetMsg,
					KLP110UF_WPARAM_W_CANCEL_BY_APP,(LPARAM)dwResult );
				break;
			default:
				PostMessage( pCurItem->hTargetWnd,pCurItem->uTargetMsg,
					KLP110UF_WPARAM_E_INVAILD_RESP,(LPARAM)dwResult );
				break;
		}//end switch
		//
	}

	if( pObj->m_pInParaBuf ){
		free( pObj->m_pInParaBuf );
		pObj->m_pInParaBuf=NULL;
	}
}

BOOL CEL_Pin_Cmd_Base::CopyInPara( PParamterItem pDesItem )
{
	//copy m_InParaItem to pDesItem
	memcpy( pDesItem,&m_InParaItem,sizeof(ParamterItem) );

	if( m_InParaItem.hNotifyEvent==NULL ){
		//async method
		if( m_InParaItem.nOutSize>0 ){
			//Out data
			if( m_pInParaBuf ){
				free( m_pInParaBuf );
				m_pInParaBuf=NULL;
			}

			m_pInParaBuf=(unsigned char*)malloc(m_InParaItem.nOutSize);
			if( m_pInParaBuf==NULL ){//memory allcation error
				pDesItem->pOutData=NULL;
				return FALSE;
			}
			//link new buffer 
			pDesItem->pOutData=m_pInParaBuf;
			//Copy data
			memcpy( m_pInParaBuf,m_InParaItem.pOutData,m_InParaItem.nOutSize );
		}
	}

	return TRUE;
}