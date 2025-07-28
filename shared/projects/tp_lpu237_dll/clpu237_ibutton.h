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
#include <vector>
#include <sstream>
#include <iomanip>

#include <inc/tg_lpu237_ibutton.h>

class clpu237_ibutton
{
public:
	typedef enum : unsigned long {
		rr_success = LPU237LOCK_DLL_RESULT_SUCCESS,
		rr_error = LPU237LOCK_DLL_RESULT_ERROR,
		rr_cancel = LPU237LOCK_DLL_RESULT_CANCEL
	}type_result_read;

private:
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_get_list)(wchar_t*);//
	typedef	HANDLE(_CALLTYPE_* typeLPU237Lock_open)(const wchar_t*);//

	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_close)(HANDLE);//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_enable)(HANDLE);//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_disable)(HANDLE);//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_cancel_wait_key)(HANDLE);//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_wait_key_with_callback)(HANDLE, type_key_callback, void*);//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_get_data)(unsigned long, unsigned char*);//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_dll_on)();//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_dll_off)();//
	typedef	unsigned long(_CALLTYPE_* typeLPU237Lock_get_id)(HANDLE hDev, unsigned char* sId);//

public:
	static clpu237_ibutton& get_instance()
	{
		static clpu237_ibutton obj;
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

	~clpu237_ibutton()
	{
		if (m_FunLPU237Lock_dll_off)
			m_FunLPU237Lock_dll_off();

		if (m_hMode)
			_free_lib(m_hMode);
	}

	void unload()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		do {
			if (m_FunLPU237Lock_dll_off)
				m_FunLPU237Lock_dll_off();

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
			m_hMode = _load_lib(s_lib);
			if (m_hMode == NULL)
				continue;
			
			m_FunLPU237Lock_get_list = reinterpret_cast<typeLPU237Lock_get_list>(_load_symbol(m_hMode, "LPU237Lock_get_list"));
			m_FunLPU237Lock_open = reinterpret_cast<typeLPU237Lock_open>(_load_symbol(m_hMode, "LPU237Lock_open"));
			m_FunLPU237Lock_close = reinterpret_cast<typeLPU237Lock_close>(_load_symbol(m_hMode, "LPU237Lock_close"));

			m_FunLPU237Lock_enable = reinterpret_cast<typeLPU237Lock_close>(_load_symbol(m_hMode, "LPU237Lock_enable"));
			m_FunLPU237Lock_disable = reinterpret_cast<typeLPU237Lock_close>(_load_symbol(m_hMode, "LPU237Lock_disable"));

			m_FunLPU237Lock_cancel_wait_key = reinterpret_cast<typeLPU237Lock_cancel_wait_key>(_load_symbol(m_hMode, "LPU237Lock_cancel_wait_key"));

			m_FunLPU237Lock_wait_key_with_callback = reinterpret_cast<typeLPU237Lock_wait_key_with_callback>(_load_symbol(m_hMode, "LPU237Lock_wait_key_with_callback"));

			m_FunLPU237Lock_get_data = reinterpret_cast<typeLPU237Lock_get_data>(_load_symbol(m_hMode, "LPU237Lock_get_data"));

			m_FunLPU237Lock_dll_on = reinterpret_cast<typeLPU237Lock_dll_on>(_load_symbol(m_hMode, "LPU237Lock_dll_on"));
			m_FunLPU237Lock_dll_off = reinterpret_cast<typeLPU237Lock_dll_off>(_load_symbol(m_hMode, "LPU237Lock_dll_off"));

			m_FunLPU237Lock_get_id = reinterpret_cast<typeLPU237Lock_get_id>(_load_symbol(m_hMode, "LPU237Lock_get_id"));

			//
			if (m_FunLPU237Lock_get_list == NULL)	continue;
			if (m_FunLPU237Lock_open == NULL)	continue;
			if (m_FunLPU237Lock_close == NULL)	continue;

			if (m_FunLPU237Lock_enable == NULL)	continue;
			if (m_FunLPU237Lock_disable == NULL)	continue;

			if (m_FunLPU237Lock_cancel_wait_key == NULL)	continue;

			if (m_FunLPU237Lock_wait_key_with_callback == NULL)	continue;

			if (m_FunLPU237Lock_get_data == NULL)	continue;

			if (m_FunLPU237Lock_dll_on == NULL)	continue;
			if (m_FunLPU237Lock_dll_off == NULL)	continue;

			if (m_FunLPU237Lock_get_id == NULL)	continue;

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
	bool LPU237Lock_get_list(std::list<std::wstring> &list_dev)
	{
		bool b_result(false);

		list_dev.clear();

		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_get_list) {
				continue;
			}

			auto n_size = m_FunLPU237Lock_get_list(NULL);
			if (n_size <= 0) {
				b_result = true;
				continue;//none device
			}

			std::vector<unsigned char> v(n_size, 0);

			if (m_FunLPU237Lock_get_list((wchar_t*)&v[0]) == LPU237LOCK_DLL_RESULT_ERROR) {
				continue;
			}

			b_result = true;
			
			clpu237_ibutton::change(list_dev, (wchar_t*)&v[0]);

		} while (false);

		return b_result;
	}

	bool LPU237Lock_open(const std::wstring& sPath,HANDLE & h_dev)
	{
		bool b_result(false);
		do{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_open) {
				continue;
			}
			if (sPath.empty()) {
				continue;
			}
			h_dev = m_FunLPU237Lock_open(sPath.c_str());
			if (h_dev == 0 || h_dev == INVALID_HANDLE_VALUE) {
				continue;
			}
			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237Lock_close(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_close) {
				continue;
			}

			if (m_FunLPU237Lock_close(hDev) == LPU237LOCK_DLL_RESULT_ERROR) {
				continue;
			}
			b_result = true;
		} while(false);
		return b_result;
	}

	bool LPU237Lock_enable(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_enable) {
				continue;
			}

			if (m_FunLPU237Lock_enable(hDev) != LPU237LOCK_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237Lock_disable(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_disable) {
				continue;
			}

			if (m_FunLPU237Lock_disable(hDev) != LPU237LOCK_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237Lock_cancel_wait_key(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_cancel_wait_key) {
				continue;
			}

			if (m_FunLPU237Lock_cancel_wait_key(hDev) != LPU237LOCK_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	/**
	* get card data by callback method. async type function.
	* @parameter hDev - device handle.
	* @parameter pFun - callback function , defined by -> typedef	void (__stdcall__* type_key_callback)(void*) 
	* @parameter pParameter - callback function parameter. library internal thread call the given callback function(pFun) with this parameter.
	* @return first - true(success, device status is changed to wait a card data.), second - buffer index(When the pFun is called,
	* LPU237Lock_get_data() will be called with this value for getting a card data.)
	*/
	std::pair<bool, unsigned long> LPU237Lock_wait_key_with_callback(HANDLE  hDev, type_key_callback pFun, void* pParameter)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237LOCK_DLL_RESULT_ERROR);

		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_wait_key_with_callback) {
				continue;
			}

			n_buffer_index = m_FunLPU237Lock_wait_key_with_callback(hDev, pFun, pParameter);
			if (n_buffer_index == LPU237LOCK_DLL_RESULT_ERROR) {
				continue;
			}

			b_result = true;
		} while (false);

		return std::make_pair(b_result, n_buffer_index);
	}

	/**
	* get card data
	* @reurn first - result of operation(true: success, false: error). 
	* 
	*	second - result of reading i-button data.
	* 
	*/
	std::pair<bool, std::wstring > LPU237Lock_get_data(unsigned long dwBufferIndex)
	{
		bool b_result(false);
		std::wstring s_ibutton;

		do {

			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_get_data) {
				continue;
			}

			unsigned long dw_cnt_or_result = m_FunLPU237Lock_get_data(dwBufferIndex, NULL);
			if (dw_cnt_or_result == LPU237LOCK_DLL_RESULT_ERROR) {
				continue;
			}
			if (dw_cnt_or_result == LPU237LOCK_DLL_RESULT_CANCEL) {
				continue;
			}
			std::vector<unsigned char> v(dw_cnt_or_result,0);
			m_FunLPU237Lock_get_data(dwBufferIndex, &v[0]);

			std::wstringstream wss;
			wss << std::hex << std::setfill(L'0');
			for (unsigned char byte : v) {
				wss << std::setw(2) << static_cast<int>(byte);
			}

			s_ibutton = wss.str();
			b_result = true;

		} while (false);
		
		return std::make_pair(b_result, s_ibutton);
	}

	bool LPU237Lock_dll_on()
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_dll_on) {
				continue;
			}

			if (m_FunLPU237Lock_dll_on() != LPU237LOCK_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237Lock_dll_off()
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237Lock_dll_off) {
				continue;
			}

			if (m_FunLPU237Lock_dll_off() != LPU237LOCK_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	unsigned long LPU237Lock_get_id(HANDLE hDev, unsigned char* sId)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237Lock_get_id)	return m_FunLPU237Lock_get_id(hDev, sId);
		else							return LPU237LOCK_DLL_RESULT_ERROR;
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
		return dlopen(clpu237_ibutton::get_mcsc_from_unicode(s_lib).c_str(), RTLD_LAZY);
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

	clpu237_ibutton() : m_hMode(NULL), m_hDev(INVALID_HANDLE_VALUE)
	{
		_ini();
	}

	void _ini()
	{
		m_FunLPU237Lock_get_list = NULL;
		m_FunLPU237Lock_open = NULL;
		m_FunLPU237Lock_close = NULL;
		m_FunLPU237Lock_enable = NULL;
		m_FunLPU237Lock_disable = NULL;
		m_FunLPU237Lock_cancel_wait_key = NULL;
		m_FunLPU237Lock_wait_key_with_callback = NULL;
		m_FunLPU237Lock_get_data = NULL;
		m_FunLPU237Lock_dll_on = NULL;
		m_FunLPU237Lock_dll_off = NULL;
		m_FunLPU237Lock_get_id = NULL;
	}

private:
	std::mutex m_mutex_q;
	std::queue<unsigned long> m_q_buffer_index;

	std::mutex m_mutex;

	HMODULE m_hMode;
	HANDLE m_hDev;

	typeLPU237Lock_get_list m_FunLPU237Lock_get_list;
	typeLPU237Lock_open m_FunLPU237Lock_open;
	typeLPU237Lock_close m_FunLPU237Lock_close;
	typeLPU237Lock_enable m_FunLPU237Lock_enable;
	typeLPU237Lock_disable m_FunLPU237Lock_disable;
	typeLPU237Lock_cancel_wait_key m_FunLPU237Lock_cancel_wait_key;
	typeLPU237Lock_wait_key_with_callback m_FunLPU237Lock_wait_key_with_callback;
	typeLPU237Lock_get_data m_FunLPU237Lock_get_data;
	typeLPU237Lock_dll_on m_FunLPU237Lock_dll_on;
	typeLPU237Lock_dll_off m_FunLPU237Lock_dll_off;
	typeLPU237Lock_get_id m_FunLPU237Lock_get_id;

private://don't call these methods
	clpu237_ibutton(const clpu237_ibutton&);
	clpu237_ibutton& operator=(const clpu237_ibutton&);

};
