////////////////////////////////////////////////////////////////
//ELListCtl.cpp

//Win32 list control support function body

#include "stdafx.h"
#include "ELListCtl.h"

#define	EL_LIST_MAX_LINE			2000	//the maximum linbe number of listbox.


void AddStringToList( HWND hListBox,LPCTSTR lpctMsg )
{
	int nTotal;
	//int nHSize;


	//Declare the following variables
	DWORD dwExtentNew,dwExtentOld;
	HDC hDCListBox;
	HFONT hFontOld,hFontNew;
	TEXTMETRIC tm;
	SIZE StrSize;

	if( GetCountOfList( hListBox ) > EL_LIST_MAX_LINE )
		ResetContent( hListBox );//reset list contents
	//
	//Use GetDC to retrieve the handle to the display context for the list box and store it in hDCListBox
	hDCListBox = GetDC( hListBox );
	//Send the list box a WM_GETFONT message to retrieve the handle to the font that the list box is using, and store this handle in hFontNew
	hFontNew = (HFONT)SendMessage(hListBox, WM_GETFONT, NULL, NULL);
	//Use SelectObject to select the font into the display context. Retain the return value from the SelectObject call in hFontOld
	hFontOld = (HFONT)SelectObject(hDCListBox, hFontNew);
	//Call GetTextMetrics to get additional information about the font being used
	GetTextMetrics(hDCListBox, (LPTEXTMETRIC)&tm);
	//For each string, the value of the extent to be used is calculated as follows

	GetTextExtentPoint32( hDCListBox,lpctMsg, _tcslen(lpctMsg),&StrSize );
	dwExtentNew = (DWORD)(StrSize.cx+tm.tmAveCharWidth);
	/*
	dwExtentNew = (DWORD)GetTextExtent(hDCListBox, lpctMsg, _tcslen(lpctMsg))
		+ tm.tmAveCharWidth;
	*/
	//After all the extents have been calculated, select the old font back into hDCListBox and then release it
	SelectObject(hDCListBox, hFontOld);
	ReleaseDC(hListBox, hDCListBox);


	//get current horizontal size
	dwExtentOld=(DWORD)SendMessage( hListBox,LB_GETHORIZONTALEXTENT,0,0 );

	if( dwExtentNew>dwExtentOld )
		SendMessage( hListBox,LB_SETHORIZONTALEXTENT,(WPARAM)dwExtentNew,0 );
	//
	SendDlgItemMessage( GetParent(hListBox),GetDlgCtrlID(hListBox),LB_ADDSTRING,0,(LPARAM)lpctMsg );

	//get total item
	nTotal=SendMessage( hListBox,LB_GETCOUNT,0,0 );
	if( nTotal>0 ){
		SendMessage( hListBox,LB_SETCURSEL,(WPARAM)nTotal-1,0 );
	}

}

void ResetContent( HWND hCtl )
{
	SendMessage( hCtl,LB_RESETCONTENT,0,0 );

	SendMessage(hCtl, LB_SETHORIZONTALEXTENT, 0, 0L);

	//This is required to remove the scrollbar.
	SendMessage(hCtl, LB_DELETESTRING, 0, 0L);
}

//get count list
int GetCountOfList( HWND hListBox )
{
	return SendMessage( hListBox,LB_GETCOUNT,0,0 );
}


int GetTextOfList( HWND hListBox,int nIndex,TCHAR *psString )
{
	int nLen;

	nLen=(int)SendMessage( hListBox,LB_GETTEXT,nIndex,(LPARAM)psString );

	return nLen;
}

