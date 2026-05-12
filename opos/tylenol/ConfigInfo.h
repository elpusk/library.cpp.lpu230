#pragma once

#include "windows.h"


#define	THE_NUMBER_OF_TRACKS		3
#define	THE_SIZE_OF_KEYS			16



class CConfigInfo
{
public:
	CConfigInfo(void);
	virtual ~CConfigInfo(void);

	static const INT32 KEY_SIZE = THE_SIZE_OF_KEYS;
	static const INT32 MAX_TRACKS = THE_NUMBER_OF_TRACKS;

	UINT32 m_nBuzzer;

	UINT32 m_nWdt;

	unsigned char m_cKeymap;

	unsigned char m_cId[THE_NUMBER_OF_TRACKS];

	unsigned char m_cEnableEnter[THE_NUMBER_OF_TRACKS];

	unsigned char m_cEnableEncryption[THE_NUMBER_OF_TRACKS];

	unsigned char m_sChangeKey[THE_NUMBER_OF_TRACKS][THE_SIZE_OF_KEYS];

	unsigned char m_sEncryptionKey[THE_NUMBER_OF_TRACKS][THE_SIZE_OF_KEYS];
	
public:
	void setBuzzer( UINT32 nBuzzer ){		m_nBuzzer = nBuzzer;	}
	void setWdt( UINT32 nWdt ){			m_nWdt = nWdt;		}

	void setKeymap(unsigned char cKeymap);
	void setId(INT32 nIndex, unsigned char cId);
	void setEnableEnter(INT32 nIndex, unsigned char cEnableEnter);
	void setEnableEncryption(INT32 nIndex, unsigned char cEnableEncryption);

	BOOL SetChangeKey( INT32 nIndex, LPCTSTR sChangeKey );
	BOOL SetEncryptionKey( INT32 nIndex, LPCTSTR EncryptionKey );

private:
	BOOL ChangeStringToHexToken( unsigned char *pcHex, const TCHAR *sHex );

};
