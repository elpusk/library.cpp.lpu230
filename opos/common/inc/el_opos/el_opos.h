

#if !defined(_EL_OPOS_H_)
#define      _EL_OPOS_H_

/////////////////////////////////////////////////////////////////////
// "CapDataEncryption", "DataEncryptionAlgorithm" Property Constants
//   (added in 1.12)
/////////////////////////////////////////////////////////////////////

// const LONG MSR_DE_NONE          = 0x00000001; //defined OposMsr.h
// const LONG MSR_DE_3DEA_DUKPT    = 0x00000002; //defined OposMsr.h
// Note: Service-specific values begin at 0x01000000.

const long EL_MSR_DE_AES_DUKPT = 0x01000000;

///////////////////////////////////////////////////////////////////
// added in 1.8.24(msr) and 1.14.3(keylock)
#define	EL_OPOS_PROVIDER				"Elpusk"
#define	EL_APPUID_KEY					"ElAppUID" //elpusk sevice object UID를 저장하는 value name

/////////////
// Elpusk OPOS MSR Service Object 의 MSR 키에 DeviceName 을 저장하는 레지스트리 value name
// default logical device name
#define	EL_CLASS_ROOT_KEY_MSR			"DefaultMsr"

// Elpusk OPOS MSR Service Object 의 기본 DeviceName
// EL_CLASS_ROOT_KEY_MSR value 의 data 이고, MSR 키의 sub key name
// default physical device name
#define	EL_DEVICE_NAME_DEFAULT_MSR		"Lpu230MSR"

// DeviceName key 의 기본 value data(CCO code 에서는 ProgID 로 사용)
#define	EL_CLASS_KEY_MSR				"OposSo.Lpu230"

// Elpusk OPOS MSR Service Object 에서 unregister 또는 open 시, elpusk device 인지 확인하기 위한 AppUID
#define	EL_APPUID_SO_MSR				"{5236A4C5-D44A-4AE5-A2B5-50551B8DD152}"

/////////////
// Elpusk OPOS KeyLock Service Object 의 Keylock 키에 DeviceName 을 저장하는 레지스트리 value name
// default logical device name
#define	EL_CLASS_ROOT_KEY_KEYLOCK			"DefaultLock"

// Elpusk OPOS KeyLock Service Object 의 기본 DeviceName
// EL_CLASS_ROOT_KEY_KEYLOCK value 의 data 이고, KeyLock 키의 sub key name
// default physical device name
#define	EL_DEVICE_NAME_DEFAULT_KEYLOCK		"Lpu230Lock"

// DeviceName key 의 기본 value data(CCO code 에서는 ProgID 로 사용)
#define	EL_CLASS_KEY_KEYLOCK				"OposSo.Lpu230Lock"

// Elpusk OPOS KeyLock Service Object 에서 unregister 또는 open 시, elpusk device 인지 확인하기 위한 AppUID
#define	EL_APPUID_SO_KEYLOCK				"{499E5C6B-4135-417F-A7D0-F67FF554F421}"

#endif	//_EL_OPOS_H_