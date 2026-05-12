////////////////////////////////////////////////////////////////
//ELListCtl.h

//Win32 list control support function header

#if !defined(__EL_LIST_CONTROL_2007101917__)
#define __EL_LIST_CONTROL_2007101917__

//add string to given list control
void AddStringToList( HWND hListBox,LPCTSTR lpctMsg );

//reset content of list control
void ResetContent( HWND hCtl );

//get count list box
int GetCountOfList( HWND hListBox );

//Get text of list box
int GetTextOfList( HWND hListBox,int nIndex,TCHAR *psString );

#endif//__EL_LIST_CONTROL_2007101917__