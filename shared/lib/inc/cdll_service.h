#pragma once

#include <mutex>
#include <iostream>

#include <mp_type.h>
#include <mp_cconvert.h>
#include <mp_clog.h>
#include <dll_service.h>

class cdll_service
{
private:
	typedef	int(_CALLTYPE_* type_sd_execute)(unsigned long, const wchar_t*, const type_fun_sd_device_io, void*, const wchar_t*, size_t, const type_cb_sd_execute, void*);
	typedef	int(_CALLTYPE_* type_sd_cancel)(unsigned long, const wchar_t*, const type_fun_sd_device_cancel, void*);
	typedef	void(_CALLTYPE_* type_sd_removed)(unsigned long);

private:
	enum : size_t {
		default_out_buffer_size = 4096
	};

public:
	typedef std::shared_ptr< cdll_service>	type_ptr_cdll_service;

public:
	cdll_service(void) : m_hMode(NULL)
	{
		_ini();
	}

	cdll_service(const std::wstring& s_dll) : m_hMode(NULL)
	{
		_ini();
		load(s_dll);
	}

public:
	virtual ~cdll_service(void)
	{
		if (m_hMode) {
#ifdef _WIN32
			FreeLibrary(m_hMode);
#else
			dlclose(m_hMode);
#endif
			_mp::clog::get_instance().log_fmt(L"[I] %s | UNLOADED : %s.\n", __WFUNCTION__, m_s_path.c_str());
		}
	}
	void unload()
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		do {
			_ini();

			if (m_hMode) {
#ifdef _WIN32
				FreeLibrary(m_hMode);
#else
				dlclose(m_hMode);
#endif
				_mp::clog::get_instance().log_fmt(L"[I] %s | UNLOADED : %s.\n", __WFUNCTION__, m_s_path.c_str());
			}
			//
			m_hMode = NULL;

		} while (false);
	}

	bool load(const std::wstring& s_dll)
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		bool b_result(false);

		do {
#ifdef _WIN32
			m_hMode = ::LoadLibrary(s_dll.c_str());
#else
			std::string ps = _mp::cstring::get_mcsc_from_unicode(s_dll);
			m_hMode = dlopen(ps.c_str(), RTLD_LAZY);
#endif

			if (m_hMode == NULL)
				continue;

#ifdef _WIN32
			m_sd_execute = reinterpret_cast<cdll_service::type_sd_execute>(::GetProcAddress(m_hMode, "sd_execute"));
			m_sd_cancel = reinterpret_cast<cdll_service::type_sd_cancel>(::GetProcAddress(m_hMode, "sd_cancel"));
			m_sd_removed = reinterpret_cast<cdll_service::type_sd_removed>(::GetProcAddress(m_hMode, "sd_removed"));
			//
			if (m_sd_execute == NULL)	continue;
			if (m_sd_cancel == NULL)	continue;
			if (m_sd_removed == NULL)	continue;
#else
			dlerror();
			m_sd_execute = reinterpret_cast<cdll_service::type_sd_execute>(dlsym(m_hMode, "sd_execute"));
			m_sd_cancel = reinterpret_cast<cdll_service::type_sd_cancel>(dlsym(m_hMode, "sd_cancel"));
			m_sd_removed = reinterpret_cast<cdll_service::type_sd_removed>(dlsym(m_hMode, "sd_removed"));

			const char* dlsym_error = dlerror();
			if (dlsym_error)
				continue;
#endif

			b_result = true;
		} while (false);

		if (!b_result) {
			_ini();
			if (m_hMode) {
#ifdef _WIN32
				FreeLibrary(m_hMode);
#else
				dlclose(m_hMode);
#endif
				m_hMode = NULL;
			}
		}
		else {
			m_s_path = s_dll;
			_mp::clog::get_instance().log_fmt(L"[I] %s | LOADED : %s.\n", __WFUNCTION__, s_dll.c_str());
		}
		return b_result;
	}

	bool sd_execute(
		unsigned long n_session, 
		const std::wstring& s_device_path, 
		const type_fun_sd_device_io p_fun_sd_device_io, 
		void* p_dev, const _mp::type_list_wstring& list_parameters, 
		const type_cb_sd_execute cb, void* p_user
	)
	{
		bool b_result(false);

		std::lock_guard<std::mutex> _lock(m_mutex);
		do {
			if (!m_sd_execute)
				continue;
			size_t n_need = _mp::cconvert::change(NULL, list_parameters);
			if (n_need == 0)
				continue;
			_mp::type_v_buffer v_in_buffer(n_need, 0);
			_mp::cconvert::change((wchar_t*)&v_in_buffer[0], list_parameters);

			if (s_device_path.empty()) {
				if (m_sd_execute(n_session, NULL, p_fun_sd_device_io, p_dev, (const wchar_t*)&v_in_buffer[0], v_in_buffer.size(), cb, p_user) == 0)
					continue;
			}
			else {
				if (m_sd_execute(n_session, s_device_path.c_str(), p_fun_sd_device_io, p_dev, (const wchar_t*)&v_in_buffer[0], v_in_buffer.size(), cb, p_user) == 0)
					continue;
			}
			b_result = true;
		} while (false);
		return b_result;
	}
	bool sd_cancel(unsigned long n_session, const std::wstring& s_device_path, const type_fun_sd_device_cancel p_fun_sd_device_cancel, void* p_dev)
	{
		bool b_result(false);

		std::lock_guard<std::mutex> _lock(m_mutex);
		do {
			if (!m_sd_cancel)
				continue;
			if (s_device_path.empty()) {
				if (m_sd_cancel(n_session, NULL, p_fun_sd_device_cancel, p_dev) == 0)
					continue;
			}
			else {
				if (m_sd_cancel(n_session, s_device_path.c_str(), p_fun_sd_device_cancel, p_dev) == 0)
					continue;
			}
			b_result = true;
		} while (false);
		return b_result;
	}

	void sd_removed(unsigned long n_session)
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		if (m_sd_removed)
			m_sd_removed(n_session);
	}


private:
	void _ini()
	{
		m_sd_execute = NULL;
		m_sd_cancel = NULL;
		m_sd_removed = NULL;
	}
private:
	std::mutex m_mutex;

#ifdef _WIN32
	HMODULE m_hMode;
#else
	void* m_hMode;
#endif //_WIN32
	cdll_service::type_sd_execute m_sd_execute;
	cdll_service::type_sd_cancel m_sd_cancel;
	cdll_service::type_sd_removed m_sd_removed;

	std::wstring m_s_path;

private:
	//don't use these methods
	cdll_service(const cdll_service&);
	cdll_service& operator=(const cdll_service&);
};

