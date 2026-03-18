#pragma once
#include <memory>
#include <mutex>
#include <codecvt>
#include <string>
#include <locale>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <array>
#include <queue>

#include <inc/tg_lpu237_fw.h>

class clpu237_fw
{
public:
	typedef enum : unsigned long {
		rr_success = LPU237_FW_RESULT_SUCCESS,
		rr_error = LPU237_FW_RESULT_ERROR,
		rr_cancel = LPU237_FW_RESULT_CANCEL
	}type_result_read;

private:
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_get_list)(wchar_t*);
	typedef	HANDLE(_CALLTYPE_* typeLPU237_fw_open)(const wchar_t*);

	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_close)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_save_setting)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_recover_setting)(HANDLE);

	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_cancel_update)();
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_update)(const unsigned char*, unsigned long, const wchar_t*, unsigned long);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_update_callback)(const unsigned char*, type_lpu237_fw_callback, void*, const wchar_t*, unsigned long);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_update_wnd)(const unsigned char*,HWND, UINT, const wchar_t*, unsigned long);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_rom_load)(const wchar_t*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_rom_get_index)(const wchar_t*, const unsigned char*, const unsigned char*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_on)();
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_off)();
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_get_id)(HANDLE, unsigned char*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_get_name)(HANDLE, unsigned char*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_get_version)(HANDLE hDev, unsigned char*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_get_version_major)(const unsigned char*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_fw_msr_get_version_minor)(const unsigned char*);

public:
	static clpu237_fw& get_instance()
	{
		static clpu237_fw obj;
		return obj;
	}

	static std::wstring get_result_string(type_result_read r)
	{
		std::wstring s(L"unknown code");
		switch (r) {
		case rr_success:
			s = L"success";
			break;
		case rr_error:
			s = L"error";
			break;
		case rr_cancel:
			s = L"cancel";
			break;
		default:
			break;
		}//end switch
		return s;
	}

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

	/*
	* p_ss_multi_dst can be null.
	* if p_ss_multi_dst is null, return the size of multi-string.( included separator null & string end mark( double nulls )
	* and unit is byte.
	* if p_ss_multi_dst isn't null, return the number of string in p_ss_multi_dst buffer.
	*/
	static size_t change(wchar_t* p_ss_multi_dst, const std::list<std::wstring>& list_s_src)
	{
		size_t n_string(0);
		do {

			size_t n_size = 0; //unit byte
			if (list_s_src.empty())
				continue;

			n_string = list_s_src.size();

			std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring path) {
				n_size += ((path.size() + 1) * sizeof(wchar_t));
				});

			n_size += sizeof(wchar_t);	//add multi null size

			if (p_ss_multi_dst == NULL) {
				n_string = n_size;	//return only need buffer size( BYTE unit, including NULL & NULLs )
				continue;
			}

			std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring& s_str) {

				for (size_t i = 0; i < s_str.length(); i++) {
					p_ss_multi_dst[i] = s_str[i];
				}//end for
				p_ss_multi_dst[s_str.length()] = L'\0';

				p_ss_multi_dst = &p_ss_multi_dst[s_str.length() + 1];
				});

			*p_ss_multi_dst = L'\0'; //make multi string

		} while (false);
		return n_string;
	}

	static size_t change(std::list<std::wstring>& list_s_dst, const wchar_t* p_ss_multi_src)
	{//p_ss_multi_src is multi string
		const wchar_t* pDest;
		std::wstring stemp;
		size_t n_count = 0;
		size_t nOffset = 0;

		do {
			if (p_ss_multi_src == NULL)
				continue;

			list_s_dst.clear();
			//
			while (p_ss_multi_src[nOffset] != 0) {

				pDest = &(p_ss_multi_src[nOffset]);
				stemp = pDest;
				list_s_dst.push_back(stemp);

				nOffset += stemp.length() + 1;//for passing null termination
				n_count++;
			}//while
		} while (false);
		return n_count;
	}

public:
	void push_buffer_index(unsigned long n_index)
	{
		std::lock_guard<std::mutex> lock(m_mutex_q);
		m_q_buffer_index.push(n_index);
	}

	std::pair<bool, unsigned long> pop_buffer_index()
	{
		bool b_result(false);
		unsigned long n_index(0xFFFFFFFF);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_q);

			if (m_q_buffer_index.empty()) {
				continue;
			}
			n_index = m_q_buffer_index.front();
			m_q_buffer_index.pop();

			b_result = true;
		} while (false);
		return std::make_pair(b_result, n_index);
	}

	~clpu237_fw()
	{
		if (m_FunLPU237_fw_dll_off)
			m_FunLPU237_fw_dll_off();

		if (m_hMode)
			_free_lib(m_hMode);
	}

	void unload()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		do {
			if (m_FunLPU237_fw_dll_off)
				m_FunLPU237_fw_dll_off();

			_ini();

			if (m_hMode)
				_free_lib(m_hMode);
			//
			m_hMode = NULL;

		} while (false);
	}


	bool load(const std::wstring& s_lib)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		bool b_result(false);

		do {
			if (m_hMode) {
				b_result = true; //already loaded
				continue;
			}
			m_hMode = _load_lib(s_lib);
			if (m_hMode == NULL)
				continue;
			
			m_FunLPU237_fw_get_list = reinterpret_cast<typeLPU237_fw_get_list>(_load_symbol(m_hMode, "LPU237_fw_get_list_w"));
			m_FunLPU237_fw_open = reinterpret_cast<typeLPU237_fw_open>(_load_symbol(m_hMode, "LPU237_fw_open_w"));
			m_FunLPU237_fw_close = reinterpret_cast<typeLPU237_fw_close>(_load_symbol(m_hMode, "LPU237_fw_close"));

			m_FunLPU237_fw_msr_save_setting = reinterpret_cast<typeLPU237_fw_msr_save_setting>(_load_symbol(m_hMode, "LPU237_fw_msr_save_setting"));
			m_FunLPU237_msr_recover_setting = reinterpret_cast<typeLPU237_fw_msr_recover_setting>(_load_symbol(m_hMode, "LPU237_fw_msr_recover_setting"));

			m_FunLPU237_fw_msr_cancel_update = reinterpret_cast<typeLPU237_fw_msr_cancel_update>(_load_symbol(m_hMode, "LPU237_fw_msr_cancel_update"));

#ifdef _WIN32
			m_FunLPU237_fw_msr_update = reinterpret_cast<typeLPU237_fw_msr_update>(_load_symbol(m_hMode, "LPU237_fw_msr_update_w"));
			m_FunLPU237_fw_msr_update_wnd = reinterpret_cast<typeLPU237_fw_msr_update_wnd>(_load_symbol(m_hMode, "LPU237_fw_msr_update_wnd_w"));
#endif // _WIN32

			m_FunLPU237_fw_msr_update_callback = reinterpret_cast<typeLPU237_fw_msr_update_callback>(_load_symbol(m_hMode, "LPU237_fw_msr_update_callback_w"));

			m_FunLPU237_fw_rom_load = reinterpret_cast<typeLPU237_fw_rom_load>(_load_symbol(m_hMode, "LPU237_fw_rom_load_w"));
			m_FunLPU237_fw_rom_get_index = reinterpret_cast<typeLPU237_fw_rom_get_index>(_load_symbol(m_hMode, "LPU237_fw_rom_get_index_w"));

			m_FunLPU237_fw_dll_on = reinterpret_cast<typeLPU237_fw_on>(_load_symbol(m_hMode, "LPU237_fw_on"));
			m_FunLPU237_fw_dll_off = reinterpret_cast<typeLPU237_fw_off>(_load_symbol(m_hMode, "LPU237_fw_off"));

			m_FunLPU237_fw_msr_get_id = reinterpret_cast<typeLPU237_fw_msr_get_id>(_load_symbol(m_hMode, "LPU237_fw_msr_get_id"));
			m_FunLPU237_fw_msr_get_name = reinterpret_cast<typeLPU237_fw_msr_get_name>(_load_symbol(m_hMode, "LPU237_fw_msr_get_name"));
			m_FunLPU237_fw_msr_get_version = reinterpret_cast<typeLPU237_fw_msr_get_version>(_load_symbol(m_hMode, "LPU237_fw_msr_get_version"));
			m_FunLPU237_fw_msr_get_version_major = reinterpret_cast<typeLPU237_fw_msr_get_version_major>(_load_symbol(m_hMode, "LPU237_fw_msr_get_version_major"));
			m_FunLPU237_fw_msr_get_version_minor = reinterpret_cast<typeLPU237_fw_msr_get_version_minor>(_load_symbol(m_hMode, "LPU237_fw_msr_get_version_minor"));

			//
			if (m_FunLPU237_fw_get_list == NULL)	continue;
			if (m_FunLPU237_fw_open == NULL)	continue;
			if (m_FunLPU237_fw_close == NULL)	continue;

			if (m_FunLPU237_fw_msr_save_setting == NULL)	continue;
			if (m_FunLPU237_msr_recover_setting == NULL)	continue;

			if (m_FunLPU237_fw_msr_cancel_update == NULL)	continue;

#ifdef _WIN32
			if (m_FunLPU237_fw_msr_update == NULL)	continue;
			if (m_FunLPU237_fw_msr_update_wnd == NULL)	continue;
#endif
			if (m_FunLPU237_fw_msr_update_callback == NULL)	continue;

			if (m_FunLPU237_fw_rom_load == NULL)	continue;
			if (m_FunLPU237_fw_rom_get_index == NULL)	continue;

			if (m_FunLPU237_fw_dll_on == NULL)	continue;
			if (m_FunLPU237_fw_dll_off == NULL)	continue;

			if (m_FunLPU237_fw_msr_get_id == NULL)	continue;
			if (m_FunLPU237_fw_msr_get_name == NULL)	continue;
			if (m_FunLPU237_fw_msr_get_version == NULL)	continue;
			if (m_FunLPU237_fw_msr_get_version_major == NULL)	continue;
			if (m_FunLPU237_fw_msr_get_version_minor == NULL)	continue;

			b_result = true;
		} while (false);

		if (!b_result) {
			_ini();
			if (m_hMode) {
				_free_lib(m_hMode);
				m_hMode = NULL;
			}
		}
		return b_result;
	}

	HANDLE get_handle()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_hDev;
	}
	//
	bool LPU237_fw_get_list(std::list<std::wstring> &list_dev)
	{
		bool b_result(false);

		list_dev.clear();

		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_get_list) {
				continue;
			}

			auto n_size = m_FunLPU237_fw_get_list(NULL);
			if (n_size <= 0) {
				b_result = true;
				continue;//none device
			}

			std::vector<unsigned char> v(n_size, 0);

			if (m_FunLPU237_fw_get_list((wchar_t*)&v[0]) == LPU237_FW_RESULT_ERROR) {
				continue;
			}

			b_result = true;
			
			clpu237_fw::change(list_dev, (wchar_t*)&v[0]);

		} while (false);

		return b_result;
	}

	bool LPU237_fw_open(const std::wstring& sPath,HANDLE & h_dev)
	{
		bool b_result(false);
		do{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_open) {
				continue;
			}
			if (sPath.empty()) {
				continue;
			}
			h_dev = m_FunLPU237_fw_open(sPath.c_str());
			if (h_dev == 0 || h_dev == INVALID_HANDLE_VALUE) {
				continue;
			}
			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_fw_close(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_close) {
				continue;
			}

			if (m_FunLPU237_fw_close(hDev) == LPU237_FW_RESULT_ERROR) {
				continue;
			}
			b_result = true;
		} while(false);
		return b_result;
	}

	bool LPU237_fw_msr_save_setting(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_save_setting) {
				continue;
			}

			if (m_FunLPU237_fw_msr_save_setting(hDev) != LPU237_FW_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_msr_recover_setting(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_msr_recover_setting) {
				continue;
			}

			if (m_FunLPU237_msr_recover_setting(hDev) != LPU237_FW_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_fw_msr_cancel_update()
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_cancel_update) {
				continue;
			}

			if (m_FunLPU237_fw_msr_cancel_update() != LPU237_FW_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	std::pair<bool,unsigned long> LPU237_fw_msr_update(const unsigned char* sId, unsigned long dwWaitTime, const wchar_t* sRomFileName, unsigned long dwIndex)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237_FW_RESULT_ERROR);
#ifdef _WIN32
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_update) {
				continue;
			}

			n_buffer_index = m_FunLPU237_fw_msr_update(sId,dwWaitTime,sRomFileName,dwIndex);
			if (n_buffer_index == LPU237_FW_RESULT_ERROR) {
				continue;
			}

			b_result = true;
		} while (false);
#endif
		return std::make_pair(b_result, n_buffer_index);

	}

	/**
	* fw update by callback method. async type function.
	* @parameter hDev - device handle.
	* @parameter pFun - callback function , defined by -> typedef	unsigned long(_CALLTYPE_* type_lpu237_fw_callback)(void*, unsigned long, unsigned long)
	* @parameter pParameter - callback function parameter. library internal thread call the given callback function(pFun) with this parameter.
	* @return true - success
	*/
	bool LPU237_fw_msr_update_callback(
		const std::vector<unsigned char>& vId
		, type_lpu237_fw_callback cbUpdate
		, void* pUser
		, const std::wstring& sRomFileName
		, unsigned long dwIndex
	)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237_FW_RESULT_ERROR);

		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_update_callback) {
				continue;
			}

			n_buffer_index = m_FunLPU237_fw_msr_update_callback(&vId[0], cbUpdate, pUser, sRomFileName.c_str(), dwIndex);
			if (n_buffer_index == LPU237_FW_RESULT_ERROR) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	std::pair<bool, unsigned long> LPU237_fw_msr_update_wnd(const unsigned char* sId, HWND hWnd, UINT uMsg, const wchar_t* sRomFileName, unsigned long dwIndex)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237_FW_RESULT_ERROR);
#ifdef _WIN32
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_update_wnd) {
				continue;
			}

			n_buffer_index = m_FunLPU237_fw_msr_update_wnd(sId, hWnd, uMsg, sRomFileName, dwIndex);
			if (n_buffer_index == LPU237_FW_RESULT_ERROR) {
				continue;
			}

			b_result = true;
	} while (false);
#endif
		return std::make_pair(b_result, n_buffer_index);
	}


	bool LPU237_fw_rom_load(const std::wstring &sRomFileName)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_rom_load) {
				continue;
			}
			if(sRomFileName.empty()) {
				continue;
			}
			if (m_FunLPU237_fw_rom_load(sRomFileName.c_str()) != LPU237_FW_RESULT_SUCCESS) {
				continue;
			}
			b_result = true;
		} while (false);
		return b_result;
	}
	std::pair<bool,unsigned long> LPU237_fw_rom_get_index(
		const std::wstring& sRomFileName
		, const std::vector<unsigned char>& vName
		, const std::vector<unsigned char>& vVersion
	)
	{
		bool b_result(false);
		unsigned long n_index(LPU237_FW_RESULT_ERROR);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_rom_get_index) {
				continue;
			}
			if (sRomFileName.empty()) {
				continue;
			}
			if(vName.empty() || vVersion.empty()) {
				continue;
			}
			n_index = m_FunLPU237_fw_rom_get_index(sRomFileName.c_str(), &vName[0], &vVersion[0]);
			if (n_index == LPU237_FW_RESULT_ERROR) {
				continue;
			}
			b_result = true;
		} while (false);
		return std::make_pair(b_result, n_index);
	}

	bool LPU237_fw_dll_on()
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_dll_on) {
				continue;
			}

			if (m_FunLPU237_fw_dll_on() != LPU237_FW_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_dll_off()
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_dll_off) {
				continue;
			}

			if (m_FunLPU237_fw_dll_off() != LPU237_FW_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	std::vector<unsigned char> LPU237_fw_msr_get_id(HANDLE hDev)
	{
		std::vector<unsigned char> vId(0);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_get_id) {
				continue;
			}

			size_t n_size = m_FunLPU237_fw_msr_get_id(hDev, NULL);
			if (n_size <= 0) {
				continue;
			}
			vId.resize(n_size, 0);
			if (m_FunLPU237_fw_msr_get_id(hDev, &vId[0]) == LPU237_FW_RESULT_ERROR) {
				vId.resize(0);
				continue;
			}

		} while (false);

		return vId;
	}
	std::vector<unsigned char> LPU237_fw_msr_get_name(HANDLE hDev)
	{
		std::vector<unsigned char> vName(0);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_get_name) {
				continue;
			}

			size_t n_size = m_FunLPU237_fw_msr_get_name(hDev, NULL);
			if (n_size <= 0) {
				continue;
			}
			vName.resize(n_size, 0);
			if (m_FunLPU237_fw_msr_get_name(hDev, &vName[0]) == LPU237_FW_RESULT_ERROR) {
				vName.resize(0);
				continue;
			}

		} while (false);

		return vName;
	}
	std::vector<unsigned char> LPU237_fw_msr_get_version(HANDLE hDev)
	{
		std::vector<unsigned char> vVersion(0);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_fw_msr_get_version) {
				continue;
			}

			size_t n_size = m_FunLPU237_fw_msr_get_version(hDev, NULL);
			if (n_size <= 0) {
				continue;
			}
			vVersion.resize(n_size, 0);
			if (m_FunLPU237_fw_msr_get_version(hDev, &vVersion[0]) == LPU237_FW_RESULT_ERROR) {
				vVersion.resize(0);
				continue;
			}

		} while (false);

		return vVersion;
	}

private:
#ifdef _WIN32
	HMODULE _load_lib(const std::wstring& s_lib)
	{
		if (s_lib.empty())
			return NULL;
		return ::LoadLibrary(s_lib.c_str());
	}
	void _free_lib(HMODULE m)
	{
		FreeLibrary(m);
	}

	FARPROC WINAPI _load_symbol(HMODULE m, const char* s_fun)
	{
		return ::GetProcAddress(m, s_fun);
	}

#else
	HMODULE _load_lib(const std::wstring& s_lib)
	{
		if (s_lib.empty())
			return NULL;
		//
		return dlopen(clpu237_fw::get_mcsc_from_unicode(s_lib).c_str(), RTLD_LAZY);
	}

	void _free_lib(HMODULE m)
	{
		dlclose(m);
	}

	void* _load_symbol(HMODULE m, const char* s_fun)
	{
		return dlsym(m, s_fun);
	}

#endif // _WIN32

	clpu237_fw() : m_hMode(NULL), m_hDev(INVALID_HANDLE_VALUE)
	{
		_ini();
	}

	void _ini()
	{
		m_FunLPU237_fw_get_list = NULL;
		m_FunLPU237_fw_open = NULL;
		m_FunLPU237_fw_close = NULL;
		m_FunLPU237_fw_msr_save_setting = NULL;
		m_FunLPU237_msr_recover_setting = NULL;
		m_FunLPU237_fw_msr_cancel_update = NULL;
		m_FunLPU237_fw_msr_update = NULL;
		m_FunLPU237_fw_msr_update_callback = NULL;
		m_FunLPU237_fw_msr_update_wnd = NULL;
		m_FunLPU237_fw_rom_load = NULL;
		m_FunLPU237_fw_rom_get_index = NULL;

		m_FunLPU237_fw_dll_on = NULL;
		m_FunLPU237_fw_dll_off = NULL;
		m_FunLPU237_fw_msr_get_id = NULL;
		m_FunLPU237_fw_msr_get_name = NULL;
		m_FunLPU237_fw_msr_get_version = NULL;
		m_FunLPU237_fw_msr_get_version_major = NULL;
		m_FunLPU237_fw_msr_get_version_minor = NULL;
	}

private:
	std::mutex m_mutex_q;
	std::queue<unsigned long> m_q_buffer_index;

	std::mutex m_mutex;

	HMODULE m_hMode;
	HANDLE m_hDev;

	typeLPU237_fw_get_list m_FunLPU237_fw_get_list;
	typeLPU237_fw_open m_FunLPU237_fw_open;
	typeLPU237_fw_close m_FunLPU237_fw_close;
	typeLPU237_fw_msr_save_setting m_FunLPU237_fw_msr_save_setting;
	typeLPU237_fw_msr_recover_setting m_FunLPU237_msr_recover_setting;
	typeLPU237_fw_msr_cancel_update m_FunLPU237_fw_msr_cancel_update;
	typeLPU237_fw_msr_update m_FunLPU237_fw_msr_update;
	typeLPU237_fw_msr_update_callback m_FunLPU237_fw_msr_update_callback;
	typeLPU237_fw_msr_update_wnd m_FunLPU237_fw_msr_update_wnd;
	typeLPU237_fw_rom_load m_FunLPU237_fw_rom_load;
	typeLPU237_fw_rom_get_index m_FunLPU237_fw_rom_get_index;
	typeLPU237_fw_on m_FunLPU237_fw_dll_on;
	typeLPU237_fw_off m_FunLPU237_fw_dll_off;
	typeLPU237_fw_msr_get_id m_FunLPU237_fw_msr_get_id;
	typeLPU237_fw_msr_get_name m_FunLPU237_fw_msr_get_name;
	typeLPU237_fw_msr_get_version m_FunLPU237_fw_msr_get_version;
	typeLPU237_fw_msr_get_version_major m_FunLPU237_fw_msr_get_version_major;
	typeLPU237_fw_msr_get_version_minor m_FunLPU237_fw_msr_get_version_minor;

private://don't call these methods
	clpu237_fw(const clpu237_fw&);
	clpu237_fw& operator=(const clpu237_fw&);

};
