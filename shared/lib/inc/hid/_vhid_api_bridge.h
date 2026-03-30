#pragma once

/**
* don't include in header file(.h file).
* include only body file.(.cpp file)
*/
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <deque>
#include <list>

#include <hidapi.h>

#include <mp_type.h>
#include <mp_cwait.h>
#include <mp_cqueue.h>
#include <hid/_vhid_info.h>
#include <hid/_hid_api_bridge.h>

/**
* the instance of this class must be created only one in the process.
*/
class _vhid_api_bridge: public _hid_api_bridge
{
public:
	typedef	std::shared_ptr<_vhid_api_bridge>	type_ptr;

private:
	//key is primitive map index.
	//value is vhid_info ptr.
	typedef std::map<int, _vhid_info::type_ptr> _type_map_ptr_vhid_info;

public:
	_vhid_api_bridge();
	_vhid_api_bridge(_mp::clog* p_clog);

	virtual ~_vhid_api_bridge();

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
	virtual std::set< std::tuple<std::string, unsigned short, unsigned short, int, std::string> >hid_enumerate();

	/**
	* @brief check is open or not
	*
	* @param path - primitive or composite path
	*
	* @return tuple type
	*
	*   first true - open, false not open or error.
	*
	*   second - the opened device of the index of map.(primitive index)
	*
	*   third - true(exclusive open), false(shared open or not open )
	*/
	virtual std::tuple<bool, int, bool> is_open(const char* path) const;

	/**
	* @brief open device with path. (hid_open_path())
	* 
	* A primitive type device and all compositive types 
	* that share the primitive type device must be able to open simultaneously,
	* regardless of shared open support. 
	* Additionally, if shared open support is provided, 
	* it must be possible to open it more than twice simultaneously.
	* 
	* @param path - primitive or composite path
	* 
	* @return compositive or primitive type map index
	* 
	*	this value must be const_map_index_min~const_map_index_max, Multiples of const_map_index_inc_unit.
	* 
	*	this index cannot be index of map directly. but you can get map index(primitive index) from this value.
	* 
	*	Use _vhid_info::get_primitive_map_index_from_compositive_map_index() for getting map index(primitive index) from this value.
	* 
	*	error : -1
	*/
	virtual int api_open_path(const char* path);

	/**
	* @brief close device. 
	*
	* @param n_map_index int - primitive or compositive type map index.
	* 
	*  if the map index is compositive type, this index will be converted to primitive type index internally.
	*/
	virtual void api_close(int n_map_index);

	/**
	* @brief get report descriptor.
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	* @param buf:unsigned char* - the buffer of returned report descriptor.
	*
	* @param buf_size : size_t - the size of buf(unit byte)
	*
	* @return -1 : error, else : the size of saved data(on buf).
	*/
	virtual int api_get_report_descriptor(int n_map_index, unsigned char* buf, size_t buf_size);

	/**
	* @brief send data to device.
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	* @param data:const unsigned char* - the buffer of tx data.
	*
	* @param length : size_t - the size of data(unit byte)
	*
	* @param next : type_next_io - dummy in this class
	*
	* @return -1 : error, else : the size of sent data(on buf).
	*/
	virtual int api_write(int n_map_index, const unsigned char* data, size_t length, _mp::type_next_io next);

	/**
	* @brief receive data from device.(hid_read()).
	* 
	* this function will read multiples of in-report size.
	*
	* rx data is received regardless of format.(for filtering format, need a transaction status. so rx data cannot be filtered here.)
	* 
	* @param n_map_index int - primitive or compositive map index.
	*
	* @param data: unsigned char* - the buffer of rx data.
	*
	* @param length : size_t - the size of data buffer(unit byte)
	*
	* @param n_report : int - the size of in report data.
	*
	* @return -1 : error, else : the size of received data(on data).
	*/
	virtual int api_read(int n_map_index, unsigned char* data, size_t length, size_t n_report);

	/**
	* @brief get last error string(hid_error())
	*
	* @param n_map_index int - primitive or compositive map index.
	*
	*
	* @return 0 : error, else : unicode string buffer(error description)
	*
	*	Don't free return memory.
	*/
	virtual const wchar_t* api_error(int n_map_index);

private:
	std::tuple<bool, int, bool> _is_open(const char* path) const;

private:
	mutable std::mutex m_mutex_for_map;

	// key is primitive map index.
	// value is pair of vhid_info ptr
	// prottected by m_mutex_for_map
	// vhid_info 하나의 장비에 연결된 모든 가상장비의 type 에 대한 open cnt 관리.
	_type_map_ptr_vhid_info m_map_ptr_hid_info;
	

private://don't call these methods.
	_vhid_api_bridge(const _vhid_api_bridge&);
	_vhid_api_bridge& operator=(const _vhid_api_bridge&);
};