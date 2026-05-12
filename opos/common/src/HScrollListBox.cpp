// HScrollListBox.cpp : implementation file
//

#include "stdafx.h"
//#include "Manager.h"
#include "HScrollListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHScrollListBox

CHScrollListBox::CHScrollListBox()
{
}

CHScrollListBox::~CHScrollListBox()
{
}


BEGIN_MESSAGE_MAP(CHScrollListBox, CListBox)
	//{{AFX_MSG_MAP(CHScrollListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_MESSAGE(LB_ADDSTRING, OnAddString)
	ON_MESSAGE(LB_INSERTSTRING, OnInsertString)
	ON_MESSAGE(LB_DELETESTRING, OnDeleteString)
	ON_MESSAGE(LB_DIR, OnDir)
	ON_MESSAGE(LB_RESETCONTENT, OnResetContent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHScrollListBox message handlers

void CHScrollListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	if ((int)lpDrawItemStruct->itemID < 0) 
	return; 

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC); 

	COLORREF crText; 
	CString sText; 
	COLORREF crNorm = (COLORREF)lpDrawItemStruct->itemData; // Color information is in item data. 
	COLORREF crHilite = RGB(255-GetRValue(crNorm), 255-GetGValue(crNorm), 255-GetBValue(crNorm)); 

	// If item has been selected, draw the highlight rectangle using the item's color. 
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) && 
		(lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)) && 
		!(this->GetStyle() & LBS_NOSEL)) { 
		CBrush brush(crNorm); 

		//CBrush brush(crHilite); 
		pDC->FillRect(&lpDrawItemStruct->rcItem, &brush); 
	} 

	// If item has been deselected, draw the rectangle using the window color. 
	if (!(lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & ODA_SELECT) && 
		!(this->GetStyle() & LBS_NOSEL)) { 
		CBrush brush(::GetSysColor(COLOR_WINDOW)); 
		pDC->FillRect(&lpDrawItemStruct->rcItem, &brush); 
	} 

	// If item has focus, draw the focus rect. 
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && (lpDrawItemStruct->itemState & ODS_FOCUS) && 
		!(this->GetStyle() & LBS_NOSEL)) 
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem); 

	// If item does not have focus, redraw (erase) the focus rect. 
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && !(lpDrawItemStruct->itemState & ODS_FOCUS) && 
		!(this->GetStyle() & LBS_NOSEL)) 
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem); 


	// Set the background mode to TRANSPARENT to draw the text. 
	int nBkMode = pDC->SetBkMode(TRANSPARENT); 

	// If the item's color information is set, use the highlight color 
	// gray text color, or normal color for the text. 
	if (lpDrawItemStruct->itemData){ 
		if ((lpDrawItemStruct->itemState & ODS_SELECTED) && !(this->GetStyle() & LBS_NOSEL)) 
			crText = pDC->SetTextColor(crHilite); 
		else if (lpDrawItemStruct->itemState & ODS_DISABLED) 
			crText = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT)); 
		else 
			crText = pDC->SetTextColor(crNorm); 
	} 
	// Else the item's color information is not set, so use the 
	// system colors for the text. 
	else{ 
		if ((lpDrawItemStruct->itemState & ODS_SELECTED) && !(this->GetStyle() & LBS_NOSEL)) 
			crText = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT)); 
		else if (lpDrawItemStruct->itemState & ODS_DISABLED) 
			crText = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT)); 
		else 
			crText = pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT)); 
	} 


	// Get and display item text. 
	GetText(lpDrawItemStruct->itemID, sText); 
	CRect rect = lpDrawItemStruct->rcItem; 

	// Setup the text format. 
	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER; 
	if (GetStyle() & LBS_USETABSTOPS) 
		nFormat |= DT_EXPANDTABS; 

	// Calculate the rectangle size before drawing the text. 
	pDC->DrawText(sText, -1, &rect, nFormat | DT_CALCRECT); 
	pDC->DrawText(sText, -1, &rect, nFormat); 

	pDC->SetTextColor(crText); 
	pDC->SetBkMode(nBkMode); 
	
}

void CHScrollListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your code to determine the size of specified item
	// ### Is the default list box item height the same as
	// the menu check height???
	lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK);	
}

void CHScrollListBox::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CListBox::PreSubclassWindow();
#ifdef _DEBUG
	// NOTE: this list box is designed to work as a single column, system-drawn 
	//		 list box. The asserts below will ensure of that.
	DWORD dwStyle = GetStyle();
	ASSERT((dwStyle & LBS_MULTICOLUMN) == 0);
	ASSERT((dwStyle & LBS_OWNERDRAWFIXED) == 0);
	//ASSERT((dwStyle & LBS_OWNERDRAWVARIABLE) == 0);
#endif

}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// CHScrollListBox message handlers
///////////////////////////////////////////////////////////////////////////////
int CHScrollListBox::GetTextLen(LPCTSTR lpszText)
{
	ASSERT(AfxIsValidString(lpszText));

	CDC *pDC = GetDC();
	ASSERT(pDC);

	CSize size;
	CFont* pOldFont = pDC->SelectObject(GetFont());
	if ((GetStyle() & LBS_USETABSTOPS) == 0)
	{
		size = pDC->GetTextExtent(lpszText, (int) _tcslen(lpszText));
		size.cx += 3;
	}
	else
	{
		// Expand tabs as well
		size = pDC->GetTabbedTextExtent(lpszText, (int) _tcslen(lpszText), 0, NULL);
		size.cx += 2;
	}
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	return size.cx;
}

///////////////////////////////////////////////////////////////////////////////
void CHScrollListBox::ResetHExtent()
{
	if (GetCount() == 0)
	{
		SetHorizontalExtent(0);
		return;
	}

	CWaitCursor cwc;
	int iMaxHExtent = 0;
	for (int i = 0; i < GetCount(); i++)
	{
		CString csText;
		GetText(i, csText);
		int iExt = GetTextLen(csText);
		if (iExt > iMaxHExtent)
			iMaxHExtent = iExt;
	}
	SetHorizontalExtent(iMaxHExtent);
}

///////////////////////////////////////////////////////////////////////////////
void CHScrollListBox::SetNewHExtent(LPCTSTR lpszNewString)
{
	int iExt = GetTextLen(lpszNewString);
	if (iExt > GetHorizontalExtent())
		SetHorizontalExtent(iExt);
}

///////////////////////////////////////////////////////////////////////////////
// OnAddString: wParam - none, lParam - string, returns - int
///////////////////////////////////////////////////////////////////////////////
LRESULT CHScrollListBox::OnAddString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		SetNewHExtent((LPCTSTR) lParam);
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////
// OnInsertString: wParam - index, lParam - string, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT CHScrollListBox::OnInsertString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		SetNewHExtent((LPCTSTR) lParam);
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////
// OnDeleteString: wParam - index, lParam - none, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT CHScrollListBox::OnDeleteString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		ResetHExtent();
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////
// OnDir: wParam - attr, lParam - wildcard, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT CHScrollListBox::OnDir(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		ResetHExtent();
	return lResult;
}

///////////////////////////////////////////////////////////////////////////////
// OnResetContent: wParam - none, lParam - none, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT CHScrollListBox::OnResetContent(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	SetHorizontalExtent(0);
	return lResult;
}


////////////////////////////////////////////////////////////////////////////////
//
// Return Value:	The zero-based index to the string in the list box. 
//						The return value is LB_ERR if an error occurs; the 
//						return value is LB_ERRSPACE if insufficient space 
//						is available to store the new string.
//
// Parameters	:	lpszItem - Points to the null-terminated 
//							string that is to be added.
//
// Remarks		:	Call this member function to add a string to a list 
//						box. Provided because CListBox::AddString is NOT
//						a virtual function.
////////////////////////////////////////////////////////////////////////////////
int CHScrollListBox::AddString(LPCTSTR lpszItem)
{
	return ((CListBox*)this)->AddString(lpszItem);
}	// AddString

////////////////////////////////////////////////////////////////////////////////
//
// Return Value:	The zero-based index to the string in the list box. 
//						The return value is LB_ERR if an error occurs; the 
//						return value is LB_ERRSPACE if insufficient space 
//						is available to store the new string.
//
// Parameters	:	lpszItem - Points to the null-terminated 
//							string that is to be added.
//						rgb - Specifies the color to be associated with the item.
//
// Remarks		:	Call this member function to add a string to a list 
//						box with a custom color.
////////////////////////////////////////////////////////////////////////////////
int CHScrollListBox::AddString(LPCTSTR lpszItem, COLORREF rgb)
{
	int nItem = AddString(lpszItem);
	if (nItem >= 0)
		SetItemData(nItem, rgb);
	return nItem;
}	// AddString

//////////////////////////////////////////////////////////////////////////////
//
// Return Value:	The zero-based index of the position at which the 
//						string was inserted. The return value is LB_ERR if 
//						an error occurs; the return value is LB_ERRSPACE if 
//						insufficient space is available to store the new string.
//
// Parameters	:	nIndex - Specifies the zero-based index of the position
//							to insert the string. If this parameter is ?, the string
//							is added to the end of the list.
//						lpszItem - Points to the null-terminated string that 
//							is to be inserted.
//
// Remarks		:	Inserts a string into the list box.	Provided because 
//						CListBox::InsertString is NOT a virtual function.
//
////////////////////////////////////////////////////////////////////////////
int CHScrollListBox::InsertString(int nIndex, LPCTSTR lpszItem)
{
	return ((CListBox*)this)->InsertString(nIndex, lpszItem);
}	// InsertString

//////////////////////////////////////////////////////////////////////////////////////
//
// Return Value:	The zero-based index of the position at which the 
//						string was inserted. The return value is LB_ERR if 
//						an error occurs; the return value is LB_ERRSPACE if 
//						insufficient space is available to store the new string.
//
// Parameters	:	nIndex - Specifies the zero-based index of the position
//							to insert the string. If this parameter is ?, the string
//							is added to the end of the list.
//						lpszItem - Points to the null-terminated string that 
//							is to be inserted.
//						rgb - Specifies the color to be associated with the item.
//
// Remarks		:	Inserts a colored string into the list box.
///////////////////////////////////////////////////////////////////////////////////
int CHScrollListBox::InsertString(int nIndex, LPCTSTR lpszItem, COLORREF rgb)
{
	int nItem = ((CListBox*)this)->InsertString(nIndex,lpszItem);
	if (nItem >= 0)
		SetItemData(nItem, rgb);
	return nItem;
}	// InsertString

/////////////////////////////////////////////////////////////////////////////////
//
// Return Value:	None.
//
// Parameters	:	nIndex - Specifies the zero-based index of the item.
//						rgb - Specifies the color to be associated with the item.
//
// Remarks		:	Sets the 32-bit value associated with the specified
//						item in the list box.
/////////////////////////////////////////////////////////////////////////////////
void CHScrollListBox::SetItemColor(int nIndex, COLORREF rgb)
{
	SetItemData(nIndex, rgb);	
	RedrawWindow();
}	// SetItemColor

