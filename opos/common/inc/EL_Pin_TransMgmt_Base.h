// EL_Pin_TransMgmt_Base.h: interface for the CEL_Pin_TansMgmt_Base class.
//////////////////////////////////////////////////////////////////////

#if !defined(__EL_PIN_TRANSACTION_MANAGMENT_BASE_H_20071005__)
#define __EL_PIN_TRANSACTION_MANAGMENT_BASE_H_20071005__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EL_Pin_IO_Base.h"
#include "EL_Pin_Cmd.h"
#include "EL_Pin_MisH.h"

//Constants
#define	DATA_FIELD_SIZE_IN_PACKET			50

enum enumPin_Trans_Type					//the type of transaction.
{
	PIN_TRANS_TYPE_S_OUT=0,				//only Single-OUT transaction.
	PIN_TRANS_TYPE_S_OUT_S_IN,			//Single-OUT + Single-IN transaction.
	PIN_TRANS_TYPE_M_OUT_S_IN,			//Multi-OUT + Single-IN transaction.
	PIN_TRANS_TYPE_S_OUT_M_IN,			//Single-OUT + Multi-Single IN transaction.
	PIN_TRANS_TYPE_M_OUT_M_IN			//Multi-OUT + Multi-Single IN transaction.
};

//structure 
#pragma pack(1)
/*
typedef struct TagEL_PACKET_H64{//64 bytes packet header
	unsigned char cType;//message type
	unsigned long ulLength;//the size ofdata field
	unsigned char cSlot;//the ICC slot number
	unsigned char cSeq;//the current transaction number
	unsigned char cSubType1;//sub message type1
	unsigned char cSubType2;//sub message type1
	unsigned char cSubType3;//sub message type1

	unsigned long ulChain;//chain number of current packet
} EL_PACKET_H64, *PEL_PACKET_H64,*LPEL_PACKET_H64;

typedef struct TagEL_PACKET64{
	EL_PACKET_H64 header;
	unsigned char sData[DATA_FIELD_SIZE_IN_PACKET];//
} EL_PACKET64, *PEL_PACKET64,*LPEL_PACKET64;
*/
typedef union TagEL_PACKET_H64{//64 bytes packet header
	struct{
	unsigned char cType;//message type
	unsigned long ulLength;//the size ofdata field
	unsigned char cSlot;//the ICC slot number
	unsigned char cSeq;//the current transaction number
	unsigned char cSubType1;//sub message type1
	unsigned char cSubType2;//sub message type1
	unsigned char cSubType3;//sub message type1

	unsigned long ulChain;//chain number of current packet
	};
	struct{
	unsigned char cType;//message type
	unsigned long ulLength;//the size ofdata field
	unsigned char cSlot;//the ICC slot number
	unsigned char cSeq;//the current transaction number
	unsigned char cSubType1;//sub message type1
	unsigned char cSubType2;//sub message type1
	unsigned char cSubType3;//sub message type1
	unsigned char cDataStart;//the start of data field
	};
} EL_PACKET_H64, *PEL_PACKET_H64,*LPEL_PACKET_H64;

typedef struct TagEL_PACKET64{
	EL_PACKET_H64 header;
	unsigned char sData[DATA_FIELD_SIZE_IN_PACKET];//
} EL_PACKET64, *PEL_PACKET64,*LPEL_PACKET64;


#pragma pack()

//
/////////////////////////////////////////////////////////

class CEL_Pin_TransMgmt_Base  
{
public:
	CEL_Pin_TransMgmt_Base();
	virtual ~CEL_Pin_TransMgmt_Base();

	virtual void Initial();//equl to constructure.

	//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////
	//for device access function and exported function
	
	//Pure virtual function.......
	//from PC to device.
	//bPair is true - transaction pair Out/in
	virtual DWORD OutTransaction( enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN,BYTE *pData=NULL,int *pnSize=NULL )=0;

	//Pure virtual function.......
	//from Device to PC.
	//IN  : *pnSize - pData buffer size
	//OUT : *pnSize - data size of pData buffer 
	//OUT : pData - including packet header.
	virtual DWORD InTransaction( BYTE *pData=NULL,int *pnSize=NULL )=0;

	//Pure virtual function.......
	//cancel Current transaction
	virtual DWORD CancelTransaction()=0;

	//Pure virtual function.......
	//check current status is idle
	virtual DWORD CheckIdleResponse()=0;

	//base function
	DWORD Open( TCHAR *sDevPath=NULL ){	//open device
		return m_PinBaseIO.Open( sDevPath );
	};
	
	DWORD Close(){	//close current opened device
		return m_PinBaseIO.Close();
	};

	DWORD Write( BYTE *pData=NULL,int *pnSize=NULL ){
		return m_PinBaseIO.Write( pData,pnSize );
	};
	DWORD Read( BYTE *pData=NULL,int *pnSize=NULL ){
		return m_PinBaseIO.Read( pData,pnSize );
	};

	unsigned char GetCurTransaction(){
		return m_cCurTransaction;//Zero is idle transaction
	};

	HANDLE GetCurHandle()
	{	return m_PinBaseIO.GetCurHandle();	};

	///////////////////////////////////////////////////////////////////

	BOOL IsFileTransMode()
	{	return m_bCurFileMode;	};

	void SetTransMode( BOOL bIsFileMode=FALSE,HANDLE hDataFile=NULL );

	int GetDevList( TCHAR *psDevList )
	{	return m_PinBaseIO.GetDevList( psDevList );	};

	int GetDevListEx( TCHAR *psDevList,enum DrvType Type )
	{	return m_PinBaseIO.GetDevListEx( psDevList,Type );	};

protected:
	//Pure virtual function.......
	//check IN phase complete 
	virtual DWORD InTransChk()=0;
	
	//Pure virtual function.......
	//check phase response on out transaction
	//bSingle is true - single transaction type.
	virtual DWORD OutTransChk( enum enumPin_Trans_Type TransType=PIN_TRANS_TYPE_S_OUT_S_IN )=0;
	
	//Pure virtual function.......
	//pass dummy respons. may be pass zero-response.
	virtual DWORD PassDummyResponse()=0;

	BYTE *m_pTransBuffer;

	EL_PACKET_H64 m_PreInHeader;//previous header
	EL_PACKET_H64 m_CurHeader;	//current header of current commnad	

	EL_PACKET64 m_CurOutPacket;	//current Out-packet
	EL_PACKET64 m_CurInPacket;	//current In-packet

	BOOL m_bCurFileMode;		//current file transfer mode.
								//the data field source of Out-phase is file.
	HANDLE m_hDataFile;			//the file handle of data on file transfer mode.

	BYTE m_cCurTransaction;//digit of current transaction

	CEL_Pin_IO_Base m_PinBaseIO;	//pin base IO object

	BYTE GetIncTransaction(){
		m_cCurTransaction++;//return increasing transction number
		if( m_cCurTransaction==0 )
			m_cCurTransaction++;

		return m_cCurTransaction;
	}; 

};

#endif // !defined(__EL_PIN_TRANSACTION_MANAGMENT_BASE_H_20071005__)
