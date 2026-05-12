
#if !defined(__TTR_DEVICE_EVENT_HEADER_20091112__)
#define __TTR_DEVICE_EVENT_HEADER_20091112__

#include "Dbt.h"

////////////////////////////////////////////////////////////
//the definition of return value of DeviceChangeMsgHandler()
#define	DEV_CHANGE_PLUGIN_WANTED_USB				1
#define	DEV_CHANGE_PLUGOUT_WANTED_USB				2
#define	DEV_CHANGE_PLUGIN_THE_OTHER_USB				3
#define	DEV_CHANGE_PLUGOUT_THE_OTHER_USB			4
#define	DEV_CHANGE_WILL_BE_PLUGOUT_WANTED_DEVICE	6
#define	DEV_CHANGE_WILL_BE_PLUGOUT_THE_OTHER_DEVICE	8


#define	DEV_CHANGE_NO_NEED_EVENT					100



/////////////////////////////////////////////////////////////////////////////
//exported function
//register user device for device-change-notification
HDEVNOTIFY RegisterEventDevice( HANDLE hTargetWnd,GUID InterfaceClassGuid );

//register user device-handle for device-change-notification
HDEVNOTIFY RegisterEventDeviceHandle( HANDLE hTargetWnd,HANDLE hDev,HDEVNOTIFY hDevnotify );

//unregister device of RegisterEventDevice()
BOOL UnRegisterEventDevice( HDEVNOTIFY hNotify );

//unregister device of RegisterEventDeviceHandle()
BOOL UnRegisterEventDeviceHandle( HDEVNOTIFY hNotify );

//this function is called on WM_DEVICECHANGE message handler
INT DeviceChangeMsgHandler( UINT nEventType, DWORD dwData,WORD wVID,WORD wPID,GUID ClassGuid );

//this function is called on WM_DEVICECHANGE message handler with Multi device(2007.04.06)
INT DeviceMultiChangeMsgHandler( UINT nEventType, DWORD dwData,WORD *pwVID,WORD *pwPID,INT nDevCnt,GUID ClassGuid );

//this function is called on WM_DEVICECHANGE message handler . return device name
INT DeviceChangeMsgHandlerEx( UINT nEventType, DWORD dwData,WORD wVID,WORD wPID,GUID ClassGuid,HANDLE hWantedDev );

/////////////////////////////////////////////////////////////////////////////
//internal function
//Get string type of VID & PID from device path
//this function is called in IsWantUSBDevice()
//psInf == NULL, psRev == NULL,  pass interface & revision.
BOOL ParsingUSBDevicePath(PTCHAR pDevicePath, PTCHAR psVID, PTCHAR psPID, PTCHAR psInf, PTCHAR psRev )

//checks whether the given VID & PID exsit in the given device path 
// if nPID<0 : No consider PID number.
// if nInf<0 : No consider interface number.
// if nRev<0 : No consider revision number.
BOOL IsWantUSBDevice(PTCHAR psDevicePath,INT nVID,INT nPID, INT nInf, INT nRev );

#endif//#if !defined(__TTR_DEVICE_EVENT_HEADER_20091112__)
