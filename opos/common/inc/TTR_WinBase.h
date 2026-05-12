//////////////////////////////////////////////////////////
//ELWinBase header file
//////////////////////////////////////////////////////////

#if !defined(__TTR_WIN_BASE_HEADER_20091112__)
#define __TTR_WIN_BASE_HEADER_20091112__

#include "Windows.h"
#include "commctrl.h"	//you must link "comctl32.lib"


//
//definition constant
//MsgLoopMultipleObjs return value
#define	OBJ_EVENT_RESULT_TIME_OUT		0
#define	OBJ_EVENT_RESULT_SIGNALED		1
#define	OBJ_EVENT_RESULT_ABANDONED		2


//
//this is call back function type will be used on parameter of MsgLoopMultipleObjs() and DlgMsgLoopMultipleObjs()
//if this function is used on DlgMsgLoopMultipleObjs()-parameter,
//return value has a specific meaning.
//meaning of return-value
//=======================================================
// value		descrpition
//=======================================================
//
//return value-defintion

#define	CB_RET_REQ_DEF_TIMEOUT			0	//Set current timeout-value to default timeout-value.
#define	CB_RET_REQ_SEF_TIMEOUT			-1	//Set current timeout-value to timeout-value of this callback function.
typedef LRESULT (CALLBACK* USERMSGHANDLER)(ULONG_PTR);


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
HWND InitUserWindowInstance(HINSTANCE hInstance, INT nCmdShow,LPCTSTR lpszClassName );

HWND InitUserListWindowInstance(HINSTANCE hInstance, INT nCmdShow,LPCTSTR lpszClassName );

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
							);


//Registers the window class.
ATOM RegisterUserWindowClass(HINSTANCE hInstance,LPCTSTR lpszClassName,WNDPROC pWndMainProc,WNDCLASSEX *pwcex);

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
							);

//window moves to the center of screen.
void WindowMoveCenter( HWND hDlg );


//Create modaless dialog(create thread. for dialog )
HWND CreateModalessDialog( 
			LPCTSTR lpTemplate,   // dialog box template name
			HWND hWndParent,      // handle to owner window
			DLGPROC lpDialogFunc,  // dialog box procedure
			UINT uExitMsg,			//exit user defined message
			HANDLE hEXitEvent		//exit announcement event		
			);


#endif//__TTR_WIN_BASE_HEADER_20091112__
