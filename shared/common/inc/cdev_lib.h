#pragma once

#include <set>
#include <tuple>
#include <string>
#include <mutex>
#include <memory>

#include <dev_lib.h>

#include <mp_clog.h>

/**
* this class is warpper class of dev_lib.dll(or .so)
*/
class cdev_lib
{
public:
	typedef	std::shared_ptr<cdev_lib>	type_ptr;

private:
	typedef	int(_CALLTYPE_* _type_dev_lib_on)(void*);
	typedef	int(_CALLTYPE_* _type_dev_lib_off)();

	typedef	std::mutex* (_CALLTYPE_* _type_dev_lib_get_mutex)();

	typedef	void* (_CALLTYPE_* _type_dev_lib_constrcutor)(int);
	typedef	void (_CALLTYPE_* _type_dev_lib_destructor)(void*);
	typedef	int (_CALLTYPE_* _type_dev_lib_is_ini)(void*);

	typedef	type_set_tuple_dev_lib_usb_id* (_CALLTYPE_* _type_dev_lib_hid_enumerate)(void*);
	typedef void (_CALLTYPE_* _type_dev_lib_hid_enumerate_free)(type_set_tuple_dev_lib_usb_id*);

	typedef	std::tuple<bool, int, bool>* (_CALLTYPE_* _type_dev_lib_is_open)(void*, const char*);
	typedef	void (_CALLTYPE_* _type_dev_lib_is_open_free)(std::tuple<bool, int, bool>*);

	typedef	int (_CALLTYPE_* _type_dev_lib_api_open_path)(void*, const char*);
	typedef	void (_CALLTYPE_* _type_dev_lib_api_close)(void*, int);
	typedef	int (_CALLTYPE_* _type_dev_lib_api_set_nonblocking)(void*, int, int);
	typedef	int (_CALLTYPE_* _type_dev_lib_api_get_report_descriptor)(void*, int, unsigned char*, size_t);
	typedef	int (_CALLTYPE_* _type_dev_lib_api_write)(void*, int, const unsigned char*, size_t, int);
	typedef	int (_CALLTYPE_* _type_dev_lib_api_read)(void*, int, unsigned char*, size_t, size_t);
	typedef	const wchar_t* (_CALLTYPE_* _type_dev_lib_api_error)(void*, int);
	typedef	void (_CALLTYPE_* _type_dev_lib_set_req_q_check_interval_in_child)(void*, long long);
	typedef	void (_CALLTYPE_* _type_dev_lib_set_hid_write_interval_in_child)(void*, long long);
	typedef	void (_CALLTYPE_* _type_dev_lib_set_hid_read_interval_in_child)(void*, long long);
	typedef	long long (_CALLTYPE_* _type_dev_lib_get_req_q_check_interval_in_child)(void*);
	typedef	long long (_CALLTYPE_* _type_dev_lib_get_hid_write_interval_in_child)(void*);
	typedef	long long (_CALLTYPE_* _type_dev_lib_get_hid_read_interval_in_child)(void*);

public:
	static cdev_lib& get_instance()
	{
		static cdev_lib obj;
		return obj;
	}

	~cdev_lib()
	{
		if (m_hMode) {
			if (m_dev_lib_off) {
				m_dev_lib_off();
			}
			_free_lib(m_hMode);
			m_hMode = NULL;
		}
	}

	void unload()
	{
		//std::lock_guard<std::mutex> lock(m_mutex);
		if (m_hMode) {
			if (m_dev_lib_off) {
				m_dev_lib_off();
			}
			_free_lib(m_hMode);
			m_hMode = NULL;
		}
		_ini();
	}

	bool load(const std::wstring& s_lib,void* p_log)
	{
		//std::lock_guard<std::mutex> lock(m_mutex);
		bool b_result(false);
		do {
			m_p_clog = (_mp::clog*)p_log;

			if (m_p_clog) {
				m_p_clog->log_fmt(L" I : cdev_lib::load() : try loading library(%ls).\n", s_lib.c_str());
				m_p_clog->trace(L" I : cdev_lib::load() : try loading library(%ls).\n", s_lib.c_str());
			}

			if (m_hMode) {
				if (m_p_clog) {
					m_p_clog->log_fmt(L" I : cdev_lib::load() : already loaded.\n");
					m_p_clog->trace(L" I : cdev_lib::load() : already loaded.\n");
				}
				b_result = true; //already loaded
				continue;
			}
			m_hMode = _load_lib(s_lib);
			if (m_hMode == NULL) {
				if (m_p_clog) {
					if (!s_lib.empty()) {
						m_p_clog->log_fmt(L"cdev_lib::load() : failed to load library(%s).\n", s_lib.c_str());
						m_p_clog->trace(L"cdev_lib::load() : failed to load library(%s).\n", s_lib.c_str());
					}
					else {
						m_p_clog->log_fmt(L"cdev_lib::load() : failed to load library(-).\n");
						m_p_clog->trace(L"cdev_lib::load() : failed to load library(-).\n");
					}
				}
				continue;
			}

			m_dev_lib_on = reinterpret_cast<cdev_lib::_type_dev_lib_on>(_load_symbol(m_hMode, "dev_lib_on"));
			m_dev_lib_off = reinterpret_cast<cdev_lib::_type_dev_lib_off>(_load_symbol(m_hMode, "dev_lib_off"));
			m_dev_lib_get_mutex = reinterpret_cast<cdev_lib::_type_dev_lib_get_mutex>(_load_symbol(m_hMode, "dev_lib_get_mutex"));
			m_dev_lib_constrcutor = reinterpret_cast<cdev_lib::_type_dev_lib_constrcutor>(_load_symbol(m_hMode, "dev_lib_constrcutor"));
			m_dev_lib_destructor = reinterpret_cast<cdev_lib::_type_dev_lib_destructor>(_load_symbol(m_hMode, "dev_lib_destructor"));
			m_dev_lib_is_ini = reinterpret_cast<cdev_lib::_type_dev_lib_is_ini>(_load_symbol(m_hMode, "dev_lib_is_ini"));
			m_dev_lib_hid_enumerate = reinterpret_cast<cdev_lib::_type_dev_lib_hid_enumerate>(_load_symbol(m_hMode, "dev_lib_hid_enumerate"));
			m_dev_lib_hid_enumerate_free = reinterpret_cast<cdev_lib::_type_dev_lib_hid_enumerate_free>(_load_symbol(m_hMode, "dev_lib_hid_enumerate_free"));
			m_dev_lib_is_open = reinterpret_cast<cdev_lib::_type_dev_lib_is_open>(_load_symbol(m_hMode, "dev_lib_is_open"));
			m_dev_lib_is_open_free = reinterpret_cast<cdev_lib::_type_dev_lib_is_open_free>(_load_symbol(m_hMode, "dev_lib_is_open_free"));
			m_dev_lib_api_open_path = reinterpret_cast<cdev_lib::_type_dev_lib_api_open_path>(_load_symbol(m_hMode, "dev_lib_api_open_path"));
			m_dev_lib_api_close = reinterpret_cast<cdev_lib::_type_dev_lib_api_close>(_load_symbol(m_hMode, "dev_lib_api_close"));
			m_dev_lib_api_set_nonblocking = reinterpret_cast<cdev_lib::_type_dev_lib_api_set_nonblocking>(_load_symbol(m_hMode, "dev_lib_api_set_nonblocking"));
			m_dev_lib_api_get_report_descriptor = reinterpret_cast<cdev_lib::_type_dev_lib_api_get_report_descriptor>(_load_symbol(m_hMode, "dev_lib_api_get_report_descriptor"));
			m_dev_lib_api_write = reinterpret_cast<cdev_lib::_type_dev_lib_api_write>(_load_symbol(m_hMode, "dev_lib_api_write"));
			m_dev_lib_api_read = reinterpret_cast<cdev_lib::_type_dev_lib_api_read>(_load_symbol(m_hMode, "dev_lib_api_read"));
			m_dev_lib_api_error = reinterpret_cast<cdev_lib::_type_dev_lib_api_error>(_load_symbol(m_hMode, "dev_lib_api_error"));
			m_dev_lib_set_req_q_check_interval_in_child = reinterpret_cast<cdev_lib::_type_dev_lib_set_req_q_check_interval_in_child>(_load_symbol(m_hMode, "dev_lib_set_req_q_check_interval_in_child"));
			m_dev_lib_set_hid_write_interval_in_child = reinterpret_cast<cdev_lib::_type_dev_lib_set_hid_write_interval_in_child>(_load_symbol(m_hMode, "dev_lib_set_hid_write_interval_in_child"));
			m_dev_lib_set_hid_read_interval_in_child = reinterpret_cast<cdev_lib::_type_dev_lib_set_hid_read_interval_in_child>(_load_symbol(m_hMode, "dev_lib_set_hid_read_interval_in_child"));
			m_dev_lib_get_req_q_check_interval_in_child = reinterpret_cast<cdev_lib::_type_dev_lib_get_req_q_check_interval_in_child>(_load_symbol(m_hMode, "dev_lib_get_req_q_check_interval_in_child"));
			m_dev_lib_get_hid_write_interval_in_child = reinterpret_cast<cdev_lib::_type_dev_lib_get_hid_write_interval_in_child>(_load_symbol(m_hMode, "dev_lib_get_hid_write_interval_in_child"));
			m_dev_lib_get_hid_read_interval_in_child = reinterpret_cast<cdev_lib::_type_dev_lib_get_hid_read_interval_in_child>(_load_symbol(m_hMode, "dev_lib_get_hid_read_interval_in_child"));

			if (m_dev_lib_on == NULL)				continue;
			if (m_dev_lib_off == NULL)				continue;
			if (m_dev_lib_get_mutex == NULL)		continue;
			if (m_dev_lib_constrcutor == NULL)		continue;
			if (m_dev_lib_destructor == NULL)		continue;
			if (m_dev_lib_is_ini == NULL)			continue;
			if (m_dev_lib_hid_enumerate == NULL)	continue;
			if (m_dev_lib_hid_enumerate_free == NULL)	continue;
			if (m_dev_lib_is_open == NULL)			continue;
			if (m_dev_lib_is_open_free == NULL)		continue;
			if (m_dev_lib_api_open_path == NULL)	continue;
			if (m_dev_lib_api_close == NULL)		continue;
			if (m_dev_lib_api_set_nonblocking == NULL)			continue;
			if (m_dev_lib_api_get_report_descriptor == NULL)	continue;
			if (m_dev_lib_api_write == NULL)		continue;
			if (m_dev_lib_api_read == NULL)			continue;
			if (m_dev_lib_api_error == NULL)		continue;
			if (m_dev_lib_set_req_q_check_interval_in_child == NULL)		continue;
			if (m_dev_lib_set_hid_write_interval_in_child == NULL)		continue;
			if (m_dev_lib_set_hid_read_interval_in_child == NULL)		continue;
			if (m_dev_lib_get_req_q_check_interval_in_child == NULL)		continue;
			if (m_dev_lib_get_hid_write_interval_in_child == NULL)		continue;
			if (m_dev_lib_get_hid_read_interval_in_child == NULL)		continue;
			//
			b_result = true;
		} while (false);

		//
		if (!b_result) {
			if (m_p_clog) {
				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_on = 0x%x.\n", (void*)m_dev_lib_on);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_on = 0x%x.\n", (void*)m_dev_lib_on);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_off = 0x%x.\n", (void*)m_dev_lib_off);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_off = 0x%x.\n", (void*)m_dev_lib_off);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_get_mutex = 0x%x.\n", (void*)m_dev_lib_get_mutex);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_get_mutex = 0x%x.\n", (void*)m_dev_lib_get_mutex);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_constrcutor = 0x%x.\n", (void*)m_dev_lib_constrcutor);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_constrcutor = 0x%x.\n", (void*)m_dev_lib_constrcutor);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_destructor = 0x%x.\n", (void*)m_dev_lib_destructor);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_destructor = 0x%x.\n", (void*)m_dev_lib_destructor);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_is_ini = 0x%x.\n", (void*)m_dev_lib_is_ini);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_is_ini = 0x%x.\n", (void*)m_dev_lib_is_ini);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_hid_enumerate = 0x%x.\n", (void*)m_dev_lib_hid_enumerate);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_hid_enumerate = 0x%x.\n", (void*)m_dev_lib_hid_enumerate);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_hid_enumerate_free = 0x%x.\n", (void*)m_dev_lib_hid_enumerate_free);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_hid_enumerate_free = 0x%x.\n", (void*)m_dev_lib_hid_enumerate_free);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_is_open = 0x%x.\n", (void*)m_dev_lib_is_open);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_is_open = 0x%x.\n", (void*)m_dev_lib_is_open);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_is_open_free = 0x%x.\n", (void*)m_dev_lib_is_open_free);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_is_open_free = 0x%x.\n", (void*)m_dev_lib_is_open_free);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_open_path = 0x%x.\n", (void*)m_dev_lib_api_open_path);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_open_path = 0x%x.\n", (void*)m_dev_lib_api_open_path);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_close = 0x%x.\n", (void*)m_dev_lib_api_close);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_close = 0x%x.\n", (void*)m_dev_lib_api_close);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_set_nonblocking = 0x%x.\n", (void*)m_dev_lib_api_set_nonblocking);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_set_nonblocking = 0x%x.\n", (void*)m_dev_lib_api_set_nonblocking);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_get_report_descriptor = 0x%x.\n", (void*)m_dev_lib_api_get_report_descriptor);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_get_report_descriptor = 0x%x.\n", (void*)m_dev_lib_api_get_report_descriptor);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_write = 0x%x.\n", (void*)m_dev_lib_api_write);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_write = 0x%x.\n", (void*)m_dev_lib_api_write);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_read = 0x%x.\n", (void*)m_dev_lib_api_read);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_read = 0x%x.\n", (void*)m_dev_lib_api_read);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_api_error = 0x%x.\n", (void*)m_dev_lib_api_error);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_api_error = 0x%x.\n", (void*)m_dev_lib_api_error);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_set_req_q_check_interval_in_child = 0x%x.\n", (void*)m_dev_lib_set_req_q_check_interval_in_child);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_set_req_q_check_interval_in_child = 0x%x.\n", (void*)m_dev_lib_set_req_q_check_interval_in_child);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_set_hid_write_interval_in_child = 0x%x.\n", (void*)m_dev_lib_set_hid_write_interval_in_child);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_set_hid_write_interval_in_child = 0x%x.\n", (void*)m_dev_lib_set_hid_write_interval_in_child);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_set_hid_read_interval_in_child = 0x%x.\n", (void*)m_dev_lib_set_hid_read_interval_in_child);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_set_hid_read_interval_in_child = 0x%x.\n", (void*)m_dev_lib_set_hid_read_interval_in_child);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_get_req_q_check_interval_in_child = 0x%x.\n", (void*)m_dev_lib_get_req_q_check_interval_in_child);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_get_req_q_check_interval_in_child = 0x%x.\n", (void*)m_dev_lib_get_req_q_check_interval_in_child);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_get_hid_write_interval_in_child = 0x%x.\n", (void*)m_dev_lib_get_hid_write_interval_in_child);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_get_hid_write_interval_in_child = 0x%x.\n", (void*)m_dev_lib_get_hid_write_interval_in_child);

				m_p_clog->log_fmt(L" E : cdev_lib::load() : m_dev_lib_get_hid_read_interval_in_child = 0x%x.\n", (void*)m_dev_lib_get_hid_read_interval_in_child);
				m_p_clog->trace(L" E : cdev_lib::load() : m_dev_lib_get_hid_read_interval_in_child = 0x%x.\n", (void*)m_dev_lib_get_hid_read_interval_in_child);
			}

			_ini();
			if (m_hMode) {
				_free_lib(m_hMode);
				m_hMode = NULL;
			}
		}
		else {
			m_dev_lib_on(p_log);
		}
		return b_result;
	}
	///////////////////////////////////////////////////////////
	void* constructor(bool b_remove_all_zero_in_report)
	{
		void* p(NULL);
		//std::lock_guard<std::mutex> lock(m_mutex);
		if (m_dev_lib_constrcutor) {
			if (b_remove_all_zero_in_report) {
				p = m_dev_lib_constrcutor(1);
			}
			else {
				p = m_dev_lib_constrcutor(0);
			}
		}
		//
		return p;
	}

	void destructor(void* p_instance)
	{
		//std::lock_guard<std::mutex> lock(m_mutex);
		if (m_dev_lib_destructor) {
			m_dev_lib_destructor(p_instance);
		}
	}

	std::mutex& get_mutex_for_hidapi()
	{
		static std::mutex dummy_mutex;

		//std::lock_guard<std::mutex> lock(m_mutex);
		if (m_dev_lib_get_mutex == NULL) {
			return dummy_mutex;
		}
		
		std::mutex* p_m = m_dev_lib_get_mutex();
		if (p_m == NULL) {
			return dummy_mutex;
		}
		return *p_m;
	}

	bool is_ini(void* p_instance)
	{
		bool b_result(false);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_is_ini == NULL) {
				continue;
			}
			if( m_dev_lib_is_ini(p_instance) ) {
				b_result = true;
			}
		} while (false);
		return b_result;
	}

	type_set_tuple_dev_lib_usb_id hid_enumerate(void* p_instance)
	{
		type_set_tuple_dev_lib_usb_id set_result;

		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_hid_enumerate == NULL) {
				continue;
			}
			if (m_dev_lib_hid_enumerate_free == NULL) {
				continue;
			}

			auto p = m_dev_lib_hid_enumerate(p_instance);
			if (!p) {
				continue;
			}
			set_result = *p;
			m_dev_lib_hid_enumerate_free(p);
		} while (false);
		return set_result;
	}

	std::tuple<bool, int, bool> is_open(void* p_instance, const char* path)
	{
		std::tuple<bool, int, bool> tuple_result(false, -1, false);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_is_open == NULL) {
				continue;
			}
			if (m_dev_lib_is_open_free == NULL) {
				continue;
			}
			auto p = m_dev_lib_is_open(p_instance, path);
			if (!p) {
				continue;
			}
			tuple_result = *p;
			m_dev_lib_is_open_free(p);
		} while (false);
		return tuple_result;
	}

	int api_open_path(void* p_instance, const char* path)
	{
		int n_result(-1);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_open_path == NULL) {
				continue;
			}
			n_result = m_dev_lib_api_open_path(p_instance, path);
		} while (false);
		return n_result;
	}

	void api_close(void* p_instance, int n_primitive_map_index)
	{
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_close == NULL) {
				continue;
			}
			m_dev_lib_api_close(p_instance, n_primitive_map_index);
		} while (false);
	}

	int api_set_nonblocking(void* p_instance, int n_primitive_map_index, int nonblock)
	{
		int n_result(-1);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_set_nonblocking == NULL) {
				continue;
			}
			n_result = m_dev_lib_api_set_nonblocking(p_instance, n_primitive_map_index,nonblock);
		} while (false);
		return n_result;
	}

	int api_get_report_descriptor(void* p_instance, int n_primitive_map_index, unsigned char* buf, size_t buf_size)
	{
		int n_result(-1);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_get_report_descriptor == NULL) {
				continue;
			}
			n_result = m_dev_lib_api_get_report_descriptor(p_instance, n_primitive_map_index, buf, buf_size);
		} while (false);
		return n_result;
	}

	int api_write(void* p_instance, int n_primitive_map_index, const unsigned char* data, size_t length, int next)
	{
		int n_result(-1);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_write == NULL) {
				continue;
			}
			n_result = m_dev_lib_api_write(p_instance, n_primitive_map_index, data, length, next);
		} while (false);
		return n_result;
	}

	int api_read(void* p_instance, int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report) 
	{
		int n_result(-1);
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_read == NULL) {
				continue;
			}
			n_result = m_dev_lib_api_read(p_instance, n_primitive_map_index, data, length, n_report);
		} while (false);
		return n_result;
	}

	const wchar_t* api_error(void* p_instance, int n_primitive_map_index)
	{
		const wchar_t* p_s(NULL);

		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_api_error == NULL) {
				continue;
			}
			p_s = m_dev_lib_api_error(p_instance, n_primitive_map_index);
		} while (false);
		return p_s;
	}

	void set_req_q_check_interval_in_child(void* p_instance, long long n_interval_mmsec)
	{
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_set_req_q_check_interval_in_child == NULL) {
				continue;
			}

			m_dev_lib_set_req_q_check_interval_in_child(p_instance, n_interval_mmsec);
		} while (false);
	}

	void set_hid_write_interval_in_child(void* p_instance, long long n_interval_mmsec)
	{
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_set_hid_write_interval_in_child == NULL) {
				continue;
			}

			m_dev_lib_set_hid_write_interval_in_child(p_instance, n_interval_mmsec);
		} while (false);
	}

	void set_hid_read_interval_in_child(void* p_instance, long long n_interval_mmsec)
	{
		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_set_hid_read_interval_in_child == NULL) {
				continue;
			}

			m_dev_lib_set_hid_read_interval_in_child(p_instance, n_interval_mmsec);
		} while (false);
	}

	long long get_req_q_check_interval_in_child(void* p_instance)
	{
		long long ll_result(-1);

		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_get_req_q_check_interval_in_child == NULL) {
				continue;
			}

			ll_result = m_dev_lib_get_req_q_check_interval_in_child(p_instance);
		} while (false);
		return ll_result;
	}

	long long get_hid_write_interval_in_child(void* p_instance)
	{
		long long ll_result(-1);

		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_get_hid_write_interval_in_child == NULL) {
				continue;
			}

			ll_result = m_dev_lib_get_hid_write_interval_in_child(p_instance);
		} while (false);
		return ll_result;
	}

	long long get_hid_read_interval_in_child(void* p_instance)
	{
		long long ll_result(-1);

		do {
			//std::lock_guard<std::mutex> lock(m_mutex);
			if (m_dev_lib_get_hid_read_interval_in_child == NULL) {
				continue;
			}

			ll_result = m_dev_lib_get_hid_read_interval_in_child(p_instance);
		} while (false);
		return ll_result;
	}


private:
	cdev_lib() : m_p_clog(nullptr),m_hMode(NULL), m_hDev(INVALID_HANDLE_VALUE)
	{
		_ini();
	}

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
		return dlopen(cdev_lib::_get_mcsc_from_unicode(s_lib).c_str(), RTLD_LAZY);
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

	void _ini()
	{
		m_dev_lib_on = nullptr;
		m_dev_lib_off = nullptr;
		m_dev_lib_get_mutex = nullptr;
		m_dev_lib_constrcutor = nullptr;
		m_dev_lib_destructor = nullptr;
		m_dev_lib_is_ini = nullptr;
		m_dev_lib_hid_enumerate = nullptr;
		m_dev_lib_hid_enumerate_free = nullptr;
		m_dev_lib_is_open = nullptr;
		m_dev_lib_is_open_free = nullptr;
		m_dev_lib_api_open_path = nullptr;
		m_dev_lib_api_close = nullptr;
		m_dev_lib_api_set_nonblocking = nullptr;
		m_dev_lib_api_get_report_descriptor = nullptr;
		m_dev_lib_api_write = nullptr;
		m_dev_lib_api_read = nullptr;
		m_dev_lib_api_error = nullptr;
		m_dev_lib_set_req_q_check_interval_in_child = nullptr;
		m_dev_lib_set_hid_write_interval_in_child = nullptr;
		m_dev_lib_set_hid_read_interval_in_child = nullptr;
		m_dev_lib_get_req_q_check_interval_in_child = nullptr;
		m_dev_lib_get_hid_write_interval_in_child = nullptr;
		m_dev_lib_get_hid_read_interval_in_child = nullptr;
	}

	static std::string _get_mcsc_from_unicode(const std::wstring& s_unicode)
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
	//std::mutex m_mutex; //don't use this mutex. this mutex cause dead lock.

	_mp::clog* m_p_clog;

	HMODULE m_hMode;
	HANDLE m_hDev;
	//
	cdev_lib::_type_dev_lib_on m_dev_lib_on;
	cdev_lib::_type_dev_lib_off m_dev_lib_off;
	cdev_lib::_type_dev_lib_get_mutex m_dev_lib_get_mutex;
	cdev_lib::_type_dev_lib_constrcutor m_dev_lib_constrcutor;
	cdev_lib::_type_dev_lib_destructor m_dev_lib_destructor;
	cdev_lib::_type_dev_lib_is_ini m_dev_lib_is_ini;
	cdev_lib::_type_dev_lib_hid_enumerate m_dev_lib_hid_enumerate;
	cdev_lib::_type_dev_lib_hid_enumerate_free m_dev_lib_hid_enumerate_free;
	cdev_lib::_type_dev_lib_is_open m_dev_lib_is_open;
	cdev_lib::_type_dev_lib_is_open_free m_dev_lib_is_open_free;
	cdev_lib::_type_dev_lib_api_open_path m_dev_lib_api_open_path;
	cdev_lib::_type_dev_lib_api_close m_dev_lib_api_close;
	cdev_lib::_type_dev_lib_api_set_nonblocking m_dev_lib_api_set_nonblocking;
	cdev_lib::_type_dev_lib_api_get_report_descriptor m_dev_lib_api_get_report_descriptor;
	cdev_lib::_type_dev_lib_api_write m_dev_lib_api_write;
	cdev_lib::_type_dev_lib_api_read m_dev_lib_api_read;
	cdev_lib::_type_dev_lib_api_error m_dev_lib_api_error;
	cdev_lib::_type_dev_lib_set_req_q_check_interval_in_child m_dev_lib_set_req_q_check_interval_in_child;
	cdev_lib::_type_dev_lib_set_hid_write_interval_in_child m_dev_lib_set_hid_write_interval_in_child;
	cdev_lib::_type_dev_lib_set_hid_read_interval_in_child m_dev_lib_set_hid_read_interval_in_child;
	cdev_lib::_type_dev_lib_get_req_q_check_interval_in_child m_dev_lib_get_req_q_check_interval_in_child;
	cdev_lib::_type_dev_lib_get_hid_write_interval_in_child m_dev_lib_get_hid_write_interval_in_child;
	cdev_lib::_type_dev_lib_get_hid_read_interval_in_child m_dev_lib_get_hid_read_interval_in_child;

private:
	cdev_lib(const cdev_lib&) = delete;
	cdev_lib& operator=(const cdev_lib&) = delete;
};