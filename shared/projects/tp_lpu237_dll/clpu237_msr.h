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

#include <inc/tg_lpu237_dll.h>

class clpu237_msr
{
public:
	typedef enum : unsigned long {
		rr_success = LPU237_DLL_RESULT_SUCCESS,
		rr_error = LPU237_DLL_RESULT_ERROR,
		rr_cancel = LPU237_DLL_RESULT_CANCEL,
		rr_error_msr = LPU237_DLL_RESULT_ERROR_MSR,
		rr_icc_inserted = LPU237_DLL_RESULT_ICC_INSERTED,
		rr_icc_removed = LPU237_DLL_RESULT_ICC_REMOVED
	}type_result_read;

private:
	typedef	unsigned long(_CALLTYPE_* typeLPU237_get_list)(wchar_t*);
	typedef	HANDLE(_CALLTYPE_* typeLPU237_open)(const wchar_t*);

	typedef	unsigned long(_CALLTYPE_* typeLPU237_close)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_enable)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_disable)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_cancel_wait_swipe)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_wait_swipe_with_waits)(HANDLE);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_wait_swipe_with_callback)(HANDLE, type_callback, void*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_wait_swipe_with_message)(HANDLE, HWND, UINT);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_get_data)(unsigned long, unsigned long, unsigned char*);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_dll_on)();
	typedef	unsigned long(_CALLTYPE_* typeLPU237_dll_off)();
	typedef	unsigned long(_CALLTYPE_* typeLPU237_get_id)(HANDLE hDev, unsigned char* sId);

	// SCR functions
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_IccPowerOn)(HANDLE hDev, unsigned char cPower, unsigned char* sRx, unsigned long* lpnRx);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_IccPowerOff)(HANDLE hDev);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_XfrBlock)(HANDLE hDev, unsigned char cBWI, const unsigned char* sTx, unsigned long nTx, unsigned char* sRx, unsigned long* lpnRx);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_GetParameters)(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_SetParameters)(HANDLE hDev, unsigned char cProtocol, unsigned char bmFindexDindex, unsigned char bmTCCKST, unsigned char bGuardTime, unsigned char bWaitingInteger, unsigned char bIFSC);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_ResetParameters)(HANDLE hDev);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_GetSlotStatus)(HANDLE hDev);
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_bypass_Escape)(HANDLE hDev, const unsigned char* sTx, unsigned long nTx, unsigned char* sRx, unsigned long* lpnRx);

	//SCR helper function
	typedef	unsigned long(_CALLTYPE_* typeLPU237_SCR_helper_GetLastError)();
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_GetFirmwareVersion)(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_GetReaderMode)(HANDLE hDev, unsigned char* lpcMode);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_SetReaderMode)(HANDLE hDev, unsigned char cMode, unsigned char cPower);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_XfrBlock)(HANDLE hDev, const unsigned char* sTx, unsigned long nTx, unsigned char* sRx, unsigned long* lpnRx);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_SetCardParameters)(HANDLE hDev, unsigned char cProtocol, unsigned char cParameter, unsigned char cNewValue);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_ResetCardParameters)(HANDLE hDev);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_result_is_success)(unsigned long dwScrResult);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_SetParameters)(HANDLE hDev, const unsigned char* sAtr, unsigned long nAtr);
	typedef	BOOL(_CALLTYPE_* typeLPU237_SCR_helper_GetATRParameterString)(wchar_t* sOut, unsigned long* lpnOut, const unsigned char* sAtr, unsigned long nAtr, wchar_t cDelimiter);

public:
	static clpu237_msr& get_instance()
	{
		static clpu237_msr obj;
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
		case rr_error_msr:
			s = L"msr reading error";
			break;
		case rr_icc_inserted:
			s = L"ICC inserted";
			break;
		case rr_icc_removed:
			s = L"ICC removed";
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

	~clpu237_msr()
	{
		if (m_FunLPU237_dll_off)
			m_FunLPU237_dll_off();

		if (m_hMode)
			_free_lib(m_hMode);
	}

	void unload()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		do {
			if (m_FunLPU237_dll_off)
				m_FunLPU237_dll_off();

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
			
			m_FunLPU237_get_list = reinterpret_cast<typeLPU237_get_list>(_load_symbol(m_hMode, "LPU237_get_list"));
			m_FunLPU237_open = reinterpret_cast<typeLPU237_open>(_load_symbol(m_hMode, "LPU237_open"));
			m_FunLPU237_close = reinterpret_cast<typeLPU237_close>(_load_symbol(m_hMode, "LPU237_close"));

			m_FunLPU237_enable = reinterpret_cast<typeLPU237_close>(_load_symbol(m_hMode, "LPU237_enable"));
			m_FunLPU237_disable = reinterpret_cast<typeLPU237_close>(_load_symbol(m_hMode, "LPU237_disable"));

			m_FunLPU237_cancel_wait_swipe = reinterpret_cast<typeLPU237_cancel_wait_swipe>(_load_symbol(m_hMode, "LPU237_cancel_wait_swipe"));

#ifdef _WIN32
			m_FunLPU237_wait_swipe_with_waits = reinterpret_cast<typeLPU237_wait_swipe_with_waits>(_load_symbol(m_hMode, "LPU237_wait_swipe_with_waits"));
			m_FunLPU237_wait_swipe_with_message = reinterpret_cast<typeLPU237_wait_swipe_with_message>(_load_symbol(m_hMode, "LPU237_wait_swipe_with_message"));
#endif // _WIN32

			m_FunLPU237_wait_swipe_with_callback = reinterpret_cast<typeLPU237_wait_swipe_with_callback>(_load_symbol(m_hMode, "LPU237_wait_swipe_with_callback"));

			m_FunLPU237_get_data = reinterpret_cast<typeLPU237_get_data>(_load_symbol(m_hMode, "LPU237_get_data"));

			m_FunLPU237_dll_on = reinterpret_cast<typeLPU237_dll_on>(_load_symbol(m_hMode, "LPU237_dll_on"));
			m_FunLPU237_dll_off = reinterpret_cast<typeLPU237_dll_off>(_load_symbol(m_hMode, "LPU237_dll_off"));

			m_FunLPU237_get_id = reinterpret_cast<typeLPU237_get_id>(_load_symbol(m_hMode, "LPU237_get_id"));

			m_FunLPU237_SCR_bypass_IccPowerOn = reinterpret_cast<typeLPU237_SCR_bypass_IccPowerOn>(_load_symbol(m_hMode, "LPU237_SCR_bypass_IccPowerOn"));
			m_FunLPU237_SCR_bypass_IccPowerOff = reinterpret_cast<typeLPU237_SCR_bypass_IccPowerOff>(_load_symbol(m_hMode, "LPU237_SCR_bypass_IccPowerOff"));
			m_FunLPU237_SCR_bypass_XfrBlock = reinterpret_cast<typeLPU237_SCR_bypass_XfrBlock>(_load_symbol(m_hMode, "LPU237_SCR_bypass_XfrBlock"));
			m_FunLPU237_SCR_bypass_GetParameters = reinterpret_cast<typeLPU237_SCR_bypass_GetParameters>(_load_symbol(m_hMode, "LPU237_SCR_bypass_GetParameters"));
			m_FunLPU237_SCR_bypass_SetParameters = reinterpret_cast<typeLPU237_SCR_bypass_SetParameters>(_load_symbol(m_hMode, "LPU237_SCR_bypass_SetParameters"));
			m_FunLPU237_SCR_bypass_ResetParameters = reinterpret_cast<typeLPU237_SCR_bypass_ResetParameters>(_load_symbol(m_hMode, "LPU237_SCR_bypass_ResetParameters"));
			m_FunLPU237_SCR_bypass_GetSlotStatus = reinterpret_cast<typeLPU237_SCR_bypass_GetSlotStatus>(_load_symbol(m_hMode, "LPU237_SCR_bypass_GetSlotStatus"));
			m_FunLPU237_SCR_bypass_Escape = reinterpret_cast<typeLPU237_SCR_bypass_Escape>(_load_symbol(m_hMode, "LPU237_SCR_bypass_Escape"));

			m_FunLPU237_SCR_helper_GetLastError = reinterpret_cast<typeLPU237_SCR_helper_GetLastError>(_load_symbol(m_hMode, "LPU237_SCR_helper_GetLastError"));
			m_FunLPU237_SCR_helper_GetFirmwareVersion = reinterpret_cast<typeLPU237_SCR_helper_GetFirmwareVersion>(_load_symbol(m_hMode, "LPU237_SCR_helper_GetFirmwareVersion"));
			m_FunLPU237_SCR_helper_GetReaderMode = reinterpret_cast<typeLPU237_SCR_helper_GetReaderMode>(_load_symbol(m_hMode, "LPU237_SCR_helper_GetReaderMode"));
			m_FunLPU237_SCR_helper_SetReaderMode = reinterpret_cast<typeLPU237_SCR_helper_SetReaderMode>(_load_symbol(m_hMode, "LPU237_SCR_helper_SetReaderMode"));
			m_FunLPU237_SCR_helper_XfrBlock = reinterpret_cast<typeLPU237_SCR_helper_XfrBlock>(_load_symbol(m_hMode, "LPU237_SCR_helper_XfrBlock"));
			m_FunLPU237_SCR_helper_SetCardParameters = reinterpret_cast<typeLPU237_SCR_helper_SetCardParameters>(_load_symbol(m_hMode, "LPU237_SCR_helper_SetCardParameters"));
			m_FunLPU237_SCR_helper_ResetCardParameters = reinterpret_cast<typeLPU237_SCR_helper_ResetCardParameters>(_load_symbol(m_hMode, "LPU237_SCR_helper_ResetCardParameters"));

			m_FunLPU237_SCR_helper_result_is_success = reinterpret_cast<typeLPU237_SCR_helper_result_is_success>(_load_symbol(m_hMode, "LPU237_SCR_helper_result_is_success"));
			m_FunLPU237_SCR_helper_SetParameters = reinterpret_cast<typeLPU237_SCR_helper_SetParameters>(_load_symbol(m_hMode, "LPU237_SCR_helper_SetParameters"));
			m_FunLPU237_SCR_helper_GetATRParameterString = reinterpret_cast<typeLPU237_SCR_helper_GetATRParameterString>(_load_symbol(m_hMode, "LPU237_SCR_helper_GetATRParameterString_w"));
			//
			if (m_FunLPU237_get_list == NULL)	continue;
			if (m_FunLPU237_open == NULL)	continue;
			if (m_FunLPU237_close == NULL)	continue;

			if (m_FunLPU237_enable == NULL)	continue;
			if (m_FunLPU237_disable == NULL)	continue;

			if (m_FunLPU237_cancel_wait_swipe == NULL)	continue;

#ifdef _WIN32
			if (m_FunLPU237_wait_swipe_with_waits == NULL)	continue;
			if (m_FunLPU237_wait_swipe_with_message == NULL)	continue;
#endif
			if (m_FunLPU237_wait_swipe_with_callback == NULL)	continue;

			if (m_FunLPU237_get_data == NULL)	continue;

			if (m_FunLPU237_dll_on == NULL)	continue;
			if (m_FunLPU237_dll_off == NULL)	continue;

			if (m_FunLPU237_get_id == NULL)	continue;

			if (m_FunLPU237_SCR_bypass_IccPowerOn == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_IccPowerOff == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_XfrBlock == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_GetParameters == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_SetParameters == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_ResetParameters == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_GetSlotStatus == NULL)	continue;
			if (m_FunLPU237_SCR_bypass_Escape == NULL)	continue;

			if (m_FunLPU237_SCR_helper_GetLastError == NULL)	continue;
			if (m_FunLPU237_SCR_helper_GetFirmwareVersion == NULL)	continue;
			if (m_FunLPU237_SCR_helper_GetReaderMode == NULL)	continue;
			if (m_FunLPU237_SCR_helper_SetReaderMode == NULL)	continue;
			if (m_FunLPU237_SCR_helper_XfrBlock == NULL)	continue;
			if (m_FunLPU237_SCR_helper_SetCardParameters == NULL)	continue;
			if (m_FunLPU237_SCR_helper_ResetCardParameters == NULL)	continue;

			if (m_FunLPU237_SCR_helper_result_is_success == NULL)	continue;
			if (m_FunLPU237_SCR_helper_SetParameters == NULL)	continue;
			if (m_FunLPU237_SCR_helper_GetATRParameterString == NULL)	continue;

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
	bool LPU237_get_list(std::list<std::wstring> &list_dev)
	{
		bool b_result(false);

		list_dev.clear();

		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_get_list) {
				continue;
			}

			auto n_size = m_FunLPU237_get_list(NULL);
			if (n_size <= 0) {
				b_result = true;
				continue;//none device
			}

			std::vector<unsigned char> v(n_size, 0);

			if (m_FunLPU237_get_list((wchar_t*)&v[0]) == LPU237_DLL_RESULT_ERROR) {
				continue;
			}

			b_result = true;
			
			clpu237_msr::change(list_dev, (wchar_t*)&v[0]);

		} while (false);

		return b_result;
	}

	bool LPU237_open(const std::wstring& sPath,HANDLE & h_dev)
	{
		bool b_result(false);
		do{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_open) {
				continue;
			}
			if (sPath.empty()) {
				continue;
			}
			h_dev = m_FunLPU237_open(sPath.c_str());
			if (h_dev == 0 || h_dev == INVALID_HANDLE_VALUE) {
				continue;
			}
			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_close(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_close) {
				continue;
			}

			if (m_FunLPU237_close(hDev) == LPU237_DLL_RESULT_ERROR) {
				continue;
			}
			b_result = true;
		} while(false);
		return b_result;
	}

	bool LPU237_enable(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_enable) {
				continue;
			}

			if (m_FunLPU237_enable(hDev) != LPU237_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_disable(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_disable) {
				continue;
			}

			if (m_FunLPU237_disable(hDev) != LPU237_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	bool LPU237_cancel_wait_swipe(HANDLE  hDev)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_cancel_wait_swipe) {
				continue;
			}

			if (m_FunLPU237_cancel_wait_swipe(hDev) != LPU237_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	std::pair<bool,unsigned long> LPU237_wait_swipe_with_waits(HANDLE  hDev)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);
#ifdef _WIN32
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_wait_swipe_with_waits) {
				continue;
			}

			n_buffer_index = m_FunLPU237_wait_swipe_with_waits(hDev);
			if (n_buffer_index == LPU237_DLL_RESULT_ERROR) {
				continue;
			}

			b_result = true;
		} while (false);
#endif
		return std::make_pair(b_result, n_buffer_index);

	}

	/**
	* get card data by callback method. async type function.
	* @parameter hDev - device handle.
	* @parameter pFun - callback function , defined by -> typedef	void (__stdcall__* type_callback)(void*) 
	* @parameter pParameter - callback function parameter. library internal thread call the given callback function(pFun) with this parameter.
	* @return first - true(success, device status is changed to wait a card data.), second - buffer index(When the pFun is called,
	* LPU237_get_data() will be called with this value for getting a card data.)
	*/
	std::pair<bool, unsigned long> LPU237_wait_swipe_with_callback(HANDLE  hDev, type_callback pFun, void* pParameter)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);

		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_wait_swipe_with_callback) {
				continue;
			}

			n_buffer_index = m_FunLPU237_wait_swipe_with_callback(hDev, pFun, pParameter);
			if (n_buffer_index == LPU237_DLL_RESULT_ERROR) {
				continue;
			}

			b_result = true;
		} while (false);

		return std::make_pair(b_result, n_buffer_index);
	}

	std::pair<bool, unsigned long> LPU237_wait_swipe_with_message(HANDLE  hDev, HWND hWnd, UINT nMsg)
	{
		bool b_result(false);
		unsigned long n_buffer_index(LPU237_DLL_RESULT_ERROR);
#ifdef _WIN32
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_wait_swipe_with_message) {
				continue;
			}

			n_buffer_index = m_FunLPU237_wait_swipe_with_message(hDev, hWnd, nMsg);
			if (n_buffer_index == LPU237_DLL_RESULT_ERROR) {
				continue;
			}

			b_result = true;
	} while (false);
#endif
		return std::make_pair(b_result, n_buffer_index);
	}


	/**
	* @brief get card data
	* @reurn first - reading result(true - success, false - error),
	*		second - array of reading result for each track(1,2,3).
	*/
	std::pair<bool, std::array<clpu237_msr::type_result_read,3> > LPU237_get_data(unsigned long dwBufferIndex, std::array<std::wstring,3> & ar_s_iso)
	{
		bool b_result(false);
		std::array<clpu237_msr::type_result_read, 3> ar_result{ clpu237_msr::rr_error,clpu237_msr::rr_error,clpu237_msr::rr_error };

		do {
			for (auto i = 0; i < ar_s_iso.size(); i++) {
				ar_s_iso[i].clear();
			}//end for

			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_get_data) {
				continue;
			}
			std::array<unsigned long, 3> ar_track{ 1,2,3 };
			unsigned long dw_cnt_or_result(0);
			//
			b_result = true;
			//
			for (auto n : ar_track) {
				dw_cnt_or_result = m_FunLPU237_get_data(dwBufferIndex, n, NULL);

				if (dw_cnt_or_result == LPU237_DLL_RESULT_ERROR) {
					ar_result[n-1] = clpu237_msr::rr_error;
					b_result = false;
					continue;
				}
				if (dw_cnt_or_result == LPU237_DLL_RESULT_CANCEL) {
					ar_result[n - 1] = clpu237_msr::rr_cancel;
					b_result = false;
					continue;
				}
				if (dw_cnt_or_result == LPU237_DLL_RESULT_ICC_INSERTED) {
					ar_result[n - 1] = clpu237_msr::rr_error;// error in lpu237
					b_result = false;
					continue;
				}
				if (dw_cnt_or_result == LPU237_DLL_RESULT_ICC_REMOVED) {
					ar_result[n - 1] = clpu237_msr::rr_error;//error in lpu237
					b_result = false;
					continue;
				}
				if (dw_cnt_or_result == LPU237_DLL_RESULT_ERROR_MSR) {
					ar_result[n - 1] = clpu237_msr::rr_error_msr;
					continue;
				}
				//
				ar_result[n - 1] = clpu237_msr::rr_success;

				if (dw_cnt_or_result == 0) {//none track data.
					ar_s_iso[n - 1].clear();
					continue;
				}
				std::vector<unsigned char> v(dw_cnt_or_result, 0);

				m_FunLPU237_get_data(dwBufferIndex, n, &v[0]);

				std::for_each(v.begin(), v.end(), [&](const unsigned char c) {
					ar_s_iso[n-1].push_back(static_cast<wchar_t>(c));
					});


			}//end for

		} while (false);
		
		return std::make_pair(b_result, ar_result);
	}

	bool LPU237_dll_on()
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_FunLPU237_dll_on) {
				continue;
			}

			if (m_FunLPU237_dll_on() != LPU237_DLL_RESULT_SUCCESS) {
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
			if (!m_FunLPU237_dll_off) {
				continue;
			}

			if (m_FunLPU237_dll_off() != LPU237_DLL_RESULT_SUCCESS) {
				continue;
			}

			b_result = true;
		} while (false);

		return b_result;
	}

	unsigned long LPU237_get_id(HANDLE hDev, unsigned char* sId)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_get_id)	return m_FunLPU237_get_id(hDev, sId);
		else							return LPU237_DLL_RESULT_ERROR;
	}

	unsigned long LPU237_SCR_bypass_IccPowerOn(HANDLE hDev, unsigned char cPower, unsigned char* sRx, unsigned long* lpnRx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_IccPowerOn)	return m_FunLPU237_SCR_bypass_IccPowerOn(hDev, cPower, sRx, lpnRx);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_IccPowerOff(HANDLE hDev)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_IccPowerOff)	return m_FunLPU237_SCR_bypass_IccPowerOff(hDev);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_XfrBlock(HANDLE hDev, unsigned char cBWI, const unsigned char* sTx, unsigned long nTx, unsigned char* sRx, unsigned long* lpnRx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_XfrBlock)	return m_FunLPU237_SCR_bypass_XfrBlock(hDev, cBWI, sTx, nTx, sRx, lpnRx);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_GetParameters(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_GetParameters)	return m_FunLPU237_SCR_bypass_GetParameters(hDev, sRx, lpnRx);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_SetParameters(HANDLE hDev, unsigned char cProtocol, unsigned char bmFindexDindex, unsigned char bmTCCKST, unsigned char bGuardTime, unsigned char bWaitingInteger, unsigned char bIFSC)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_SetParameters)	return m_FunLPU237_SCR_bypass_SetParameters(hDev, cProtocol, bmFindexDindex, bmTCCKST, bGuardTime, bWaitingInteger, bIFSC);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_ResetParameters(HANDLE hDev)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_ResetParameters)	return m_FunLPU237_SCR_bypass_ResetParameters(hDev);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_GetSlotStatus(HANDLE hDev)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_GetSlotStatus)	return m_FunLPU237_SCR_bypass_GetSlotStatus(hDev);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_bypass_Escape(HANDLE hDev, const unsigned char* sTx, unsigned long nTx, unsigned char* sRx, unsigned long* lpnRx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_bypass_Escape)	return m_FunLPU237_SCR_bypass_Escape(hDev, sTx, nTx, sRx, lpnRx);
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	unsigned long LPU237_SCR_helper_GetLastError()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_GetLastError)	return m_FunLPU237_SCR_helper_GetLastError();
		else							return LPU237_DLL_SCR_RESULT_STATUS_CMD_FAIL | LPU237_DLL_SCR_RESULT_STATUS_ICC_UNKNOWN;
	}

	BOOL LPU237_SCR_helper_GetFirmwareVersion(HANDLE hDev, unsigned char* sRx, unsigned long* lpnRx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_GetFirmwareVersion)	return m_FunLPU237_SCR_helper_GetFirmwareVersion(hDev, sRx, lpnRx);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_GetReaderMode(HANDLE hDev, unsigned char* lpcMode)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_GetReaderMode)	return m_FunLPU237_SCR_helper_GetReaderMode(hDev, lpcMode);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetReaderMode(HANDLE hDev, unsigned char cMode, unsigned char cPower)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_SetReaderMode)	return m_FunLPU237_SCR_helper_SetReaderMode(hDev, cMode, cPower);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_XfrBlock(HANDLE hDev, const unsigned char* sTx, unsigned long nTx, unsigned char* sRx, unsigned long* lpnRx)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_XfrBlock)	return m_FunLPU237_SCR_helper_XfrBlock(hDev, sTx, nTx, sRx, lpnRx);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetCardParameters(HANDLE hDev, unsigned char cProtocol, unsigned char cParameter, unsigned char cNewValue)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_SetCardParameters)	return m_FunLPU237_SCR_helper_SetCardParameters(hDev, cProtocol, cParameter, cNewValue);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_ResetCardParameters(HANDLE hDev)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_ResetCardParameters)	return m_FunLPU237_SCR_helper_ResetCardParameters(hDev);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_result_is_success(unsigned long dwScrResult)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_result_is_success)	return m_FunLPU237_SCR_helper_result_is_success(dwScrResult);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_SetParameters(HANDLE hDev, const unsigned char* sAtr, unsigned long nAtr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_SetParameters)	return m_FunLPU237_SCR_helper_SetParameters(hDev, sAtr, nAtr);
		else							return FALSE;
	}

	BOOL LPU237_SCR_helper_GetATRParameterString(wchar_t* sOut, unsigned long* lpnOut, const unsigned char* sAtr, unsigned long nAtr, wchar_t cDelimiter)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_FunLPU237_SCR_helper_GetATRParameterString)	return m_FunLPU237_SCR_helper_GetATRParameterString(sOut, lpnOut, sAtr, nAtr, cDelimiter);
		else							return FALSE;
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
		return dlopen(clpu237_msr::get_mcsc_from_unicode(s_lib).c_str(), RTLD_LAZY);
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

	clpu237_msr() : m_hMode(NULL), m_hDev(INVALID_HANDLE_VALUE)
	{
		_ini();
	}

	void _ini()
	{
		m_FunLPU237_get_list = NULL;
		m_FunLPU237_open = NULL;
		m_FunLPU237_close = NULL;
		m_FunLPU237_enable = NULL;
		m_FunLPU237_disable = NULL;
		m_FunLPU237_cancel_wait_swipe = NULL;
		m_FunLPU237_wait_swipe_with_waits = NULL;
		m_FunLPU237_wait_swipe_with_callback = NULL;
		m_FunLPU237_wait_swipe_with_message = NULL;
		m_FunLPU237_get_data = NULL;
		m_FunLPU237_dll_on = NULL;
		m_FunLPU237_dll_off = NULL;
		m_FunLPU237_get_id = NULL;

		m_FunLPU237_SCR_bypass_IccPowerOn = NULL;
		m_FunLPU237_SCR_bypass_IccPowerOff = NULL;
		m_FunLPU237_SCR_bypass_XfrBlock = NULL;
		m_FunLPU237_SCR_bypass_GetParameters = NULL;
		m_FunLPU237_SCR_bypass_SetParameters = NULL;
		m_FunLPU237_SCR_bypass_ResetParameters = NULL;
		m_FunLPU237_SCR_bypass_GetSlotStatus = NULL;
		m_FunLPU237_SCR_bypass_Escape = NULL;

		m_FunLPU237_SCR_helper_GetLastError = NULL;
		m_FunLPU237_SCR_helper_GetFirmwareVersion = NULL;
		m_FunLPU237_SCR_helper_GetReaderMode = NULL;
		m_FunLPU237_SCR_helper_SetReaderMode = NULL;
		m_FunLPU237_SCR_helper_XfrBlock = NULL;
		m_FunLPU237_SCR_helper_SetCardParameters = NULL;
		m_FunLPU237_SCR_helper_ResetCardParameters = NULL;
		m_FunLPU237_SCR_helper_result_is_success = NULL;
		m_FunLPU237_SCR_helper_SetParameters = NULL;
		m_FunLPU237_SCR_helper_GetATRParameterString = NULL;
	}

private:
	std::mutex m_mutex_q;
	std::queue<unsigned long> m_q_buffer_index;

	std::mutex m_mutex;

	HMODULE m_hMode;
	HANDLE m_hDev;

	typeLPU237_get_list m_FunLPU237_get_list;
	typeLPU237_open m_FunLPU237_open;
	typeLPU237_close m_FunLPU237_close;
	typeLPU237_enable m_FunLPU237_enable;
	typeLPU237_disable m_FunLPU237_disable;
	typeLPU237_cancel_wait_swipe m_FunLPU237_cancel_wait_swipe;
	typeLPU237_wait_swipe_with_waits m_FunLPU237_wait_swipe_with_waits;
	typeLPU237_wait_swipe_with_callback m_FunLPU237_wait_swipe_with_callback;
	typeLPU237_wait_swipe_with_message m_FunLPU237_wait_swipe_with_message;
	typeLPU237_get_data m_FunLPU237_get_data;
	typeLPU237_dll_on m_FunLPU237_dll_on;
	typeLPU237_dll_off m_FunLPU237_dll_off;
	typeLPU237_get_id m_FunLPU237_get_id;

	typeLPU237_SCR_bypass_IccPowerOn		m_FunLPU237_SCR_bypass_IccPowerOn;
	typeLPU237_SCR_bypass_IccPowerOff		m_FunLPU237_SCR_bypass_IccPowerOff;
	typeLPU237_SCR_bypass_XfrBlock				m_FunLPU237_SCR_bypass_XfrBlock;
	typeLPU237_SCR_bypass_GetParameters		m_FunLPU237_SCR_bypass_GetParameters;
	typeLPU237_SCR_bypass_SetParameters		m_FunLPU237_SCR_bypass_SetParameters;
	typeLPU237_SCR_bypass_ResetParameters	m_FunLPU237_SCR_bypass_ResetParameters;
	typeLPU237_SCR_bypass_GetSlotStatus		m_FunLPU237_SCR_bypass_GetSlotStatus;
	typeLPU237_SCR_bypass_Escape				m_FunLPU237_SCR_bypass_Escape;

	typeLPU237_SCR_helper_GetLastError				m_FunLPU237_SCR_helper_GetLastError;
	typeLPU237_SCR_helper_GetFirmwareVersion		m_FunLPU237_SCR_helper_GetFirmwareVersion;
	typeLPU237_SCR_helper_GetReaderMode			m_FunLPU237_SCR_helper_GetReaderMode;
	typeLPU237_SCR_helper_SetReaderMode			m_FunLPU237_SCR_helper_SetReaderMode;
	typeLPU237_SCR_helper_XfrBlock						m_FunLPU237_SCR_helper_XfrBlock;
	typeLPU237_SCR_helper_SetCardParameters			m_FunLPU237_SCR_helper_SetCardParameters;
	typeLPU237_SCR_helper_ResetCardParameters		m_FunLPU237_SCR_helper_ResetCardParameters;
	typeLPU237_SCR_helper_result_is_success			m_FunLPU237_SCR_helper_result_is_success;
	typeLPU237_SCR_helper_SetParameters				m_FunLPU237_SCR_helper_SetParameters;
	typeLPU237_SCR_helper_GetATRParameterString	m_FunLPU237_SCR_helper_GetATRParameterString;

private://don't call these methods
	clpu237_msr(const clpu237_msr&);
	clpu237_msr& operator=(const clpu237_msr&);

};
