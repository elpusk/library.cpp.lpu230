#pragma once

#include <chid_briage.h>
#include <cdev_lib.h>

chid_brage::chid_brage()
{
	m_p_api_brage_instance = cdev_lib::get_instance().constructor();
}

chid_brage::~chid_brage()
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().destructor(m_p_api_brage_instance);
	}
}

std::mutex& chid_brage::get_mutex_for_hidapi()
{
	return cdev_lib::get_instance().get_mutex_for_hidapi();
}

bool chid_brage::is_ini()
{
	if (m_p_api_brage_instance) {
		return cdev_lib::get_instance().is_ini(m_p_api_brage_instance);
	}
	return false;
}

std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> chid_brage::hid_enumerate()
{
	std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> set_result;
	if (m_p_api_brage_instance) {
		set_result = cdev_lib::get_instance().hid_enumerate(m_p_api_brage_instance);
	}
	return set_result;
}

std::tuple<bool, int, bool> chid_brage::is_open(const char* path)
{
	std::tuple<bool, int, bool> r(false,-1,false);
	if (m_p_api_brage_instance) {
		r = cdev_lib::get_instance().is_open(m_p_api_brage_instance, path);
	}
	return r;
}

int chid_brage::api_open_path(const char* path)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_open_path(m_p_api_brage_instance, path);
	}
	return n_r;
}

void chid_brage::api_close(int n_primitive_map_index)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().api_close(m_p_api_brage_instance, n_primitive_map_index);
	}
}

int chid_brage::api_set_nonblocking(int n_primitive_map_index, int nonblock)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_set_nonblocking(m_p_api_brage_instance, n_primitive_map_index,nonblock);
	}
	return n_r;
}
int chid_brage::api_get_report_descriptor(void* p_instance, int n_primitive_map_index, unsigned char* buf, size_t buf_size)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_get_report_descriptor(m_p_api_brage_instance, n_primitive_map_index, buf, buf_size);
	}
	return n_r;
}
int chid_brage::api_write(void* p_instance, int n_primitive_map_index, const unsigned char* data, size_t length, int next)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_write(m_p_api_brage_instance, n_primitive_map_index, data, length,(int)next);
	}
	return n_r;
}

int chid_brage::api_read(void* p_instance, int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_read(m_p_api_brage_instance, n_primitive_map_index, data, length, n_report);
	}
	return n_r;
}

const wchar_t* chid_brage::api_error(void* p_instance, int n_primitive_map_index)
{
	const wchar_t* s(NULL);
	if (m_p_api_brage_instance) {
		s = cdev_lib::get_instance().api_error(p_instance, n_primitive_map_index);
	}
	return s;
}

void chid_brage::set_req_q_check_interval_in_child(void* p_instance, long long n_interval_mmsec)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().set_req_q_check_interval_in_child(m_p_api_brage_instance, n_interval_mmsec);
	}
}

void chid_brage::set_hid_write_interval_in_child(void* p_instance, long long n_interval_mmsec)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().set_hid_write_interval_in_child(m_p_api_brage_instance, n_interval_mmsec);
	}
}

void chid_brage::set_hid_read_interval_in_child(void* p_instance, long long n_interval_mmsec)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().set_hid_read_interval_in_child(m_p_api_brage_instance, n_interval_mmsec);
	}
}

long long chid_brage::get_req_q_check_interval_in_child(void* p_instance)
{
	long long ll(-1);
	if (m_p_api_brage_instance) {
		ll = cdev_lib::get_instance().get_req_q_check_interval_in_child(m_p_api_brage_instance);
	}
	return ll;
}

long long chid_brage::get_hid_write_interval_in_child(void* p_instance)
{
	long long ll(-1);
	if (m_p_api_brage_instance) {
		ll = cdev_lib::get_instance().get_hid_write_interval_in_child(m_p_api_brage_instance);
	}
	return ll;

}

long long chid_brage::get_hid_read_interval_in_child(void* p_instance)
{
	long long ll(-1);
	if (m_p_api_brage_instance) {
		ll = cdev_lib::get_instance().get_hid_read_interval_in_child(m_p_api_brage_instance);
	}
	return ll;
}
