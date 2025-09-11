#pragma once

#include <mp_os_type.h>
#include <mutex>
#include <set>
#include <tuple>
#include <string>

/*****************************************************
* Don't include mp_xxx.h files.(except mp_os_type.h)
*/




#ifndef _WIN32
extern "C" {
#endif

bool _CALLTYPE_ dev_lib_on();

bool _CALLTYPE_ dev_lib_off();

// the exported function is determined by _hid_api_briage class public method.

/**
* @brief get muetx of lib. Don't free return memory.
*/
std::mutex* _CALLTYPE_ dev_lib_get_mutex();
void* _CALLTYPE_ dev_lib_constrcutor();
void _CALLTYPE_ dev_lib_destructor(void* p_instance);
bool _CALLTYPE_ dev_lib_is_ini(void* p_instance);

/**
* @brief get connected information of devices.
*
* @return  each item's
*
*	1'st - std::string, device path,
*
*	2'nd - unsigned short, usb vendor id,
*
*	3'th - unsigned short, usb product id,
*
*	4'th - int, usb interface number,
*
*	5'th - std::string, extra data
*/
std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string>>* _CALLTYPE_ dev_lib_hid_enumerate(void* p_instance);

/**
* @brief free memory of the set returned by dev_lib_hid_enumerate()
* @param p_tuple - pointer of the set returned by dev_lib_hid_enumerate()
*/
void _CALLTYPE_ dev_lib_hid_enumerate_free(std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string>>*p_tuple);


/**
* @brief check is open or not
*
* @param path - primitive path
*
* @return
*	first true - open, false not open or error
*
*	second - the opened device of the index of map.
*
*	third - true(exclusive open), false(shared open or not open )
*/
std::tuple<bool, int, bool>* _CALLTYPE_ dev_lib_is_open(void* p_instance, const char* path);

/**
* @brief free memory of the tuple returned by dev_lib_is_open()
* @param p_tuple - pointer of the tuple returned by dev_lib_is_open()
*/
void _CALLTYPE_ dev_lib_is_open_free(std::tuple<bool, int, bool>*p_tuple);

/**
* @brief open device
*
*   In windows, open with FILE_SHARE_READ|FILE_SHARE_WRITE flag
*
*   In linux, open by libusb_open()
*
* @param path : string - primitive device path
*
* @return the primitive type map index of map(m_map_hid_dev)
*
*	-1 : error( including already open status )
*/
int _CALLTYPE_ dev_lib_api_open_path(void* p_instance, const char* path);

/**
* @brief close device. internally, hid_close() will be called.
*
* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
*/
void _CALLTYPE_ dev_lib_api_close(void* p_instance, int n_primitive_map_index);


/**
* @brief set blocking mode.(hid_set_nonblocking())
*
* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
*
* @param nonblock:int - 0 is blocking mode, 1 is nonblocking mode
*
* @return 0 : success, -1 : error
*/
int _CALLTYPE_ dev_lib_api_set_nonblocking(void* p_instance, int n_primitive_map_index, int nonblock);

/**
* @brief get report descriptor.(hid_get_report_descriptor())
*
* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
*
* @param buf:unsigned char* - the buffer of returned report descriptor.
*
* @param buf_size : size_t - the size of buf(unit byte)
*
* @return -1 : error, else : the size of saved data(on buf).
*/
int _CALLTYPE_ dev_lib_api_get_report_descriptor(void* p_instance, int n_primitive_map_index, unsigned char* buf, size_t buf_size);

/**
* @brief send data to device.(hid_write())
*
* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
*
* @param data:const unsigned char* - the buffer of tx data.
*
* @param length : size_t - the size of data(unit byte)
*
* @param next : type_next_io - _mp::type_next_io type
*
* @return -1 : error, else : the size of sent data(on buf).
*/
int _CALLTYPE_ dev_lib_api_write(void* p_instance, int n_primitive_map_index, const unsigned char* data, size_t length, int next);

/**
* @brief receive data from device.(hid_read())
*
* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
*
* @param data: unsigned char* - the buffer of rx data.
*
* @param length : size_t - the size of data buffer(unit byte)
*
* @param n_report : int - the size of in report data.
*
* @return -1 : error, else : the size of received data(on data).
*/
int _CALLTYPE_ dev_lib_api_read(void* p_instance, int n_primitive_map_index, unsigned char* data, size_t length, size_t n_report);

/**
* @brief get last error string(hid_error())
*
* @param n_primitive_map_index int - primitive type map index(m_map_hid_dev)
*
*
* @return 0 : error, else : unicode string buffer(error description)
*
*	Don't free return memory.
*/
const wchar_t* _CALLTYPE_ dev_lib_api_error(void* p_instance, int n_primitive_map_index);

void _CALLTYPE_ dev_lib_set_req_q_check_interval_in_child(void* p_instance, long long n_interval_mmsec);
void _CALLTYPE_ dev_lib_set_hid_write_interval_in_child(void* p_instance, long long n_interval_mmsec);
void _CALLTYPE_ dev_lib_set_hid_read_interval_in_child(void* p_instance, long long n_interval_mmsec);

long long _CALLTYPE_ dev_lib_get_req_q_check_interval_in_child(void* p_instance);
long long _CALLTYPE_ dev_lib_get_hid_write_interval_in_child(void* p_instance);
long long _CALLTYPE_ dev_lib_get_hid_read_interval_in_child(void* p_instance);


#ifndef _WIN32
}//extern "C"
#endif
