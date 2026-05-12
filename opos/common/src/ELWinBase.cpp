#include "stdafx.h"
#include "ELWinBase.h"

//////////////////////////////////////////////////////////
//ELWinBase body file
//////////////////////////////////////////////////////////
#define	DEF_WAITTIME_SETUP			3000	//default wait-time for setup completion

//used in CreateModalessDialog()
typedef	struct TagMODALESS_PARA{
		LPCTSTR lpTemplatel;   // dialog box template name
		HWND hWndParent;      // handle to owner window
		DLGPROC lpDialogFunc;  // dialog box procedure
		HWND *phDlg;	//[OUT]created dialog handle
		HANDLE hSetupCompleteEvent; //thread set this event after setup has been completed.
		HANDLE hExitEvent; //thread set this event when exists thread.(terminates).
		UINT uExitMsg;	//exit user defined message

}MODALESS_PARA, *LPMODALESS_PARA,*PMODALESS_PARA;



DWORD WINAPI EL_ModalessDlgthread( LPVOID lpParam );

//
//   FUNCTION: InitUserWindowInstance(HANDLE, int)
//
//   PURPOSE: creates main window
//
//   COMMENTS:
//
//        In this function, we return window-handle and
//        create and display the main program window.
//
HWND InitUserWindowInstance(HINSTANCE hInstance, int nCmdShow,LPCTSTR lpszClassName )
{
   HWND hWnd;

   hWnd = CreateWindowEx(
	   0,//WS_EX_TRANSPARENT,//Ex-style
	   lpszClassName,
	   NULL,//set title
	   WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0,
	   CW_USEDEFAULT, 0,
	   NULL,
	   NULL,
	   hInstance,
	   NULL
	   );

   if (!hWnd)
      return hWnd;

   ShowWindow(hWnd, nCmdShow);//
   UpdateWindow(hWnd);

   return hWnd;
}

//
//   FUNCTION: InitUserListWindowInstance(HANDLE, int)
//
//   PURPOSE: creates main list window type
//
//   COMMENTS:
//
//        In this function, we return window-handle and
//        create and display the main program window.
//
HWND InitUserListWindowInstance(HINSTANCE hInstance, int nCmdShow,LPCTSTR lpszClassName )
{
	HWND hWnd;
	INITCOMMONCONTROLSEX icex;
	RECT rcl;
	LVCOLUMN lvListCol;

	// Ensure that the common control DLL is loaded. 
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 

	hWnd = CreateWindowEx(
		0L,
		WC_LISTVIEW,//lpszClassName,//WC_LISTVIEW,
		NULL,
		WS_OVERLAPPEDWINDOW | LVS_REPORT | LVS_SHOWSELALWAYS,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		NULL, 
		NULL,//(HMENU) ID_LISTVIEW,
		hInstance,
		NULL
		);
	if(hWnd==NULL)
		return NULL;

	GetClientRect(hWnd, &rcl);

	// setting list view colum.
	lvListCol.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM; 
	lvListCol.fmt = LVCFMT_LEFT; 
	lvListCol.cx = rcl.right - rcl.left; 

	// the first colum. 
	lvListCol.pszText = _T("Data information"); 
	lvListCol.iSubItem = 0; 

	ListView_InsertColumn( hWnd,0,&lvListCol );

	ShowWindow(hWnd, nCmdShow);//
	UpdateWindow(hWnd);

	return hWnd;
}

//
//In this function, main-window message loop
//
// return : GetLastError()==ERROR_SUCCESS : then returned value is wParam of message
//			GetLastError()!=ERROR_SUCCESS : then returned value equal to GetLastError()
//pfnCallBack[nCount]		->timeout-callback function
//pfnCallBack[0~nCount-1]	->event handler
LRESULT MsgLoopMultipleObjs( 
							DWORD nCount,	// number of handles in array
							CONST HANDLE *pHandles,	// object-handle array
							USERMSGHANDLER pfnCallBack[],	//callback function array
							HACCEL hAccTable,		// handle to accelerator table
							DWORD dwMilliseconds	// time-out interval
							)
{
	LRESULT lResult=0;
	DWORD dwResult;
	BOOL bIsEnd=FALSE;
	MSG msg;
	ULONG lEventResult;

	while( !bIsEnd ){
		if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE ) ){
			if( !TranslateAccelerator(msg.hwnd, hAccTable, &msg) ){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if( msg.message==WM_QUIT ){//msg.wParam is exit code.
				bIsEnd=TRUE;		lResult=(LRESULT)msg.wParam;
				SetLastError( ERROR_SUCCESS );
			}
		}
		else{
			dwResult=MsgWaitForMultipleObjects(
				nCount,			// number of handles in array
				pHandles,		// object-handle array
				FALSE,			// wait option
				dwMilliseconds,	// time-out interval(INFINITE)
				QS_ALLINPUT		// input-event type
			);
			if( dwResult==WAIT_FAILED ){
				bIsEnd=TRUE;
				lResult=(LRESULT)GetLastError();//return with Error.
				SetLastError( (DWORD)lResult );
			}
			else if( dwResult==WAIT_OBJECT_0+nCount ){
				if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE ) ){
					if( !TranslateAccelerator(msg.hwnd, hAccTable, &msg) ){
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					if( msg.message==WM_QUIT ){//msg.wParam is exit code.
						bIsEnd=TRUE;		lResult=(LRESULT)msg.wParam;
						SetLastError( ERROR_SUCCESS );
					}
				}
				else
					continue;
			}
			else if(dwResult==WAIT_TIMEOUT){
				//time-out event
				lEventResult=OBJ_EVENT_RESULT_TIME_OUT;
				if( pfnCallBack!=NULL ){
					if( pfnCallBack[nCount]!=NULL )
						pfnCallBack[nCount](lEventResult);
				}
				
			}
			else{//event signaled.
				if( dwResult>=WAIT_ABANDONED_0 ){
					//WAIT_ABANDONED_0 to (WAIT_ABANDONED_0 + nCount ? 1)
					lEventResult=OBJ_EVENT_RESULT_ABANDONED;
					if( pfnCallBack!=NULL ){
						if( pfnCallBack[dwResult]!=NULL )
							pfnCallBack[dwResult](lEventResult);
					}
				}
				else{
					//WAIT_OBJECT_0 to (WAIT_OBJECT_0 + nCount ? 1)
					lEventResult=OBJ_EVENT_RESULT_SIGNALED;
					if( pfnCallBack!=NULL ){
						if( pfnCallBack[dwResult]!=NULL )
							pfnCallBack[dwResult](lEventResult);
					}
				}
			}
		}
	}//end while

	return lResult;
}

//Registers the window class.
ATOM RegisterUserWindowClass(HINSTANCE hInstance,LPCTSTR lpszClassName,WNDPROC pWndMainProc,WNDCLASSEX *pwcex)
{
	WNDCLASSEX wcex;

	if( pwcex==NULL ){//default class style
		if( lpszClassName==NULL )
			return 0;
		//
		wcex.cbSize = sizeof(WNDCLASSEX); 

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= pWndMainProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= LoadIcon(NULL,(LPCTSTR)IDI_APPLICATION);
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= lpszClassName;
		wcex.hIconSm		= LoadIcon(NULL,(LPCTSTR)IDI_APPLICATION);

		return RegisterClassEx(&wcex);
	}
	else
		return RegisterClassEx(pwcex);
}

//
//In this function, main-modaless dialog message loop
//
// return : GetLastError()==ERROR_SUCCESS : then returned value is wParam of message
//			GetLastError()!=ERROR_SUCCESS : then returned value equal to GetLastError()
//pfnCallBack[nCount]		->timeout-callback function
//pfnCallBack[0~nCount-1]	->event handler
LRESULT DlgMsgLoopMultipleObjs( 
							HWND hDlg,//dialog handle
							DWORD nCount,	// number of handles in array
							CONST HANDLE *pHandles,	// object-handle array
							USERMSGHANDLER pfnCallBack[],	//callback function array
							HACCEL hAccTable,		// handle to accelerator table
							DWORD dwTOutArrary[],	// time-out interval arrary. dwTOutArrary[nCount] is default value
							BOOL bConsiderTimeOut,	// TRUE :dwMilliseconds includes window-message processing time.
							BOOL bIsDisableESC_ENTER	//Disable exit bt ESC or Enter key
							)
{
	LRESULT lResult=0;
	DWORD dwResult=0;
	BOOL bIsEnd=FALSE;
	MSG msg;
	ULONG lEventResult;
	BOOL bResult;
	DWORD dwCurTimeOut=0;
	DWORD dwStartTime=0;
	DWORD dwProcessTime=0;
	BOOL bProcessMsg=TRUE;
	int nCurTOutIndex=0;//current index of timeout-arrary.(dwTOutArrary)
	LRESULT lCBResult=0;//callback function result.

	nCurTOutIndex=nCount;
	dwCurTimeOut=dwTOutArrary[nCurTOutIndex];//set default timeout value

	while( !bIsEnd ){
		//pre-message pump
		if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE )==TRUE ){
			/////////////////////////////////
			//window message processing part.
			if( bConsiderTimeOut )
				dwStartTime=GetTickCount();

			bProcessMsg=TRUE;

			if( bIsDisableESC_ENTER ){
				if( msg.message==WM_KEYDOWN ){
					if( msg.wParam==VK_ESCAPE ){
						bProcessMsg=FALSE;
					}
					else if( msg.wParam==VK_RETURN ){
						bProcessMsg=FALSE;
					}
				}
			}

			if( bProcessMsg ){
				if( !IsWindow( hDlg ) || !IsDialogMessage( hDlg,&msg) ){
					//check dialog message.......
					if( hAccTable!=NULL )
						bResult=TranslateAccelerator( hDlg,hAccTable,&msg);
					else
						bResult=FALSE;

					if( !bResult ){
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					if( msg.message==WM_QUIT ){//msg.wParam is exit code.
						bIsEnd=TRUE;		lResult=(LRESULT)msg.wParam;
						SetLastError( ERROR_SUCCESS );
					}
				}
			}

			if( bConsiderTimeOut ){
				dwProcessTime=dwProcessTime+(GetTickCount()-dwStartTime);

				if( dwProcessTime>dwTOutArrary[nCurTOutIndex] )
					dwCurTimeOut=0;//over processing time greater then time-out
			}
		}
		else{
			/////////////////////////////////
			//event processing part.
			if( bConsiderTimeOut )
				dwStartTime=GetTickCount();
			//

			dwResult=MsgWaitForMultipleObjects(
				nCount,			// number of handles in array
				pHandles,		// object-handle array
				FALSE,			// wait option
				dwCurTimeOut,	// time-out interval(INFINITE)
				QS_ALLINPUT		// input-event type
			);
			if( dwResult==WAIT_FAILED ){
				bIsEnd=TRUE;
				lResult=(LRESULT)GetLastError();//return with Error.
				SetLastError( (DWORD)lResult );
			}
			else if( dwResult==WAIT_OBJECT_0+nCount ){

				if( PeekMessage( &msg, NULL, 0, 0,PM_REMOVE )==TRUE ){
					
					bProcessMsg=TRUE;

					if( bIsDisableESC_ENTER ){
						if( msg.message==WM_KEYDOWN ){
							if( msg.wParam==VK_ESCAPE ){
								bProcessMsg=FALSE;
							}
							else if( msg.wParam==VK_RETURN ){
								bProcessMsg=FALSE;
							}
						}
					}
					
					if( bProcessMsg ){
						if( !IsWindow( hDlg ) || !IsDialogMessage( hDlg,&msg) ){
							//check dialog message.......

							if( hAccTable!=NULL )
								bResult=TranslateAccelerator( hDlg,hAccTable,&msg);
							else
								bResult=FALSE;

							if( !bResult ){
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
							if( msg.message==WM_QUIT ){//msg.wParam is exit code.
								bIsEnd=TRUE;		lResult=(LRESULT)msg.wParam;
								SetLastError( ERROR_SUCCESS );
							}
						}
					}
				}

				if( bConsiderTimeOut ){
					dwProcessTime=dwProcessTime+(GetTickCount()-dwStartTime);

					if( dwProcessTime>dwTOutArrary[nCurTOutIndex] )
						dwCurTimeOut=0;//over processing time greater then time-out
				}
			}
			else if(dwResult==WAIT_TIMEOUT){
				//time-out event
				lEventResult=OBJ_EVENT_RESULT_TIME_OUT;
				if( pfnCallBack!=NULL ){
					if( pfnCallBack[nCount]!=NULL ){
						lCBResult=pfnCallBack[nCount](lEventResult);
						//change current time value
						if( lCBResult==CB_RET_REQ_DEF_TIMEOUT )
							nCurTOutIndex=nCount;
						else if( lCBResult==CB_RET_REQ_SEF_TIMEOUT )
							nCurTOutIndex=nCount;
					}
				}

				dwCurTimeOut=dwTOutArrary[nCurTOutIndex];//reset timeout to default
				dwProcessTime=0;			//reset processing time
				
			}
			else{//event signaled.
				if( dwResult>=WAIT_ABANDONED_0 ){
					//WAIT_ABANDONED_0 to (WAIT_ABANDONED_0 + nCount ? 1)
					lEventResult=OBJ_EVENT_RESULT_ABANDONED;
					if( pfnCallBack!=NULL ){
						if( pfnCallBack[dwResult]!=NULL ){
							lCBResult=pfnCallBack[dwResult](lEventResult);
							//change current time value
							if( lCBResult==CB_RET_REQ_DEF_TIMEOUT )
								nCurTOutIndex=nCount;
							else if( lCBResult==CB_RET_REQ_SEF_TIMEOUT )
								nCurTOutIndex=dwResult;
						}
					}
				}
				else{
					//WAIT_OBJECT_0 to (WAIT_OBJECT_0 + nCount ? 1)
					lEventResult=OBJ_EVENT_RESULT_SIGNALED;
					if( pfnCallBack!=NULL ){
						if( pfnCallBack[dwResult]!=NULL ){
							lCBResult=pfnCallBack[dwResult](lEventResult);
							//change current time value
							if( lCBResult==CB_RET_REQ_DEF_TIMEOUT )
								nCurTOutIndex=nCount;
							else if( lCBResult==CB_RET_REQ_SEF_TIMEOUT )
								nCurTOutIndex=dwResult;
						}
					}
				}

				dwCurTimeOut=dwTOutArrary[nCurTOutIndex];//reset timeout to default
				dwProcessTime=0;			//reset processing time
			}
		}
	}//end while

	return lResult;
}


void WindowMoveCenter( HWND hDlg )
{
	HWND hWndOwner;
	RECT rc, rcDlg, rcOwner;

	//...........................................
	//moves dialog to the center of screen.......
	//...........................................
	// Get the owner window and dialog box rectangles. 
	if((hWndOwner=GetParent(hDlg)) == NULL){
		hWndOwner=GetDesktopWindow();
	}

	GetWindowRect(hWndOwner, &rcOwner);
	GetWindowRect(hDlg, &rcDlg);
	CopyRect(&rc, &rcOwner); 

	// Offset the owner and dialog box rectangles so that 
	// right and bottom values represent the width and 
	// height, and then offset the owner again to discard 
	// space taken up by the dialog box. 

	OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
	OffsetRect(&rc, -rc.left, -rc.top); 
	OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

	// The new position is the sum of half the remaining 
	// space and the owner's original position. 

	SetWindowPos( hDlg,HWND_TOP,
		rcOwner.left + (rc.right / 2),
		rcOwner.top + (rc.bottom / 2),
		0,0,// ignores size arguments
		SWP_NOSIZE); 

}


//Create modaless dialog(create thread. for dialog )
HWND CreateModalessDialog( 
			LPCTSTR lpTemplate,   // dialog box template name
			HWND hWndParent,      // handle to owner window
			DLGPROC lpDialogFunc,  // dialog box procedure
			UINT uExitMsg,			//exit user defined message
			HANDLE hEXitEvent			//exit announcement event		
			)
{
	HWND hWnd=NULL;
	HANDLE hThread=NULL;
	DWORD dwThreadId=0;
	MODALESS_PARA para;
	HANDLE hEvent=NULL;
	DWORD dwResult=0;

	hEvent=CreateEvent( NULL,TRUE,FALSE,NULL );

	para.lpTemplatel=lpTemplate;
	para.hWndParent=hWndParent;
	para.lpDialogFunc=lpDialogFunc;
	para.hSetupCompleteEvent=hEvent;
	para.phDlg=&hWnd;
	para.uExitMsg=uExitMsg;
	para.hExitEvent=hEXitEvent;

	hThread=CreateThread( NULL,0,EL_ModalessDlgthread,&para,0,&dwThreadId );

	if( hThread!=NULL ){
		//wait for setup completion
		dwResult=WaitForSingleObject( hEvent,INFINITE );//DEF_WAITTIME_SETUP );
		if( dwResult==WAIT_OBJECT_0 ){
			//creatd dialog has been saved to hWnd
		}
	
		CloseHandle( hThread );
	}

	//
	CloseHandle( hEvent );
	return hWnd;
}

DWORD WINAPI EL_ModalessDlgthread( LPVOID lpParam )
{
	DWORD dwResult=0;
	PMODALESS_PARA pPara=NULL;
	HWND hDlg=NULL;
	HWND hParentDlg=NULL;
	HANDLE hExit;
	UINT uExitMsg;
	DWORD dwTOutArrary[1];

	//get parameters
	pPara=(PMODALESS_PARA)lpParam;

	//
	hDlg=*(pPara->phDlg)=CreateDialog( NULL,pPara->lpTemplatel,NULL,pPara->lpDialogFunc );

	hParentDlg=pPara->hWndParent;//save parents window handle for commuication.
	hExit=pPara->hExitEvent;// exits event
	uExitMsg=pPara->uExitMsg;

	//
	SetEvent(pPara->hSetupCompleteEvent);//announce setup-completion.

	ShowWindow( hDlg,SW_SHOW );

	//set default timeout
	dwTOutArrary[0]=INFINITE;

	dwResult=(LRESULT)DlgMsgLoopMultipleObjs( 
							hDlg,//dialog handle
							0,	// number of handles in array
							NULL,	// object-handle array
							NULL,	//callback function array
							NULL,		// handle to accelerator table
							dwTOutArrary,	// time-out interval
							FALSE,	// TRUE :dwMilliseconds includes window-message processing time.
							TRUE	//Disable exit bt ESC or Enter key
							);

	/////////////////////////////
	//announce terminate thread.
	PostMessage( hParentDlg,uExitMsg,0,0 );

	if( hExit!=NULL )
		SetEvent( hExit );

	return dwResult;
}