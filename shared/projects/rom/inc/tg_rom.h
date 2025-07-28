#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <string>

#include <mp_os_type.h>

class CRom
{
public:
	enum type_result : uint32_t {
		result_success = 0,
		result_error = 1,
		result_error_not_found = 2,
		result_error_invalid_parameter = 3,
		result_error_shorter_then_expected = 4,
		result_error_not_loaded_dll = 5,
		result_error_not_open_file = 6,
		result_error_greater_then_expected = 7,
		result_error_over_capacity = 8
	};

	enum mask_update_condition : uint32_t {
		condition_no = 0x00000000,
		condition_eq = 0x00000001,	//equal
		condition_neq = 0x00000002,	//not equal
		condition_gt = 0x00000004,	//greater then
		condition_lt = 0x00000008	//less then
	};

#pragma pack(push,1)

	enum {
		MAX_ROMFILE_HEAD_ITEAM = 45,
		MAX_MODEL_NAME_SIZE = 30,
		MAX_RFU_SIZE = 128,
		//MAX_SIZE_APP					= 24*1024	// version 1.0
		MAX_SIZE_APP = 95443539	//version 1.1
	};

	typedef struct tagROMFILE_HEAD_ITEAM {
		uint32_t dwSize;			//the firmware data size
		uint32_t dwOffset;		//the firmware data starting offset from start of file.
		uint8_t sVersion[4];		// the version of firmware.
		// sVersion[0] : major version
		// sVersion[1] : minor version
		// sVersion[2] : bug fix version
		// sVersion[3] : build version
		uint8_t sModel[MAX_MODEL_NAME_SIZE + 1];	// device model name.

		uint32_t dwUpdateCondition;	//the combination of type_update_condition

		union {
			uint8_t sRFU[MAX_RFU_SIZE];//version 1.0
			uint8_t sHash[32];	//version 1.1 - SHA256 hash code of this firmware
		};

	} ROMFILE_HEAD_ITEAM, * PROMFILE_HEAD_ITEAM, * LPROMFILE_HEAD_ITEAM;

	// rom file head.
	typedef struct tagROMFILE_HEAD {
		uint32_t dwHeaderSize;	//the size of Header : dwHeaderSize = sizeof(ROMFILE_HEAD)
		uint8_t sFormatVersion[4];	// the version of header format. Now 1.0.0.0
		// sFormatVersion[0] : major version
		// sFormatVersion[1] : minor version
		// sFormatVersion[2] : bug fix version
		// sFormatVersion[3] : build version
//
		uint8_t sRFU[MAX_RFU_SIZE];

		uint32_t dwItem;			// the number of item.(max MAX_ROMFILE_HEAD_ITEAM )
		ROMFILE_HEAD_ITEAM Item[MAX_ROMFILE_HEAD_ITEAM];

	} ROMFILE_HEAD, * PROMFILE_HEAD, * LPROMFILE_HEAD;
#pragma pack(pop)

public:
	CRom(const wchar_t* lpctDllFile)
	{
		if (IncreaseReferenceCount() == 1) {
			HMODULE hMod = CRom::GetDllModule(lpctDllFile, true);	//loadlibrary
			assert(hMod);

			type_result result = CRom::load_header(NULL, NULL, lpctDllFile);
			assert(result == result_success);

			int n = CRom::get_updatable_item_index(NULL, NULL, 0, 0, 0, 0, lpctDllFile);
			assert(n == 0);

			result = CRom::get_item(NULL, 0, NULL, lpctDllFile);
			assert(result == result_success);

			n = CRom::readBinary_of_item(NULL, 0, 0, NULL, NULL, lpctDllFile);
			assert(n != 0);

			result = CRom::create_header(NULL, lpctDllFile);
			assert(result == result_success);

			result = CRom::add_item(NULL, NULL, 0, 0, 0, 0, NULL, 0, lpctDllFile);
			assert(result == result_success);
		}

		memset(&m_Header, 0, sizeof(m_Header));
	}

	virtual ~CRom()
	{
		if (IncreaseReferenceCount(false) == 0) {
			::FreeLibrary(GetDllModule());
		}
	}

private:
	typedef	type_result(WINAPI* type_tg_rom_load_header)(const wchar_t*, PROMFILE_HEAD);
	typedef	int (WINAPI* type_tg_rom_get_updatable_item_index)(const PROMFILE_HEAD, const uint8_t*, uint8_t, uint8_t, uint8_t, uint8_t);
	typedef	type_result(WINAPI* type_tg_rom_get_item)(const PROMFILE_HEAD, int, PROMFILE_HEAD_ITEAM);
	typedef	unsigned int (WINAPI* type_tg_rom_readBinary_of_item)(unsigned char* sRead, unsigned int dwRead, unsigned int dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctRomFile);

	typedef	type_result(WINAPI* type_tg_rom_create_header)(const wchar_t* lpctRomFile);
	typedef	type_result(WINAPI* type_tg_rom_add_item)(
		const wchar_t* lpctRomFile,
		const wchar_t* lpctBinFile,
		uint8_t cMajor,
		uint8_t cMinor,
		uint8_t cBugFix,
		uint8_t cBuild,
		uint8_t* sModel,
		uint32_t dwUpdateCondition
		);

private:

private:
	std::wstring m_sRomFileName;
	ROMFILE_HEAD m_Header;


	static unsigned int IncreaseReferenceCount(bool bIncrease = true)
	{
		static volatile LONG m_nRefCount = 0;

		if (bIncrease) {
			::InterlockedIncrement(&m_nRefCount);
		}
		else {
			if (m_nRefCount > 0)
				::InterlockedDecrement(&m_nRefCount);
		}

		return m_nRefCount;
	}

	static HMODULE GetDllModule(const wchar_t* lpctDllFile = NULL, bool bFirst = false)
	{
		static HMODULE hMod = NULL;

		if (bFirst) {
			//LoadPackagedLibrary
			hMod = ::LoadLibrary(lpctDllFile);
		}

		return hMod;
	}

	///////////////////////////////////////////////////////
	//internal dll exported function warpping mathod.
	static type_result load_header(const wchar_t* lpctRomFile, PROMFILE_HEAD pHeader, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_load_header m_cb_tg_rom_load_header = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_load_header = (type_tg_rom_load_header)::GetProcAddress(GetDllModule(lpctDllFile), "tg_rom_load_header");
			if (m_cb_tg_rom_load_header)
				return result_success;
			else
				return result_error_not_loaded_dll;
		}

		if (m_cb_tg_rom_load_header)
			return m_cb_tg_rom_load_header(lpctRomFile, pHeader);
		else
			return result_error_not_loaded_dll;

	}

	static int get_updatable_item_index(const PROMFILE_HEAD pHeader, const uint8_t* sModel, uint8_t cMajor, uint8_t cMinor, uint8_t cBugFix, uint8_t cBuild, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_get_updatable_item_index m_cb_tg_rom_get_updatable_item_index = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_get_updatable_item_index = (type_tg_rom_get_updatable_item_index)::GetProcAddress(GetDllModule(lpctDllFile), "tg_rom_get_updatable_item_index");
			if (m_cb_tg_rom_get_updatable_item_index)
				return 0;
			else
				return -1;
		}

		if (m_cb_tg_rom_get_updatable_item_index)
			return m_cb_tg_rom_get_updatable_item_index(pHeader, sModel, cMajor, cMinor, cBugFix, cBuild);
		else
			return -1;
	}

	static type_result get_item(const PROMFILE_HEAD pHeader, int nIndex, PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_get_item m_cb_tg_rom_get_item = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_get_item = (type_tg_rom_get_item)::GetProcAddress(GetDllModule(lpctDllFile), "tg_rom_get_item");
			if (m_cb_tg_rom_get_item)
				return result_success;
			else
				return result_error_not_loaded_dll;
		}

		if (m_cb_tg_rom_get_item)
			return m_cb_tg_rom_get_item(pHeader, nIndex, pItem);
		else
			return result_error_not_loaded_dll;
	}

	static unsigned int readBinary_of_item(unsigned char* sRead, unsigned int dwRead, unsigned int dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctRomFile, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_readBinary_of_item m_cb_tg_rom_readBinary_of_item = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_readBinary_of_item = (type_tg_rom_readBinary_of_item)::GetProcAddress(GetDllModule(lpctDllFile), "tg_rom_readBinary_of_item");
			if (m_cb_tg_rom_readBinary_of_item)
				return 1;
			else
				return 0;
		}

		if (m_cb_tg_rom_readBinary_of_item)
			return m_cb_tg_rom_readBinary_of_item(sRead, dwRead, dwOffset, pItem, lpctRomFile);
		else
			return 0;
	}

	static type_result create_header(const wchar_t* lpctRomFile, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_create_header m_cb_tg_rom_create_header = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_create_header = (type_tg_rom_create_header)::GetProcAddress(GetDllModule(lpctDllFile), "tg_rom_create_header");
			if (m_cb_tg_rom_create_header)
				return result_success;
			else
				return result_error_not_loaded_dll;
		}

		if (m_cb_tg_rom_create_header)
			return m_cb_tg_rom_create_header(lpctRomFile);
		else
			return result_error_not_loaded_dll;
	}

	static type_result add_item(
		const wchar_t* lpctRomFile,
		const wchar_t* lpctBinFile,
		uint8_t cMajor,
		uint8_t cMinor,
		uint8_t cBugFix,
		uint8_t cBuild,
		uint8_t* sModel,
		uint32_t dwUpdateCondition,
		const wchar_t* lpctDllFile = NULL
	)
	{
		static type_tg_rom_add_item m_cb_tg_rom_add_item = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_add_item = (type_tg_rom_add_item)::GetProcAddress(GetDllModule(lpctDllFile), "tg_rom_add_item");
			if (m_cb_tg_rom_add_item)
				return result_success;
			else
				return result_error_not_loaded_dll;
		}

		if (m_cb_tg_rom_add_item)
			return m_cb_tg_rom_add_item(
				lpctRomFile,
				lpctBinFile,
				cMajor, cMinor, cBugFix, cBuild,
				sModel,
				dwUpdateCondition
			);
		else
			return result_error_not_loaded_dll;
	}


public:
	///////////////////////////////////////////////////////
	//dll exported function warpping mathod.
	type_result LoadHeader(const wchar_t* lpctRomFile, PROMFILE_HEAD pHeader)
	{
		type_result result = CRom::load_header(lpctRomFile, &m_Header);

		if (result != result_success)
			return result;
		//
		::memcpy(pHeader, &m_Header, sizeof(m_Header));
		m_sRomFileName = lpctRomFile;
		return result;
	}

	int GetUpdatableItemIndex(const uint8_t* sModel, uint8_t cMajor, uint8_t cMinor, uint8_t cBugFix, uint8_t cBuild)
	{
		return CRom::get_updatable_item_index(&m_Header, sModel, cMajor, cMinor, cBugFix, cBuild);
	}

	type_result GetItem(int nIndex, PROMFILE_HEAD_ITEAM pItem)
	{
		return CRom::get_item(&m_Header, nIndex, pItem);
	}

	unsigned int ReadBinaryOfItem(unsigned char* sRead, unsigned int dwRead, unsigned int dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem)
	{
		return CRom::readBinary_of_item(sRead, dwRead, dwOffset, pItem, m_sRomFileName.c_str());
	}

	type_result CreateHeader(const wchar_t* lpctRomFile)
	{
		type_result result = CRom::create_header(lpctRomFile);
		if (result_success == result) {

			result = CRom::load_header(lpctRomFile, &m_Header);

			if (result_success == result)
				m_sRomFileName = lpctRomFile;
		}
		return result;
	}

	type_result AddItem(
		const wchar_t* lpctBinFile,
		uint8_t cMajor,
		uint8_t cMinor,
		uint8_t cBugFix,
		uint8_t cBuild,
		uint8_t* sModel,
		uint32_t dwUpdateCondition
	)
	{
		return CRom::add_item(m_sRomFileName.c_str(), lpctBinFile, cMajor, cMinor, cBugFix, cBuild, sModel, dwUpdateCondition);
	}


private:
	CRom();
	CRom(const CRom&);
	CRom& operator = (const CRom&);
};


