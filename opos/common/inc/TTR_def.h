
#if !defined(__TTRL_DEFINE_HEADER_20091104__)
#define __TTRL_DEFINE_HEADER_20091104__

#include "windows.h"

///////////////////////////////////////////////
//TTR serise definition header file
//////////////////////////////////////////////	

// TTR callback function type
typedef LRESULT (CALLBACK* TTR_CBFunType)(LPVOID lpUserPara,DWORD dwResult,LPVOID lpData,int nSize);

#define	TTR_DEF_SPIN_COUNT				2000	//default spin counter
												//see InitializeCriticalSectionAndSpinCount()

#define	TTR_DEF_EVENT_NAME_SIZE			128		//TTR serise event object name size
#define	TTR_DEF_MUTEX_NAME_SIZE			128		//TTR serise mutex object name size
#define	TTR_DEF_NAME_BUF_SIZE			128		//TTR serise name  buffer size
#define	TTR_DEF_STR_BUF_SIZE			128		//TTR serise string  buffer size

#define	TTR_MAX_USB_DEV_NUMBER			127		//the maximun number of Usb device.
#define	TTR_DEF_WAIT_CHECK_INTERVAL		15		//the default value of waiting interval check
												//[unit] msec
#define	TTR_DEF_WAIT_TIME				10		//default waiting time for WaitForSingleObject Function
#define	TTR_DEF_WAIT_TIME_EXIT_THREAD	3000	//default waiting time forTERMINATING THREAD [unit:msec]


//////////////////////////////////////////////////////
#define	TTR_DEF_EXIT_CODE_NORMAL			0		//default exit code in normal case
#define	TTR_DEF_EXIT_CODE_ERROR				1000	//default exit code in error case
#define	TTR_DEF_EXIT_CODE_ERROR_MAL			1001	//default exit code in memory allocation error case
#define	TTR_DEF_EXIT_CODE_ERROR_LOAD_RES	1002	//default exit code in loading resource error case
#define	TTR_DEF_EXIT_CODE_ERROR_INI_OBJ		1003	//default exit code in internal object object error case
#define	TTR_DEF_EXIT_CODE_ERROR_CREATE_WIN	1004	//default exit code in creating window error case
#define	TTR_DEF_EXIT_CODE_ERROR_CREATE_OBJ	1005	//default exit code in creating objects error case

//////////////////////////////////////////////////////
#define	TTR_DEF_EXIT_CODE_WARN				2000	//default exit code in warn case
#define	TTR_DEF_EXIT_CODE_WARN_SINGLE_INST	2001	//voliate single instance rule.

/////////////////////////////////////////

#endif	//__TTRL_DEFINE_HEADER_20091104__