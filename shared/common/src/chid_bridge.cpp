#pragma once

#include <chid_bridge.h>
#include <cdev_lib.h>

chid_bridge::chid_bridge()
{
	m_p_api_brage_instance = cdev_lib::get_instance().constructor();
}

chid_bridge::~chid_bridge()
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().destructor(m_p_api_brage_instance);
	}
}

std::mutex& chid_bridge::get_mutex_for_hidapi()
{
	return cdev_lib::get_instance().get_mutex_for_hidapi();
}

bool chid_bridge::is_ini()
{
	if (m_p_api_brage_instance) {
		return cdev_lib::get_instance().is_ini(m_p_api_brage_instance);
	}
	return false;
}

std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> chid_bridge::hid_enumerate()
{
	std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> set_result;
	if (m_p_api_brage_instance) {
		set_result = cdev_lib::get_instance().hid_enumerate(m_p_api_brage_instance);
	}
	return set_result;
}

std::tuple<bool, int, bool> chid_bridge::is_open(const char* path)
{
	std::tuple<bool, int, bool> r(false,-1,false);
	if (m_p_api_brage_instance) {
		r = cdev_lib::get_instance().is_open(m_p_api_brage_instance, path);
	}
	return r;
}

int chid_bridge::api_open_path(const char* path)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_open_path(m_p_api_brage_instance, path);
	}
	return n_r;
}

void chid_bridge::api_close(int n_primitive_map_index)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().api_close(m_p_api_brage_instance, n_primitive_map_index);
	}
}

int chid_bridge::api_get_report_descriptor(int n_primitive_map_index, unsigned char* buf, size_t buf_size)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_get_report_descriptor(m_p_api_brage_instance, n_primitive_map_index, buf, buf_size);
	}
	return n_r;
}
int chid_bridge::api_write(int n_primitive_map_index, const unsigned char* data, size_t length, int next)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_write(m_p_api_brage_instance, n_primitive_map_index, data, length,(int)next);
	}
	return n_r;
}

int chid_bridge::api_read(int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report)
{
	int n_r(-1);
	if (m_p_api_brage_instance) {
		n_r = cdev_lib::get_instance().api_read(m_p_api_brage_instance, n_primitive_map_index, data, length, n_report);
	}
	return n_r;
}

const wchar_t* chid_bridge::api_error(int n_primitive_map_index)
{
	const wchar_t* s(NULL);
	if (m_p_api_brage_instance) {
		s = cdev_lib::get_instance().api_error(m_p_api_brage_instance, n_primitive_map_index);
	}
	return s;
}

void chid_bridge::set_req_q_check_interval_in_child(long long n_interval_mmsec)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().set_req_q_check_interval_in_child(m_p_api_brage_instance, n_interval_mmsec);
	}
}

void chid_bridge::set_hid_write_interval_in_child(long long n_interval_mmsec)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().set_hid_write_interval_in_child(m_p_api_brage_instance, n_interval_mmsec);
	}
}

void chid_bridge::set_hid_read_interval_in_child(long long n_interval_mmsec)
{
	if (m_p_api_brage_instance) {
		cdev_lib::get_instance().set_hid_read_interval_in_child(m_p_api_brage_instance, n_interval_mmsec);
	}
}

long long chid_bridge::get_req_q_check_interval_in_child()
{
	long long ll(-1);
	if (m_p_api_brage_instance) {
		ll = cdev_lib::get_instance().get_req_q_check_interval_in_child(m_p_api_brage_instance);
	}
	return ll;
}

long long chid_bridge::get_hid_write_interval_in_child()
{
	long long ll(-1);
	if (m_p_api_brage_instance) {
		ll = cdev_lib::get_instance().get_hid_write_interval_in_child(m_p_api_brage_instance);
	}
	return ll;

}

long long chid_bridge::get_hid_read_interval_in_child()
{
	long long ll(-1);
	if (m_p_api_brage_instance) {
		ll = cdev_lib::get_instance().get_hid_read_interval_in_child(m_p_api_brage_instance);
	}
	return ll;
}
