////////////////////////////////////////////////////////////////
//ELListCtl.h

//Win32 list control support function header

#if !defined(__TTR_LIST_CONTROL_20091112__)
#define __TTR_LIST_CONTROL_20091112__

//add string to given list control
void AddStringToList( HWND hListBox,LPCTSTR lpctMsg );

//reset content of list control
void ResetContent( HWND hCtl );

//get count list box
INT GetCountOfList( HWND hListBox );

//Get text of list box
INT GetTextOfList( HWND hListBox,INT nIndex,TCHAR *psString );

#endif//__TTR_LIST_CONTROL_20091112__