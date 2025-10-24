#pragma once

#include <map>
#include <vector>
#include <mp_type.h>

namespace gemcore_scr_interface
{

	enum PC_to_RDR : unsigned char{// 6.1 Command Pipe, Bulk-OUT Messages
		PC_to_RDR_IccPowerOn					= 0x62,
		PC_to_RDR_IccPowerOff					= 0x63,
		PC_to_RDR_GetSlotStatus					= 0x65,
		PC_to_RDR_XfrBlock							= 0x6F,
		PC_to_RDR_GetParameters					= 0x6C,
		PC_to_RDR_ResetParameters				= 0x6D,
		PC_to_RDR_SetParameters					= 0x61,
		PC_to_RDR_Escape							= 0x6B,
		PC_to_RDR_IccClock							= 0x6E,//not supported
		PC_to_RDR_T0APDU							= 0x6A,//not supported
		PC_to_RDR_Secure							= 0x69,//not supported
		PC_to_RDR_Mechanical						= 0x71,//not supported
		PC_to_RDR_Abort							= 0x72,
		PC_to_RDR_SetDataRateAndClockFrequency		= 0x73//not supported
	};

	enum RDR_to_PC : unsigned char{
		//6.2 Response Pipe, Bulk-IN Messages
		RDR_to_PC_DataBlock					= 0x80,
		RDR_to_PC_SlotStatus					= 0x81,
		RDR_to_PC_Parameters					= 0x82,
		RDR_to_PC_Escape						= 0x83,
		RDR_to_PC_DataRateAndClockFrequency	= 0x84,//not supported
		//6.3 Interrupt-IN Messages 
		RDR_to_PC_NotifySlotChange		= 0x50,//not supported
		RDR_to_PC_HardwareError			= 0x51//not supported
	};

	enum Slot_Error : unsigned char{//Table 6.2-2 Slot error register when bmCommandStatus = 1
		CMD_ABORTED							= 0xFF,
		ICC_MUTE								= 0xFE,
		XFR_PARITY_ERROR						= 0xFD,
		XFR_OVERRUN							= 0xFC,
		HW_ERROR								= 0xFB,
		BAD_ATR_TS								= 0xF8,
		BAD_ATR_TCK							= 0xF7,
		ICC_PROTOCOL_NOT_SUPPORTED	= 0xF6,
		ICC_CLASS_NOT_SUPPORTED			= 0xF5,
		PROCEDURE_BYTE_CONFLICT			= 0xF4,
		DEACTIVATED_PROTOCOL				= 0xF3,
		BUSY_WITH_AUTO_SEQUENCE		= 0xF2,
		PIN_TIMEOUT							= 0xF0,
		PIN_CANCELLED							= 0xEF,
		CMD_SLOT_BUSY						= 0xE0
	};

	enum typeIccPower : unsigned char{
		Power_Unknown = 0x00,
		Power_5_0V = 0x01,
		Power_3_0V = 0x02,
		Power_1_8V = 0x03
	};
	enum typeIccProtocol : unsigned char{
		Protocol_T0 = 0x00,
		Protocol_T1 = 0x01
	};

	enum typeConvention : unsigned char{
		Convention_direct = 0x00,
		Convention_inverse = 0x02,
	};

/*
// 6.3.1 RDR_to_PC_NotifySlotChange
#define ICC_NOT_PRESENT                        0x00
#define ICC_PRESENT                            0x01
#define ICC_CHANGE                             0x02
#define ICC_INSERTED_EVENT                     (ICC_PRESENT+ICC_CHANGE)
*/
#pragma pack(push,1) //GCC 4.0 (since 2005)

// ccid message format SPEC .P27
typedef	struct tag_ccid_pc_to_rdr_header{
	unsigned char cMessageType;
	unsigned long dwLength;
	unsigned char cSlot;
	unsigned char cSeq;
	unsigned char asCmd[3];	//this menmber id change by each commad.
} ccid_pc_to_rdr_header;

typedef	struct tag_ccid_rdr_to_pc_header{
	unsigned char cMessageType;
	unsigned long dwLength;
	unsigned char cSlot;
	unsigned char cSeq;
	unsigned char cStatus;
	unsigned char cError;
	unsigned char asCmd[1];	//this menmber id change by each commad.
} ccid_rdr_to_pc_header;


// SCR messsage format for GemCore POS Pro.
typedef	struct tag_scr_message{
	unsigned char cSync;	//0x03
	unsigned char cCtrl;
	union{
		ccid_pc_to_rdr_header ccid_hdr_pc_to_rdr;
		ccid_rdr_to_pc_header ccid_hdr_rdr_to_pc;
	};
	unsigned char sData[1];		// this is variable size is determined by ccid_hdr.dwLength
	//unsigned char cLrc;		// cLrc is from cSync to sData
} scr_message;

#pragma pack(pop)

// return scr message size with scr_message pointer.
#define	GET_SIZE_SCR_PC_TO_RDR_MESSAGE(pMSG)	(pMSG->ccid_hdr_pc_to_rdr.dwLength+sizeof(ccid_pc_to_rdr_header)+3)
#define	GET_SIZE_SCR_RDR_TO_PC_MESSAGE(pMSG)	(pMSG->ccid_hdr_rdr_to_pc.dwLength+sizeof(ccid_rdr_to_pc_header)+3)


#define	SCR_SYNC_VALUE		0x03

#define	SCR_ACK_VALUE		0x06
#define	SCR_NAK_VALUE		0x15	//The fields CCID_HDR() and DATA() must not be present if the CTRL character value is NAK (15h).

//PC_TO_RDR_ESCAPE' bCommandEscape field value
#define	SCR_ESCAPE_CMD_GET_FW_VERSION			0x02
#define	SCR_ESCAPE_CMD_SLOT_LEVEL					0x1f
#define	SCR_ESCAPE_CMD_CARD_PARAMETERS			0x95
#define	SCR_ESCAPE_CMD_RESET_CARD_PARAMETERS	0x96
#define	SCR_ESCAPE_CMD_CALULATION_CHECKSUM		0x94

// for service.
#define	SCARD_CONVENTION_DIRECT	0x0000
#define	SCARD_CONVENTION_INVERSE	0x0002
#define	MAX_ATR_SIZE	33


//	unsigned char cProtocol(0);//T1 default cCWI=13, cBWI=4
//	unsigned char cFI(0), cDI(1), cN(0), cWI(10), cIFSC(32), cBWI(0), cCWI(0), cTCK(0), cT1(0);
//	bool bUsedLRC(false), bUsedCRC(false), bUsedDirectConvention(true), bUsedTCK(false);

typedef struct _SMARTCARD_PARAMEMTER
{
	bool bUsedDefaultValue;
	unsigned char cProtocol;
	unsigned char cFI;	//0
	unsigned char cDI;	//1
	unsigned char cN; //0
	union{
		struct{//T0
			unsigned char cWI;//  deafult : 10
		};

		struct{//T1
			unsigned char cBWI;// deafult : 4
			unsigned char cCWI;// deafult : 13
			unsigned char cIFSC;// deafult : 32
			unsigned char cChckSumType;	// deafult : 0 LRC
		};
	};

} SMARTCARD_PARAMEMTER, *PSMARTCARD_PARAMEMTER;

//////////////////////////////////////////////////////////////////

class CGemCoreScr_Helper
{
public:
	class CCardInfo{
	public:
		typedef	std::vector<SMARTCARD_PARAMEMTER>	typeVectorParameters;

		enum : unsigned char{		default_FI = 0,	default_DI = 1		};//common
		enum : unsigned char{		default_N = 0		};//common
		enum : unsigned char{		default_WI = 10		};//for T0
		enum : unsigned char{	
			default_BWI = 4,
			default_CWI = 13,
			default_IFSC = 32,
			deafult_CheckSumType = 0	//default LRC0
		};//for T1

	public:
		CCardInfo();
		~CCardInfo();

		CCardInfo( const _mp::type_v_buffer & vAtr );
		CCardInfo( const CCardInfo & info );

		CCardInfo & operator=( const CCardInfo & info );
		CCardInfo & operator=( _mp::type_v_buffer & vAtr );
		bool analysis_atr( const _mp::type_v_buffer & vAtr );

		typeVectorParameters get_parameters() const{		return m_vParameters;	}
		bool get_global_FIDI( unsigned char *pcFIDI = NULL ) const;
		bool get_global_N( unsigned char *pcN = NULL ) const;
		bool get_specific_WI( unsigned char *pcWI = NULL ) const;

		bool get_TA2( unsigned char *pcTA2 = NULL ) const;

		_mp::type_v_buffer get_atr() const {		return m_vAtr;	}
		_mp::type_v_buffer get_historical_bytes() const {	return m_vHistory;	}
		typeConvention get_conversion() const{			return m_Conversion;	}

		bool Is_allow_mode_change_by_TA2();
		bool Is_explicit_parameters_by_TA2();

	private:
		void ini();

	private:
		typedef	std::map<std::wstring, unsigned char > typeMapAtr;

	private:
		bool m_bTckOk;	//check TCK
		typeConvention m_Conversion;//TS
		_mp::type_v_buffer m_vAtr;
		_mp::type_v_buffer m_vHistory;
		_mp::type_v_buffer m_vTCK;

		typeMapAtr m_mapAtr;
		typeVectorParameters m_vParameters;
	};

public:
	// helper .......
	// get report string from extentions
	static std::wstring get_atr_report( const CCardInfo & CardInfo, const wchar_t cDelimitor = L'\n' );

	// Decodes the ATR and fills the structure 
	static bool decode_atr( CCardInfo & OutCardInfo, const _mp::type_v_buffer& vAtr );

	// extened build command all 
	static void build_ex_PC_to_RDR_XfrBlock(_mp::type_v_buffer& vtx, unsigned char cBWI, const std::wstring & sPdu );
	static void build_ex_PC_to_RDR_SetParameters( 
		_mp::type_v_buffer& vtx,
		typeIccProtocol protocol,
		unsigned char bmFindexDindex,
		unsigned char bmTCCKST,
		unsigned char bGuardTime,
		unsigned char bWaitingInteger,
		unsigned char bIFSC = 0	//T1 only
		);


	// build command
	static void build_PC_to_RDR_IccPowerOn( _mp::type_v_buffer & vtx, typeIccPower power );
	static void build_PC_to_RDR_IccPowerOff( _mp::type_v_buffer & vtx );
	static void build_PC_to_RDR_GetSlotStatus( _mp::type_v_buffer & vtx );
	static void build_PC_to_RDR_XfrBlock( _mp::type_v_buffer & vtx, unsigned char cBWI, const _mp::type_v_buffer & vdata );

	static void build_PC_to_RDR_SetParameters( _mp::type_v_buffer & vtx, typeIccProtocol protocol, const _mp::type_v_buffer & vdata );
	static void build_PC_to_RDR_GetParameters( _mp::type_v_buffer & vtx );
	static void build_PC_to_RDR_ResetParameters( _mp::type_v_buffer & vtx );
	static void build_PC_to_RDR_Escape( _mp::type_v_buffer & vtx, const _mp::type_v_buffer & vdata );
	static void build_PC_to_RDR_Abort( _mp::type_v_buffer & vtx );

	class CResponse{
	public:
		CResponse( const _mp::type_v_buffer & vrx );
		CResponse();
		~CResponse();

		CResponse( const CResponse & rep );
		CResponse & operator=( const CResponse & rep );

		void setRaw( const _mp::type_v_buffer & vrx );

		std::wstring getString(){	return m_sDescription;	}
		std::wstring getStringMessageType(){	return m_sMessageType;	}
		std::wstring getStringParameters(){	return m_sParameters;	}

		_mp::type_v_buffer getRaw(){	return m_vRaw;	}
		_mp::type_v_buffer getCcidRaw();
		_mp::type_v_buffer getCcidDataField();

	private:
		enum{
			const_size_min = 3
		};
	private:
		void ini();
		bool analysis();
	private:
		std::wstring m_sDescription;
		_mp::type_v_buffer m_vRaw;
		//
		std::wstring m_sMessageType;
		std::wstring m_sParameters;
	};

private:
	CGemCoreScr_Helper(void);
	~CGemCoreScr_Helper(void);

	static void build_PC_to_RDR( _mp::type_v_buffer & vtx, PC_to_RDR cMessageType ,unsigned char cP1, const _mp::type_v_buffer & vdata= _mp::type_v_buffer(0) );
	static unsigned char get_sequence_number();

private:
	//don't call these methods.
	CGemCoreScr_Helper( const CGemCoreScr_Helper & );
	CGemCoreScr_Helper & operator=( const CGemCoreScr_Helper & );

};
}	//gemcore_scr_interface
