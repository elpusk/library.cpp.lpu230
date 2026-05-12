// CmdQueue.cpp: implementation of the CTTR_ShareCQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdio.h"
#include "TTR_ShareCQ.h"
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTTR_ShareCQueue::CTTR_ShareCQueue()
{	
	m_pCQMmf = NULL;		//use local memory of processor
	
	//
	m_pnNextRPos = new INT;
	m_pnNextWPos = new INT;
	
	//ini queue
	m_pCQ = new TTRSharedItem[TTR_SHARE_CQ_DEF_Q_SIZE];
	memset( m_pCQ,0,sizeof(TTRSharedItem)*TTR_SHARE_CQ_DEF_Q_SIZE );
	
		
	//Inital position queue.
	*m_pnNextRPos=0;	
	*m_pnNextWPos=0;
	
	//Create Mutex
	srand( (unsigned)time( NULL ) );

	m_hMutexSync = TTR_CreateUniqueMutex( m_sMutexSyncName,rand() );

}


CTTR_ShareCQueue::CTTR_ShareCQueue( BOOL bShared,TCHAR *sMutexSyncName /*=NULL*/,TCHAR *sMapObjName /*=NULL*/ )
{
	if( bShared ){
		//Use shared memory
		m_pCQMmf = new TTR_MMF;
		
		memset( m_pCQMmf,0,sizeof(TTR_MMF) );
		
		// create share memory
		//size *m_pnIsLock + size *m_pnNextRPos + size *m_pnNextWPos + size TTRSharedItem[TTR_SHARE_CQ_DEF_Q_SIZE]
		m_pCQMmf->dwSize = sizeof(INT) + sizeof(INT) + sizeof(INT) + sizeof(TTRSharedItem)*TTR_SHARE_CQ_DEF_Q_SIZE;
		if( sMapObjName )
			_tcscpy( m_pCQMmf->sName,sMapObjName );
		else
			TTR_CreateRandomName( m_pCQMmf->sName,GetTickCount(),0 );//auto generating
		
		if( TTR_CreateFileMem( m_pCQMmf ) ){
			//success
			m_pnIsLock = (LPINT)(m_pCQMmf->pMem);
			m_pnNextRPos = (LPINT)( ((LPBYTE)m_pCQMmf->pMem)+sizeof(INT) );
			m_pnNextWPos = (LPINT)( ((LPBYTE)m_pCQMmf->pMem)+sizeof(INT)+sizeof(INT) );
			m_pCQ = (LPTTRSharedItem)( ((LPBYTE)m_pCQMmf->pMem)+sizeof(INT)+sizeof(INT)+sizeof(INT) );
		}
		else{
			// Error
		}
	}
	else{
		//Use local processor memory
		m_pCQMmf = NULL;		//use local memory of processor
		
		//
		m_pnIsLock = new INT;
		m_pnNextRPos = new INT;
		m_pnNextWPos = new INT;
		
		//ini queue
		m_pCQ = new TTRSharedItem[TTR_SHARE_CQ_DEF_Q_SIZE];
		memset( m_pCQ,0,sizeof(TTRSharedItem)*TTR_SHARE_CQ_DEF_Q_SIZE );		
	}
	//
	
	//Inital position queue.

	if( m_pCQMmf ){

		if( m_pCQMmf->bIsCreate ){
			//shared memory is created.
			*m_pnIsLock = 0;
			*m_pnNextRPos=0;	
			*m_pnNextWPos=0;
		}
	}
	else{
		//shared memory is created.
		*m_pnIsLock = 0;
		*m_pnNextRPos=0;	
		*m_pnNextWPos=0;
	}
	
	
	if( sMutexSyncName==NULL ){
		//Create Mutex & gernerating Mutex Name
		srand( (unsigned)time( NULL ) );

		m_hMutexSync = TTR_CreateUniqueMutex( m_sMutexSyncName,rand() );
	}
	else{
		//Open Mutex( if dosen't exist,create mutex ).
		m_hMutexSync = TTR_CreateMutex( FALSE,FALSE,sMutexSyncName );

		_tcscpy( m_sMutexSyncName,sMutexSyncName );	//save given mutex name
	}	
	
}

CTTR_ShareCQueue::~CTTR_ShareCQueue()
{
	if( m_hMutexSync )
		CloseHandle( m_hMutexSync );
		
	
	//remove shared memory	
	if( m_pCQMmf ){
		
		TTR_DeleteFileMem( m_pCQMmf );
		
		delete( m_pCQMmf );
	}
	else{
		delete( m_pnIsLock );
		delete(m_pnNextRPos);
		delete(m_pnNextWPos);
		delete(m_pCQ);
	}
		
}


//enter critical section( return mutex handle )
HANDLE CTTR_ShareCQueue::EnterQ(void)
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return hMutex;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		TTR_SHARE_CQ_DEF_TIMEOUT);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return NULL;
	}


	return hMutex;
}

//release critical section
void CTTR_ShareCQueue::LeaveQ( HANDLE hMutex )
{
	if( hMutex ){

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

	}	
}


//copy m_sMutexSyncName data to sMutexSyncName, return length of name
INT CTTR_ShareCQueue::GetSyncMutexName( TCHAR *sMutexSyncName )
{
	INT nLen;
	
	if( sMutexSyncName ){
		_tcscpy( sMutexSyncName,m_sMutexSyncName );
		
		nLen = _tcslen( sMutexSyncName );
	}else{
		nLen = _tcslen( m_sMutexSyncName );
	}
	
	return nLen;
}

//copy m_pCQMmf->sName data to sMapObjName, return length of name
INT CTTR_ShareCQueue::GetMapObjName( TCHAR *sMapObjName )
{
	INT nLen;

	if( m_pCQMmf==NULL )
		return 0;
	
	if( sMapObjName ){
		_tcscpy( sMapObjName,m_pCQMmf->sName );
		
		nLen = _tcslen( sMapObjName );
	}else{
		nLen = _tcslen( sMapObjName );
	}
	
	return nLen;
}



BOOL CTTR_ShareCQueue::IsEmpty()	//Check
{
	BOOL bResult;
	HANDLE hMutex;

	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;


	if( *m_pnNextRPos==*m_pnNextWPos ){

		*m_pnNextRPos=*m_pnNextWPos=0;//reset
		bResult = TRUE;
	}
	else{
		bResult = FALSE;
	}

	LeaveQ(hMutex);
	return bResult;
}
//

void CTTR_ShareCQueue::Empty()	//clear queue.
{
	HANDLE hMutex;
	
	hMutex=EnterQ();
	if( hMutex==NULL )
		return;

	*m_pnNextRPos=*m_pnNextWPos=0;

	LeaveQ(hMutex);
}

BOOL CTTR_ShareCQueue::Lock( BOOL bLock /*=TRUE*/)	//lock enqueue 
{
	HANDLE hMutex;
	BOOL bResult = TRUE;
	
	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;

	if( m_pnIsLock ){
		if( bLock ){
			*m_pnIsLock = 1;//lock enqueue
		}
		else{
			*m_pnIsLock = 0;//release lock enqueue
		}
	}
	else
		bResult = FALSE;

	LeaveQ(hMutex);
	return bResult;
}
//

BOOL CTTR_ShareCQueue::EnQueue( TTRSharedItem &Item )		//add item to queue
{
	HANDLE hMutex;
	BOOL bResult = TRUE;

	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;

	if( *m_pnIsLock== 0 ){
		memcpy( &(m_pCQ[*m_pnNextWPos]),&Item,sizeof(TTRSharedItem) );
		//
		if( *m_pnNextWPos==(TTR_SHARE_CQ_DEF_Q_SIZE-1) )
			*m_pnNextWPos=0;
		else
			*m_pnNextWPos++;
	}
	else//locked
		bResult = FALSE;
	//
	LeaveQ(hMutex);
	return bResult;
}

BOOL CTTR_ShareCQueue::EnQueueFirst( TTRSharedItem &Item )		//add item to the starting point of queue
{
	HANDLE hMutex;
	BOOL bResult = TRUE;

	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;
	//
	if( *m_pnIsLock== 0 ){
		*m_pnNextRPos=*m_pnNextWPos=0;	//reset queue.
		memcpy( &(m_pCQ[*m_pnNextWPos]),&Item,sizeof(TTRSharedItem) );
		//
		if( *m_pnNextWPos==(TTR_SHARE_CQ_DEF_Q_SIZE-1) )
			*m_pnNextWPos=0;
		else
			*m_pnNextWPos++;
	}
	else//locked
		bResult = FALSE;
	//
	LeaveQ(hMutex);
	return bResult;
}
//

//

BOOL CTTR_ShareCQueue::DeQueue( LPTTRSharedItem pItem )	//Gets item from queue.
{
	HANDLE hMutex;

	if( pItem==NULL )
		return FALSE;
	//
	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;

	//
	memcpy( pItem,&(m_pCQ[*m_pnNextWPos]),sizeof(TTRSharedItem) );
	//
	if( *m_pnNextRPos==(TTR_SHARE_CQ_DEF_Q_SIZE-1) )
		*m_pnNextRPos=0;
	else
		*m_pnNextRPos++;
	//
	LeaveQ(hMutex);
	return TRUE;
}
//

//reset parameter0 
BOOL CTTR_ShareCQueue::ResetPara0( DWORD dwConditionPara0 )	
{
	HANDLE hMutex;
	INT i;

	if( dwConditionPara0==0 )
		return FALSE;
	//
	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;
	//
	for( i=0; i<TTR_SHARE_CQ_DEF_Q_SIZE; i++ ){

		if( m_pCQ[i].dwPara[0] == dwConditionPara0 )
			m_pCQ[i].dwPara[0] = 0;// reset para0
	}//end for
	
	//
	LeaveQ(hMutex);
	return TRUE;


}

//Gets item from queue. but Read position counter isn't increase.
BOOL CTTR_ShareCQueue::Peek( LPTTRSharedItem pItem )
{
	HANDLE hMutex;

	if( pItem==NULL )
		return FALSE;
	//
	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;
	//
	memcpy( pItem,&(m_pCQ[*m_pnNextWPos]),sizeof(TTRSharedItem) );
	//
	LeaveQ(hMutex);
	return TRUE;
}

//Gets MajCmd-item from queue. but Read position counter isn't increase.
BOOL CTTR_ShareCQueue::PeekMajor( LPDWORD pdwMajCmd )
{
	HANDLE hMutex;

	if( pdwMajCmd==NULL )
		return FALSE;
	//
	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;

	//
	*pdwMajCmd=m_pCQ[*m_pnNextRPos].dwMajCmd;
	//
	LeaveQ(hMutex);
	return TRUE;
}

//Gets MinCmd-item from queue. but Read position counter isn't increase.
BOOL CTTR_ShareCQueue::PeekMinor( LPDWORD pdwMinCmd )
{
	HANDLE hMutex;

	if( pdwMinCmd==NULL )
		return FALSE;
	//
	hMutex=EnterQ();
	if( hMutex==NULL )
		return FALSE;

	//
	*pdwMinCmd=m_pCQ[*m_pnNextRPos].dwMinCmd;
	//
	LeaveQ(hMutex);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////

//Get result from Item(share Q item )( static )
BOOL CTTR_ShareCQueue::GetSharedResult( 
								LPTTRSharedItem pItem,
								 LPDWORD pdwResult,
								 LPBYTE lpData,
								 LPLONG lpDataSize,
								 LPDWORD pdwDevIndex
								 )
{
	return GetSharedResult( 
		pItem->hMapObj,
		pdwResult,
		lpData,
		lpDataSize,
		pdwDevIndex
		 );
}


//Get result from Item(share Q item )( static )
BOOL CTTR_ShareCQueue::GetSharedResult( 
								HANDLE hMapObj,
								 LPDWORD pdwResult,
								 LPBYTE lpData,
								 LPLONG lpDataSize,
								 LPDWORD pdwDevIndex
								 )
{
	DWORD dwWaitResult; 

	LPVOID pvShared=NULL;
	LPHANDLE lphMutexLockMap;
	LPDWORD lpdwShareResult;
	LPDWORD lpdwShareDevIndex;
	LPBYTE lpShareData;
	LPLONG lpShareDataSize;

	if( hMapObj==NULL )
		return FALSE;

	pvShared = MapViewOfFile(
					hMapObj,        // File Map obj to view
					FILE_MAP_WRITE,   // Read/Write access
					0,                // high: map from beginning
					0,                // low:
					0);               // default: map entire file
	if( pvShared == NULL ){
		return FALSE;
	}

	//
	lphMutexLockMap = ((LPHANDLE)pvShared + SQ_MMF_OFF_HMUTEX );
	lpdwShareDevIndex = ((LPDWORD)pvShared + SQ_MMF_OFF_DEVINDEX );
	lpdwShareResult = (LPDWORD)( (LPBYTE)(pvShared)+SQ_MMF_OFF_RESULT );
	lpShareDataSize = (LPLONG)( (LPBYTE)(pvShared)+SQ_MMF_OFF_SIZE );
	lpShareData = (LPBYTE)( (LPBYTE)(pvShared)+SQ_MMF_OFF_DATA );

	if( *lphMutexLockMap==NULL )
		return FALSE;


	//////////////////////////////////////////////

	dwWaitResult=WaitForSingleObject(
		*lphMutexLockMap,			// handle to mutex
		TTR_SHARE_CQ_DEF_TIMEOUT);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		return FALSE;
	}

	//enter critical section
	if( lpDataSize!=NULL && lpData!=NULL ){
		*lpDataSize = *lpShareDataSize;
		if( *lpDataSize>0 ){
			if( *lpDataSize > (SQ_MMF_MAX_DATA-SQ_MMF_OFF_DATA) ){
				*lpDataSize=(SQ_MMF_MAX_DATA-SQ_MMF_OFF_DATA);
				memcpy( lpData,lpShareData,(SQ_MMF_MAX_DATA-SQ_MMF_OFF_DATA) );
			}
			else
				memcpy( lpData,lpShareData,*lpDataSize );
		}
	}


	if( pdwResult!=NULL )
		*pdwResult = *lpdwShareResult;

	if( pdwDevIndex!=NULL )
		*pdwDevIndex = *lpdwShareDevIndex;

	//leave critical section
	ReleaseMutex(*lphMutexLockMap);

	UnmapViewOfFile(pvShared);
	//
	return TRUE;
}


//set result to Item( static )
BOOL CTTR_ShareCQueue::SetSharedResult( 
								LPTTRSharedItem pItem,
								 DWORD dwResult,
								 LPBYTE lpData,
								 LONG lDataSize,
								 DWORD dwDevIndex
								 )
{
	return SetSharedResult( 
		pItem->hMapObj,
		dwResult,
		lpData,
		lDataSize,
		dwDevIndex
		);

}

//set result to Map object ( static )
//out : hMapObj
BOOL CTTR_ShareCQueue::SetSharedResult(
							   HANDLE hMapObj,
								DWORD dwResult,
								LPBYTE lpData,
								LONG lDataSize,
								DWORD dwDevIndex
								)
{
	DWORD dwWaitResult; 

	LPVOID pvShared=NULL;
	LPHANDLE lphMutexLockMap;
	LPDWORD lpdwShareResult;
	LPDWORD lpdwShareDevIndex;
	LPBYTE lpShareData;
	LPLONG lpShareDataSize;

	if( hMapObj==NULL )
		return FALSE;

	pvShared = MapViewOfFile(
					hMapObj,        // File Map obj to view
					FILE_MAP_WRITE,   // Read/Write access
					0,                // high: map from beginning
					0,                // low:
					0);               // default: map entire file
	if( pvShared == NULL ){
		return FALSE;
	}

	//
	lphMutexLockMap = ((LPHANDLE)pvShared + SQ_MMF_OFF_HMUTEX );
	lpdwShareDevIndex = ((LPDWORD)pvShared + SQ_MMF_OFF_DEVINDEX );
	lpdwShareResult = (LPDWORD)( (LPBYTE)(pvShared)+SQ_MMF_OFF_RESULT );
	lpShareDataSize = (LPLONG)( (LPBYTE)(pvShared)+SQ_MMF_OFF_SIZE );
	lpShareData = (LPBYTE)( (LPBYTE)(pvShared)+SQ_MMF_OFF_DATA );

	if( *lphMutexLockMap==NULL )
		return FALSE;

	//////////////////////////////////////////

	dwWaitResult=WaitForSingleObject(
		*lphMutexLockMap,		// handle to mutex
		TTR_SHARE_CQ_DEF_TIMEOUT);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		return FALSE;
	}
	//

	//enter critical section
	*lpShareDataSize=lDataSize;

	if( lpData!=NULL ){
		if( lDataSize>0 ){
			if( lDataSize> (SQ_MMF_MAX_DATA-SQ_MMF_OFF_DATA) ){
				lDataSize= (SQ_MMF_MAX_DATA-SQ_MMF_OFF_DATA);
				memcpy( lpShareData,lpData,(SQ_MMF_MAX_DATA-SQ_MMF_OFF_DATA) );
			}
			else
				memcpy( lpShareData,lpData,lDataSize );
		}
	}

	*lpdwShareResult=dwResult;
	*lpdwShareDevIndex=dwDevIndex;
	//
	//leave critical section
	ReleaseMutex(*lphMutexLockMap);

	UnmapViewOfFile(pvShared);

	//
	return TRUE;
}

//Create shared Item( static )
LPTTRSharedItem CTTR_ShareCQueue::CreateSharedCQItem()
{
	LPTTRSharedItem pShare=NULL;
	HANDLE hMapObj;
	HANDLE hMutexLockMap;
	LPHANDLE lphMutexLockMap;
	LPVOID pvShared=NULL;

	//
	pShare = new TTRSharedItem;

	memset( pShare,0,sizeof(TTRSharedItem) );


	// Create a none named file mapping object.
	hMapObj = CreateFileMapping(
				INVALID_HANDLE_VALUE,  // Use paging file
				NULL,                 // No security attributes
				PAGE_READWRITE,       // Read/Write access
				0,                    // Mem Size: high 32 bits
				SQ_MMF_MAX_DATA,          // Mem Size: low 32 bits
				NULL);   // Name of map object

	if( hMapObj==NULL ){
		delete( pShare );
		return NULL;
	}

	hMutexLockMap = CreateMutex( NULL,FALSE,NULL );//NO owner. No name
	if( hMutexLockMap==NULL ){
		CloseHandle( hMapObj );
		delete( pShare );
		return NULL;
	}

	pvShared = MapViewOfFile(
				hMapObj,        // File Map obj to view
				FILE_MAP_WRITE,   // Read/Write access
				0,                // high: map from beginning
				0,                // low:
				0);               // default: map entire file

	if( pvShared==NULL ){
		CloseHandle( hMutexLockMap );
		CloseHandle( hMapObj );
		delete( pShare );
		return NULL;
	}
	
	lphMutexLockMap = (LPHANDLE)pvShared;

	*lphMutexLockMap = hMutexLockMap;	//save mutex handle
	pShare->hMapObj = hMapObj;			//save map file object handle

	if( !UnmapViewOfFile(pvShared) ){
		CloseHandle( hMutexLockMap );
		CloseHandle( hMapObj );
		delete( pShare );
		return NULL;
	}

	pShare->hEvent = TTR_CreateUniqueEvent( pShare->sEventName,(int)GetCurrentThreadId() );
	if( pShare->hEvent == NULL ){
		CloseHandle( hMutexLockMap );
		CloseHandle( hMapObj );
		delete( pShare );
		return NULL;
	}

	//
	return pShare;
}


//Delete shared Item( static )
BOOL CTTR_ShareCQueue::DeleteSharedCQItem( LPTTRSharedItem pItem )
{

	LPHANDLE lphMutexLockMap;
	LPVOID pvShared=NULL;

	if( pItem == NULL ){
		return FALSE;
	}

	if( pItem->hMapObj ){

		pvShared = MapViewOfFile(
					pItem->hMapObj,        // File Map obj to view
					FILE_MAP_WRITE,   // Read/Write access
					0,                // high: map from beginning
					0,                // low:
					0);               // default: map entire file

		if( pvShared==NULL ){
			return FALSE;
		}
		
		lphMutexLockMap = (LPHANDLE)pvShared;

		if( *lphMutexLockMap ){
			CloseHandle( *lphMutexLockMap );
		}

		if( !UnmapViewOfFile(pvShared) ){
			return FALSE;
		}

		CloseHandle( pItem->hMapObj );

	}

	if( pItem->hEvent ){
		CloseHandle( pItem->hEvent );
	}

	delete( pItem );

	return TRUE;

}

//save Tx data to Item 
BOOL CTTR_ShareCQueue::SetTxToShareItem( LPTTRSharedItem pItem,DWORD dwTx,LPBYTE lpbTx )
{
	if( pItem==NULL )
		return FALSE;

	if( dwTx == 0 )
		return TRUE;

	if( lpbTx == NULL )
		return FALSE;

	if( dwTx<TTR_SHARE_ITEM_DEF_BUF_SIZE ){
		memcpy( pItem->sTx,lpbTx,dwTx );	pItem->dwTx = dwTx;
	}
	else{
		memcpy( pItem->sData,lpbTx,dwTx );	pItem->dwData = dwTx;
	}

	return TRUE;
}

