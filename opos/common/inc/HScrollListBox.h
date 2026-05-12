#if !defined(AFX_HSCROLLLISTBOX_H__58A2628B_9941_41FE_9269_81AA976D1F4A__INCLUDED_)
#define AFX_HSCROLLLISTBOX_H__58A2628B_9941_41FE_9269_81AA976D1F4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HScrollListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHScrollListBox window

class CHScrollListBox : public CListBox
{
// Construction
public:
	CHScrollListBox();
	int AddString(LPCTSTR lpszItem);											// Adds a string to the list box
	int AddString(LPCTSTR lpszItem, COLORREF rgb);						// Adds a colored string to the list box
	int InsertString(int nIndex, LPCTSTR lpszItem);						// Inserts a string to the list box
	int InsertString(int nIndex, LPCTSTR lpszItem, COLORREF rgb);	// Inserts a colored string to the list box
	void SetItemColor(int nIndex, COLORREF rgb);							// Sets the color of an item in the list box

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHScrollListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHScrollListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHScrollListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg LRESULT OnAddString(WPARAM wParam, LPARAM lParam); // wParam - none, lParam - string, returns - int
	afx_msg LRESULT OnInsertString(WPARAM wParam, LPARAM lParam); // wParam - index, lParam - string, returns - int 
	afx_msg LRESULT OnDeleteString(WPARAM wParam, LPARAM lParam); // wParam - index, lParam - none, returns - int 
	afx_msg LRESULT OnResetContent(WPARAM wParam, LPARAM lParam); // wParam - none, lParam - none, returns - int 
	afx_msg LRESULT OnDir(WPARAM wParam, LPARAM lParam); // wParam - attr, lParam - wildcard, returns - int 

	DECLARE_MESSAGE_MAP()
private:
	void ResetHExtent();
	void SetNewHExtent(LPCTSTR lpszNewString);
	int GetTextLen(LPCTSTR lpszText);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HSCROLLLISTBOX_H__58A2628B_9941_41FE_9269_81AA976D1F4A__INCLUDED_)
