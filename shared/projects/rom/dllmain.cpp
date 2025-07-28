// dllmain.cpp : Defines the entry point for the DLL application.

#include "inc/dllmain.h"

#include <fstream>
#include <wchar.h>

#include <KISA_SHA256.h>

#ifndef _WIN32
#endif // _WIN32

///////////////////////////////////////////////////////////
// exported function body.
//
#ifdef _WIN32
BOOL APIENTRY DllMain(HMODULE hModule,
	uint32_t  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#else
// linux
//linux only
static void _so_init(void) __attribute__((constructor));
static void _so_fini(void) __attribute__((destructor));
//when calls dlopen().
void _so_init(void)
{
	//printf("Shared library loaded\n");
	// NOT executed
}

//when calls dlclose().
void _so_fini(void)
{
	//printf("Shared library unloaded\n");
	// NOT executed
}

#endif // _WIN32

//////////////////////////
// for load rom file.
CRom::type_result _CALLTYPE_ tg_rom_load_header(const wchar_t* lpctRomFile, CRom::PROMFILE_HEAD pHeader)
{
	if (lpctRomFile == NULL || pHeader == NULL)
		return CRom::result_error_invalid_parameter;
	//
	std::ifstream Firmware;

	std::string s_rom_file = CRom::get_mcsc_from_unicode(lpctRomFile);

	Firmware.open(s_rom_file.c_str(), std::ios::binary | std::ios::in);

	if (!Firmware)
		return CRom::result_error_not_found;

	// check length
	Firmware.seekg(0, Firmware.end);
	unsigned long nFileSize = (unsigned long)Firmware.tellg();
	if (nFileSize < (unsigned long)sizeof(CRom::ROMFILE_HEAD)) {
		return CRom::result_error_shorter_then_expected;
	}

	Firmware.seekg(0);
	Firmware.read(reinterpret_cast<char*>(pHeader), (std::streamsize)sizeof(CRom::ROMFILE_HEAD));
	Firmware.close();
	return CRom::result_success;
}

int _CALLTYPE_ tg_rom_get_updatable_item_index(const CRom::PROMFILE_HEAD pHeader, const uint8_t* sModel, uint8_t cMajor, uint8_t cMinor, uint8_t cBugFix, uint8_t cBuild)
{
	int nIndex = -1;

	if (pHeader == NULL || sModel == NULL)
		return nIndex;
	if (pHeader->dwItem == 0 || pHeader->dwItem > CRom::MAX_ROMFILE_HEAD_ITEAM)
		return nIndex;

	//check Model name

	uint32_t i, j = 0;
	bool bEqualModel = true;

	for (i = 0; i < pHeader->dwItem; i++) {

		bEqualModel = true;
		j = 0;
		while (sModel[j] != 0x00 && pHeader->Item[i].sModel[j] != 0x00) {

			if (pHeader->Item[i].sModel[j] != sModel[j]) {
				bEqualModel = false;
				break;
			}

			j++;
		}//end while

		if (bEqualModel) {
			nIndex = i;
			break;	//exit for
		}

	}//end for

	bool bConditionOK = false;
	uint32_t dwCondition = pHeader->Item[nIndex].dwUpdateCondition;

	if (nIndex > -1) {
		// found name is equal.......
		if (pHeader->Item[nIndex].sVersion[0] > cMajor) {
			//rom file version  greater then given version.
			if (dwCondition & CRom::condition_gt || dwCondition & CRom::condition_neq)
				bConditionOK = true;
		}
		else if (pHeader->Item[nIndex].sVersion[0] < cMajor) {
			//rom file version  less then given version.
			if (dwCondition & CRom::condition_lt || dwCondition & CRom::condition_neq)
				bConditionOK = true;
		}
		else {
			//rom file version  equal given version.
			if (dwCondition & CRom::condition_eq) {

				if (pHeader->Item[nIndex].sVersion[1] == cMinor &&
					pHeader->Item[nIndex].sVersion[2] == cBugFix &&
					pHeader->Item[nIndex].sVersion[3] == cBuild
					) {
					bConditionOK = true;
				}
			}
		}
		///
		uint8_t sRomVer[] = { pHeader->Item[nIndex].sVersion[1], pHeader->Item[nIndex].sVersion[2], pHeader->Item[nIndex].sVersion[3] };
		uint8_t sGivenVer[] = { cMinor, cBugFix, cBuild };
		int nVer = 0;
		while (!bConditionOK && nVer < 3) {

			if (sRomVer[nVer] > sGivenVer[nVer]) {
				//rom file version  greater then given version.
				if (dwCondition & CRom::condition_gt || dwCondition & CRom::condition_neq)
					bConditionOK = true;
			}
			else if (sRomVer[nVer] < sGivenVer[nVer]) {
				//rom file version  less then given version.
				if (dwCondition & CRom::condition_lt || dwCondition & CRom::condition_neq)
					bConditionOK = true;
			}

			nVer++;
		}//end while

		if (!bConditionOK)
			nIndex = -1;

	}//if( nIndex > -1 )

	return nIndex;
}

CRom::type_result _CALLTYPE_ tg_rom_get_item(const CRom::PROMFILE_HEAD pHeader, int nIndex, CRom::PROMFILE_HEAD_ITEAM pItem)
{
	CRom::type_result result = CRom::result_success;

	if (pHeader == NULL || nIndex < 0 || nIndex >(CRom::MAX_ROMFILE_HEAD_ITEAM - 1) || pItem == NULL)
		return CRom::result_error_invalid_parameter;
	//
	if (nIndex > (pHeader->dwItem - 1))
		return CRom::result_error_invalid_parameter;

	memcpy(pItem, &(pHeader->Item[nIndex]), sizeof(CRom::ROMFILE_HEAD_ITEAM));

	return result;
}

unsigned int _CALLTYPE_ tg_rom_readBinary_of_item(unsigned char* sRead, unsigned int dwRead, unsigned int dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctRomFile)
{
	unsigned int nRead = 0;

	if (pItem == NULL || lpctRomFile == NULL || sRead == NULL || dwRead == 0)
		return nRead;
	//
	std::string s_rom_file = CRom::get_mcsc_from_unicode(lpctRomFile);

	std::ifstream rom;

	rom.open(s_rom_file.c_str(), std::ios::binary | std::ios::in);
	if (!rom)
		return nRead;
	//
	rom.seekg(pItem->dwOffset, rom.beg);
	rom.seekg(dwOffset, rom.cur);

	rom.read(reinterpret_cast<char*>(sRead), dwRead);

	nRead = rom.gcount();

	rom.close();

	return nRead;
}

/////////////////////////////
// for building rom file.
CRom::type_result _CALLTYPE_ tg_rom_create_header(const wchar_t* lpctRomFile)
{
	CRom::type_result result = CRom::result_success;

	if (lpctRomFile == NULL)
		return CRom::result_error_invalid_parameter;

	std::string s_rom_file = CRom::get_mcsc_from_unicode(lpctRomFile);
	std::ofstream romfile;

	romfile.open(s_rom_file.c_str(), std::ofstream::binary | std::ios::out | std::ios::trunc);

	if (!romfile.is_open())
		return CRom::result_error_not_open_file;
	//

	romfile.seekp(0);

	CRom::ROMFILE_HEAD header;

	memset(&header, 0, sizeof(header));

	header.dwHeaderSize = sizeof(CRom::ROMFILE_HEAD);
	header.sFormatVersion[0] = 1;
	header.sFormatVersion[1] = 1;//2016/6/10
	header.sFormatVersion[2] = 0;
	header.sFormatVersion[3] = 0;
	//
	romfile.write(reinterpret_cast<const char*>(&header), (std::streamsize)sizeof(header));
	romfile.flush();

	romfile.close();

	return result;
}

CRom::type_result _CALLTYPE_ tg_rom_add_item(
	const wchar_t* lpctRomFile,
	const wchar_t* lpctBinFile,
	uint8_t cMajor,
	uint8_t cMinor,
	uint8_t cBugFix,
	uint8_t cBuild,
	uint8_t* sModel,
	uint32_t dwUpdateCondition
)
{
	CRom::type_result result = CRom::result_success;

	if (lpctRomFile == NULL || lpctBinFile == NULL)
		return CRom::result_error_invalid_parameter;
	//
	std::string s_rom_file = CRom::get_mcsc_from_unicode(lpctRomFile);
	std::ifstream rRom;

	rRom.open(s_rom_file.c_str(), std::ios::binary | std::ios::in);

	if (!rRom)
		return CRom::result_error_not_found;

	rRom.seekg(0, rRom.end);
	unsigned long nLen = rRom.tellg();

	if (nLen < (unsigned long)sizeof(CRom::ROMFILE_HEAD)) {
		rRom.close();
		return CRom::result_error_shorter_then_expected;
	}

	//// load header.
	CRom::ROMFILE_HEAD header;
	rRom.seekg(0, rRom.beg);
	rRom.read(reinterpret_cast<char*>(&header), (std::streamsize)sizeof(header));
	rRom.close();

	if (header.dwItem == CRom::MAX_ROMFILE_HEAD_ITEAM) {
		return  CRom::result_error_over_capacity;
	}

	//here load header OK.
	std::string s_bin_file = CRom::get_mcsc_from_unicode(lpctBinFile);
	std::ifstream Bin;

	Bin.open(s_bin_file.c_str(), std::ios::binary | std::ios::in);

	if (!Bin)
		return CRom::result_error_not_found;

	//
	Bin.seekg(0, Bin.end);
	nLen = Bin.tellg();

	if (nLen > CRom::MAX_SIZE_APP) {
		Bin.close();
		return CRom::result_error_greater_then_expected;
	}

	//
	Bin.seekg(0, Bin.beg);

	std::fstream wRom;
	wRom.open(s_rom_file.c_str(), std::ofstream::binary | std::ios::out | std::ios::in);
	if (!wRom.is_open()) {
		Bin.close();
		return CRom::result_error_not_open_file;
	}

	//
	wRom.seekp(0, wRom.end);

	//create item header
	CRom::ROMFILE_HEAD_ITEAM item;

	::memset(&item, 0, sizeof(item));
	item.dwSize = static_cast<uint32_t>(nLen);
	item.dwOffset = wRom.tellp();
	item.sVersion[0] = cMajor;
	item.sVersion[1] = cMinor;
	item.sVersion[2] = cBugFix;
	item.sVersion[3] = cBuild;
	item.dwUpdateCondition = dwUpdateCondition;

	if (sModel) {

		//get model length except terminator NULL.
		int nModel = 0;

		while (sModel[nModel]) {
			nModel++;
		};

		if (nModel > CRom::MAX_MODEL_NAME_SIZE) {
			Bin.close();
			wRom.close();
			return CRom::result_error_greater_then_expected;
		}
		//
		::memcpy(item.sModel, sModel, nModel);
	}

	//
	SHA256_INFO sha_info;
	SHA256_Init(&sha_info);


	//update header	
	::memcpy(&header.Item[header.dwItem], &item, sizeof(item));
	header.dwItem++;

	//merge binary file to rom file
	// bin -> wRom
#define	BUF_LEN	 4*1024
	int nBuffer = BUF_LEN;
	char sBuffer[BUF_LEN];
	unsigned int i = 0;
	int j;

	while (Bin && !Bin.eof()) {

		nBuffer = BUF_LEN;
		Bin.read(sBuffer, nBuffer);

		nBuffer = Bin.gcount();
		wRom.write(sBuffer, nBuffer);
		if (i == 0) {
			for (j = 0; j < nBuffer; j++) {
				if (j % 2 == 0)
					sBuffer[j] = sBuffer[j] ^ 0xC0;
				else
					sBuffer[j] = sBuffer[j] ^ 0xFF;
			}//end ofr
		}

		SHA256_Process(&sha_info, (const  uint8_t*)sBuffer, nBuffer);
		++i;
	}//end while


	SHA256_Close(&sha_info, header.Item[header.dwItem - 1].sHash);
	//write header.
	wRom.seekp(0, wRom.beg);
	wRom.write(reinterpret_cast<const char*>(&header), (std::streamsize)sizeof(header));
	//
	Bin.close();
	wRom.flush();
	wRom.close();
	return result;
}