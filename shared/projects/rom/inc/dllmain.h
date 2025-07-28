#pragma once
#include <inc/tg_rom.h>

///////////////////////////////////////////////////////////
// exported function prototype.
//

CRom::type_result _CALLTYPE_ tg_rom_load_header(const wchar_t* lpctRomFile, CRom::PROMFILE_HEAD pHeader);

int _CALLTYPE_ tg_rom_get_updatable_item_index(const CRom::PROMFILE_HEAD pHeader, const uint8_t* sModel, uint8_t cMajor, uint8_t cMinor, uint8_t cBugFix, uint8_t cBuild);

CRom::type_result _CALLTYPE_ tg_rom_get_item(const CRom::PROMFILE_HEAD pHeader, int nIndex, CRom::PROMFILE_HEAD_ITEAM pItem);

// return reading number
unsigned int _CALLTYPE_ tg_rom_readBinary_of_item(unsigned char* sRead, unsigned int dwRead, unsigned int dwOffset, CRom::PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctRomFile);

/////////////////////////////
// for building rom file.

// Create Rom file with default head values.
CRom::type_result _CALLTYPE_ tg_rom_create_header(const wchar_t* lpctRomFile);

// add item to rom file.
CRom::type_result _CALLTYPE_ tg_rom_add_item(
	const wchar_t* lpctRomFile,
	const wchar_t* lpctBinFile,
	uint8_t cMajor,
	uint8_t cMinor,
	uint8_t cBugFix,
	uint8_t cBuild,
	uint8_t* sModel,
	uint32_t dwUpdateCondition
);