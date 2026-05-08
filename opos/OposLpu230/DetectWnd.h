#pragma once

#include <map>

using namespace std;


class CDetectWnd
{

public:
	typedef	void (CALLBACK* CBType)( LPVOID lpCBParameter );

public:
	virtual ~CDetectWnd(void);

	static CDetectWnd *GetInstance( HINSTANCE hInstance = NULL, bool bFree = false );
	bool Bump();

	void AddEventCallBack( UINT Msg, WPARAM wParam, CBType pCB );
	void RemoveEventCallBack( UINT Msg, WPARAM wParam );

	LPVOID getCBParameter(){		return m_lpCBParameter;	}
	void setCBParameter( LPVOID lpCBParameter ){	m_lpCBParameter = lpCBParameter; }

	bool Register();
	bool Unregister();

private:
	CDetectWnd(void);
	CDetectWnd( const CDetectWnd & );


	static LRESULT CALLBACK DetectWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

private:
	HDEVNOTIFY m_hDevNotify;

	ATOM m_atomClass;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	map< pair<UINT,WPARAM>, CBType> m_mapEvtToCallBack;

	LPVOID m_lpCBParameter;

};

