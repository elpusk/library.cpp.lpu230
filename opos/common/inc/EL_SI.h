//////////////////////////////////////////////////////////
//EL scripter Interpreter header file
//////////////////////////////////////////////////////////


#if !defined(__EL_SI_HEADER_2007062529__)
#define __EL_SI_HEADER_2007062529__

////////////////////////////////////////////////
//1. include file
////////////////////////////////////////////////

////////////////////////////////////////////////
//2. define constatns
////////////////////////////////////////////////
//#define	??		??

//
#define	ELIS_CONST_NUMBER		20	//the number of constant of constatns-tale
#define	ELIS_REG_G_NUMBER		10	//the number of general register
#define	ELIS_FUN_NUMBER			9	//the number of Basic function

//general register size
#define	ELIS_REG_00_SIZE		256
#define	ELIS_REG_01_SIZE		256
#define	ELIS_REG_02_SIZE		16
#define	ELIS_REG_03_SIZE		16
#define	ELIS_REG_04_SIZE		2
#define	ELIS_REG_05_SIZE		2
#define	ELIS_REG_06_SIZE		2
#define	ELIS_REG_07_SIZE		1
#define	ELIS_REG_08_SIZE		1
#define	ELIS_REG_09_SIZE		1

//SFR(specfical function register) size
#define	ELIS_REG_SW_SIZE		2
#define	ELIS_REG_RV_SIZE		1
#define	ELIS_REG_PW_SIZE		16
#define	ELIS_REG_LN_SIZE		2
#define	ELIS_REG_IR_SIZE		256
#define	ELIS_REG_RT_SIZE		1
#define	ELIS_REG_DL_SIZE		2
#define	ELIS_REG_DT_SIZE		256

//interanl access register size
#define	ELIS_REG_L0_SIZE		1
#define	ELIS_REG_L1_SIZE		1
#define	ELIS_REG_IP_SIZE		2

//OP code assignmemt
#define	ELIS_OP_CP				0
#define	ELIS_OP_IE				1
#define	ELIS_OP_IG				2
#define	ELIS_OP_IL				3
#define	ELIS_OP_GO				4
#define	ELIS_OP_JT				5
#define	ELIS_OP_JF				6

#define	ELIS_OP_F00				0x40
#define	ELIS_OP_F01				0x41
#define	ELIS_OP_F02				0x42
#define	ELIS_OP_F03				0x43
#define	ELIS_OP_F04				0x44
#define	ELIS_OP_F05				0x45
#define	ELIS_OP_F06				0x46
#define	ELIS_OP_F07				0x47
#define	ELIS_OP_F08				0x48

//register index assignmemt
#define	ELIS_REG_G0				0
#define	ELIS_REG_G1				1
#define	ELIS_REG_G2				2
#define	ELIS_REG_G3				3
#define	ELIS_REG_G4				4
#define	ELIS_REG_G5				5
#define	ELIS_REG_G6				6
#define	ELIS_REG_G7				7
#define	ELIS_REG_G8				8
#define	ELIS_REG_G9				9

#define	ELIS_REG_SW				64
#define	ELIS_REG_RV				65
#define	ELIS_REG_PW				66
#define	ELIS_REG_LN				67
#define	ELIS_REG_IR				68
#define	ELIS_REG_RT				69
#define	ELIS_REG_DL				70
#define	ELIS_REG_DT				71
#define	ELIS_REG_L0				125
#define	ELIS_REG_L1				126
#define	ELIS_REG_IP				127

////////////////////////////////////////////////
//3. the definition of data structure
////////////////////////////////////////////////

#pragma pack(1)
//Elpusk interface scripter instruction code
typedef struct TagELIS_INS_CODE{
	unsigned char op;

	union{
		struct{
		unsigned char Operand0;
		unsigned char Operand1;
		};
		unsigned short label;//16 bit address
	};
	unsigned char Operand2;
} ELIS_INS_CODE,*PELIS_INS_CODE,*LPELIS_INS_CODE;

//thie structure is item of system constants-table.
typedef struct TagELIS_COST_ITEM{
	unsigned char cSize;
	unsigned char *pConst;
} ELIS_COST_ITEM, *PELIS_COST_ITEM, *LPELIS_COST_ITEM;

#pragma pack()

typedef struct TagELIS_VM{
	//general register pointer
	unsigned char *pGREG[ELIS_REG_G_NUMBER];

	//SFR
	unsigned short SW;	// 2bytes
	unsigned char RV;
	unsigned char PW[ELIS_REG_PW_SIZE];
	unsigned short LN;	// 2bytes
	unsigned char IR[ELIS_REG_IR_SIZE];
	unsigned char RT;
	unsigned short DL;	// 2bytes
	unsigned char DT[ELIS_REG_DT_SIZE];

	//internal access register
	unsigned short IP;
	unsigned char L0;
	unsigned char L1;

	PELIS_INS_CODE pPST;	//scripter starting pointer
	PELIS_COST_ITEM pCST;	//constants table statring table

	void (*F[ELIS_FUN_NUMBER])( struct TagELIS_VM *pVM );//basic function pointer arrary
	
}ELIS_VM, *PELIS_VM, *LPELIS_VM;
////////////////////////////////////////////////
//4. the definition of supported function prototype
////////////////////////////////////////////////

//initialize elpusk interface scripter virtual system
// processing good : return 1;
// processing failure : return 0;
unsigned char ELIS_IniVM( PELIS_VM pVM,void *pPST );

//Execute elpusk interface scripter.
// processing good : return 1;
// processing failure : return 0;
unsigned char ELIS_ExeVMC( PELIS_VM pVM );

////////////////////////////////////////////////
#endif//__EL_SI_HEADER_2007062529__

//the enf of file