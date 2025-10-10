#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <cstring>
#include <mutex>

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
	CRom(const wchar_t* lpctDllFile) : m_b_ini(false)
	{
		do {
			if (IncreaseReferenceCount() != 1) {
				m_b_ini = true;
				continue;	//already loaded.
			}
			HMODULE hMod = CRom::GetDllModule(lpctDllFile, true);	//loadlibrary
			if(!hMod) {
				//assert(false && "Failed to load tg_rom.dll or libtg_rom.so");
				CRom::_get_last_error_string_ref() = L"GetDllModule() is NULL";
				continue;
			}

			type_result result = CRom::load_header(NULL, NULL, lpctDllFile);
			if (result != result_success) {
				CRom::_get_last_error_string_ref() = L"Failed to load header from tg_rom.dll or libtg_rom.so";
				continue;
			}

			int n = CRom::get_updatable_item_index(NULL, NULL, 0, 0, 0, 0, lpctDllFile);
			if (n != 0) {
				CRom::_get_last_error_string_ref() = L"Failed to get updatable item index from tg_rom.dll or libtg_rom.so";
				continue;
			}

			result = CRom::get_item(NULL, 0, NULL, lpctDllFile);
			if (result != result_success) {
				CRom::_get_last_error_string_ref() = L"Failed to get item from tg_rom.dll or libtg_rom.so";
				continue;
			}

			n = CRom::readBinary_of_item(NULL, 0, 0, NULL, NULL, lpctDllFile);
			if (n == 0) {
				CRom::_get_last_error_string_ref() = L"Failed to read binary of item from tg_rom.dll or libtg_rom.so";
				continue;
			}

			result = CRom::create_header(NULL, lpctDllFile);
			if (result != result_success) {
				CRom::_get_last_error_string_ref() = L"Failed to create header in tg_rom.dll or libtg_rom.so";
				continue;
			}

			result = CRom::add_item(NULL, NULL, 0, 0, 0, 0, NULL, 0, lpctDllFile);
			if (result != result_success) {
				CRom::_get_last_error_string_ref() = L"Failed to add item in tg_rom.dll or libtg_rom.so";
				continue;
			}
			m_sDllFileName = std::wstring(lpctDllFile);
			m_b_ini = true;
		} while (false);

		memset(&m_Header, 0, sizeof(m_Header));
	}

	virtual ~CRom()
	{
		if (IncreaseReferenceCount(false) == 0) {
			CRom::_free_lib(GetDllModule());
		}
	}

	bool is_ini() const
	{
		return m_b_ini;
	}

	const std::wstring& get_dll_file_name() const
	{
		return m_sDllFileName;
	}

	const std::wstring& get_last_error() const
	{
		return CRom::_get_last_error_string_ref();
	}

private:
	typedef	type_result(_CALLTYPE_* type_tg_rom_load_header)(const wchar_t*, PROMFILE_HEAD);
	typedef	int (_CALLTYPE_* type_tg_rom_get_updatable_item_index)(const PROMFILE_HEAD, const uint8_t*, uint8_t, uint8_t, uint8_t, uint8_t);
	typedef	type_result(_CALLTYPE_* type_tg_rom_get_item)(const PROMFILE_HEAD, int, PROMFILE_HEAD_ITEAM);
	typedef	uint32_t(_CALLTYPE_* type_tg_rom_readBinary_of_item)(unsigned char* sRead, uint32_t dwRead, uint32_t dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctRomFile);

	typedef	type_result(_CALLTYPE_* type_tg_rom_create_header)(const wchar_t* lpctRomFile);
	typedef	type_result(_CALLTYPE_* type_tg_rom_add_item)(
		const wchar_t* lpctRomFile,
		const wchar_t* lpctBinFile,
		uint8_t cMajor,
		uint8_t cMinor,
		uint8_t cBugFix,
		uint8_t cBuild,
		uint8_t* sModel,
		uint32_t dwUpdateCondition
		);


public:
	static std::string get_mcsc_from_unicode(const std::wstring& s_unicode)
	{
		std::string s_mcsc;

		do {
			if (s_unicode.empty())
				continue;
			//
#ifdef _WIN32
			size_t size_needed = 0;
			wcstombs_s(&size_needed, nullptr, 0, s_unicode.c_str(), _TRUNCATE);
			if (size_needed > 0)
			{
				s_mcsc.resize(size_needed);
				wcstombs_s(&size_needed, &s_mcsc[0], size_needed, s_unicode.c_str(), _TRUNCATE);
			}
#else
			size_t size_needed = std::wcstombs(nullptr, s_unicode.c_str(), 0);
			if (size_needed != (size_t)-1)
			{
				s_mcsc.resize(size_needed);
				std::wcstombs(&s_mcsc[0], s_unicode.c_str(), size_needed);
			}
#endif
			else
			{
				s_mcsc.clear(); //default for error
			}

		} while (false);

		return s_mcsc;
	}

private:
	static std::wstring & _get_last_error_string_ref()
	{
		static std::wstring s_last_error;
		return s_last_error;
	}
private:
	std::wstring m_sDllFileName;	//the dll file name.
	std::wstring m_sRomFileName;
	ROMFILE_HEAD m_Header;
	bool m_b_ini;


#ifdef _WIN32
	static HMODULE _load_lib(const std::wstring& s_lib)
	{
		if (s_lib.empty())
			return NULL;
		return ::LoadLibrary(s_lib.c_str());
	}
	static void _free_lib(HMODULE m)
	{
		FreeLibrary(m);
	}

	static FARPROC WINAPI _load_symbol(HMODULE m, const char* s_fun)
	{
		return ::GetProcAddress(m, s_fun);
	}

#else
	static HMODULE _load_lib(const std::wstring& s_lib)
	{
		if (s_lib.empty())
			return NULL;
		//
		return dlopen(CRom::get_mcsc_from_unicode(s_lib).c_str(), RTLD_LAZY);
	}

	static void _free_lib(HMODULE m)
	{
		dlclose(m);
	}

	static void* _load_symbol(HMODULE m, const char* s_fun)
	{
		return dlsym(m, s_fun);
	}

#endif // _WIN32

	static unsigned int IncreaseReferenceCount(bool bIncrease = true)
	{
		static volatile unsigned int m_nRefCount = 0;
		static std::mutex m;

		std::lock_guard<std::mutex> lock(m); // Ensure thread safety

		if (bIncrease) {
			++m_nRefCount;
		}
		else {
			if (m_nRefCount > 0) {
				--m_nRefCount;
			}
		}

		return m_nRefCount;
	}

	static HMODULE GetDllModule(const wchar_t* lpctDllFile = NULL, bool bFirst = false)
	{
		static HMODULE hMod = NULL;

		if (bFirst && lpctDllFile) {
			hMod = CRom::_load_lib(std::wstring(lpctDllFile));
		}
		else {
			CRom::_get_last_error_string_ref() = L"dll name is empty.";
		}

		return hMod;
	}

	///////////////////////////////////////////////////////
	//internal dll exported function warpping mathod.
	static type_result load_header(const wchar_t* lpctRomFile, PROMFILE_HEAD pHeader, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_load_header m_cb_tg_rom_load_header = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_load_header = (type_tg_rom_load_header)CRom::_load_symbol(GetDllModule(lpctDllFile), "tg_rom_load_header");
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
			m_cb_tg_rom_get_updatable_item_index = (type_tg_rom_get_updatable_item_index)CRom::_load_symbol(GetDllModule(lpctDllFile), "tg_rom_get_updatable_item_index");
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
			m_cb_tg_rom_get_item = (type_tg_rom_get_item)CRom::_load_symbol(GetDllModule(lpctDllFile), "tg_rom_get_item");
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

	static uint32_t readBinary_of_item(unsigned char* sRead, uint32_t dwRead, uint32_t dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem, const wchar_t* lpctRomFile, const wchar_t* lpctDllFile = NULL)
	{
		static type_tg_rom_readBinary_of_item m_cb_tg_rom_readBinary_of_item = NULL;

		if (lpctDllFile) {
			m_cb_tg_rom_readBinary_of_item = (type_tg_rom_readBinary_of_item)CRom::_load_symbol(GetDllModule(lpctDllFile), "tg_rom_readBinary_of_item");
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
			m_cb_tg_rom_create_header = (type_tg_rom_create_header)CRom::_load_symbol(GetDllModule(lpctDllFile), "tg_rom_create_header");
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
			m_cb_tg_rom_add_item = (type_tg_rom_add_item)CRom::_load_symbol(GetDllModule(lpctDllFile), "tg_rom_add_item");
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
		memcpy(pHeader, &m_Header, sizeof(m_Header));
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

	uint32_t ReadBinaryOfItem(unsigned char* sRead, uint32_t dwRead, uint32_t dwOffset, const CRom::PROMFILE_HEAD_ITEAM pItem)
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


