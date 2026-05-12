// EL_Pin_ReCode.h: 
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//th definition of elpusk pinpad generic return code
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PINPAD_RETURN_CODE__)
#define __EL_PINPAD_RETURN_CODE__

//wparam parameter in Async commnad
#define	K1A_PIN_PAD_WPARAM_S_SUCCESS		0x0000	//Get response OK.
#define	K1A_PIN_PAD_WPARAM_E_INVAILD_RESP	0x0001	//invalied response
#define	K1A_PIN_PAD_WPARAM_W_REMOVE_ICC		0x0002	//warning removed ICC event in GetPINEx or GetPINExAuto
#define	K1A_PIN_PAD_WPARAM_W_INSERT_ICC		0x0003	//warning inserted ICC event in GetPINEx or GetPINExAuto

///////////////////////////////
//common Return code definition
#define	K1A_PIN_PAD_S_SUCCESS			0x00000000	//processing Success.
#define K1A_PIN_PAD_E_INVAILD_PARA		0x80000001	//invaild parameter
#define	K1A_PIN_PAD_E_UNKOWN			0x8000FFFF	//invaild handle
#define K1A_PIN_PAD_E_INVAILD_RESP		0x8000FFFE	//invaild response
#define K1A_PIN_PAD_E_TIMEOUT_RESP		0x8000FFFD	//response is timeout

//OpenPINDevice
#define	K1A_PIN_PAD_E_INVALID_HANDLE	0x80000002	//invaild handle
#define	K1A_PIN_PAD_W_ALEADY_OPEN		0x40000002	//Aleady open.

//ClosePINDevice
#define	K1A_PIN_PAD_W_ALEADY_CLOSE		0x40000002	//Aleady close.

//GetPIN

//GetPINEx
#define	K1A_PIN_PAD_E_LOCKED_ASYNC		0x80000003	//not released Async-system.

//RawReadPINDevice
#define	K1A_PIN_PAD_E_READ_FILE			0x80000004	//ReadFile function error

//RawWritePINDevice
#define	K1A_PIN_PAD_E_WRITE_FILE		0x80000005	//WriteFile function error

//PowerON
#define	K1A_PIN_PAD_E_NO_ATR			0x80000010	//fail get ATR
#define	K1A_PIN_PAD_E_NOT_EXSIT			0x80000011	//ICC isn't present on IC slot

#define	K1A_PIN_PAD_W_MIS_LEN_ACC		0x40000020	//The response data length isn't
													//equal to requested
													//Aaccout information length

//wparam parameter in Async commnad
#define	KLP110UF_WPARAM_S_SUCCESS		K1A_PIN_PAD_WPARAM_S_SUCCESS		//Get response OK.
#define	KLP110UF_WPARAM_E_INVAILD_RESP	K1A_PIN_PAD_WPARAM_E_INVAILD_RESP	//invalied response
#define	KLP110UF_WPARAM_W_REMOVE_ICC	K1A_PIN_PAD_WPARAM_W_REMOVE_ICC		//warning removed ICC event in GetPINEx or GetPINExAuto
#define	KLP110UF_WPARAM_W_INSERT_ICC	K1A_PIN_PAD_WPARAM_W_INSERT_ICC		//warning inserted ICC event in GetPINEx or GetPINExAuto
#define	KLP110UF_WPARAM_W_CANCEL_BY_APP	0x0004								//warning cancel by app command in GetPINEx or GetPINExAuto
//-add 2007.12.20
#define	KLP110UF_WPARAM_E_UNKNOWN		0x0005								//error unknown in GetPINEx or GetPINExAuto


/*********** KLP110UF DLL return value *****************************/

#define	KLP110UF_S_SUCCESS			K1A_PIN_PAD_S_SUCCESS			//processing Success.
#define KLP110UF_E_INVAILD_PARA		K1A_PIN_PAD_E_INVAILD_PARA		//invaild parameter
#define	KLP110UF_E_UNKOWN			K1A_PIN_PAD_E_UNKOWN			//invaild handle
#define	KLP110UF_E_UNKNOWN			K1A_PIN_PAD_E_UNKOWN			//invaild handle -- add 2007.9.13 fixing-spelling
#define KLP110UF_E_INVAILD_RESP		K1A_PIN_PAD_E_INVAILD_RESP		//invaild response
#define KLP110UF_E_TIMEOUT_RESP		K1A_PIN_PAD_E_TIMEOUT_RESP		//response is timeout

//add KLP110UF_S_TRAIN_PHASE. 2007.10.11 : for multi Sigma(OUT)+Sigma(IN) transaction.
#define	KLP110UF_S_TRAIN_PHASE		0x00000200						//complete train-phase....... success

//add KLP110UF_W_ZERO_RESP. 2007.4.30
#define	KLP110UF_W_ZERO_RESP		0x40000030						//pinpad response to zero 64 bytes
//add KLP110UF_W_MORE_PROCESS. 2007.7.6
#define	KLP110UF_W_MORE_PROCESS		0x40000031						//current command is requested more processing

//add KLP110UF_E_REMOVED_DEVICE. 2004.7.14 for DLL internal usage.
#define KLP110UF_E_REMOVED_DEVICE	0x8000FFFC						//Removed device

//OpenPINDevice
#define	KLP110UF_E_INVALID_HANDLE	K1A_PIN_PAD_E_INVALID_HANDLE	//invaild handle
#define	KLP110UF_W_ALEADY_OPEN		K1A_PIN_PAD_W_ALEADY_OPEN		//Aleady open.

//ClosePINDevice
#define	KLP110UF_W_ALEADY_CLOSE		K1A_PIN_PAD_W_ALEADY_CLOSE		//Aleady close.

//GetPIN
#define	KLP110UF_W_CANCEL_BY_APP	0x40000003						//cancel by app command
#define	KLP110UF_W_ALEADY_GETPIN	0x40000004						//aleady executed GetPin

//GetPINEx
#define	KLP110UF_E_LOCKED_ASYNC		K1A_PIN_PAD_E_LOCKED_ASYNC		//not released Async-system.

//RawReadPINDevice
#define	KLP110UF_E_READ_FILE		K1A_PIN_PAD_E_READ_FILE			//ReadFile function error

//RawWritePINDevice
#define	KLP110UF_E_WRITE_FILE		K1A_PIN_PAD_E_WRITE_FILE		//WriteFile function error

//PowerON
#define	KLP110UF_E_NO_ATR			K1A_PIN_PAD_E_NO_ATR			//fail get ATR
#define	KLP110UF_E_NOT_EXSIT		K1A_PIN_PAD_E_NOT_EXSIT			//ICC isn't present on IC slot

//IsCardExist
#define	KLP110UF_S_INSERT_ICC		0x00000100						//inserted ICC on pinpad
#define	KLP110UF_S_REMOVE_ICC		0x00000101						//removed ICC on pinpad

//DisplayAccDataPINDevice
#define	KLP110UF_W_IDLE				0x40000005		//Not select account
#define	KLP110UF_W_CANCEL_BY_USER	0x40000006		//pinpad user has pressed cancel button.
#define	KLP110UF_E_SEL_RECORD		0x80000013		//select record error

#endif // !defined(__EL_PINPAD_RETURN_CODE__)
