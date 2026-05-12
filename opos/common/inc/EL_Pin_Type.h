// EL_Pin_Type.h: declaration of relative pinpad enumations.......
// 2007.12.27 : this file is detached from "EL_Pin_IO_Base.h" file
//////////////////////////////////////////////////////////////////////

#if !defined(_EL_PIN_PAD_TYPE_H_2007_12_27_0001_)
#define _EL_PIN_PAD_TYPE_H_2007_12_27_0001_


//compare Version-string and opened pinpad result
enum PinPadComparae{
	PINPAD_COM_UNKNOWN,	// 비교 결과 알수 없는 에러
	PINPAD_COM_EQUAL,	// 핀패드의 펨웨어와 버전 스트링 값은 동일 함.
	PINPAD_COM_OLD,		// 핀패드의 펨웨어가 버전 스트링 값보다 낮은 버전임.
	PINPAD_COM_NEW,		// 핀패드의 펨웨어가 버전 스트링 값보다 높은 버전임.
	PINPAD_COM_HW_EQUAL,// 핀패드의	펨웨어와 버전 스트링 값은 서로 다른 모델이나 
						// 하드웨어 호환성은 있음.
	PINPAD_COM_HW_MIS	// 핀패드의	펨웨어와 버전 스트링 값은 서로 다른 모델이도 
						// 하드웨어 호환성도 없음.
};

//pinad local language Ttype
enum PinPadLangType{
	PINPAD_LANG_TYPE_UNKNOWN=0,//알수없는 타입
	PINPAD_LANG_TYPE_00,	//korean type
	PINPAD_LANG_TYPE_01		//english type
};

//pinpad hardware type
enum PinPadHWType{
	PINPAD_HW_TYPE_UNKNOWN=0,//알수없는 타입
	PINPAD_HW_TYPE_00,	//110U		USB	음성무
	PINPAD_HW_TYPE_01,	//110UR		USB+RS232	음성무
	PINPAD_HW_TYPE_02,	//110UR2	USB+RS232	음성유
	PINPAD_HW_TYPE_03,	//140U		USB	음성유
	PINPAD_HW_TYPE_04,	//150U		HID USB++RS232 음성?
};

enum PinPadType	// Declare enum type pinpad
{
	PINPAD_TYPE_UNKNOWN=0,//알수없는 타입
	PINPAD_TYPE_00,//서울 저축은행 타입 
	PINPAD_TYPE_01,//신한은행
	PINPAD_TYPE_02,//프라임상호저축은행
	PINPAD_TYPE_03,//한신저축은행
	PINPAD_TYPE_04,//대구은행
	PINPAD_TYPE_05,//이비카드(시외버스터미날)
	PINPAD_TYPE_06,//굿모닝 신한증권 
	PINPAD_TYPE_07,//신협 - forth
	PINPAD_TYPE_08,//에카뱅크(토마토저축은행)
	PINPAD_TYPE_09,//한국증권금융
	PINPAD_TYPE_10,//서울증권
	PINPAD_TYPE_11,//수협
	PINPAD_TYPE_12,//우리은행 - duplicate
	PINPAD_TYPE_13,//유화증권
	PINPAD_TYPE_14,//하나은행
	PINPAD_TYPE_15,//서울선물
	PINPAD_TYPE_16,//상호저축은행(FSB) - duplicate
	PINPAD_TYPE_17,//sample pinpad - 2007.07.25
	PINPAD_TYPE_18 //삼성화재 or Basic pinpad- 2008.09.17
};

/////////////////////////////////////////////////////////
//Warning!!!!
//this string is must be mapping to enum type pinpad. -_-;;
#define	S_PINPAD_TYPE_UNKNOWN		"0"	//알수없는 타입
#define	S_PINPAD_TYPE_00			"1"	//서울 저축은행 타입 
#define	S_PINPAD_TYPE_01			"2"	//신한은행
#define	S_PINPAD_TYPE_02			"3"	//프라임상호저축은행
#define	S_PINPAD_TYPE_03			"4"	//한신저축은행
#define	S_PINPAD_TYPE_04			"5"	//대구은행
#define	S_PINPAD_TYPE_05			"6"	//이비카드(시외버스터미날)
#define	S_PINPAD_TYPE_06			"7"	//굿모닝 신한증권 
#define	S_PINPAD_TYPE_07			"8"	//신협 - forth
#define	S_PINPAD_TYPE_08			"9"	//에카뱅크(토마토저축은행)
#define	S_PINPAD_TYPE_09			"10"	//한국증권금융
#define	S_PINPAD_TYPE_10			"11"	//서울증권
#define	S_PINPAD_TYPE_11			"12"	//수협
#define	S_PINPAD_TYPE_12			"13"	//우리은행 - duplicate
#define	S_PINPAD_TYPE_13			"14"	//유화증권
#define	S_PINPAD_TYPE_14			"15"	//하나은행
#define	S_PINPAD_TYPE_15			"16"	//서울선물
#define	S_PINPAD_TYPE_16			"17"	 //상호저축은행(FSB) - duplicate
#define	S_PINPAD_TYPE_17			"18"	//sample pinpad - 2007.07.25
#define	S_PINPAD_TYPE_18			"19"	//삼성화재 or Basic pinpad- 2008.09.17

#endif // !defined(_EL_PIN_PAD_TYPE_H_2007_12_27_0001_)
