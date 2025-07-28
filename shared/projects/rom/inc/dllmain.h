#pragma once
#include <inc/tg_rom.h>

///////////////////////////////////////////////////////////
// exported function prototype.
//

CRom::type_result WINAPI tg_rom_load_header(LPCTSTR lpctRomFile, CRom::PROMFILE_HEAD pHeader);

int WINAPI tg_rom_get_updatable_item_index(const CRom::PROMFILE_HEAD pHeader, const BYTE* sModel, BYTE cMajor, BYTE cMinor, BYTE cBugFix, BYTE cBuild);

CRom::type_result WINAPI tg_rom_get_item(const CRom::PROMFILE_HEAD pHeader, int nIndex, CRom::PROMFILE_HEAD_ITEAM pItem);

// return reading number
unsigned int WINAPI tg_rom_readBinary_of_item(unsigned char* sRead, unsigned int dwRead, unsigned int dwOffset, CRom::PROMFILE_HEAD_ITEAM pItem, LPCTSTR lpctRomFile);

/////////////////////////////
// for building rom file.

// Create Rom file with default head values.
CRom::type_result WINAPI tg_rom_create_header(LPCTSTR lpctRomFile);

// add item to rom file.
CRom::type_result WINAPI tg_rom_add_item(
	LPCTSTR lpctRomFile,
	LPCTSTR lpctBinFile,
	BYTE cMajor,
	BYTE cMinor,
	BYTE cBugFix,
	BYTE cBuild,
	BYTE* sModel,
	DWORD dwUpdateCondition
);