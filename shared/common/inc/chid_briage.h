#pragma once

#include <memory>
#include <mutex>
#include <set>

/**
* notice. before using this class, you must run and success cdev_lib.get_instance().load(L"dev_lib.dll")
* or cdev_lib.get_instance().load(L"libdev_lib.so")
*/
class chid_brage {
public:
	typedef std::shared_ptr<chid_brage>	type_ptr;

public:
	chid_brage();

	virtual ~chid_brage();

	std::mutex& get_mutex_for_hidapi();

	bool is_ini();

	std::set<std::tuple<std::string, unsigned short, unsigned short, int, std::string>> hid_enumerate();

	std::tuple<bool, int, bool> is_open(const char* path);

	int api_open_path(const char* path);

	void api_close(int n_primitive_map_index);

	int api_set_nonblocking(int n_primitive_map_index, int nonblock);
	int api_get_report_descriptor(void* p_instance, int n_primitive_map_index, unsigned char* buf, size_t buf_size);

	/**
	* @param : next - _mp::type_next_io type
	*/
	int api_write(void* p_instance, int n_primitive_map_index, const unsigned char* data, size_t length, int next);

	int api_read(void* p_instance, int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report);

	const wchar_t* api_error(void* p_instance, int n_primitive_map_index);

	void set_req_q_check_interval_in_child(void* p_instance, long long n_interval_mmsec);

	void set_hid_write_interval_in_child(void* p_instance, long long n_interval_mmsec);

	void set_hid_read_interval_in_child(void* p_instance, long long n_interval_mmsec);

	long long get_req_q_check_interval_in_child(void* p_instance);

	long long get_hid_write_interval_in_child(void* p_instance);

	long long get_hid_read_interval_in_child(void* p_instance);
private:
	void* m_p_api_brage_instance;

private:
	chid_brage(const chid_brage&) = delete;
	chid_brage& operator=(const chid_brage&) = delete;
};