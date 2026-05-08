#include "StdAfx.h"
#include "DetectWnd.h"
#include "TTR_Dev.h"

CDetectWnd*CDetectWnd::GetInstance( HINSTANCE hInstance /*=NULL*/, bool bFree /*= false*/ )
{
	/*
	static CDetectWnd Wnd;

	//
	if( hInstance ){
		Wnd.m_hInstance = hInstance;
	}

	return &Wnd;
	*/

	static CDetectWnd *pWnd = NULL;

	if( bFree ) {

		if( pWnd ){
			delete pWnd;
			pWnd = NULL;
		}
	}
	else{

		if( pWnd == NULL )
			pWnd = new CDetectWnd();
		//
		if( hInstance ){
			pWnd->m_hInstance = hInstance;
		}
	}

	return pWnd;

}


CDetectWnd::CDetectWnd(void) : 
	m_hDevNotify(NULL),
	m_atomClass(NULL),
	m_hInstance(NULL),
	m_hWnd(NULL),
	m_lpCBParameter(NULL)
{
	ATLTRACE( _T(" $ CDetectWnd()::CDetectWnd().\n") );
}


CDetectWnd::~CDetectWnd(void)
{
	Unregister();

	ATLTRACE( _T(" $ CDetectWnd()::~CDetectWnd().\n") );
}

bool CDetectWnd::Register()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof (WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DetectWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = 0;
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = _T("Stand By Detector");
	wcex.hIconSm = 0;
	m_atomClass = RegisterClassEx(&wcex);

	if( m_atomClass ){
		HWND hWnd = ::CreateWindow(_T("Stand By Detector"), NULL, WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							NULL, NULL, m_hInstance, NULL);

		if( hWnd ){
			m_hWnd = hWnd;

			m_hDevNotify = CTTR_Device::RegisterEventDevice( hWnd, CTTR_Device::CLASS_GUID_USB );

			return true;
		}

	}

	return false;
}

bool CDetectWnd::Unregister()
{
	bool bResult = false;

	if( m_hWnd ){

		if( m_hDevNotify )
			CTTR_Device::UnRegisterEventDevice( m_hDevNotify );

		if( ::DestroyWindow( m_hWnd ) ){
			m_hWnd = NULL;
			bResult = true;
		}
	}

	return bResult;
}


LRESULT CALLBACK CDetectWnd::DetectWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	pair<UINT,WPARAM> Evt(Msg,wParam);

	CBType EventCallBack = NULL;

	CDetectWnd *pDetectWnd = CDetectWnd::GetInstance();

	EventCallBack = pDetectWnd->m_mapEvtToCallBack[Evt];

	if( EventCallBack ){
		EventCallBack( pDetectWnd->getCBParameter() );
	}

	if( Msg == WM_DESTROY ){

	}

	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

bool CDetectWnd::Bump()
{
	MSG Msg;

	if( ::PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE ) ){
		::TranslateMessage(&Msg);
		::DispatchMessage(&Msg);
		return true;
	}
	else
		return false;
}

void CDetectWnd::AddEventCallBack( UINT Msg, WPARAM wParam, CBType pCB )
{
	m_mapEvtToCallBack[ make_pair(Msg,wParam) ] = pCB;
}

void CDetectWnd::RemoveEventCallBack( UINT Msg, WPARAM wParam )
{
	m_mapEvtToCallBack.erase( make_pair(Msg,wParam) );
}
