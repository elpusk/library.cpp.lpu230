
#include <mp_clog.h>
#include <dev_lib.h>

#include <hid/_vhid_api_briage.h>

#ifndef _WIN32
//linux only
static void _so_init(void) __attribute__((constructor));
static void _so_fini(void) __attribute__((destructor));
//when calls dlopen().
void _so_init(void)
{
	//printf("Shared library loaded\n");
	// NOT executed
}

//when calls dlclose().
void _so_fini(void)
{
	//printf("Shared library unloaded\n");
	// NOT executed
}
#endif // _WIN32

static _mp::clog* gp_log(nullptr);

int _CALLTYPE_ dev_lib_on(void* p_log)
{
	int n_result(1);

	do {
		gp_log = (_mp::clog*)p_log;
	} while (false);
	return n_result;
}


int _CALLTYPE_ dev_lib_off()
{
	int n_result(1);

	do {

	} while (false);
	return n_result;

}

std::mutex* _CALLTYPE_ dev_lib_get_mutex()
{
	std::mutex& m(_hid_api_briage::get_mutex_for_hidapi());
	return & m;
}

void* _CALLTYPE_ dev_lib_constrcutor(int n_remove_all_zero_in_report)
{
	_vhid_api_briage* p_vhid_api(nullptr);

	if (n_remove_all_zero_in_report == 0) {
		p_vhid_api = new _vhid_api_briage(gp_log,false);
	}
	else {
		p_vhid_api = new _vhid_api_briage(gp_log, true);
	}
	
	return p_vhid_api;
}

void _CALLTYPE_ dev_lib_destructor(void* p_instance)
{
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		delete p_vhid_api;

	} while (false);
}

int _CALLTYPE_ dev_lib_is_ini(void* p_instance)
{
	int n_result(0);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		if (p_vhid_api->is_ini()) {
			n_result = 1;
		}
		//
	} while (false);

	return n_result;
}

type_set_tuple_dev_lib_usb_id* _CALLTYPE_ dev_lib_hid_enumerate(void* p_instance)
{
	type_set_tuple_dev_lib_usb_id* p_result(nullptr);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		auto result = p_vhid_api->hid_enumerate();
		p_result = new type_set_tuple_dev_lib_usb_id(result);
		//
	} while (false);

	return p_result;
}

void _CALLTYPE_ dev_lib_hid_enumerate_free(type_set_tuple_dev_lib_usb_id* p_tuple)
{
	if(p_tuple) {
		delete p_tuple;
	}
}

std::tuple<bool, int, bool>* _CALLTYPE_ dev_lib_is_open(void* p_instance, const char* path)
{
	std::tuple<bool, int, bool>* p_result(nullptr);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		auto result = p_vhid_api->is_open(path);
		p_result = new std::tuple<bool, int, bool>(result);
		//
	} while (false);

	return p_result;
}

void _CALLTYPE_ dev_lib_is_open_free(std::tuple<bool, int, bool>* p_tuple)
{
	if(p_tuple) {
		delete p_tuple;
	}
}

int _CALLTYPE_ dev_lib_api_open_path(void* p_instance, const char* path)
{
	int n_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		n_result = p_vhid_api->api_open_path(path);
		//
	} while (false);

	return n_result;
}

void _CALLTYPE_ dev_lib_api_close(void* p_instance, int n_primitive_map_index)
{
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		p_vhid_api->api_close(n_primitive_map_index);
		//
	} while (false);
}

int _CALLTYPE_ dev_lib_api_set_nonblocking(void* p_instance, int n_primitive_map_index, int nonblock)
{
	int n_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		n_result = p_vhid_api->api_set_nonblocking(n_primitive_map_index, nonblock);
		//
	} while (false);

	return n_result;
}

int _CALLTYPE_ dev_lib_api_get_report_descriptor(void* p_instance, int n_primitive_map_index, unsigned char* buf, size_t buf_size)
{
	int n_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		n_result = p_vhid_api->api_get_report_descriptor(n_primitive_map_index, buf, buf_size);
		//
	} while (false);

	return n_result;
}

int _CALLTYPE_ dev_lib_api_write(void* p_instance, int n_primitive_map_index, const unsigned char* data, size_t length, int next)
{
	int n_result(false);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		n_result = p_vhid_api->api_write(n_primitive_map_index, data, length, (_mp::type_next_io)next);
		//
	} while (false);

	return n_result;
}

int _CALLTYPE_ dev_lib_api_read(void* p_instance, int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report)
{
	int n_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		n_result = p_vhid_api->api_read(n_primitive_map_index, data, length, n_report);
		//
	} while (false);

	return n_result;
}

const wchar_t* _CALLTYPE_ dev_lib_api_error(void* p_instance, int n_primitive_map_index)
{
	const wchar_t *s_result(NULL);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		s_result = p_vhid_api->api_error(n_primitive_map_index);
		//
	} while (false);

	return s_result;
}

void _CALLTYPE_ dev_lib_set_req_q_check_interval_in_child(void* p_instance, long long n_interval_mmsec)
{
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		p_vhid_api->set_req_q_check_interval_in_child(n_interval_mmsec);
		//
	} while (false);
}

void _CALLTYPE_ dev_lib_set_hid_write_interval_in_child(void* p_instance, long long n_interval_mmsec)
{
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		p_vhid_api->set_hid_write_interval_in_child(n_interval_mmsec);
		//
	} while (false);
}

void _CALLTYPE_ dev_lib_set_hid_read_interval_in_child(void* p_instance, long long n_interval_mmsec)
{
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		p_vhid_api->set_hid_read_interval_in_child(n_interval_mmsec);
		//
	} while (false);
}

long long _CALLTYPE_ dev_lib_get_req_q_check_interval_in_child(void* p_instance)
{
	long long ll_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		ll_result = p_vhid_api->get_req_q_check_interval_in_child();
		//
	} while (false);

	return ll_result;
}

long long _CALLTYPE_ dev_lib_get_hid_write_interval_in_child(void* p_instance)
{
	long long ll_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		ll_result = p_vhid_api->get_hid_write_interval_in_child();
		//
	} while (false);

	return ll_result;
}

long long _CALLTYPE_ dev_lib_get_hid_read_interval_in_child(void* p_instance)
{
	long long ll_result(-1);
	do {
		if (!p_instance) {
			continue;
		}

		_vhid_api_briage* p_vhid_api((_vhid_api_briage*)p_instance);
		ll_result = p_vhid_api->get_hid_read_interval_in_child();
		//
	} while (false);

	return ll_result;
}
