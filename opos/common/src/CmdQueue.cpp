// CmdQueue.cpp: implementation of the CCmdQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdio.h"
#include "CmdQueue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmdQueue::CCmdQueue()
{	
}

CCmdQueue::~CCmdQueue()
{
}

BOOL CCmdQueue::NewClass(int nindex)
{
	TCHAR sEventName[EVENT_NAME_SIZE];

	//Inital queue.
	m_nNextReadPos=0;	
	m_nNextWritePos=0;
	//
	m_lRXSize=0;

	_stprintf( sEventName,_T("ShareClass_QueueAcc_Mutex_20091102_%d"),nindex );
	_tcscpy( m_sMutexSyncName,sEventName );
	//
	m_hMutexSync=CreateMutex(NULL, FALSE, m_sMutexSyncName);   // Create mutex
	if( m_hMutexSync==NULL )
		return FALSE;

	_stprintf( sEventName,_T("ShareClass_QueueRes_Mutex_20091102_%d"),nindex );
	_tcscpy( m_sMutexResultName,sEventName );
	m_hMutexResult=CreateMutex(NULL, FALSE, m_sMutexResultName);   // Create mutex
	if( m_hMutexResult==NULL ){
		CloseHandle( m_hMutexSync );
		return FALSE;
	}
	else{
		return TRUE;
	}
}

BOOL CCmdQueue::DeleteClass()
{
	ReleaseMutex(m_hMutexSync);
	CloseHandle(m_hMutexSync);
	return TRUE;
}

BOOL CCmdQueue::IsEmpty()	//Check
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}

	if( m_nNextReadPos==m_nNextWritePos ){
		m_nNextReadPos=m_nNextWritePos=0;	//reset Ver1.7
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return TRUE;
	}
	else{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
}
//

void CCmdQueue::Empty()	//clear queue.
{
	//EnterCriticalSection(&(m_csSync));
	//
	DWORD dwWaitResult; 
	HANDLE hMutex;
	
	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return;
	}

	m_nNextReadPos=m_nNextWritePos=0;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
}
//

BOOL CCmdQueue::EnQueue( CmdItem &Item )		//add item to queue
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}

	m_CirQueue[m_nNextWritePos].cMajCmd=Item.cMajCmd;
	m_CirQueue[m_nNextWritePos].cMinCmd=Item.cMinCmd;
	m_CirQueue[m_nNextWritePos].cInfo=Item.cInfo;
	m_CirQueue[m_nNextWritePos].cPara=Item.cPara;
	m_CirQueue[m_nNextWritePos].dwDevIndex=Item.dwDevIndex;
	memcpy( m_CirQueue[m_nNextWritePos].pData,Item.pData,ADD_INFO_ITEM_BUFFER_SIZE);
	memcpy( m_CirQueue[m_nNextWritePos].pBuf,Item.pBuf,ITEM_BUFFER_SIZE);
	m_CirQueue[m_nNextWritePos].dwSize=Item.dwSize;
	memcpy( m_CirQueue[m_nNextWritePos].pBuf1,Item.pBuf1,ITEM_BUFFER_SIZE);
	m_CirQueue[m_nNextWritePos].dwSize1=Item.dwSize1;
	m_CirQueue[m_nNextWritePos].hWnd=Item.hWnd;
	m_CirQueue[m_nNextWritePos].nMsg=Item.nMsg;
	m_CirQueue[m_nNextWritePos].nMsg1=Item.nMsg1;
	m_CirQueue[m_nNextWritePos].pCallBackFun=Item.pCallBackFun;
	memcpy( m_CirQueue[m_nNextWritePos].sEventName,Item.sEventName,sizeof(TCHAR)*EVENT_NAME_SIZE );
	//
	if( m_nNextWritePos==(MAX_CMD_QUEUE_SIZE-1) )
		m_nNextWritePos=0;
	else
		m_nNextWritePos++;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return TRUE;
}

BOOL CCmdQueue::EnQueueFirst( CmdItem &Item )		//add item to the starting point of queue
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}

	//
	m_nNextReadPos=m_nNextWritePos=0;	//reset queue.
	m_CirQueue[m_nNextWritePos].cMajCmd=Item.cMajCmd;
	m_CirQueue[m_nNextWritePos].cMinCmd=Item.cMinCmd;
	m_CirQueue[m_nNextWritePos].cInfo=Item.cInfo;
	m_CirQueue[m_nNextWritePos].cPara=Item.cPara;
	m_CirQueue[m_nNextWritePos].dwDevIndex=Item.dwDevIndex;
	memcpy( m_CirQueue[m_nNextWritePos].pData,Item.pData,ADD_INFO_ITEM_BUFFER_SIZE);
	memcpy( m_CirQueue[m_nNextWritePos].pBuf,Item.pBuf,ITEM_BUFFER_SIZE);
	m_CirQueue[m_nNextWritePos].dwSize=Item.dwSize;
	memcpy( m_CirQueue[m_nNextWritePos].pBuf1,Item.pBuf1,ITEM_BUFFER_SIZE);
	m_CirQueue[m_nNextWritePos].dwSize1=Item.dwSize1;
	m_CirQueue[m_nNextWritePos].hWnd=Item.hWnd;
	m_CirQueue[m_nNextWritePos].nMsg=Item.nMsg;
	m_CirQueue[m_nNextWritePos].nMsg1=Item.nMsg1;
	m_CirQueue[m_nNextWritePos].pCallBackFun=Item.pCallBackFun;
	memcpy( m_CirQueue[m_nNextWritePos].sEventName,Item.sEventName,sizeof(TCHAR)*EVENT_NAME_SIZE );
	//
	if( m_nNextWritePos==(MAX_CMD_QUEUE_SIZE-1) )
		m_nNextWritePos=0;
	else
		m_nNextWritePos++;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return TRUE;
}
//

//

BOOL CCmdQueue::DeQueue( LPCmdItem pItem )	//Gets item from queue.
{
	HANDLE hMutex;
	DWORD dwWaitResult; 

	if( pItem==NULL )
		return FALSE;
	//
	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
	//
	pItem->cMajCmd=m_CirQueue[m_nNextReadPos].cMajCmd;
	pItem->cMinCmd=m_CirQueue[m_nNextReadPos].cMinCmd;
	pItem->cInfo=m_CirQueue[m_nNextReadPos].cInfo;
	pItem->cPara=m_CirQueue[m_nNextReadPos].cPara;
	pItem->dwDevIndex=m_CirQueue[m_nNextWritePos].dwDevIndex;
	if( pItem->pData!=NULL )
		memcpy(pItem->pData,m_CirQueue[m_nNextReadPos].pData,ADD_INFO_ITEM_BUFFER_SIZE);
	if( pItem->pBuf!=NULL )
		memcpy(pItem->pBuf,m_CirQueue[m_nNextReadPos].pBuf,ITEM_BUFFER_SIZE);
	pItem->dwSize=m_CirQueue[m_nNextReadPos].dwSize;
	if( pItem->pBuf1!=NULL )
		memcpy(pItem->pBuf1,m_CirQueue[m_nNextReadPos].pBuf1,ITEM_BUFFER_SIZE);
	pItem->dwSize1=m_CirQueue[m_nNextReadPos].dwSize1;
	pItem->hWnd=m_CirQueue[m_nNextReadPos].hWnd;
	pItem->nMsg=m_CirQueue[m_nNextReadPos].nMsg;
	pItem->nMsg1=m_CirQueue[m_nNextReadPos].nMsg1;
	pItem->pCallBackFun=m_CirQueue[m_nNextReadPos].pCallBackFun;

	if( pItem->sEventName!=NULL )
		_tcscpy(pItem->sEventName,m_CirQueue[m_nNextReadPos].sEventName);
	//
	if( m_nNextReadPos==(MAX_CMD_QUEUE_SIZE-1) )
		m_nNextReadPos=0;
	else
		m_nNextReadPos++;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return TRUE;
}
//

//Gets item from queue. but Read position counter isn't increase.
BOOL CCmdQueue::CheckQueue( LPCmdItem pItem )
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	if( pItem==NULL )
		return FALSE;
	//
	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
	//
	//
	pItem->cMajCmd=m_CirQueue[m_nNextReadPos].cMajCmd;
	pItem->cMinCmd=m_CirQueue[m_nNextReadPos].cMinCmd;
	pItem->cInfo=m_CirQueue[m_nNextReadPos].cInfo;
	pItem->cPara=m_CirQueue[m_nNextReadPos].cPara;
	pItem->dwDevIndex=m_CirQueue[m_nNextWritePos].dwDevIndex;
	if( pItem->pData!=NULL )
		memcpy(pItem->pData,m_CirQueue[m_nNextReadPos].pData,ADD_INFO_ITEM_BUFFER_SIZE);
	if( pItem->pBuf!=NULL )
		memcpy(pItem->pBuf,m_CirQueue[m_nNextReadPos].pBuf,ITEM_BUFFER_SIZE);
	pItem->dwSize=m_CirQueue[m_nNextReadPos].dwSize;
	if( pItem->pBuf1!=NULL )
		memcpy(pItem->pBuf1,m_CirQueue[m_nNextReadPos].pBuf1,ITEM_BUFFER_SIZE);
	pItem->dwSize1=m_CirQueue[m_nNextReadPos].dwSize1;
	pItem->hWnd=m_CirQueue[m_nNextReadPos].hWnd;
	pItem->nMsg=m_CirQueue[m_nNextReadPos].nMsg;
	pItem->nMsg1=m_CirQueue[m_nNextReadPos].nMsg1;
	pItem->pCallBackFun=m_CirQueue[m_nNextReadPos].pCallBackFun;

	if( pItem->sEventName!=NULL )
		_tcscpy(pItem->sEventName,m_CirQueue[m_nNextReadPos].sEventName);
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return TRUE;
}

//Gets MajCmd-item from queue. but Read position counter isn't increase.
BOOL CCmdQueue::CheckMajItem( LPBYTE pcMajCmd )
{
	DWORD dwWaitResult;
	HANDLE hMutex;

	if( pcMajCmd==NULL )
		return FALSE;
	//
	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
	//
	*pcMajCmd=m_CirQueue[m_nNextReadPos].cMajCmd;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return TRUE;
}

//Gets MinCmd-item from queue. but Read position counter isn't increase.
BOOL CCmdQueue::CheckMinQueue( LPBYTE pcMinCmd )
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	if( pcMinCmd==NULL )
		return FALSE;
	//
	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexSyncName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
	//
	*pcMinCmd=m_CirQueue[m_nNextReadPos].cMinCmd;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return TRUE;
}

BOOL CCmdQueue::GetResult( DWORD *pdwResult,LPBYTE lpData,LONG* plDataSize,DWORD *pdwDevIndex )
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexResultName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
	//
	if( plDataSize!=NULL && lpData!=NULL ){
		*plDataSize=m_lRXSize;
		if( m_lRXSize>0 ){
			if( m_lRXSize>ITEM_BUFFER_SIZE ){
				*plDataSize=ITEM_BUFFER_SIZE;
				memcpy( lpData,m_sRXData,ITEM_BUFFER_SIZE );
			}
			else
				memcpy( lpData,m_sRXData,m_lRXSize );
		}
	}

	if( pdwResult!=NULL )
		*pdwResult=m_dwResult;
	if( pdwDevIndex!=NULL )
		*pdwDevIndex=m_dwDevIndex;

	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	//
	return TRUE;
}


BOOL CCmdQueue::SetResult( DWORD dwResult,LPBYTE lpData,LONG lDataSize,DWORD dwDevIndex )
{
	DWORD dwWaitResult; 
	HANDLE hMutex;

	hMutex=OpenMutex(
		SYNCHRONIZE,  // access
		TRUE,    // inheritance option
		m_sMutexResultName          // object name
	);
	if( hMutex==NULL )
		return FALSE;

	dwWaitResult=WaitForSingleObject(
		hMutex,		// handle to mutex
		5000L);		// five-second time-out interval

	if(dwWaitResult!=WAIT_OBJECT_0){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return FALSE;
	}
	//
	m_lRXSize=lDataSize;
	if( lpData!=NULL ){
		if( m_lRXSize>0 ){
			if( m_lRXSize>ITEM_BUFFER_SIZE ){
				m_lRXSize=ITEM_BUFFER_SIZE;
				memcpy( m_sRXData,lpData,ITEM_BUFFER_SIZE );
			}
			else
				memcpy( m_sRXData,lpData,m_lRXSize );
		}
	}

	m_dwResult=dwResult;
	m_dwDevIndex=dwDevIndex;
	//
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	//
	return TRUE;
}