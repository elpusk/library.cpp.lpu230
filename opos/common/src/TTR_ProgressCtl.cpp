////////////////////////////////////////////////////////////////
//TTR_ProgressCtl.cpp

//Win32 list control support function body

#include "stdafx.h"
#include "TTR_ProgressCtl.h"
#include "commctrl.h"

//Sets the minimum and maximum values for a progress bar and
//redraws the bar to reflect the new range.
//Returns the previous range values if successful, or zero otherwise.
//The low-order word specifies the previous minimum value,
//and the high-order word specifies the previous maximum value
DWORD SetRangeOfProgress( HWND hProgress,INT nMin,INT nMax )
{
	DWORD dwResult=0;
	
	dwResult=SendMessage( hProgress,(UINT)PBM_SETRANGE,0,(LPARAM)MAKELPARAM(nMin,nMax) );
	
	return dwResult;
}

//Sets the current position for a progress bar and redraws the bar to reflect the new position
//Returns the previous position
INT SetPosOfProgress( HWND hProgress,INT nNewPos )
{
	INT nResult=0;

	nResult=SendMessage( hProgress,(UINT)PBM_SETPOS,(WPARAM)nNewPos,0 );

	return nResult;
}

//Specifies the step increment for a progress bar
//Returns the previous step increment
INT SetStepOfProgress( HWND hProgress,INT nStepInc )
{
	INT nResult=0;

	nResult=SendMessage( hProgress,(UINT)PBM_SETSTEP,(WPARAM)nStepInc,0 );

	return nResult;
}

//Advances the current position for a progress bar by the step increment
//and redraws the bar to reflect the new position.
//Returns the previous position
INT SetItOfProgress( HWND hProgress )
{
	INT nResult=0;

	nResult=SendMessage( hProgress,(UINT)PBM_STEPIT,0,0 );

	return nResult;
}
