////////////////////////////////////////////////////////////////
//TTR_ProgressCtl.h

//Win32 list control support function header

#if !defined(__TTR_PROGRESS_CONTROL_2009120800__)
#define __TTR_PROGRESS_CONTROL_2009120800__

//Sets the minimum and maximum values for a progress bar and
//redraws the bar to reflect the new range.
//Returns the previous range values if successful, or zero otherwise.
//The low-order word specifies the previous minimum value,
//and the high-order word specifies the previous maximum value
DWORD SetRangeOfProgress( HWND hProgress,INT nMin,INT nMax );

//Sets the current position for a progress bar and redraws the bar to reflect the new position
//Returns the previous position
INT SetPosOfProgress( HWND hProgress,INT nNewPos );

//Specifies the step increment for a progress bar
//Returns the previous step increment
INT SetStepOfProgress( HWND hProgress,INT nStepInc );

//Advances the current position for a progress bar by the step increment
//and redraws the bar to reflect the new position.
//Returns the previous position
INT SetItOfProgress( HWND hProgress );

#endif//__TTR_PROGRESS_CONTROL_2009120800__