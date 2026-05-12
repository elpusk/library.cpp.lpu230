#include "stdafx.h"
#include "EL_SI.h"

////////////////////////////////////////////////
//internal gobler variables
//virtual machine constatns table pointer
ELIS_COST_ITEM gELISConstTable[ELIS_CONST_NUMBER];

ELIS_VM gELISVM;//virtual machine pointer

//virtual system general register
unsigned char gELIS_GREG0[ELIS_REG_00_SIZE];
unsigned char gELIS_GREG1[ELIS_REG_01_SIZE];
unsigned char gELIS_GREG2[ELIS_REG_02_SIZE];
unsigned char gELIS_GREG3[ELIS_REG_03_SIZE];
unsigned char gELIS_GREG4[ELIS_REG_04_SIZE];
unsigned char gELIS_GREG5[ELIS_REG_05_SIZE];
unsigned char gELIS_GREG6[ELIS_REG_06_SIZE];
unsigned char gELIS_GREG7[ELIS_REG_07_SIZE];
unsigned char gELIS_GREG8[ELIS_REG_08_SIZE];
unsigned char gELIS_GREG9[ELIS_REG_09_SIZE];


////////////////////////////////////////////////
//internal function prototype
void ELIS_BasicFun0( PELIS_VM pVM );
void ELIS_BasicFun1( PELIS_VM pVM );
void ELIS_BasicFun2( PELIS_VM pVM );
void ELIS_BasicFun3( PELIS_VM pVM );
void ELIS_BasicFun4( PELIS_VM pVM );
void ELIS_BasicFun5( PELIS_VM pVM );
void ELIS_BasicFun6( PELIS_VM pVM );
void ELIS_BasicFun7( PELIS_VM pVM );
void ELIS_BasicFun8( PELIS_VM pVM );

void ELIS_CP_Fun( PELIS_VM pVM );
void ELIS_IE_Fun( PELIS_VM pVM );
void ELIS_IG_Fun( PELIS_VM pVM );
void ELIS_IL_Fun( PELIS_VM pVM );
void ELIS_GO_Fun( PELIS_VM pVM );
void ELIS_JT_Fun( PELIS_VM pVM );
void ELIS_JF_Fun( PELIS_VM pVM );

////////////////////////////////////////////////
//internal function body
void ELIS_BasicFun0( PELIS_VM pVM )
{
}

void ELIS_BasicFun1( PELIS_VM pVM )
{
}

void ELIS_BasicFun2( PELIS_VM pVM )
{
}

void ELIS_BasicFun3( PELIS_VM pVM )
{
}

void ELIS_BasicFun4( PELIS_VM pVM )
{
}

void ELIS_BasicFun5( PELIS_VM pVM )
{
}

void ELIS_BasicFun6( PELIS_VM pVM )
{
}

void ELIS_BasicFun7( PELIS_VM pVM )
{
}

void ELIS_BasicFun8( PELIS_VM pVM )
{
}

void ELIS_CP_Fun( PELIS_VM pVM )
{
	unsigned char *pdes;
	unsigned char *psrc;

	//destination
	switch(pVM->pPST[pVM->IP].Operand0){
		case ELIS_REG_G0:
			pdes=pVM->pGREG[ELIS_REG_G0];
			pVM->L0=pVM->pPST[pVM->IP].Operand2;
			break;
		case ELIS_REG_G1:
			pdes=pVM->pGREG[ELIS_REG_G0];
			pVM->L1=pVM->pPST[pVM->IP].Operand2;
			break;
		case ELIS_REG_G2:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G3:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G4:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G5:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G6:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G7:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G8:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_G9:
			pdes=pVM->pGREG[ELIS_REG_G0];	break;
		case ELIS_REG_SW:	pdes=(unsigned char*)&pVM->SW;	break;
		case ELIS_REG_RV:	pdes=&pVM->RV;	break;
		case ELIS_REG_PW:	pdes=pVM->PW;	break;
		case ELIS_REG_LN:	pdes=(unsigned char*)&pVM->LN;	break;
		case ELIS_REG_IR:	pdes=pVM->IR;	break;
		case ELIS_REG_RT:	pdes=&pVM->RT;	break;
		case ELIS_REG_DL:	pdes=(unsigned char*)&pVM->DL;	break;
		case ELIS_REG_DT:	pdes=pVM->DT;	break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	//source
	if( pVM->pPST[pVM->IP].Operand1>127 ){
		//constants table
		psrc=pVM->pCST[pVM->pPST[pVM->IP].Operand1-128].pConst;
	}
	else{
		switch(pVM->pPST[pVM->IP].Operand1){
			case ELIS_REG_G0:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G1:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G2:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G3:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G4:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G5:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G6:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G7:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G8:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_G9:
				psrc=pVM->pGREG[ELIS_REG_G0];	break;
			case ELIS_REG_SW:	psrc=(unsigned char*)&pVM->SW;	break;
			case ELIS_REG_RV:	psrc=&pVM->RV;	break;
			case ELIS_REG_PW:	psrc=pVM->PW;	break;
			case ELIS_REG_LN:	psrc=(unsigned char*)&pVM->LN;	break;
			case ELIS_REG_IR:	psrc=pVM->IR;	break;
			case ELIS_REG_RT:	psrc=&pVM->RT;	break;
			case ELIS_REG_DL:	psrc=(unsigned char*)&pVM->DL;	break;
			case ELIS_REG_DT:	psrc=pVM->DT;	break;
			default:	pVM->RV=0;	pVM->IP++;	return;
		}//end switch
	}

	memcpy( pdes,psrc,pVM->pPST[pVM->IP].Operand2 );
	pVM->RV=1;//success
	pVM->IP++;
}

void ELIS_IE_Fun( PELIS_VM pVM )
{
	unsigned char *pdes;
	unsigned char *psrc;
	int ndes,nsrc,i;

	//operand0
	switch(pVM->pPST[pVM->IP].Operand0){
		case ELIS_REG_G0:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=pVM->L0;
			break;
		case ELIS_REG_G1:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=pVM->L1;
			break;
		case ELIS_REG_G2:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_02_SIZE;
			break;
		case ELIS_REG_G3:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_03_SIZE;
			break;
		case ELIS_REG_G4:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_04_SIZE;
			break;
		case ELIS_REG_G5:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_05_SIZE;
			break;
		case ELIS_REG_G6:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_06_SIZE;
			break;
		case ELIS_REG_G7:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_07_SIZE;
			break;
		case ELIS_REG_G8:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_08_SIZE;
			break;
		case ELIS_REG_G9:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_09_SIZE;
			break;
		case ELIS_REG_SW:
			pdes=(unsigned char*)&pVM->SW;
			ndes=ELIS_REG_SW_SIZE;
			break;
		case ELIS_REG_RV:
			pdes=&pVM->RV;
			ndes=ELIS_REG_RV_SIZE;
			break;
		case ELIS_REG_PW:
			pdes=pVM->PW;
			ndes=ELIS_REG_PW_SIZE;
			break;
		case ELIS_REG_LN:
			pdes=(unsigned char*)&pVM->LN;
			ndes=ELIS_REG_LN_SIZE;
			break;
		case ELIS_REG_IR:
			pdes=pVM->IR;
			ndes=ELIS_REG_IR_SIZE;
			break;
		case ELIS_REG_RT:
			pdes=&pVM->RT;
			ndes=ELIS_REG_RT_SIZE;
			break;
		case ELIS_REG_DL:
			pdes=(unsigned char*)&pVM->DL;
			ndes=ELIS_REG_DL_SIZE;
			break;
		case ELIS_REG_DT:
			pdes=pVM->DT;
			ndes=ELIS_REG_DT_SIZE;
			break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	//operand1
	switch(pVM->pPST[pVM->IP].Operand1){
		case ELIS_REG_G0:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=pVM->L0;
			break;
		case ELIS_REG_G1:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=pVM->L1;
			break;
		case ELIS_REG_G2:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_02_SIZE;
			break;
		case ELIS_REG_G3:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_03_SIZE;
			break;
		case ELIS_REG_G4:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_04_SIZE;
			break;
		case ELIS_REG_G5:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_05_SIZE;
			break;
		case ELIS_REG_G6:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_06_SIZE;
			break;
		case ELIS_REG_G7:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_07_SIZE;
			break;
		case ELIS_REG_G8:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_08_SIZE;
			break;
		case ELIS_REG_G9:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_09_SIZE;
			break;
		case ELIS_REG_SW:
			psrc=(unsigned char*)&pVM->SW;
			nsrc=ELIS_REG_SW_SIZE;
			break;
		case ELIS_REG_RV:
			psrc=&pVM->RV;
			nsrc=ELIS_REG_RV_SIZE;
			break;
		case ELIS_REG_PW:
			psrc=pVM->PW;
			nsrc=ELIS_REG_PW_SIZE;
			break;
		case ELIS_REG_LN:
			psrc=(unsigned char*)&pVM->LN;
			nsrc=ELIS_REG_LN_SIZE;
			break;
		case ELIS_REG_IR:
			psrc=pVM->IR;
			nsrc=ELIS_REG_IR_SIZE;
			break;
		case ELIS_REG_RT:
			psrc=&pVM->RT;
			nsrc=ELIS_REG_RT_SIZE;
			break;
		case ELIS_REG_DL:
			psrc=(unsigned char*)&pVM->DL;
			nsrc=ELIS_REG_DL_SIZE;
			break;
		case ELIS_REG_DT:
			psrc=pVM->DT;
			nsrc=ELIS_REG_DT_SIZE;
			break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	if( nsrc!=ndes ){
		pVM->RV=0;//not equal
		pVM->IP++;
		return;
	}

	for( i=0; i<nsrc; i++ ){
		if( psrc[i]!=pdes[i] ){
			pVM->RV=0;//not equal
			pVM->IP++;
			return;
		}
	}//end for

	pVM->RV=1;//equal
	pVM->IP++;
}

//operand0>operand1
void ELIS_IG_Fun( PELIS_VM pVM )
{
	unsigned char *pdes;
	unsigned char *psrc;
	int ndes,nsrc;

	//operand0
	switch(pVM->pPST[pVM->IP].Operand0){
		case ELIS_REG_G0:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=pVM->L0;
			break;
		case ELIS_REG_G1:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=pVM->L1;
			break;
		case ELIS_REG_G2:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_02_SIZE;
			break;
		case ELIS_REG_G3:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_03_SIZE;
			break;
		case ELIS_REG_G4:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_04_SIZE;
			break;
		case ELIS_REG_G5:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_05_SIZE;
			break;
		case ELIS_REG_G6:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_06_SIZE;
			break;
		case ELIS_REG_G7:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_07_SIZE;
			break;
		case ELIS_REG_G8:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_08_SIZE;
			break;
		case ELIS_REG_G9:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_09_SIZE;
			break;
		case ELIS_REG_SW:
			pdes=(unsigned char*)&pVM->SW;
			ndes=ELIS_REG_SW_SIZE;
			break;
		case ELIS_REG_RV:
			pdes=&pVM->RV;
			ndes=ELIS_REG_RV_SIZE;
			break;
		case ELIS_REG_PW:
			pdes=pVM->PW;
			ndes=ELIS_REG_PW_SIZE;
			break;
		case ELIS_REG_LN:
			pdes=(unsigned char*)&pVM->LN;
			ndes=ELIS_REG_LN_SIZE;
			break;
		case ELIS_REG_IR:
			pdes=pVM->IR;
			ndes=ELIS_REG_IR_SIZE;
			break;
		case ELIS_REG_RT:
			pdes=&pVM->RT;
			ndes=ELIS_REG_RT_SIZE;
			break;
		case ELIS_REG_DL:
			pdes=(unsigned char*)&pVM->DL;
			ndes=ELIS_REG_DL_SIZE;
			break;
		case ELIS_REG_DT:
			pdes=pVM->DT;
			ndes=ELIS_REG_DT_SIZE;
			break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	//operand1
	switch(pVM->pPST[pVM->IP].Operand1){
		case ELIS_REG_G0:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=pVM->L0;
			break;
		case ELIS_REG_G1:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=pVM->L1;
			break;
		case ELIS_REG_G2:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_02_SIZE;
			break;
		case ELIS_REG_G3:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_03_SIZE;
			break;
		case ELIS_REG_G4:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_04_SIZE;
			break;
		case ELIS_REG_G5:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_05_SIZE;
			break;
		case ELIS_REG_G6:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_06_SIZE;
			break;
		case ELIS_REG_G7:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_07_SIZE;
			break;
		case ELIS_REG_G8:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_08_SIZE;
			break;
		case ELIS_REG_G9:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_09_SIZE;
			break;
		case ELIS_REG_SW:
			psrc=(unsigned char*)&pVM->SW;
			nsrc=ELIS_REG_SW_SIZE;
			break;
		case ELIS_REG_RV:
			psrc=&pVM->RV;
			nsrc=ELIS_REG_RV_SIZE;
			break;
		case ELIS_REG_PW:
			psrc=pVM->PW;
			nsrc=ELIS_REG_PW_SIZE;
			break;
		case ELIS_REG_LN:
			psrc=(unsigned char*)&pVM->LN;
			nsrc=ELIS_REG_LN_SIZE;
			break;
		case ELIS_REG_IR:
			psrc=pVM->IR;
			nsrc=ELIS_REG_IR_SIZE;
			break;
		case ELIS_REG_RT:
			psrc=&pVM->RT;
			nsrc=ELIS_REG_RT_SIZE;
			break;
		case ELIS_REG_DL:
			psrc=(unsigned char*)&pVM->DL;
			nsrc=ELIS_REG_DL_SIZE;
			break;
		case ELIS_REG_DT:
			psrc=pVM->DT;
			nsrc=ELIS_REG_DT_SIZE;
			break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	if( nsrc!=ndes ){
		pVM->RV=0;//error
		pVM->IP++;
		return;
	}

	if( nsrc>2 ){
		pVM->RV=0;//error
		pVM->IP++;
		return;
	}

	if( pdes[1]>psrc[1] ){
		pVM->RV=1;//greater
		pVM->IP++;
		return;
	}
	else if( pdes[1]==psrc[1] ){
		if( pdes[0]>psrc[0] ){
			pVM->RV=1;//greater
			pVM->IP++;
			return;
		}
	}

	pVM->RV=0;//equal or less
	pVM->IP++;
}

//operand0 < operand1
void ELIS_IL_Fun( PELIS_VM pVM )
{
	unsigned char *pdes;
	unsigned char *psrc;
	int ndes,nsrc;

	//operand0
	switch(pVM->pPST[pVM->IP].Operand0){
		case ELIS_REG_G0:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=pVM->L0;
			break;
		case ELIS_REG_G1:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=pVM->L1;
			break;
		case ELIS_REG_G2:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_02_SIZE;
			break;
		case ELIS_REG_G3:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_03_SIZE;
			break;
		case ELIS_REG_G4:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_04_SIZE;
			break;
		case ELIS_REG_G5:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_05_SIZE;
			break;
		case ELIS_REG_G6:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_06_SIZE;
			break;
		case ELIS_REG_G7:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_07_SIZE;
			break;
		case ELIS_REG_G8:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_08_SIZE;
			break;
		case ELIS_REG_G9:
			pdes=pVM->pGREG[ELIS_REG_G0];
			ndes=ELIS_REG_09_SIZE;
			break;
		case ELIS_REG_SW:
			pdes=(unsigned char*)&pVM->SW;
			ndes=ELIS_REG_SW_SIZE;
			break;
		case ELIS_REG_RV:
			pdes=&pVM->RV;
			ndes=ELIS_REG_RV_SIZE;
			break;
		case ELIS_REG_PW:
			pdes=pVM->PW;
			ndes=ELIS_REG_PW_SIZE;
			break;
		case ELIS_REG_LN:
			pdes=(unsigned char*)&pVM->LN;
			ndes=ELIS_REG_LN_SIZE;
			break;
		case ELIS_REG_IR:
			pdes=pVM->IR;
			ndes=ELIS_REG_IR_SIZE;
			break;
		case ELIS_REG_RT:
			pdes=&pVM->RT;
			ndes=ELIS_REG_RT_SIZE;
			break;
		case ELIS_REG_DL:
			pdes=(unsigned char*)&pVM->DL;
			ndes=ELIS_REG_DL_SIZE;
			break;
		case ELIS_REG_DT:
			pdes=pVM->DT;
			ndes=ELIS_REG_DT_SIZE;
			break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	//operand1
	switch(pVM->pPST[pVM->IP].Operand1){
		case ELIS_REG_G0:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=pVM->L0;
			break;
		case ELIS_REG_G1:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=pVM->L1;
			break;
		case ELIS_REG_G2:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_02_SIZE;
			break;
		case ELIS_REG_G3:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_03_SIZE;
			break;
		case ELIS_REG_G4:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_04_SIZE;
			break;
		case ELIS_REG_G5:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_05_SIZE;
			break;
		case ELIS_REG_G6:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_06_SIZE;
			break;
		case ELIS_REG_G7:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_07_SIZE;
			break;
		case ELIS_REG_G8:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_08_SIZE;
			break;
		case ELIS_REG_G9:
			psrc=pVM->pGREG[ELIS_REG_G0];
			nsrc=ELIS_REG_09_SIZE;
			break;
		case ELIS_REG_SW:
			psrc=(unsigned char*)&pVM->SW;
			nsrc=ELIS_REG_SW_SIZE;
			break;
		case ELIS_REG_RV:
			psrc=&pVM->RV;
			nsrc=ELIS_REG_RV_SIZE;
			break;
		case ELIS_REG_PW:
			psrc=pVM->PW;
			nsrc=ELIS_REG_PW_SIZE;
			break;
		case ELIS_REG_LN:
			psrc=(unsigned char*)&pVM->LN;
			nsrc=ELIS_REG_LN_SIZE;
			break;
		case ELIS_REG_IR:
			psrc=pVM->IR;
			nsrc=ELIS_REG_IR_SIZE;
			break;
		case ELIS_REG_RT:
			psrc=&pVM->RT;
			nsrc=ELIS_REG_RT_SIZE;
			break;
		case ELIS_REG_DL:
			psrc=(unsigned char*)&pVM->DL;
			nsrc=ELIS_REG_DL_SIZE;
			break;
		case ELIS_REG_DT:
			psrc=pVM->DT;
			nsrc=ELIS_REG_DT_SIZE;
			break;
		default:	pVM->RV=0;	pVM->IP++;	return;
	}//end switch


	if( nsrc!=ndes ){
		pVM->RV=0;//error
		pVM->IP++;
		return;
	}

	if( nsrc>2 ){
		pVM->RV=0;//error
		pVM->IP++;
		return;
	}

	if( pdes[1]<psrc[1] ){
		pVM->RV=1;//less
		pVM->IP++;
		return;
	}
	else if( pdes[1]==psrc[1] ){
		if( pdes[0]<psrc[0] ){
			pVM->RV=1;//less
			pVM->IP++;
			return;
		}
	}

	pVM->RV=0;//equal or greater
	pVM->IP++;
}

void ELIS_GO_Fun( PELIS_VM pVM )
{
	pVM->IP=pVM->pPST[pVM->IP].label;
}

void ELIS_JT_Fun( PELIS_VM pVM )
{
	if( pVM->RV==1 )
		pVM->IP=pVM->pPST[pVM->IP].label;
	else
		pVM->IP++;
}

void ELIS_JF_Fun( PELIS_VM pVM )
{
	if( pVM->RV==0 )
		pVM->IP=pVM->pPST[pVM->IP].label;
	else
		pVM->IP++;
}


////////////////////////////////////////////////
// the definition of supported function body
////////////////////////////////////////////////

//initialize elpusk interface scripter virtual system
// processing good : return 1;
// processing failure : return 0;
unsigned char ELIS_IniVM( PELIS_VM pVM,void *pPST )
{
	//general register pointer
	pVM->pGREG[0]=gELIS_GREG0;
	pVM->pGREG[1]=gELIS_GREG1;
	pVM->pGREG[2]=gELIS_GREG2;
	pVM->pGREG[3]=gELIS_GREG3;
	pVM->pGREG[4]=gELIS_GREG4;
	pVM->pGREG[5]=gELIS_GREG5;
	pVM->pGREG[6]=gELIS_GREG6;
	pVM->pGREG[7]=gELIS_GREG7;
	pVM->pGREG[8]=gELIS_GREG8;
	pVM->pGREG[9]=gELIS_GREG9;

	memset( pVM->pGREG[0],0x00, ELIS_REG_00_SIZE );
	memset( pVM->pGREG[1],0x00, ELIS_REG_01_SIZE );
	memset( pVM->pGREG[2],0x00, ELIS_REG_02_SIZE );
	memset( pVM->pGREG[3],0x00, ELIS_REG_03_SIZE );
	memset( pVM->pGREG[4],0x00, ELIS_REG_04_SIZE );
	memset( pVM->pGREG[5],0x00, ELIS_REG_05_SIZE );
	memset( pVM->pGREG[6],0x00, ELIS_REG_06_SIZE );
	memset( pVM->pGREG[7],0x00, ELIS_REG_07_SIZE );
	memset( pVM->pGREG[8],0x00, ELIS_REG_08_SIZE );
	memset( pVM->pGREG[9],0x00, ELIS_REG_09_SIZE );

	//SFR
	pVM->SW=0;	// 2bytes
	pVM->RV=0;
	memset( pVM->PW,0x00, ELIS_REG_PW_SIZE );
	pVM->LN=0;	// 2bytes
	memset( pVM->IR,0x00, ELIS_REG_IR_SIZE );
	pVM->RT=0;
	pVM->DL=0;	// 2bytes
	memset( pVM->DT,0x00, ELIS_REG_DT_SIZE );

	//internal access register
	pVM->IP=0;
	pVM->L0=0;
	pVM->L1=0;

	pVM->pPST=(PELIS_INS_CODE)pPST;	//scripter starting pointer
	pVM->pCST=gELISConstTable;	//constants table statring table

	//basic function pointer arrary
	pVM->F[0]=ELIS_BasicFun0;
	pVM->F[1]=ELIS_BasicFun0;
	pVM->F[2]=ELIS_BasicFun0;
	pVM->F[3]=ELIS_BasicFun0;
	pVM->F[4]=ELIS_BasicFun0;
	pVM->F[5]=ELIS_BasicFun0;
	pVM->F[6]=ELIS_BasicFun0;
	pVM->F[7]=ELIS_BasicFun0;
	pVM->F[8]=ELIS_BasicFun0;

	return 1;
}

//Execute elpusk interface scripter.
// processing good : return 1;
// processing failure : return 0;
unsigned char ELIS_ExeVMC( PELIS_VM pVM )
{
	ELIS_INS_CODE sINS;

	//load current instructure
	memcpy( &sINS,&(pVM->pPST[pVM->IP]),4 );

	switch(sINS.op){
		case ELIS_OP_CP:	ELIS_CP_Fun(pVM);	return pVM->RV;
		case ELIS_OP_IE:	ELIS_IE_Fun(pVM);	return pVM->RV;
		case ELIS_OP_IG:	ELIS_IG_Fun(pVM);	return pVM->RV;
		case ELIS_OP_IL:	ELIS_IL_Fun(pVM);	return pVM->RV;
		case ELIS_OP_GO:	ELIS_GO_Fun(pVM);	return pVM->RV;
		case ELIS_OP_JT:	ELIS_JT_Fun(pVM);	return pVM->RV;
		case ELIS_OP_JF:	ELIS_JF_Fun(pVM);	return pVM->RV;
		//
		case ELIS_OP_F00:	ELIS_BasicFun0(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F01:	ELIS_BasicFun1(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F02:	ELIS_BasicFun2(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F03:	ELIS_BasicFun3(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F04:	ELIS_BasicFun4(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F05:	ELIS_BasicFun5(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F06:	ELIS_BasicFun6(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F07:	ELIS_BasicFun7(pVM);	pVM->IP++;	return pVM->RV;
		case ELIS_OP_F08:	ELIS_BasicFun8(pVM);	pVM->IP++;	return pVM->RV;
		default:
			return 0;
	}//end switch

	return 1;
}


