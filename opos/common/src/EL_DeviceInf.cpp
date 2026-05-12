//////////////////////////////////////////////////////////////////////////////
// device Interface class body
#include "stdafx.h"
#include "EL_DeviceInf.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif


typedef HDEVINFO (* APISetupDiGetClassDevs)(const GUID* ClassGuid,PCSTR Enumerator,HWND hwndParent,DWORD Flags);
APISetupDiGetClassDevs lpSetupDiGetClassDevs;
//
typedef BOOL (* APISetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);
APISetupDiDestroyDeviceInfoList lpSetupDiDestroyDeviceInfoList;
//
typedef BOOL (* APISetupDiEnumDeviceInterfaces)(HDEVINFO DeviceInfoSet,PSP_DEVINFO_DATA DeviceInfoData,const GUID* InterfaceClassGuid,DWORD MemberIndex,PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData);
APISetupDiEnumDeviceInterfaces lpSetupDiEnumDeviceInterfaces;
//
#ifdef UNICODE
typedef BOOL (* APISetupDiGetDeviceInterfaceDetail)(HDEVINFO DeviceInfoSet,PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData,DWORD DeviceInterfaceDetailDataSize,PDWORD RequiredSize,PSP_DEVINFO_DATA DeviceInfoData);
#else
typedef BOOL (* APISetupDiGetDeviceInterfaceDetail)(HDEVINFO DeviceInfoSet,PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,DWORD DeviceInterfaceDetailDataSize,PDWORD RequiredSize,PSP_DEVINFO_DATA DeviceInfoData);
#endif
APISetupDiGetDeviceInterfaceDetail lpSetupDiGetDeviceInterfaceDetail;
//typedef int ( * __stdcall ReadPinPassword)(HWND hTagetWnd, UINT nSendMsg, DWORD dwTimeout, int nFieldIndex) ;

//ReadPinPassword lpReadPinpad; 

//typedef int ( * __stdcall ClosePinpad)(void) ;

//////////////////////////////////////////////////////////////////////////////
// CDevInterfaceObj constructor
CDevInterfaceObj::CDevInterfaceObj(
	GUID* pClassGuid, 
	PDWORD status ) 
{	//
	BOOL bOK=FALSE;
	//DWORD flags = DIGCF_DEVICEINTERFACE | DIGCF_PRESENT ;
	DWORD flags = DIGCF_DEVICEINTERFACE;
	HINSTANCE hSetUpInstance = LoadLibrary(_T("Setupapi.dll"));
	//
	if( hSetUpInstance==NULL ){
		bOK=FALSE;
	}
	else{
		bOK=TRUE;
	}
	//
	m_hInfo = INVALID_HANDLE_VALUE;
	ZeroMemory(&m_Guid,sizeof(GUID));

	try{
		if( TRUE ){
			*status = ERROR_INVALID_PARAMETER;
			m_Guid = *pClassGuid;
			//
			#ifdef UNICODE
			lpSetupDiGetClassDevs=(APISetupDiGetClassDevs)GetProcAddress(hSetUpInstance,(LPCSTR)"SetupDiGetClassDevsW") ; 
			#else
			lpSetupDiGetClassDevs=(APISetupDiGetClassDevs)GetProcAddress(hSetUpInstance,(LPCSTR)"SetupDiGetClassDevsA") ;
			#endif
			//
			m_hInfo = lpSetupDiGetClassDevs(pClassGuid, NULL, NULL, flags);

			if( m_hInfo == INVALID_HANDLE_VALUE )
				*status = GetLastError();
			else
				*status = ERROR_SUCCESS;
			//
			FreeLibrary( hSetUpInstance );
		}
		else{
			m_hInfo = INVALID_HANDLE_VALUE;
		}
	}
	catch (...){
		m_hInfo = INVALID_HANDLE_VALUE;
		FreeLibrary( hSetUpInstance );
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDevInterfaceObj destructor
//
CDevInterfaceObj::~CDevInterfaceObj(void)
{
	BOOL bOK=FALSE;
	BOOL bm;
	//
	HINSTANCE hSetUpInstance = LoadLibrary(_T("Setupapi.dll"));
	//
	if( hSetUpInstance==NULL ){
		bOK=FALSE;
	}
	else{
		bOK=TRUE;
	}
	//
	if( m_hInfo != INVALID_HANDLE_VALUE ){
		if( bOK ){
			lpSetupDiDestroyDeviceInfoList=(APISetupDiDestroyDeviceInfoList)GetProcAddress(hSetUpInstance,"SetupDiDestroyDeviceInfoList") ; 

			if( !lpSetupDiDestroyDeviceInfoList(m_hInfo) ){
				//destory error
				bm=FALSE;
			}
		}
	}
	//
	if( bOK ){
		m_hInfo = INVALID_HANDLE_VALUE;
		FreeLibrary( hSetUpInstance );
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDevInterface constructor
//
CDevInterface::CDevInterface(
	CDevInterfaceObj* pClassObject, 
	DWORD Index,
	PDWORD Error )
{
	m_Class = pClassObject;
	BOOL status;
	BOOL bOK=FALSE;
	DWORD ReqLen;
	HINSTANCE hSetUpInstance = LoadLibrary(_T("Setupapi.dll"));
	m_Detail = NULL;
	m_Data.cbSize = sizeof SP_DEVICE_INTERFACE_DATA;
	int i;
	//
	//
	if( hSetUpInstance==NULL ){
		bOK=FALSE;
	}
	else{
		bOK=TRUE;
	}
	//
	if( bOK ){
		lpSetupDiEnumDeviceInterfaces=(APISetupDiEnumDeviceInterfaces)GetProcAddress(hSetUpInstance,(LPCSTR)"SetupDiEnumDeviceInterfaces") ; 
		#ifdef UNICODE
		lpSetupDiGetDeviceInterfaceDetail=(APISetupDiGetDeviceInterfaceDetail)GetProcAddress(hSetUpInstance,(LPCSTR)"SetupDiGetDeviceInterfaceDetailW"); 
		#else
		lpSetupDiGetDeviceInterfaceDetail=(APISetupDiGetDeviceInterfaceDetail)GetProcAddress(hSetUpInstance,(LPCSTR)"SetupDiGetDeviceInterfaceDetailA"); 
		#endif
	}
	//

	try{
		*Error = ERROR_INVALID_PARAMETER;
		//
		if( bOK ){
			status = lpSetupDiEnumDeviceInterfaces(
				m_Class->GetHandle(), 
				NULL, 
				m_Class->GetGuid(), 
				Index, 
				&m_Data	);
			//
			if( !status ){
				*Error = GetLastError();
				FreeLibrary( hSetUpInstance );
				return;
			}					  
			//
			lpSetupDiGetDeviceInterfaceDetail(
				m_Class->GetHandle(),
				&m_Data,
				NULL,
				0,
				&ReqLen,
				NULL );

			*Error = GetLastError();
			//
			if( *Error != ERROR_INSUFFICIENT_BUFFER ){
				FreeLibrary( hSetUpInstance );
				return;
			}
			//
			m_Detail = PSP_INTERFACE_DEVICE_DETAIL_DATA(new TCHAR[ReqLen]);
			//
			if( !m_Detail ){
				*Error = ERROR_NOT_ENOUGH_MEMORY;
				FreeLibrary( hSetUpInstance );
				return;
			}
			//
			//m_Detail->cbSize = sizeof PSP_DEVICE_INTERFACE_DETAIL_DATA;
			m_Detail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

			status = lpSetupDiGetDeviceInterfaceDetail(
				m_Class->GetHandle(),
				&m_Data,
				m_Detail,
				ReqLen,
				&ReqLen,
				NULL );
			//
			if( !status ){
				*Error = GetLastError();
				delete m_Detail;
				m_Detail = NULL;
				FreeLibrary( hSetUpInstance );
				return;
			}
			else{
				TCHAR *pchar=NULL;
				i=0;
				pchar=m_Detail->DevicePath;
				BOOL bSearchOK=FALSE;
				//search string
				pchar=_tcschr( pchar,_T('v') );
				if( pchar!=NULL ){
					pchar=_tcschr( pchar,_T('i') );
					if( pchar!=NULL ){
						pchar=_tcschr( pchar,_T('d') );
						if( pchar!=NULL ){
							if( pchar[2]==_T('1') &&
								pchar[3]==_T('3') &&
								pchar[4]==_T('4') &&
								pchar[5]==_T('b') &&
								pchar[11]==_T('0') &&
								pchar[12]==_T('3') &&
								pchar[13]==_T('0') &&
								pchar[14]==_T('2') ){
								bSearchOK=TRUE;
							}
							else
								bSearchOK=FALSE;
						}
						else
							bSearchOK=FALSE;
					}
					else
						bSearchOK=FALSE;
				}
				else
					bSearchOK=FALSE;

				//
				//i=strcspn( ,"Vid_134b&Pid_0302" );
				//
				if( !bSearchOK ){
					*Error=ERROR_PATH_NOT_FOUND;
					delete m_Detail;
					m_Detail = NULL;
					FreeLibrary( hSetUpInstance );
				}
			}
		}
		//
		*Error = ERROR_SUCCESS;
	}
	catch (...){
		if( bOK ){
			FreeLibrary( hSetUpInstance );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDevInterface destructor
//
CDevInterface::~CDevInterface(void)
{
	if (m_Detail){
		delete m_Detail;
		m_Detail = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDevInterface::DevicePath
//
TCHAR* CDevInterface::DevicePath(void)
{
	try{
		if( m_Detail)
			return m_Detail->DevicePath;
		else
			return NULL;
	}
	catch(...){
		return NULL;
	}
}

